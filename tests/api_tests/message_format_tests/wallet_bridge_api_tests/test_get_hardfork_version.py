from hive_local_tools import run_for


@run_for("testnet", "mainnet_5m", "live_mainnet")
def test_get_hardfork_version(node):
    node.api.wallet_bridge.get_hardfork_version()
