# mutations of milestone 756: one or two lines of each function

TESTS = "test_small_756"

MATCHMAKING = "source/networking/online_matchmaking.c"

MUTATIONS = [
    (MATCHMAKING, "\tif (*(const long *)(data+4)==2)", "\tif (*(const long *)(data+4)!=2)"),
    (MATCHMAKING, "unknown_0054e8e0[gamepad_index]+UNKNOWN_0054E8E0_UNKNOWN4E0_OFFSET;", "unknown_0054e8e0[gamepad_index]+UNKNOWN_0054E8E0_UNKNOWN4E0_OFFSET+4;"),
    (MATCHMAKING, "\t\treturn 0x9C1;", "\t\treturn 0x9C0;"),
    (MATCHMAKING, "\t\treturn 0x6C4;", "\t\treturn 0x6C5;"),
    (MATCHMAKING, "\t\treturn 0x681;", "\t\treturn 0x581;"),
    (MATCHMAKING, "\t\treturn value>0 && !(value&1);", "\t\treturn value>0 && (value&1);"),
    (MATCHMAKING, "\t\treturn value==2;", "\t\treturn value==1;"),
    (MATCHMAKING, "\t\treturn value==8;", "\t\treturn value==4;"),
    (MATCHMAKING, "\tcase 2:\n\tcase 4:\n\t\treturn value==4;", "\tcase 2:\n\t\treturn value==4;\n\tcase 4:"),
]
