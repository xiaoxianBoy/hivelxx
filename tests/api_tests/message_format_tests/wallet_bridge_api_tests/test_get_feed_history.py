from hive_local_tools import run_for


@run_for("testnet", "mainnet_5m", "live_mainnet")
def test_get_feed_history(node):
    node.api.wallet_bridge.get_feed_history()
