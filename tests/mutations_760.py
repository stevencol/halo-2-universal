# mutations of milestone 760: one or two lines of each function

TESTS = "test_small_760"

NETWORKING = "source/interface/user_interface_networking.c"

MUTATIONS = [
    (NETWORKING, "\tcase 2:\n\t\treturn 8;", "\tcase 2:\n\t\treturn 2;"),
    (NETWORKING, "\t\tcase 3:\n\t\t\treturn 5;", "\t\tcase 3:\n\t\t\treturn 4;"),
    (NETWORKING, "\t\tcase 5:\n\t\t\treturn 7;", "\t\tcase 5:\n\t\t\treturn 0;"),
    (NETWORKING, "\tcase 4:\n\t\treturn 2;\n\tdefault:\n\t\treturn 0;\n\t}\n}", "\tcase 4:\n\t\treturn 1;\n\tdefault:\n\t\treturn 0;\n\t}\n}"),
    (NETWORKING, "\treturn network_life_cycle_unknown_00063e90(unknown)==0;", "\treturn network_life_cycle_unknown_00063e90(unknown)!=0;"),
    (NETWORKING, "\treturn unknown ? *unknown : 0;", "\treturn unknown ? 1 : 0;"),
    (NETWORKING, "\tcase 1:\n\t\treturn 1;\n\tdefault:\n\t\treturn 2;", "\tcase 1:\n\t\treturn 2;\n\tdefault:\n\t\treturn 2;"),
    (NETWORKING, "\treturn network_life_cycle_session_get_unknown49B0()==2;", "\treturn network_life_cycle_session_get_unknown49B0()>=2;"),
    (NETWORKING, "\treturn network_life_cycle_session_get_unknown18()==2;", "\treturn (byte)network_life_cycle_session_get_unknown18()==2;"),
]
