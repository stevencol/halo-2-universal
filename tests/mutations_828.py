# mutations of milestone 828, written by make_mutations.py

TESTS = "test_small_828"

SOURCE_GAME_GAME_C = "source/game/game.c"
SOURCE_SCENARIO_SCENARIO_C = "source/scenario/scenario.c"
SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C = "source/game/game_unknown_0002b4a0.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_C, '\t\tcount|= values[0]>0.0f ? 4 : 8;', '\t\tcount|= values[0]>=0.0f ? 4 : 8;'),
    (SOURCE_GAME_GAME_C, '\t\tcount= values[1]>0.0f ? 1 : 0.0f>values[1] ? 2 : 0;', '\t\tcount= values[1]>0.0f ? 1 : 0.0f>values[2] ? 2 : 0;'),
    (SOURCE_GAME_GAME_C, '\tif (!game || !game->active || *(const long *)((const byte *)game+0x08)!=1)', '\tif (!game || !game->active || *(const long *)((const byte *)game+0x9)!=1)'),
    (SOURCE_GAME_GAME_C, '\t\t\t\tseed= seed*0x19660D+0x3C6EF35F;', ''),
    (SOURCE_SCENARIO_SCENARIO_C, '\tif (global_scenario && globals && *(const long *)(globals+0x170))', '\tif (global_scenario && globals || *(const long *)(globals+0x170))'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\tif (*(const long *)thing==campaign && value>map && (result==NONE || value<result))', '\t\tif (*(const long *)thing!=number && value>above && (result==NONE || value<result))'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\tlong value= *(const long *)(thing+0x04);', '\t\tlong value= *(const long *)(thing+0x5);'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\tblock= *(const byte *const *)(globals+0x174);', ''),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\tunknown_004c1528= 0;', ''),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\tif (*(const long *)(unknown_004c0b80+0x2A78))', '\tif (*(const long *)(unknown_004c0b80+0x2a79))'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\tunknown_004c0b7c= 0;', ''),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\t\tunknown_004c0b78= unknown_004c0b7c;', ''),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\t\treturn *(byte *const *)(definition+0x28)+((bits>>9)&0x1FF)*0x5C+4;', '\t\treturn *(byte *const *)(definition+0x28)+((bits>>9)&0x1FF)*0x5d+4;'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\treturn *(byte *const *)(bsp+0x13C)+*(const short *)(*(const byte *const *)(bsp+0x144)+((bits>>18)&0x7FF)*0x58+0x34)*0xC8;', '\treturn *(byte *const *)(bsp+0x13C)+*(const short *)(*(const byte *const *)(bsp+0x145)+((bits>>18)&0x7FF)*0x58+0x34)*0xC8;'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\t\treturn *(byte *const *)(bsp+0xA0)+((bits>>9)&0x1FF)*0xB0;', '\t\treturn *(byte *const *)(bsp+0xA0)+((bits>>10)&0x1FF)*0xB0;'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\tunsigned long bits= *(const unsigned long *)(entry+0x0C);', '\tunsigned long bits= *(const unsigned long *)(entry+0xd);'),
]
