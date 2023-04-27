
#include <hive/chain/rc/rc_utility.hpp>
#include <hive/chain/rc/rc_objects.hpp>
#include <hive/chain/database.hpp>
#include <hive/chain/util/remove_guard.hpp>

#include <fc/reflect/variant.hpp>
#include <fc/uint128.hpp>

namespace hive { namespace chain {

using fc::uint128_t;

int64_t resource_credits::compute_cost(
  const rc_price_curve_params& curve_params,
  int64_t current_pool,
  int64_t resource_count,
  int64_t rc_regen
  )
{
  /*
  ilog( "resource_credits::compute_cost( ${params}, ${pool}, ${res}, ${reg} )",
    ("params", curve_params)
    ("pool", current_pool)
    ("res", resource_count)
    ("reg", rc_regen) );
  */
  FC_ASSERT( rc_regen > 0 );

  if( resource_count <= 0 )
  {
    if( resource_count < 0 )
      return -compute_cost( curve_params, current_pool, -resource_count, rc_regen );
    return 0;
  }
  uint128_t num = uint128_t( rc_regen );
  num *= curve_params.coeff_a;
  // rc_regen * coeff_a is already risking overflowing 128 bits,
  //   so shift it immediately before multiplying by resource_count
  num >>= curve_params.shift;
  // err on the side of rounding not in the user's favor
  num += 1;
  num *= uint64_t( resource_count );

  uint128_t denom = uint128_t( curve_params.coeff_b );

  // Negative pool doesn't increase price beyond p_max
  //   i.e. define p(x) = p(0) for all x < 0
  denom += (current_pool > 0) ? uint64_t(current_pool) : uint64_t(0);
  uint128_t num_denom = num / denom;
  // Add 1 to avoid 0 result in case of various rounding issues,
  // err on the side of rounding not in the user's favor
  // ilog( "result: ${r}", ("r", num_denom.to_uint64()+1) );
  return fc::uint128_to_uint64(num_denom)+1;
}

void resource_credits::update_account_after_rc_delegation( const account_object& account,
  uint32_t now, int64_t delta, bool regenerate_mana ) const
{
  db.modify< account_object >( account, [&]( account_object& acc )
  {
    auto max_rc = acc.get_maximum_rc().value;
    util::manabar_params manabar_params( max_rc, HIVE_RC_REGEN_TIME );
    if( regenerate_mana )
    {
      acc.rc_manabar.regenerate_mana< true >( manabar_params, now );
    }
    else if( acc.rc_manabar.last_update_time != now )
    {
      //most likely cause: there is no regenerate() call in corresponding pre_apply_operation_visitor handler
      wlog( "NOTIFYALERT! Account ${a} not regenerated prior to RC change, noticed on block ${b}",
        ( "a", acc.get_name() )( "b", db.head_block_num() ) );
    }
    //rc delegation changes are immediately reflected in current_mana in both directions;
    //if negative delta was not taking away delegated mana it would be easy to pump RC;
    //note that it is different when actual VESTs are involved
    acc.rc_manabar.current_mana = std::max( acc.rc_manabar.current_mana + delta, int64_t( 0 ) );
    acc.last_max_rc = max_rc + delta;
    acc.received_rc += delta;
  } );
}

void resource_credits::update_account_after_vest_change( const account_object& account,
  uint32_t now, bool _fill_new_mana, bool _check_for_rc_delegation_overflow ) const
{
  if( account.rc_manabar.last_update_time != now )
  {
    //most likely cause: there is no regenerate() call in corresponding pre_apply_operation_visitor handler
    wlog( "NOTIFYALERT! Account ${a} not regenerated prior to VEST change, noticed on block ${b}",
      ( "a", account.get_name() )( "b", db.head_block_num() ) );
  }

  if( _check_for_rc_delegation_overflow && account.get_delegated_rc() > 0 )
  {
    //the following action is needed when given account lost VESTs and it now might have less
    //than it already delegated with rc delegations (second part of condition is a quick exit
    //check for times before RC delegations are activated (HF26))
    int64_t delegation_overflow = -account.get_maximum_rc( true ).value;
    int64_t initial_overflow = delegation_overflow;

    if( delegation_overflow > 0 )
    {
      int16_t remove_limit = db.get_remove_threshold();
      remove_guard obj_perf( remove_limit );
      remove_delegations( delegation_overflow, account.get_id(), now, obj_perf );

      if( delegation_overflow > 0 )
      {
        // there are still active delegations that need to be cleared, but we've already exhausted the object removal limit;
        // set an object to continue removals in following blocks while blocking explicit changes in RC delegations on account
        auto* expired = db.find< rc_expired_delegation_object, by_account >( account.get_id() );
        if( expired != nullptr )
        {
          // supplement existing object from still unfinished previous delegation removal...
          db.modify( *expired, [&]( rc_expired_delegation_object& e )
          {
            e.expired_delegation += delegation_overflow;
          } );
        }
        else
        {
          // ...or create new one
          db.create< rc_expired_delegation_object >( account, delegation_overflow );
        }
      }

      // We don't update last_max_rc and the manabars here because it happens below
      db.modify( account, [&]( account_object& acc )
      {
        acc.delegated_rc -= initial_overflow;
      } );
    }
  }

  int64_t new_last_max_rc = account.get_maximum_rc().value;
  int64_t drc = new_last_max_rc - account.last_max_rc.value;
  drc = _fill_new_mana ? drc : 0;

  if( new_last_max_rc != account.last_max_rc )
  {
    db.modify( account, [&]( account_object& acc )
    {
      //note: rc delegations behave differently because if they behaved the following way there would
      //be possible to easily fill up mana through giving and immediately taking away rc delegations
      acc.last_max_rc = new_last_max_rc;
      //only positive delta is applied directly to mana, so receiver can immediately start using it;
      //negative delta is caused by either power down or reduced incoming delegation; in both cases
      //there is a delay that makes transfered vests unusable for long time (not shorter than full
      //rc regeneration) therefore we can skip applying negative delta without risk of "pumping" rc;
      //by not applying negative delta we preserve mana that affected account "earned" so far...
      acc.rc_manabar.current_mana += std::max( drc, int64_t( 0 ) );
      //...however we have to reduce it when its current level would exceed new maximum (issue #191)
      if( acc.rc_manabar.current_mana > acc.last_max_rc )
        acc.rc_manabar.current_mana = acc.last_max_rc.value;
    } );
  }
}

bool resource_credits::has_expired_delegation( const account_object& account ) const
{
  auto* expired = db.find< rc_expired_delegation_object, by_account >( account.get_id() );
  return expired != nullptr;
}

void resource_credits::handle_expired_delegations() const
{
  // clear as many delegations as possible within limit starting from oldest ones (smallest id)
  const auto& expired_idx = db.get_index<rc_expired_delegation_index, by_id>();
  auto expired_it = expired_idx.begin();
  if( expired_it == expired_idx.end() )
    return;

  uint32_t now = db.head_block_time().sec_since_epoch();
  int16_t remove_limit = db.get_remove_threshold();
  remove_guard obj_perf( remove_limit );

  do
  {
    const auto& expired = *expired_it;
    int64_t delegation_overflow = expired.expired_delegation;
    remove_delegations( delegation_overflow, expired.from, now, obj_perf );

    if( delegation_overflow > 0 )
    {
      // still some delegations remain for next block cycle
      db.modify( expired, [&]( rc_expired_delegation_object& e )
      {
        e.expired_delegation = delegation_overflow;
      } );
      break;
    }
    else
    {
      // no more delegations to clear for user related to this particular delegation overflow
      db.remove( expired ); //remove even if we've hit the removal limit - that overflow is empty already
    }

    expired_it = expired_idx.begin();
  }
  while( !obj_perf.done() && expired_it != expired_idx.end() );
}

void resource_credits::remove_delegations( int64_t& delegation_overflow, account_id_type delegator_id,
  uint32_t now, remove_guard& obj_perf ) const
{
  const auto& rc_del_idx = db.get_index<rc_direct_delegation_index, by_from_to>();
  // Maybe add a new index to sort by from / amount delegated so it's always the bigger delegations that is modified first instead of the id order ?
  // start_id just means we iterate over all the rc delegations
  auto rc_del_itr = rc_del_idx.lower_bound( boost::make_tuple( delegator_id, account_id_type::start_id() ) );

  while( delegation_overflow > 0 && rc_del_itr != rc_del_idx.end() && rc_del_itr->from == delegator_id )
  {
    const auto& rc_delegation = *rc_del_itr;
    ++rc_del_itr;

    int64_t delta_rc = std::min( delegation_overflow, int64_t( rc_delegation.delegated_rc ) );
    account_id_type to_id = rc_delegation.to;

    // If the needed RC allow us to leave the delegation untouched, we just change the delegation to take it into account
    if( rc_delegation.delegated_rc > ( uint64_t ) delta_rc )
    {
      db.modify( rc_delegation, [&]( rc_direct_delegation_object& rc_del )
      {
        rc_del.delegated_rc -= delta_rc;
      } );
    }
    else
    {
      // Otherwise, we remove it
      if( !obj_perf.remove( db, rc_delegation ) )
        break;
    }

    const auto& to_account = db.get_account( to_id );
    //since to_account was not originally expected to be affected by operation that is being
    //processed, we need to regenerate its mana before rc delegation is modified
    update_account_after_rc_delegation( to_account, now, -delta_rc, true );

    delegation_overflow -= delta_rc;
  }
}

} }
