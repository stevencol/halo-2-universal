# mutations of milestone 827, written by make_mutations.py

TESTS = "test_small_827"

SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C = "source/game/game_unknown_0003f2b0.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_NETWORKING_GAME_VOICE_C = "source/networking/game_voice.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\tunknown_00509440= 0;', '\tunknown_00509440= 1;'),
    # taken out: one number more is written past the sixteen, and >= clamps to the same 0 and 1
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\tfor (index= 0; index<8; index++)', '\tfor (index= 1; index<8; index++)'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\tunknown_004c8740[index]= NONE;', '\t\tunknown_004c8740[index]= 0;'),
    (SOURCE_UNITS_UNITS_C, '\tif (unit[0x3DC]!=1 && unit[0x3DC]!=3)', '\tif (unit[0x3dd]!=1 && unit[0x3DC]!=3)'),
    (SOURCE_UNITS_UNITS_C, '\tif ((signed char)unit[0x399]<lrintf((real)game_time_globals->ticks_per_second*0.18f))', '\tif ((signed char)unit[0x39a]<lrintf((real)game_time_globals->ticks_per_second*0.18f))'),
    (SOURCE_UNITS_UNITS_C, '\treturn *(const long *)(unit+0x14)==NONE;', '\treturn *(const long *)(unit+0x14)==0;'),
    (SOURCE_UNITS_UNITS_C, '\tif ((unit[0xC1]&1) || !(unit[0x348]&1))', '\tif ((unit[0xC1]&1) && !(unit[0x348]&1))'),
    (SOURCE_NETWORKING_GAME_VOICE_C, '\t\tvalue= 1.0f;', ''),
    # taken out: without the value the real is not set (undefined), and the compiler leaves the same
    (SOURCE_NETWORKING_GAME_VOICE_C, '\tif (!unknown_0055e4d0[*(const long *)(unknown_004e9ae8+UNKNOWN_004E9AE8_UNKNOWNC14_OFFSET)] || player_index==NONE)', '\tif (!unknown_0055e4d0[*(const long *)(unknown_004e9ae8+UNKNOWN_004E9AE8_UNKNOWNC14_OFFSET)] || player_index!=NONE)'),
    (SOURCE_NETWORKING_GAME_VOICE_C, '\treturn !(value>=1.0f);', '\treturn !(value>1.0f);'),
    (SOURCE_NETWORKING_GAME_VOICE_C, '\tindex= *(const short *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x28);', '\tindex= *(const short *)((const byte *)player_data->data+(player_index&0x10000)*0x21C+0x28);'),
    (SOURCE_NETWORKING_GAME_VOICE_C, '\t\treturn TRUE;\n\t}\n\tvalue= (real)unknown_004e9af0[index]*game_time_globals->seconds_per_tick*4.0f;\n\n\treturn !(value>=1.0f);', '\t\treturn TRUE;\n\t}\n\n\treturn !(value>=1.0f);'),
]
