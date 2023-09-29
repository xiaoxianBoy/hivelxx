#pragma once

#include <hive/chain/block_log_reader.hpp>

#include <hive/chain/fork_database.hpp>

namespace hive { namespace chain {

  class fork_db_block_reader : public block_log_reader
  {
  public:
    fork_db_block_reader( fork_database& fork_db, block_log& block_log );
    virtual ~fork_db_block_reader() = default;

    virtual void start_reader( const std::shared_ptr<full_block_type>& head_block ) override;
    virtual void close_reader() override;

    virtual std::shared_ptr<full_block_type> fetch_block_by_id( 
      const block_id_type& id ) const override;

    virtual bool is_known_block( const block_id_type& id ) const override;

    virtual bool is_known_block_unlocked( const block_id_type& id ) const override;

    virtual std::deque<block_id_type>::const_iterator find_first_item_not_in_blockchain(
      const std::deque<block_id_type>& item_hashes_received ) const override;

    virtual block_id_type find_block_id_for_num( uint32_t block_num ) const override;

    virtual std::shared_ptr<full_block_type> fetch_block_by_number( uint32_t block_num,
      fc::microseconds wait_for_microseconds = fc::microseconds() ) const override;

    virtual std::vector<std::shared_ptr<full_block_type>> fetch_block_range( 
      const uint32_t starting_block_num, const uint32_t count, 
      fc::microseconds wait_for_microseconds = fc::microseconds() ) const override;

    virtual std::vector<block_id_type> get_blockchain_synopsis(
      const block_id_type& reference_point, 
      uint32_t number_of_blocks_after_reference_point ) const override;

    virtual std::vector<block_id_type> get_block_ids(
      const std::vector<block_id_type>& blockchain_synopsis,
      uint32_t& remaining_item_count,
      uint32_t limit) const override;

  private:
  	/** Needed by p2p plugin only.
     *  Check among reversible blocks on main branch then among irreversible.
    */
    bool is_included_block_unlocked( const block_id_type& block_id ) const;
    /// Searches block log only, returns empty when not found there
    block_id_type get_block_id_for_num( uint32_t block_num ) const;

  private:
    fork_database& _fork_db;
  };

} }
