# mutations of milestone 795, written by make_mutations.py

TESTS = "test_small_795"

SOURCE_GAME_GAME_ENGINE_C = "source/game/game_engine.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_ENGINE_C, '\tif (MAIN_GAME_GLOBALS_LONG(0x1B8)<=0)\n\t{\n\t\treturn FALSE;\n\t}\n\tplayer= PLAYER_DATA(player_index);', '\tif (MAIN_GAME_GLOBALS_LONG(0x1B8)<=0)\n\t{\n\t\treturn TRUE;\n\t}\n\tplayer= PLAYER_DATA(player_index);'),
    (SOURCE_GAME_GAME_ENGINE_C, '\treturn *(const long *)(player+0x2C)==NONE && *(const short *)(player+0x1AC)==0;', '\treturn *(const long *)(player+0x2C)==NONE && *(const short *)(player+0x1ad)==0;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif (MAIN_GAME_GLOBALS_LONG(0x1B8)<=0)', '\tif (MAIN_GAME_GLOBALS_LONG(0x1B8)<0)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tplayer= PLAYER_DATA(player_index);', ''),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tif (*(const short *)(unknown_004e9ae8+0xC20+index*0x10)==value)', '\t\tif (*(const short *)(unknown_004e9ae8+0xC20+index*0x11)==value)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tfor (index= 0; index<*(const long *)(unknown_004e9ae8+0xC18); index++)', '\tfor (index= 0; index<*(const long *)(unknown_004e9ae8+0xc19); index++)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t\treturn unknown_004e9ae8+0xC1C+index*0x10;', '\t\t\treturn unknown_004e9ae8+0xC1C+index*0x11;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif (!GAME_ENGINE_GET() || index==NONE)', '\tif (!GAME_ENGINE_GET() || index!=NONE)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t}\n\tif (MAIN_GAME_GLOBALS_LONG(0x180)==7 && (*unknown_00510c9c&(1UL<<(index&0x1F)))\n\t\t&& (MAIN_GAME_GLOBALS_LONG(0x22C)&2))', '\t}\n\tif (MAIN_GAME_GLOBALS_LONG(0x180)==7 || (*unknown_00510c9c&(1UL<<(index&0x1F)))\n\t\t&& (MAIN_GAME_GLOBALS_LONG(0x22C)&2))'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t&& (MAIN_GAME_GLOBALS_LONG(0x22C)&2))', '\t\t|| (MAIN_GAME_GLOBALS_LONG(0x22C)&2))'),
    (SOURCE_GAME_GAME_ENGINE_C, '\treturn MAIN_GAME_GLOBALS_LONG(0x1C4)!=1;', '\treturn MAIN_GAME_GLOBALS_LONG(0x1C4)!=2;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tif (value==*(const long *)(*(const byte *const *)(element+0x6C)+index*8+4))', '\t\tif (value==*(const long *)(*(const byte *const *)(element+0x6C)+index*9+4))'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tconst byte *element= *(const byte *const *)(tag+0x0C);', '\tconst byte *element= *(const byte *const *)(tag+0xd);'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tfor (index= 0; index<*(const long *)(element+0x68); index++)', '\tfor (index= 0; index<*(const long *)(element+0x68)-1; index++)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t}\n\t}\n\n\treturn NONE;\n}\n\n/* 0x0015fef0: TRUE without a kind of game; for the kind 7 with the bit of', '\t\t}\n\t}\n\n\treturn 0;\n}\n\n/* 0x0015fef0: TRUE without a kind of game; for the kind 7 with the bit of'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif (MAIN_GAME_GLOBALS_LONG(0x180)==7 && (*unknown_00510c9c&(1UL<<(index&0x1F))))', '\tif (MAIN_GAME_GLOBALS_LONG(0x180)==7 && (*unknown_00510c9c&(1UL<<(index&0x20))))'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif (MAIN_GAME_GLOBALS_LONG(0x180)==7 && (*unknown_00510c9c&(1UL<<(index&0x1F))))\n\t{\n\t\treturn MAIN_GAME_GLOBALS_LONG(0x22C)&1;\n\t}\n', '\tif (MAIN_GAME_GLOBALS_LONG(0x180)==7 && (*unknown_00510c9c&(1UL<<(index&0x1F))))\n\t{\n\t\treturn MAIN_GAME_GLOBALS_LONG(0x22C)&2;\n\t}\n'),
    (SOURCE_GAME_GAME_ENGINE_C, '\treturn (MAIN_GAME_GLOBALS_LONG(0x184)>>1)&1;', '\treturn (MAIN_GAME_GLOBALS_LONG(0x184)>>2)&1;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif (!GAME_ENGINE_GET())\n\t{\n\t\treturn TRUE;\n\t}\n\tif (MAIN_GAME_GLOBALS_LONG(0x180)==7 && (*unknown_00510c9c&(1UL<<(index&0x1F))))', '\tif (!GAME_ENGINE_GET())\n\t{\n\t\treturn FALSE;\n\t}\n\tif (MAIN_GAME_GLOBALS_LONG(0x180)==7 && (*unknown_00510c9c&(1UL<<(index&0x1F))))'),
    (SOURCE_GAME_GAME_ENGINE_C, '{\n\treturn GAME_ENGINE_GET() && (MAIN_GAME_GLOBALS_LONG(0x184)&1) && index>=0 && index<8\n\t\t&& (*(const unsigned short *)(unknown_004e9ae8+0x0C)&(1UL<<index));', '{\n\treturn GAME_ENGINE_GET() && (MAIN_GAME_GLOBALS_LONG(0x184)&1) || index>=0 && index<8\n\t\t&& (*(const unsigned short *)(unknown_004e9ae8+0x0C)&(1UL<<index));'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t&& (*(const unsigned short *)(unknown_004e9ae8+0x0C)&(1UL<<index));', '\t\t|| (*(const unsigned short *)(unknown_004e9ae8+0x0C)&(1UL<<index));'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t&& (*(const unsigned short *)(unknown_004e9ae8+0x0E)&(1UL<<index));', '\t\t&& (*(const unsigned short *)(unknown_004e9ae8+0xf)&(1UL<<index));'),
    (SOURCE_GAME_GAME_ENGINE_C, '{\n\treturn GAME_ENGINE_GET() && (MAIN_GAME_GLOBALS_LONG(0x184)&1) && index>=0 && index<8\n\t\t&& (*(const unsigned short *)(unknown_004e9ae8+0x0E)&(1UL<<index));', '{\n\treturn GAME_ENGINE_GET() && (MAIN_GAME_GLOBALS_LONG(0x184)&2) && index>=0 && index<8\n\t\t&& (*(const unsigned short *)(unknown_004e9ae8+0x0E)&(1UL<<index));'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t}\n\t}\n\n\treturn NONE;\n}\n\n/* 0x00162b10: whether the game is of the kind 9 and the definition of an', '\t\t}\n\t}\n\n\treturn 0;\n}\n\n/* 0x00162b10: whether the game is of the kind 9 and the definition of an'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tfor (tries= 0; tries<7; tries++)', '\tfor (tries= 0; tries<8; tries++)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tif (index==8)', '\t\tif (index==9)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tif (bits&(1UL<<(index&0x1F)))', '\t\tif (bits&(1UL<<(index&0x20)))'),
    (SOURCE_GAME_GAME_ENGINE_C, '\treturn *(const short *)((const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address+0x290)==1;', '\treturn *(const short *)((const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0x10000].address+0x290)==1;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif (MAIN_GAME_GLOBALS_LONG(0x180)!=9 || object_index==NONE)\n\t{\n\t\treturn FALSE;\n\t}\n\tobject= (const byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;', '\tif (MAIN_GAME_GLOBALS_LONG(0x180)!=9 || object_index==NONE)\n\t{\n\t\treturn TRUE;\n\t}\n\tobject= (const byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif (MAIN_GAME_GLOBALS_LONG(0x180)!=9 || object_index==NONE)', '\tif (MAIN_GAME_GLOBALS_LONG(0x180)==9 || object_index==NONE)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tobject= (const byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;', '\tobject= (const byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0x10000].datum;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tvalues[index]= value;', ''),
    # taken out: one long more is written past the buffer of the test, which is not compared
]
