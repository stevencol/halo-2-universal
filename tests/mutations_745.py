# mutations of milestone 745: one or two lines of each function

TESTS = "test_small_745"

LIFE = "source/networking/network_life_cycle.c"

MUTATIONS = [
    (LIFE, "\tlong task= *(const long *)(something+0xC);\n\n\tif (task!=NONE)", "\tlong task= *(const long *)(something+0xC);\n\n\tif (task!=0)"),
    (LIFE, "\t\t*(long *)(something+0xC)= NONE;", "\t\t*(long *)(something+0xC)= 0;"),
    (LIFE, "\tsomething[8]= FALSE;", "\tsomething[9]= FALSE;"),
    (LIFE, "\tnetwork_life_cycle_close_tasks(unknown_0047712c);", ""),
    (LIFE, "\tunknown_00477088= FALSE;", "\tunknown_00477088= TRUE;"),
    (LIFE, "\tif (!something[0xA])", "\tif (!something[0xB])"),
    (LIFE, "\tkey= &unknown_004cf7d4[*(const long *)(something+0x2C)];", "\tkey= &unknown_004cf7d4[*(const long *)(something+0x28)];"),
    (LIFE, "\t\tXNetUnregisterKey(key->identifier);\n\t\tkey->valid= FALSE;", "\t\tXNetUnregisterKey(key->key);\n\t\tkey->valid= FALSE;"),
    (LIFE, "\t\tXNetUnregisterKey(key->identifier);\n\t\tkey->valid= FALSE;\n\t}\n\tsomething[0xA]= FALSE;", "\t\tXNetUnregisterKey(key->identifier);\n\t}\n\tsomething[0xA]= FALSE;"),
    (LIFE, "\tsomething[0xA]= FALSE;\n}", "}"),
    (LIFE, "\tif (!unknown_00510518)\n\t{\n\t\treturn;\n\t}\n\tif (unknown_00510530!=NONE)", "\tif (unknown_00510530!=NONE)"),
    (LIFE, "\t\tunknown_00510530= NONE;", "\t\tunknown_00510530= 0;"),
    (LIFE, "\tunknown_00510514= unknown;", "\tunknown_00510514= 0;"),
    (LIFE, "\tunknown_00510518= 0;\n\tunknown_00510514", "\tunknown_00510514"),
]
