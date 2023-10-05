import pytest
import test_tools as tt
from hive_local_tools.functional.python.operation import get_transaction_timestamp


@pytest.mark.testnet
@pytest.mark.parametrize(
    "authority_type, comparison_type",
    (
        ("active", "__eq__"),
        ("posting", "__eq__"),
        ("owner", "__gt__"),
    ),
)
def test_update_account_owner_authority(alice, authority_type, comparison_type):
    """
    Test cases 1, 2 and 3 from issue: https://gitlab.syncad.com/hive/hive/-/issues/519
    """
    alice.use_authority(authority_type)
    new_key = alice.generate_new_key()
    rc_before_operation = alice.get_rc_current_mana()
    if authority_type != "owner":
        with pytest.raises(tt.exceptions.CommunicationError) as exception:
            alice.update_single_account_detail(key_type="owner", key=new_key)
        error_response = exception.value.response["error"]["message"]
        assert "Missing Owner Authority" in error_response
        assert_msg = "RC should be equal to the value before failed transaction"
    else:
        alice.update_single_account_detail(key_type="owner", key=new_key)
        assert_msg = "RC wasn't decreased after performing account_update operation"
        alice.assert_account_details_were_changed(new_owner=new_key)
    rc_after_operation = alice.get_rc_current_mana()
    assert getattr(rc_before_operation, comparison_type)(rc_after_operation), assert_msg


@pytest.mark.testnet
@pytest.mark.parametrize(
    "authority_type, comparison_type",
    (
        ("active", "__gt__"),
        ("posting", "__eq__"),
        ("owner", "__gt__"),
    ),
)
def test_update_account_active_authority(alice, authority_type, comparison_type):
    """
    Test cases 4,5 and 6 from issue: https://gitlab.syncad.com/hive/hive/-/issues/519
    """
    alice.use_authority(authority_type)
    new_key = alice.generate_new_key()
    rc_before_operation = alice.get_rc_current_mana()
    if authority_type == "posting":
        with pytest.raises(tt.exceptions.CommunicationError) as exception:
            alice.update_single_account_detail(key_type="active", key=new_key)
        error_response = exception.value.response["error"]["message"]
        assert "Missing Active Authority" in error_response
        assert_msg = "RC should be equal to the value before failed transaction"
    else:
        alice.update_single_account_detail(key_type="active", key=new_key)
        assert_msg = "RC wasn't decreased after performing account_update operation"
        alice.assert_account_details_were_changed(new_active=new_key)
    rc_after_operation = alice.get_rc_current_mana()
    assert getattr(rc_before_operation, comparison_type)(rc_after_operation), assert_msg


@pytest.mark.testnet
@pytest.mark.parametrize(
    "authority_type, comparison_type",
    (
        ("active", "__gt__"),
        ("posting", "__eq__"),
        ("owner", "__gt__"),
    ),
)
def test_update_account_posting_authority(alice, authority_type, comparison_type):
    """
    Test cases 7,8 and 9 from issue: https://gitlab.syncad.com/hive/hive/-/issues/519
    """
    alice.use_authority(authority_type)
    new_key = alice.generate_new_key()
    rc_before_operation = alice.get_rc_current_mana()
    if authority_type == "posting":
        with pytest.raises(tt.exceptions.CommunicationError) as exception:
            alice.update_single_account_detail(key_type="posting", key=new_key)
        error_response = exception.value.response["error"]["message"]
        assert "Missing Active Authority" in error_response
        assert_msg = "RC should be equal to the value before failed transaction"
    else:
        alice.update_single_account_detail(key_type="posting", key=new_key)
        assert_msg = "RC wasn't decreased after performing account_update operation"
        alice.assert_account_details_were_changed(new_posting=new_key)
    rc_after_operation = alice.get_rc_current_mana()
    assert getattr(rc_before_operation, comparison_type)(rc_after_operation), assert_msg


@pytest.mark.testnet
@pytest.mark.parametrize(
    "authority_type, comparison_type",
    (
        ("active", "__gt__"),
        ("posting", "__eq__"),
        ("owner", "__gt__"),
    ),
)
def test_update_account_memo_key(alice, authority_type, comparison_type):
    """
    Test cases 10, 11 and 12 from issue: https://gitlab.syncad.com/hive/hive/-/issues/519
    """
    alice.use_authority(authority_type)
    new_key = alice.generate_new_key()
    rc_before_operation = alice.get_rc_current_mana()
    if authority_type == "posting":
        with pytest.raises(tt.exceptions.CommunicationError) as exception:
            alice.update_single_account_detail(key_type="memo", key=new_key)
        error_response = exception.value.response["error"]["message"]
        assert "Missing Active Authority" in error_response
        assert_msg = "RC should be equal to the value before failed transaction"
    else:
        alice.update_single_account_detail(key_type="memo", key=new_key)
        assert_msg = "RC wasn't decreased after performing account_update operation"
        alice.assert_account_details_were_changed(new_memo=new_key)
    rc_after_operation = alice.get_rc_current_mana()
    assert getattr(rc_before_operation, comparison_type)(rc_after_operation), assert_msg


@pytest.mark.testnet
@pytest.mark.parametrize(
    "authority_type, comparison_type",
    (
        ("active", "__gt__"),
        ("posting", "__eq__"),
        ("owner", "__gt__"),
    ),
)
def test_update_json_metadata(alice, authority_type, comparison_type):
    """
    Test cases 13, 14 and 15 from issue: https://gitlab.syncad.com/hive/hive/-/issues/519
    """
    alice.use_authority(authority_type)
    new_json_meta = '{"foo": "bar"}'
    rc_before_operation = alice.get_rc_current_mana()
    if authority_type == "posting":
        with pytest.raises(tt.exceptions.CommunicationError) as exception:
            alice.update_single_account_detail(json_meta=new_json_meta)
        error_response = exception.value.response["error"]["message"]
        assert "Missing Active Authority" in error_response
        assert_msg = "RC should be equal to the value before failed transaction"
    else:
        alice.update_single_account_detail(json_meta=new_json_meta)
        assert_msg = "RC wasn't decreased after performing account_update operation"
        alice.assert_account_details_were_changed(new_json_meta=new_json_meta)
    rc_after_operation = alice.get_rc_current_mana()
    assert getattr(rc_before_operation, comparison_type)(rc_after_operation), assert_msg


@pytest.mark.testnet
def test_update_all_account_parameters_using_owner_authority(alice):
    """
    Test case 16 from issue: https://gitlab.syncad.com/hive/hive/-/issues/519
    """
    alice.use_authority("owner")
    new_json_meta = '{"foo": "bar"}'
    new_owner, new_active, new_posting, new_memo = (
        alice.generate_new_key(),
        alice.generate_new_key(),
        alice.generate_new_key(),
        alice.generate_new_key(),
    )
    rc_before_operation = alice.get_rc_current_mana()
    alice.update_all_account_details(
        owner=new_owner, active=new_active, posting=new_posting, memo=new_memo, json_meta=new_json_meta
    )
    rc_after_operation = alice.get_rc_current_mana()
    alice.assert_account_details_were_changed(
        new_owner=new_owner,
        new_active=new_active,
        new_posting=new_posting,
        new_memo=new_memo,
        new_json_meta=new_json_meta,
    )
    assert rc_before_operation > rc_after_operation, "RC wasn't decreased after performing account_update operation"


@pytest.mark.testnet
def test_update_all_account_parameters_except_owner_key_using_active_authority(alice):
    """
    Test case 17 from issue: https://gitlab.syncad.com/hive/hive/-/issues/519
    """
    alice.use_authority("active")
    new_json_meta = '{"foo": "bar"}'
    current_active, current_posting, new_memo = (
        alice.get_current_key("active"),
        alice.get_current_key("posting"),
        alice.generate_new_key(),
    )
    new_weight = 2
    for key_type, key in zip(("active", "posting", "memo"), (current_active, current_posting, new_memo)):
        transaction = (
            alice.update_single_account_detail(key_type=key_type, key=key, weight=new_weight)
            if key_type is not "memo"
            else alice.update_single_account_detail(key_type=key_type, key=key)
        )

        real_rc_before = alice.rc_manabar.calculate_current_value(
            get_transaction_timestamp(alice.node, transaction) - tt.Time.seconds(3)
        )

        alice.rc_manabar.update()
        real_rc_after = alice.rc_manabar.calculate_current_value(
            get_transaction_timestamp(alice.node, transaction) - tt.Time.seconds(3)
        )
        assert (
            real_rc_before == real_rc_after + transaction["rc_cost"]
        ), f"RC wasn't decreased after changing {key_type} key."

    rc_before_operation = alice.get_rc_current_mana()
    alice.update_single_account_detail(json_meta=new_json_meta)
    rc_after_operation = alice.get_rc_current_mana()
    assert rc_before_operation > rc_after_operation, "RC wasn't decreased after changing json metadata."

    alice.assert_account_details_were_changed(
        new_active=[current_active, new_weight],
        new_posting=[current_posting, new_weight],
        new_memo=new_memo,
        new_json_meta=new_json_meta,
    )


@pytest.mark.testnet
@pytest.mark.parametrize(
    "iterations",
    (
        2,  # Change owner authority 2 times in less than HIVE_OWNER_UPDATE_LIMIT
        3,  # Change owner authority 3 times in less than HIVE_OWNER_UPDATE_LIMIT
    ),
)
@pytest.mark.testnet
def test_update_owner_authority_two_and_three_times_within_one_hour(alice, iterations):
    """
    Test case 18, 19 from issue: https://gitlab.syncad.com/hive/hive/-/issues/519
    """
    alice.use_authority("owner")
    for iteration in range(2, 2 + iterations):
        current_key = alice.get_current_key("owner")
        rc_before_operation = alice.get_rc_current_mana()
        if iteration == 4:
            with pytest.raises(tt.exceptions.CommunicationError) as exception:
                alice.update_single_account_detail(key_type="owner", key=current_key, weight=iteration)
            rc_after_operation = alice.get_rc_current_mana()
            error_response = exception.value.response["error"]["message"]
            assert "Owner authority can only be updated twice an hour" in error_response
            assert_msg = "RC should be equal to the value before failed transaction"
            assert rc_before_operation == rc_after_operation, assert_msg
        else:
            alice.update_single_account_detail(key_type="owner", key=current_key, weight=iteration)
            alice.assert_account_details_were_changed(new_owner=[current_key, iteration])
            assert_msg = "RC wasn't decreased after performing account_update operation"
            rc_after_operation = alice.get_rc_current_mana()
            assert rc_before_operation > rc_after_operation, assert_msg
