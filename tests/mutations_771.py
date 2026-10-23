# mutations of milestone 771: one or more lines of each function

TESTS = "test_small_771"

PLAYERS = "source/game/players.c"
LEVELS = "source/main/levels.c"

MUTATIONS = [
    (PLAYERS, "\tlong object_index= *(const long *)(player+0x2C);", "\tlong object_index= *(const long *)(player+0x30);"),
    (PLAYERS, "\t\tobject_index= *(const long *)(player+0x30);\n\t\tif (object_index==NONE)\n\t\t{\n\t\t\treturn FALSE;", "\t\tobject_index= *(const long *)(player+0x30);\n\t\tif (object_index==NONE)\n\t\t{\n\t\t\treturn TRUE;"),
    (PLAYERS, "\tobject_get_origin(object_index, origin);\n\n\treturn TRUE;", "\tobject_get_origin(object_index, origin);\n\n\treturn FALSE;"),
    (LEVELS, "\tif (*unknown!=LEVELS_UNKNOWN_0019B70B_KIND)", "\tif (*unknown!=4)"),
    (LEVELS, "\treturn number==1 || number==3 || number==13;", "\treturn number==1 || number==3 || number==12;"),
    (LEVELS, "\tmemcpy(copy, unknown, sizeof(copy));", "\tmemcpy(copy, unknown, sizeof(copy)-4);"),
]
