# mutations of milestone 759: one or two lines of each function

TESTS = "test_small_759"

SESSION = "source/networking/network_session.c"
LIFE = "source/networking/network_life_cycle.c"

MUTATIONS = [
    (SESSION, "\tlong value= roster->player_unknown248[player_index];", "\tlong value= roster->player_unknown288[player_index];"),
    (SESSION, "\tif (hopper_index==0)\n\t{\n\t\treturn value;", "\tif (hopper_index==1)\n\t{\n\t\treturn value;"),
    (SESSION, "network_session_roster_group_maximum_unknown248(roster, roster->player_groups[player_index]);", "network_session_roster_group_maximum_unknown248(roster, player_index);"),
    (SESSION, "\tif (maximum==NONE || value>network_hopper_unknown_001931a0(maximum, hopper))", "\tif (maximum==NONE || value<network_hopper_unknown_001931a0(maximum, hopper))"),
    (SESSION, "\treturn network_hopper_unknown_001931a0(maximum, hopper);\n}", "\treturn network_hopper_unknown_001931a0(maximum-1, hopper);\n}"),
    (LIFE, "\tset= network_session_leader_set_unknown49c8(session, unknown49c8);", "\tset= network_session_leader_set_unknown49c8(session, 0);"),
    (LIFE, "\tunknown_00074aa0();\n\tunknown_00065770(session);", "\tunknown_00065770(session);"),
    (LIFE, "\t\thopper_index= *(const long *)(session+NETWORK_SESSION_UNKNOWN49C8_OFFSET);", "\t\thopper_index= *(const long *)(session+NETWORK_SESSION_UNKNOWN49C8_OFFSET+4);"),
    (LIFE, "\treturn *(const long *)(other+0x170)<=*(const long *)(other+0x16C);", "\treturn *(const long *)(other+0x170)<*(const long *)(other+0x16C);"),
    (LIFE, "\t\t*asked= FALSE;\n\n\t\treturn FALSE;", "\t\treturn FALSE;"),
    (LIFE, "\tif (!unknown_004cf95c)\n\t{\n\t\treturn TRUE;", "\tif (!unknown_004cf95c)\n\t{\n\t\treturn FALSE;"),
    (LIFE, "\tif (state==_network_session_state7 && !session[NETWORK_SESSION_UNKNOWN7420_OFFSET])\n\t{\n\t\treturn FALSE;", "\tif (state==_network_session_state7 && !session[NETWORK_SESSION_UNKNOWN7420_OFFSET])\n\t{\n\t\treturn TRUE;"),
    (LIFE, "\t*asked= TRUE;\n\n\treturn !network_session_host_unknown_0005aeb0(session, NONE);", "\t*asked= TRUE;\n\n\treturn network_session_host_unknown_0005aeb0(session, NONE);"),
]
