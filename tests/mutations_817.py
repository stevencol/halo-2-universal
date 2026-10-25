# mutations of milestone 817, written by make_mutations.py

TESTS = "test_small_817"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_GAME_GAME_UNKNOWN_0013BF00_C = "source/game/game_unknown_0013bf00.c"
SOURCE_MAIN_MAIN_C = "source/main/main.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\t\treturn 0x08000300;', '\t\treturn 0x8000301;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tcase 0:', '\tcase 1:'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\treturn NONE;', '\t\treturn 0;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tcase 1:', '\tcase 2:'),
    (SOURCE_GAME_GAME_UNKNOWN_0013BF00_C, '\tlong *entry= (long *)(unknown_004e69d0+index*0x270);', '\tlong *entry= (long *)(unknown_004e69d0+index*0x271);'),
    (SOURCE_GAME_GAME_UNKNOWN_0013BF00_C, '\tfor (slot= 0; slot<7; slot++)', '\tfor (slot= 0; slot<7-1; slot++)'),
    (SOURCE_GAME_GAME_UNKNOWN_0013BF00_C, '\tmemset(entry, 0, 0x270);', '\tmemset(entry, 1, 0x270);'),
    (SOURCE_GAME_GAME_UNKNOWN_0013BF00_C, '\t\tentry[slot]= NONE;', '\t\tentry[slot]= 0;'),
    (SOURCE_GAME_GAME_UNKNOWN_0013BF00_C, '\tlong index;\n\n\tfor (index= 0; index<4; index++)\n\t{\n\t\tgame_unknown_0013ac42(index);', '\tlong index;\n\n\tfor (index= 1; index<4; index++)\n\t{\n\t\tgame_unknown_0013ac42(index);'),
    (SOURCE_MAIN_MAIN_C, '\tunknown_0055e504= havok_unknown_00147090();', ''),
    (SOURCE_MAIN_MAIN_C, '\tunknown_0055e508= value;', ''),
]
