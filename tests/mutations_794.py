# mutations of milestone 794, written by make_mutations.py

TESTS = "test_small_794"

SOURCE_GAME_GAME_ENGINE_C = "source/game/game_engine.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_ENGINE_C, '\tunknown_004e9ae8[0xF4]|= (byte)(1<<(index&0x1F));', '\tunknown_004e9ae8[0xF4]|= (byte)(1<<(index&0x20));'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tunknown_004e9ae8[0xF4]|= (byte)(1<<(index&0x1F));\n\tif (player_index!=NONE)\n\t{', '\tunknown_004e9ae8[0xF4]|= (byte)(1<<(index&0x1F));\n\tif (player_index!=0)\n\t{'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tlong player_index= index!=NONE ? PLAYERS_GLOBALS_PLAYER(index) : NONE;', '\tlong player_index= index==NONE ? PLAYERS_GLOBALS_PLAYER(index) : NONE;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t((real *)(unknown_004e9ae8+0xE4))[index]= 1.0f;', '\t((real *)(unknown_004e9ae8+0xe5))[index]= 1.0f;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tvalue= *(const short *)(unknown_004e9ae8+0xE0);', ''),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif (!(value>=1.0f))\n\t{\n\t\treturn FALSE;\n\t}\n\tplayer_index= unknown_004b9ed8!=NONE ? PLAYERS_GLOBALS_PLAYER(unknown_004b9ed8) : NONE;', '\tif (!(value>=1.0f))\n\t{\n\t\treturn TRUE;\n\t}\n\tplayer_index= unknown_004b9ed8!=NONE ? PLAYERS_GLOBALS_PLAYER(unknown_004b9ed8) : NONE;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tplayer_index= unknown_004b9ed8!=NONE ? PLAYERS_GLOBALS_PLAYER(unknown_004b9ed8) : NONE;', '\tplayer_index= unknown_004b9ed8==NONE ? PLAYERS_GLOBALS_PLAYER(unknown_004b9ed8) : NONE;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\treturn player_index==NONE || *(const long *)(PLAYER_DATA(player_index)+0x2C)==NONE;', '\treturn player_index==NONE || *(const long *)(PLAYER_DATA(player_index)+0x2C)==0;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tvalue= unknown_004b9ed8!=NONE ? ((const real *)(unknown_004e9ae8+0xE4))[unknown_004b9ed8] : 1.0f;', '\tvalue= unknown_004b9ed8!=NONE ? ((const real *)(unknown_004e9ae8+0xe5))[unknown_004b9ed8] : 1.0f;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\treturn !((MAIN_GAME_GLOBALS_LONG(0x22C)>>4)&1);', '\t\treturn !((MAIN_GAME_GLOBALS_LONG(0x22C)>>5)&1);'),
    (SOURCE_GAME_GAME_ENGINE_C, 'boolean game_engine_unknown_00159d40(\n\tvoid)\n{\n\tlong kind= MAIN_GAME_GLOBALS_LONG(0x180);\n\n\tif (kind==1 || kind==9)\n\t{', 'boolean game_engine_unknown_00159d40(\n\tvoid)\n{\n\tlong kind= MAIN_GAME_GLOBALS_LONG(0x181);\n\n\tif (kind==1 || kind==9)\n\t{'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\treturn !((MAIN_GAME_GLOBALS_LONG(0x22C)>>4)&1);\n\t}\n\tif (kind==3)\n\t{\n\t\treturn !(MAIN_GAME_GLOBALS_LONG(0x22C)&1);', '\t\treturn !((MAIN_GAME_GLOBALS_LONG(0x22C)>>4)&1);\n\t}\n\tif (kind==4)\n\t{\n\t\treturn !(MAIN_GAME_GLOBALS_LONG(0x22C)&1);'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tlong kind= MAIN_GAME_GLOBALS_LONG(0x180);\n\n\tif (kind==1 || kind==9)\n\t{\n\t\treturn !((MAIN_GAME_GLOBALS_LONG(0x22C)>>4)&1);', '\tlong kind= MAIN_GAME_GLOBALS_LONG(0x180);\n\n\tif (kind==1 || kind!=9)\n\t{\n\t\treturn !((MAIN_GAME_GLOBALS_LONG(0x22C)>>4)&1);'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\treturn (MAIN_GAME_GLOBALS_LONG(0x22C)>>4)&1;\n\t}\n\tif (kind==3)\n\t{\n\t\treturn MAIN_GAME_GLOBALS_LONG(0x22C)&1;', '\t\treturn (MAIN_GAME_GLOBALS_LONG(0x22C)>>4)&1;\n\t}\n\tif (kind==4)\n\t{\n\t\treturn MAIN_GAME_GLOBALS_LONG(0x22C)&1;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tlong kind= MAIN_GAME_GLOBALS_LONG(0x180);\n\n\tif (kind==1 || kind==9)\n\t{\n\t\treturn (MAIN_GAME_GLOBALS_LONG(0x22C)>>4)&1;', '\tlong kind= MAIN_GAME_GLOBALS_LONG(0x180);\n\n\tif (kind==2 || kind==9)\n\t{\n\t\treturn (MAIN_GAME_GLOBALS_LONG(0x22C)>>4)&1;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\treturn MAIN_GAME_GLOBALS_LONG(0x22C)&1;', '\t\treturn MAIN_GAME_GLOBALS_LONG(0x22d)&1;'),
    (SOURCE_GAME_GAME_ENGINE_C, 'boolean game_engine_unknown_00159d80(\n\tvoid)\n{\n\tlong kind= MAIN_GAME_GLOBALS_LONG(0x180);\n\n\tif (kind==1 || kind==9)\n\t{', 'boolean game_engine_unknown_00159d80(\n\tvoid)\n{\n\tlong kind= MAIN_GAME_GLOBALS_LONG(0x181);\n\n\tif (kind==1 || kind==9)\n\t{'),
    (SOURCE_GAME_GAME_ENGINE_C, '\treturn GAME_ENGINE_GET() && MAIN_GAME_GLOBALS_LONG(0x180)==7 && (*unknown_00510c9c&(1UL<<(index&0x1F)))', '\treturn GAME_ENGINE_GET() && MAIN_GAME_GLOBALS_LONG(0x180)==7 || (*unknown_00510c9c&(1UL<<(index&0x1F)))'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t&& (MAIN_GAME_GLOBALS_LONG(0x22C)&0x20);', '\t\t|| (MAIN_GAME_GLOBALS_LONG(0x22C)&0x20);'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif (*((const byte *)main_game_globals+0x0C)!=4)', '\tif (*((const byte *)main_game_globals+0xd)!=4)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tobject[0xAF]= 0xFF;', '\tobject[0xb0]= 0xFF;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tbyte *object= (byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;', '\tbyte *object= (byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0x10000].datum;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\treturn GAME_ENGINE_GET() && *(const short *)(unknown_004e9ae8+0x6C)==1', '\treturn GAME_ENGINE_GET() && *(const short *)(unknown_004e9ae8+0x6C)==2'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t&& (*((const byte *)main_game_globals+0x0C)==4 || *(const long *)(unknown_004e9ae8+0xC04)==1);', '\t\t&& (*((const byte *)main_game_globals+0x0C)==4 && *(const long *)(unknown_004e9ae8+0xC04)==1);'),
]
