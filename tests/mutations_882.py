# mutations of milestone 882, written by make_mutations.py

TESTS = "test_small_882"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_INPUT_INPUT_XBOX_C = "source/input/input_xbox.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\t\tnames[0], names[1], names[2], urgent, other);', '\t\tnames[0], names[1], names[3], urgent, other);'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (*(const long *)(state+0x68)==NONE)', '\tif (*(const long *)(state+0x68)!=NONE)'),
    (SOURCE_PHYSICS_HAVOK_C, '\thavok_unknown_001cc2f0(mode, weapon_class, weapon_type, names, state, 0x07000101);', '\thavok_unknown_001cc2f0(mode, weapon_class, weapon_type, names, state, 0x7000102);'),
    (SOURCE_PHYSICS_HAVOK_C, '\thavok_unknown_001ddd00((const byte *)(uintptr_t)global_tag_instances[*(const long *)(state+0x68)&0xFFFF].address,', '\thavok_unknown_001ddd00((const byte *)(uintptr_t)global_tag_instances[*(const long *)(state+0x68)&0x10000].address,'),
    (SOURCE_INPUT_INPUT_XBOX_C, '\tfor (index= 0; index<sizeof(buttons)/sizeof(buttons[0]); index++)', '\tfor (index= 0; index<sizeof(buttons)/sizeof(buttons[0])-1; index++)'),
    (SOURCE_INPUT_INPUT_XBOX_C, '\tstatic const long buttons[]= { 0, 1, 2, 3, 4, 5, 6, 7, 12, 13 };', '\tstatic const long buttons[]= { 0, 1, 2, 3, 4, 5, 6, 7, 13, 13 };'),
    (SOURCE_INPUT_INPUT_XBOX_C, '\t\t\treturn TRUE;', '\t\t\treturn FALSE;'),
    (SOURCE_INPUT_INPUT_XBOX_C, '\treturn FALSE;', '\treturn TRUE;'),
]
