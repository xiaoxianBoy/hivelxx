import test_tools as tt

from ..local_tools import run_for


@run_for('testnet', 'mainnet_5m', 'mainnet_64m')
def test_get_accounts(prepared_node, should_prepare):
    if should_prepare:
        wallet = tt.Wallet(attach_to=prepared_node)
        wallet.api.create_account('initminer', 'gtg', '{}')
    prepared_node.api.condenser.get_accounts(['gtg'])