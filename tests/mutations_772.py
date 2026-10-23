# mutations of milestone 772: one or two lines of each function

TESTS = "test_small_772"

ENTRIES = "source/game/game_unknown_001991f0.c"
NETWORKING = "source/interface/user_interface_networking.c"

MUTATIONS = [
    (ENTRIES, "\tunknown_00511028= TRUE;", "\tunknown_00511020= TRUE;"),
    (ENTRIES, "\tunknown_0051102c= time(NULL);", "\tunknown_0051102c= 0;"),
    (ENTRIES, "\tif (index==NONE)\n\t{\n\t\tgame_unknown_001991f0_mark(", "\tif (index!=NONE)\n\t{\n\t\tgame_unknown_001991f0_mark("),
    (ENTRIES, "game_unknown_001991f0_find(data, unknown_004cf7cc)", "game_unknown_001991f0_find(data, unknown_004cf7cc+1)"),
    (ENTRIES, "\tunknown_00511020= TRUE;\n\tunknown_00511024= time(NULL);", "\tunknown_00511020= FALSE;\n\tunknown_00511024= time(NULL);"),
    (ENTRIES, "[GAME_UNKNOWN_001991F0_UNKNOWN9_OFFSET]= TRUE;", "[GAME_UNKNOWN_001991F0_UNKNOWN8_OFFSET]= TRUE;"),
    (ENTRIES, "\tif (index!=NONE)\n\t{\n\t\tunknown_0051e8d4", "\tif (index>0)\n\t{\n\t\tunknown_0051e8d4"),
    (NETWORKING, "\treturn user_interface_networking_unknown_0019999e(unknown_0019989d());", "\treturn unknown_0019989d()<=1;"),
    (NETWORKING, "\tif (!network_life_cycle_get_state_session(&session))\n\t{\n\t\treturn NONE;", "\tif (!network_life_cycle_get_state_session(&session))\n\t{\n\t\treturn 0;"),
    (NETWORKING, "\tif (!network_life_cycle_get_session(&session))\n\t{\n\t\treturn NONE;\n\t}\n\n\treturn user_interface_networking_find_player(session, identifier);", "\tnetwork_life_cycle_get_session(&session);\n\n\treturn user_interface_networking_find_player(session, identifier);"),
]
