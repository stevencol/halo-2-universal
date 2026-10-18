# mutations of milestone 740: one or two lines of each function

TESTS = "test_small_740"

LIFE = "source/networking/network_life_cycle.c"
VOICE = "source/networking/game_voice.c"

MUTATIONS = [
    (LIFE, "\t\t\treturn *(const long *)(session+NETWORK_SESSION_UNKNOWN49A4_OFFSET);", "\t\t\treturn *(const long *)(session+NETWORK_SESSION_UNKNOWN49A4_OFFSET+4);"),
    (LIFE, "\t\t\treturn *(const long *)(session+NETWORK_SESSION_UNKNOWN498C_OFFSET);", "\t\t\treturn *(const long *)(session+NETWORK_SESSION_UNKNOWN498C_OFFSET+1);"),
    (LIFE, "\t\t\treturn session+NETWORK_SESSION_UNKNOWN49A1_OFFSET;", "\t\t\treturn session+NETWORK_SESSION_UNKNOWN49A1_OFFSET+1;"),
    (LIFE, "long network_life_cycle_session_get_unknown49AC(\n\tvoid)\n{\n\tif (unknown_00527330)", "long network_life_cycle_session_get_unknown49AC(\n\tvoid)\n{\n\tif (!unknown_00527330)"),
    (LIFE, "\t\t\treturn *(const long *)(session+NETWORK_SESSION_UNKNOWN49B0_OFFSET);\n\t\t}\n\t}\n\n\treturn 0;", "\t\t\treturn *(const long *)(session+NETWORK_SESSION_UNKNOWN49B0_OFFSET);\n\t\t}\n\t}\n\n\treturn NONE;"),
    (LIFE, "\t\t\treturn *(const long *)(session+NETWORK_SESSION_UNKNOWN18_OFFSET);", "\t\t\treturn *(const long *)(session+NETWORK_SESSION_UNKNOWN14_OFFSET);"),
    (LIFE, "state<=_network_session_state8)\n\t\t{\n\t\t\treturn *(const long *)(session+NETWORK_SESSION_UNKNOWN18_OFFSET);", "state<=_network_session_state7)\n\t\t{\n\t\t\treturn *(const long *)(session+NETWORK_SESSION_UNKNOWN18_OFFSET);"),
    (LIFE, "state<=_network_session_state8 &&\n\t\t\tsession[NETWORK_SESSION_UNKNOWN49A8_OFFSET])\n\t\t{\n\t\t\treturn *(const long *)(session+NETWORK_SESSION_UNKNOWN49B0_OFFSET);", "state<=_network_session_state8)\n\t\t{\n\t\t\treturn *(const long *)(session+NETWORK_SESSION_UNKNOWN49B0_OFFSET);"),
    (LIFE, "\tunknown_004cd8ca= TRUE;", "\tunknown_004cd8ca= 2;"),
    (LIFE, "\tunknown_004cd8f4= unknown_004cf96c;", "\tunknown_004cd8f4= unknown_004cf968;"),
    (LIFE, "\tif (!unknown_004cd8ca)\n\t{\n\t\treturn FALSE;", "\tif (0)\n\t{\n\t\treturn FALSE;"),
    (LIFE, "\t*unknown_004cf968_reference= unknown_004cd8f0;", "\t*unknown_004cf968_reference= unknown_004cd8f4;"),
    (VOICE, "\tunknown_004cdc50[index]= 0;", "\tunknown_004cdc50[0]= 0;"),
    (VOICE, "\t\tprofile[0xCC+index]= 0;", "\t\tprofile[0xCC]= 0;"),
    (VOICE, "\t\t*(long *)(profile+0xB4+index*4)= 0;", "\t\t*(long *)(profile+0xB0+index*4)= 0;"),
    (VOICE, "\tmemcpy(identifier, profile+UNKNOWN_004CD904_IDENTIFIER_OFFSET, UNKNOWN_004CD904_IDENTIFIER_SIZE);", "\tmemcpy(identifier, profile+UNKNOWN_004CD904_IDENTIFIER_OFFSET, UNKNOWN_004CD904_IDENTIFIER_SIZE-1);"),
]
