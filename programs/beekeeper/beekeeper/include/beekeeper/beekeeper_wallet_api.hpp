#pragma once

#include <core/beekeeper_wallet_manager.hpp>
#include <core/utilities.hpp>

#include <hive/plugins/json_rpc/utility.hpp>

#include <hive/protocol/types.hpp>

#include <fc/optional.hpp>
#include <fc/variant.hpp>
#include <fc/vector.hpp>

namespace beekeeper {

namespace detail
{
  class beekeeper_api_impl;
}

class beekeeper_wallet_api
{
  public:
    beekeeper_wallet_api( std::shared_ptr<beekeeper::beekeeper_wallet_manager> wallet_mgr );
    ~beekeeper_wallet_api();

    DECLARE_API(
      (create)
      (open)
      (close)
      (set_timeout)
      (lock_all)
      (lock)
      (unlock)
      (import_key)
      (remove_key)
      (list_wallets)
      (get_public_keys)
      (sign_digest)
      (sign_binary_transaction)
      (sign_transaction)
      (get_info)
      (create_session)
      (close_session)
    )

  private:
    std::unique_ptr< detail::beekeeper_api_impl > my;
};

} // beekeeper