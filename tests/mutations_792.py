# mutations of milestone 792, written by make_mutations.py

TESTS = "test_small_792"

SOURCE_MAIN_MAIN_C = "source/main/main.c"
SOURCE_GAME_GAME_UNKNOWN_00131FF0_C = "source/game/game_unknown_00131ff0.c"
SOURCE_GAME_GAME_UNKNOWN_00134D90_C = "source/game/game_unknown_00134d90.c"

MUTATIONS = [
    (SOURCE_MAIN_MAIN_C, '\tif (!display_settings.frame_vblanks || !main_game_globals || !main_game_globals->active)', '\tif (!display_settings.frame_vblanks || !main_game_globals && !main_game_globals->active)'),
    (SOURCE_MAIN_MAIN_C, '\t\treturn FALSE;', '\t\treturn TRUE;'),
    (SOURCE_MAIN_MAIN_C, '\treturn main_time_globals.unknown0 ? main_time_globals.unknown1 : TRUE;', '\treturn main_time_globals.unknown0 ? main_time_globals.unknown1 : FALSE;'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\treturn (((const unsigned long *)(thing+0x21C))[bit>>5]&(1UL<<(bit&0x1F)))!=0;', '\treturn (((const unsigned long *)(thing+0x21C))[bit>>5]&(1UL<<(bit&0x1F)))!=1;'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\treturn (flag4000 ? 0x4000 : 0) | (flag2000 ? 0x2000 : 0) | (flag1000 ? 0x1000 : 0) | (flag0800 ? 0x800 : 0)', '\treturn (flag4000 ? 0x4000 : 0) | (flag2000 ? 0x2000 : 0) | (flag1000 ? 0x1000 : 0) | (flag0800 ? 0x800 : 1)'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\t\t| (flag0400 ? 0x400 : 0) | (flag0200 ? 0x200 : 0) | (flag8000 ? 0x8000 : 0);', '\t\t| (flag0400 ? 0x400 : 0) | (flag0200 ? 0x201 : 0) | (flag8000 ? 0x8000 : 0);'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\tif (count<=0)', '\tif (count<0)'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\tused= *(const unsigned short *)(thing+0xA5C);', ''),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\tlongs= (count+0x1F)>>5;', ''),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\treturn (unsigned long *)(thing+0x25C)+used;', '\treturn (unsigned long *)(thing+0x25d)+used;'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\tmemset(thing+0x21C, 0, 0x40);', '\tmemset(thing+0x21C, 1, 0x40);'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\tmemset(thing+0x1C, 0xFF, 0x80*4);', '\tmemset(thing+0x1C, 0x100, 0x80*4);'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\tfor (index= 0; index<4; index++)', '\tfor (index= 1; index<4; index++)'),
    (SOURCE_GAME_GAME_UNKNOWN_00131FF0_C, '\tmemset(thing+0x25C, 0, *(const unsigned short *)(thing+0xA5C)*4);', '\tmemset(thing+0x25C, 0, *(const unsigned short *)(thing+0xA5C)*5);'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tif (data && global_scenario && number)', '\tif (data || global_scenario && number)'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\t\tlong count= *(const long *)((const byte *)global_scenario+0x3C0);', '\t\tlong count= *(const long *)((const byte *)global_scenario+0x3c1);'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\t\tfor (which= 0; which<count; which++)', '\t\tfor (which= 0; which<count-1; which++)'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\t\tconst long *entries= *(const long *const *)((const byte *)global_scenario+0x3C4);', '\t\tconst long *entries= *(const long *const *)((const byte *)global_scenario+0x3c5);'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tfor (index= 0; index<*(const long *)(tag+0x1C); index++)', '\tfor (index= 0; index<*(const long *)(tag+0x1d); index++)'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\t\t\tcount+= (*(const byte *const *)(tag+0x28))[elements[index]*0x5C+0x25];', '\t\t\tcount+= (*(const byte *const *)(tag+0x29))[elements[index]*0x5C+0x25];'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\t\tcount= *(const long *)(*(const byte *const *)(tag+0x38)+(signed char)tag[which+0x3C]*0x0C+0x04)', '\t\tcount= *(const long *)(*(const byte *const *)(tag+0x38)+(signed char)tag[which+0x3C]*0x0C+0x5)'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tconst byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;', '\tconst byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0x10000].address;'),
]
