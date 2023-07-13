#pragma once

#include <core/beekeeper_app_init.hpp>

namespace beekeeper {

class beekeeper_wasm_app: public beekeeper_app_init
{
  private:

    boost::program_options::variables_map args;

  protected:

    void set_program_options() override;
    std::pair<bool, bool> initialize( int argc, char** argv ) override;
    bool start() override;

    const boost::program_options::variables_map& get_args() const override;
    bfs::path get_data_dir() const override;
    void setup_notifications( const boost::program_options::variables_map& args ) override;

    std::shared_ptr<beekeeper::beekeeper_wallet_manager> create_wallet( const boost::filesystem::path& cmd_wallet_dir, uint64_t cmd_unlock_timeout, uint32_t cmd_session_limit ) override;

  public:

    beekeeper_wasm_app();
    ~beekeeper_wasm_app() override;
};

}