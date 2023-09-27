#pragma once

#include <hive/chain/block_log_reader.hpp>

#include <hive/chain/fork_database.hpp>

namespace hive { namespace chain {

  class fork_db_block_reader : public block_log_reader
  {
  public:
    fork_db_block_reader( const fork_database& fork_db, block_log& block_log );
    virtual ~fork_db_block_reader() = default;

    virtual bool is_known_block( const block_id_type& id ) const override;

    virtual bool is_known_block_unlocked( const block_id_type& id ) const override;

    virtual std::deque<block_id_type>::const_iterator find_first_item_not_in_blockchain(
      const std::deque<block_id_type>& item_hashes_received ) const override;

    virtual block_id_type find_block_id_for_num( uint32_t block_num ) const override;

    virtual std::vector<std::shared_ptr<full_block_type>> fetch_block_range( 
      const uint32_t starting_block_num, const uint32_t count, 
      fc::microseconds wait_for_microseconds = fc::microseconds() ) const override;

  private:
    const fork_database& _fork_db;
  };

} }
