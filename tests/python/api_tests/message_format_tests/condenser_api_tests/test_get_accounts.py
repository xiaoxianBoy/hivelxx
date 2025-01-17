from __future__ import annotations

import test_tools as tt
from hive_local_tools import run_for


@run_for("testnet", "mainnet_5m", "live_mainnet")
def test_get_accounts(node: tt.InitNode | tt.RemoteNode, should_prepare: bool) -> None:
    preparation_for_testnet_node(node, should_prepare)
    node.api.condenser.get_accounts(["gtg"], True)


@run_for("testnet", "mainnet_5m", "live_mainnet")
def test_get_accounts_with_default_second_argument(node: tt.InitNode | tt.RemoteNode, should_prepare: bool) -> None:
    preparation_for_testnet_node(node, should_prepare)
    node.api.condenser.get_accounts(["gtg"])


def preparation_for_testnet_node(node: tt.InitNode | tt.RemoteNode, should_prepare: bool) -> None:
    if should_prepare:
        wallet = tt.Wallet(attach_to=node)
        wallet.api.create_account("initminer", "gtg", "{}")
