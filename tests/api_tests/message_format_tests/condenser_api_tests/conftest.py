import pytest

import test_tools as tt


@pytest.fixture
def node():
    init_node = tt.InitNode()
    init_node.run()
    return init_node


@pytest.fixture
def wallet(prepared_node):
    return tt.Wallet(attach_to=prepared_node)