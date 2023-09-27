#include <hive/chain/fork_db_block_reader.hpp>

#include <hive/chain/fork_database.hpp>

namespace hive { namespace chain {

fork_db_block_reader::fork_db_block_reader( const fork_database& fork_db, block_log& the_log )
  : block_log_reader( the_log ), _fork_db( fork_db )
{}

bool fork_db_block_reader::is_known_block( const block_id_type& id ) const
{
  try {
    if( _fork_db.fetch_block( id ) )
      return true;

    return block_log_reader::is_known_block( id );
  } FC_CAPTURE_AND_RETHROW()
}

bool fork_db_block_reader::is_known_block_unlocked( const block_id_type& id ) const
{ 
  try {
    if (_fork_db.fetch_block_unlocked(id, true /* only search linked blocks */))
      return true;

    return block_log_reader::is_known_block_unlocked( id );
  } FC_CAPTURE_AND_RETHROW()
}

std::deque<block_id_type>::const_iterator fork_db_block_reader::find_first_item_not_in_blockchain(
  const std::deque<block_id_type>& item_hashes_received ) const
{
  return _fork_db.with_read_lock([&](){
    return std::partition_point(item_hashes_received.begin(), item_hashes_received.end(), [&](const block_id_type& block_id) {
      return is_known_block_unlocked(block_id);
    });
  });
}

block_id_type fork_db_block_reader::find_block_id_for_num( uint32_t block_num )const
{
  block_id_type result;

  try
  {
    if( block_num != 0 )
    {
      // See if fork DB has the item
      shared_ptr<fork_item> fitem = _fork_db.fetch_block_on_main_branch_by_number( block_num );
      if( fitem )
      {
        result = fitem->get_block_id();
      }
      else
      {
        // Next we check if block_log has it. Irreversible blocks are there.
        result = block_log_reader::find_block_id_for_num( block_num );
      }
    }
  }
  FC_CAPTURE_AND_RETHROW( (block_num) )

  if( result == block_id_type() )
    FC_THROW_EXCEPTION(fc::key_not_found_exception, "block number not found");

  return result;
}

std::vector<std::shared_ptr<full_block_type>> fork_db_block_reader::fetch_block_range( 
  const uint32_t starting_block_num, const uint32_t count, 
  fc::microseconds wait_for_microseconds /*= fc::microseconds()*/ ) const
{ 
  try {
    // for debugging, put the head block back so it should straddle the last irreversible
    // const uint32_t starting_block_num = head_block_num() - 30;
    FC_ASSERT(starting_block_num > 0, "Invalid starting block number");
    FC_ASSERT(count > 0, "Why ask for zero blocks?");
    FC_ASSERT(count <= 1000, "You can only ask for 1000 blocks at a time");
    idump((starting_block_num)(count));

    vector<fork_item> fork_items = _fork_db.fetch_block_range_on_main_branch_by_number( starting_block_num, count, wait_for_microseconds );
    idump((fork_items.size()));
    if (!fork_items.empty())
      idump((fork_items.front().get_block_num()));

    // if the fork database returns some blocks, it means:
    // - that the last block in the range [starting_block_num, starting_block_num + count - 1]
    // - any block before the first block it returned should be in the block log
    uint32_t remaining_count = fork_items.empty() ? count : fork_items.front().get_block_num() - starting_block_num;
    idump((remaining_count));
    std::vector<std::shared_ptr<full_block_type>> result;

    if (remaining_count)
      result = block_log_reader::fetch_block_range(starting_block_num, remaining_count, wait_for_microseconds);

    result.reserve(result.size() + fork_items.size());
    for (fork_item& item : fork_items)
      result.push_back(item.full_block);

    return result;
  } FC_LOG_AND_RETHROW()
}

} } //hive::chain
