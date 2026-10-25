# mutations of milestone 823, written by make_mutations.py

TESTS = "test_small_823"

SOURCE_MAIN_MAIN_C = "source/main/main.c"
SOURCE_EFFECTS_PARTICLE_SYSTEMS_C = "source/effects/particle_systems.c"
SOURCE_GAME_GAME_UNKNOWN_0013BF00_C = "source/game/game_unknown_0013bf00.c"
SOURCE_INTERFACE_USER_INTERFACE_C = "source/interface/user_interface.c"

MUTATIONS = [
    # taken out: one byte more is written past the 0x50 that the test compares
    (SOURCE_MAIN_MAIN_C, '\tmemcpy(destination, source, 0x50);\n\treturn TRUE;\n}', '\tmemcpy(destination, source, 0x50);\n\treturn FALSE;\n}'),
    (SOURCE_MAIN_MAIN_C, '\twhile (node && *(const long *)(node+0x70)!=value)', '\twhile (node && *(const long *)(node+0x70)==value)'),
    (SOURCE_MAIN_MAIN_C, '\tbyte *node= *(byte *const *)(data+0x14);', '\tbyte *node= *(byte *const *)(data+0x15);'),
    (SOURCE_MAIN_MAIN_C, '\t\tnode= *(byte *const *)(node+0x18);', ''),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\twhile (index!=NONE)', '\twhile (index==NONE)'),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\t\tif (*(const short *)(entry+0x0A)==value)', '\t\tif (*(const short *)(entry+0x0A)!=value)'),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\t\tindex= *(const short *)(entry+0x08);', '\t\tindex= *(const short *)(entry+0x9);'),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\t\tconst byte *entry= entries+index*0x6C;', '\t\tconst byte *entry= entries+index*0x6d;'),
    (SOURCE_GAME_GAME_UNKNOWN_0013BF00_C, '\tunknown_00510c4c= game_state_malloc(0x1E4);', ''),
    (SOURCE_GAME_GAME_UNKNOWN_0013BF00_C, '\tmemset(unknown_004e6950, 0, sizeof(unknown_004e6950));', '\tmemset(unknown_004e6950, 1, sizeof(unknown_004e6950));'),
    (SOURCE_INTERFACE_USER_INTERFACE_C, '\treturn !user_interface_unknown_0014a224() && unknown_0054d5b8-unknown_0054e7fc>=200;', '\treturn !user_interface_unknown_0014a224() || unknown_0054d5b8-unknown_0054e7fc>=200;'),
]
