# mutations of milestone 763: one or two lines of each function

TESTS = "test_small_763"

NETWORKING = "source/interface/user_interface_networking.c"

MUTATIONS = [
    (NETWORKING, "\tif (!unknown_004d8ba0 || !network_life_cycle_local_peer_is_host())", "\tif (!unknown_004d8ba0)"),
    (NETWORKING, "\tcase _network_life_cycle_state5:\n\tcase _network_life_cycle_state6:", "\tcase _network_life_cycle_state6:"),
    (NETWORKING, "\t\treturn !user_interface_networking_hopper_unknown_00193470();", "\t\treturn user_interface_networking_hopper_unknown_00193470();"),
    (NETWORKING, "\treturn network_life_cycle_get_unknown4d08(unknown4d08, unknown4d0c, &unknown4d10);", "\treturn network_life_cycle_get_unknown4d08(unknown4d0c, unknown4d08, &unknown4d10);"),
    (NETWORKING, "\tindex= player_index&0xFFFF;\n\tif (!network_life_cycle_get_session(", "\tindex= player_index;\n\tif (!network_life_cycle_get_session("),
    (NETWORKING, "\treturn index<MAXIMUM_NETWORK_SESSION_PLAYERS && (players&(1u<<index))!=0;\n}\n\n/* 0x0019a9b4", "\treturn (players&(1u<<index))!=0;\n}\n\n/* 0x0019a9b4"),
    (NETWORKING, "\tif (!network_life_cycle_get_other_session(&session) || !network_session_established(session))", "\tif (!network_life_cycle_get_other_session(&session))"),
    (NETWORKING, "\tif (session && identifier && network_session_established(session))", "\tif (session && network_session_established(session))"),
    (NETWORKING, "\t\t\tif ((players&(1u<<index)) && network_player_identifier_equal(", "\t\t\tif (network_player_identifier_equal("),
    (NETWORKING, "\t\t\t\tfound= index;", "\t\t\t\tfound= index+1;"),
]
