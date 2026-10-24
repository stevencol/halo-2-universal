# mutations of milestone 809, written by make_mutations.py

TESTS = "test_small_809"

SOURCE_CACHE_TAG_UNKNOWN_0016D180_C = "source/cache/tag_unknown_0016d180.c"
SOURCE_GAME_GAME_UNKNOWN_00131FF0_C = "source/game/game_unknown_00131ff0.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_MAIN_LEVELS_C = "source/main/levels.c"

MUTATIONS = [
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\t\treturn *(byte *const *)(tag+0x5C)+index*0x50+0x20;', '\t\treturn *(byte *const *)(tag+0x5C)+index*0x51+0x20;'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\t\treturn *(byte *const *)(tag+0x74)+index*0x100+0x20;', '\t\treturn *(byte *const *)(tag+0x75)+index*0x100+0x20;'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\t\treturn *(byte *const *)(tag+0x6C)+index*0x60+0x20;', '\t\treturn *(byte *const *)(tag+0x6d)+index*0x60+0x20;'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\tcase 1:', '\tcase 2:'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\tif ((signed char)thing[0x2A60]<0)', '\tif ((signed char)thing[0x2a61]<0)'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\tunsigned long result= (((flags>>2)&0x2C00)|(flags&0x4000))>>9 | (flags&0x200)<<5;', '\tunsigned long result= (((flags>>2)&0x2C00)|(flags&0x4000))>>9 | (flags&0x200)<<6;'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\t\tif (unknown_004b9f8c!=NONE && unknown_004b9ffc==0.0f && !(flags&0x400)', '\t\tif (unknown_004b9f8c!=NONE && unknown_004b9ffc==0.0f && !(flags&0x401)'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\t\tresult|= 1;', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (*(const long *)(data+0x20)!=NONE && *(const short *)(data+0x26)!=NONE)', '\tif (*(const long *)(data+0x20)!=NONE || *(const short *)(data+0x26)!=NONE)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (!unknown_0046fbf4 || *(const long *)(data+0x20)==NONE || *(const short *)(data+0x26)==NONE || 0.45f>value)', '\tif (!unknown_0046fbf4 || *(const long *)(data+0x21)==NONE || *(const short *)(data+0x26)==NONE || 0.45f>value)'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tfirst|= 0x380;', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tsecond|= 0x380;', ''),
    (SOURCE_MAIN_LEVELS_C, '\tif (!block)\n\t{\n\t\treturn NONE;\n\t}\n\tfor (index= 0; index<block[0]; index++)', '\tif (!block)\n\t{\n\t\treturn 0;\n\t}\n\tfor (index= 0; index<block[0]; index++)'),
    (SOURCE_MAIN_LEVELS_C, '\t\tif (element[0]==value && element[1]!=NONE && highest<=element[1])', '\t\tif (element[0]==value && element[1]==NONE && highest<=element[1])'),
    (SOURCE_MAIN_LEVELS_C, '\tlong highest= NONE;', '\tlong highest= 0;'),
    (SOURCE_MAIN_LEVELS_C, '\t\t\thighest= element[1];', '\t\t\thighest= element[2];'),
]
