import test_tools as tt

from ..local_tools import create_and_cancel_vesting_delegation, run_for
from ....local_tools import date_from_now


# This test cannot be performed on 5 million blocklog because it doesn't contain any vesting delegation expirations.
# See the readme.md file in this directory for further explanation.
@run_for('testnet', 'mainnet_64m')
def test_find_vesting_delegation_expirations(prepared_node, should_prepare):
    if should_prepare:
        wallet = tt.Wallet(attach_to=prepared_node)
        wallet.create_account('alice', hives=tt.Asset.Test(100), vests=tt.Asset.Test(100))
        wallet.api.create_account('alice', 'bob', '{}')
        create_and_cancel_vesting_delegation(wallet, 'alice', 'bob')
    account = \
        prepared_node.api.database.list_vesting_delegation_expirations(start=['', date_from_now(weeks=-100), 0],
                                                                       limit=100, order='by_account_expiration')[
            'delegations'][0]['delegator']
    delegations = prepared_node.api.database.find_vesting_delegation_expirations(account=account)['delegations']
    assert len(delegations) != 0
