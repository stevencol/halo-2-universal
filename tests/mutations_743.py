# mutations of milestone 743: one or two lines of each function

TESTS = "test_small_743"

LIFE = "source/networking/network_life_cycle.c"

MUTATIONS = [
    (LIFE, "\tif (*(const long *)(session+NETWORK_SESSION_STATE_OFFSET)!=_network_session_state_none &&\n\t\t!network_session_unknown_00058d90(session))\n\t{\n\t\tnetwork_session_leave(session, FALSE);", "\tif (*(const long *)(session+NETWORK_SESSION_STATE_OFFSET)!=_network_session_state_none &&\n\t\t!network_session_unknown_00058d90(session))\n\t{\n\t\tnetwork_session_leave(session, TRUE);"),
    (LIFE, "\tif (!something[0xC])\n\t{\n\t\tnetwork_life_cycle_leave_session(something);\n\t}\n\t*(long *)(something+0x18)= NONE;", "\tif (!something[0xD])\n\t{\n\t\tnetwork_life_cycle_leave_session(something);\n\t}\n\t*(long *)(something+0x18)= NONE;"),
    (LIFE, "\t*(long *)(something+0x18)= NONE;", "\t*(long *)(something+0x18)= 0;"),
    (LIFE, "\t*(long *)(something+0x1C)= 0;", "\t*(long *)(something+0x20)= 0;"),
    (LIFE, "\tif (!something[0xC])\n\t{\n\t\tnetwork_life_cycle_leave_session(something);\n\t}\n\t*(long *)(something+0xFC)= 0;", "\tif (something[0xC])\n\t{\n\t\tnetwork_life_cycle_leave_session(something);\n\t}\n\t*(long *)(something+0xFC)= 0;"),
    (LIFE, "\t*(long *)(something+0x100)= 0;\n\t*(long *)(something+0x104)= 0;\n}", "\t*(long *)(something+0x100)= 0;\n}"),
    (LIFE, "\tnetwork_session_host_set_unknown4980(session, 7);", "\tnetwork_session_host_set_unknown4980(session, 6);"),
    (LIFE, "\t*(long *)(something+0x968)= time(NULL);", "\t*(long *)(something+0x96C)= time(NULL);"),
    (LIFE, "\t(*(long *)(something+0xA8C))++;", "\t(*(long *)(something+0xA90))++;"),
    (LIFE, "\tsomething[0xA78]= TRUE;", "\tsomething[0xA78]= FALSE;"),
    (LIFE, "\t\tstate<_network_session_state3 || state>_network_session_state8)", "\t\tstate<_network_session_state3 || state>_network_session_state9)"),
    (LIFE, "\tif (state==_network_session_state_none || network_session_unknown_00058d90(session) ||", "\tif (state==_network_session_state_none ||"),
    (LIFE, "\tif (unknown4980>=2 && unknown4980<=4)", "\tif (unknown4980>=1 && unknown4980<=4)"),
    (LIFE, "\t\tvalue= 9;", "\t\tvalue= 7;"),
    (LIFE, "\t*(long *)(something+0x978)= 2;", "\t*(long *)(something+0x978)= 1;"),
]
