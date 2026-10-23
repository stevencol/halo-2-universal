# mutations of milestone 762: one line of each function

TESTS = "test_small_762"

NETWORKING = "source/interface/user_interface_networking.c"

MUTATIONS = [
    (NETWORKING, "==4 || user_interface_networking_unknown_00199d7c()==6)", "==4 || user_interface_networking_unknown_00199d7c()==5)"),
    (NETWORKING, "\t\treturn network_life_cycle_unknown_000594a0(unknown0, unknown4, unknown8);", "\t\treturn network_life_cycle_unknown_000594a0(unknown0, unknown8, unknown4);"),
    (NETWORKING, "\treturn player_count;\n}\n\n/* 0x00199ef8", "\treturn (short)player_count;\n}\n\n/* 0x00199ef8"),
    (NETWORKING, "\tif (network_life_cycle_get_other_session(&session) && network_session_established(session))", "\tif (network_life_cycle_get_other_session(&session))"),
    (NETWORKING, "\t\tunknown8= *(const long *)(membership+USER_INTERFACE_NETWORKING_MEMBERSHIP_UNKNOWN8_OFFSET);", "\t\tunknown8= *(const long *)(membership+NETWORK_SESSION_MEMBERSHIP_PLAYER_COUNT_OFFSET);"),
    (NETWORKING, "\treturn network_life_cycle_local_peer_is_host() ? network_life_cycle_set_unknown49c4() : FALSE;", "\treturn network_life_cycle_local_peer_is_host() ? network_life_cycle_set_unknown49c4() : TRUE;"),
]
