# mutations of milestone 840, written by make_mutations.py

TESTS = "test_small_840"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\t\tif (child && ((*(const unsigned long *)(UNIT_DEFINITION_GET(child)+0xBC)>>27)&1) && ((*(const unsigned long *)(child+0x04)>>26)&1))', '\t\tif (child && ((*(const unsigned long *)(UNIT_DEFINITION_GET(child)+0xBC)>>27)&1) && ((*(const unsigned long *)(child+0x5)>>26)&1))'),
    (SOURCE_UNITS_UNITS_C, '\tparent_index= *(const long *)(unit+0x14);', '\tparent_index= *(const long *)(unit+0x15);'),
    (SOURCE_UNITS_UNITS_C, '\tfor (child_index= *(const long *)(parent+0x10); child_index!=NONE; child_index= *(const long *)(UNIT_OBJECT_GET(child_index)+0x0C))', '\tfor (child_index= *(const long *)(parent+0x10); child_index==NONE; child_index= *(const long *)(UNIT_OBJECT_GET(child_index)+0x0C))'),
    (SOURCE_UNITS_UNITS_C, '\tlong child_index;\n\n\tif (unit_index==NONE)\n\t{\n\t\treturn NONE;', '\tlong child_index;\n\n\tif (unit_index==0)\n\t{\n\t\treturn NONE;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tnames[3]= *(const unsigned long *)(owner+0x7C);', '\t\tnames[3]= *(const unsigned long *)(owner+0x7d);'),
    (SOURCE_PHYSICS_HAVOK_C, '\tfound= havok_unknown_001dd560((const byte *)(uintptr_t)global_tag_instances[*(const long *)(owner+0x68)&0xFFFF].address+0x54, (long)names[2], 8);', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (names[0]==0x07000001)', '\tif (names[0]!=0x07000001)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (found && *(const unsigned long *)(found+4)!=NONE && *(const unsigned long *)(found+4)!=0x030000D9 && name!=0x0400054B)', '\tif (found && *(const unsigned long *)(found+4)!=NONE && *(const unsigned long *)(found+5)!=0x030000D9 && name!=0x0400054B)'),
]
