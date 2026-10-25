# mutations of milestone 830, written by make_mutations.py

TESTS = "test_small_830"

SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"
SOURCE_MAIN_MAIN_C = "source/main/main.c"
SOURCE_NETWORKING_GAME_VOICE_C = "source/networking/game_voice.c"
SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    (SOURCE_MATH_REAL_MATH_C, '\treal difference= vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2]-1.0f;', '\treal difference= vector[0]*vector[0]+vector[2]*vector[1]+vector[2]*vector[2]-1.0f;'),
    # taken out: a difference that is no number is not less than 0.001 either, and exactly 0.001 is not a square of a vector the test makes
    (SOURCE_MAIN_MAIN_C, '\tresult= flags&0x400000 ? result|0x20 : result&0xFFDF;', '\tresult= flags&0x400000 ? result|0x20 : result&0xffe0;'),
    # taken out: bit 0 is shifted away, the byte past the 12 is set after, and at the start the rising part is 0 too
    (SOURCE_MAIN_MAIN_C, '\tresult= flags&0x40000 ? result|4 : result&0xFFFB;', ''),
    (SOURCE_MAIN_MAIN_C, '\treturn flags&0x200000 ? result|0x40 : result&0xFFBF;', '\treturn flags&0x200000 ? result|0x40 : result&0xffc0;'),
    (SOURCE_NETWORKING_GAME_VOICE_C, '\t}\n\tindex= *(const short *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x28);\n\tvalue= (real)unknown_004e9af0[index]*game_time_globals->seconds_per_tick*4.0f;', '\t}\n\tvalue= (real)unknown_004e9af0[index]*game_time_globals->seconds_per_tick*4.0f;'),
    (SOURCE_NETWORKING_GAME_VOICE_C, '\tindex= *(const short *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x28);\n\tvalue= (real)unknown_004e9af0[index]*game_time_globals->seconds_per_tick*4.0f;\n', '\tindex= *(const short *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x28);\n'),
    (SOURCE_NETWORKING_GAME_VOICE_C, '\tindex= *(const short *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x28);\n\tvalue= (real)unknown_004e9af0[index]*game_time_globals->seconds_per_tick*4.0f;\n\n\treturn !(value>=1.0f);\n}\n', '\tindex= *(const short *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x28);\n\tvalue= (real)unknown_004e9af0[index]*game_time_globals->seconds_per_tick*4.0f;\n\n\treturn !(value>1.0f);\n}\n'),
    (SOURCE_NETWORKING_GAME_VOICE_C, '\tif (!unknown_0055e4d0[*(const long *)(unknown_004e9ae8+UNKNOWN_004E9AE8_UNKNOWNC14_OFFSET)])\n\t{\n\t\treturn TRUE;\n\t}\n\tindex= *(const short *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x28);', '\tif (!unknown_0055e4d0[*(const long *)(unknown_004e9ae8+UNKNOWN_004E9AE8_UNKNOWNC14_OFFSET)])\n\t{\n\t\treturn FALSE;\n\t}\n\tindex= *(const short *)((const byte *)player_data->data+(player_index&0xFFFF)*0x21C+0x28);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (value>values[3]-values[4])', '\tif (value>=values[3]-values[4])'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\treturn (values[3]-value)*values[5];', '\t\treturn (values[4]-value)*values[5];'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (values[1]+values[0]>value)', '\tif (values[2]+values[0]>value)'),
    (SOURCE_UNITS_UNITS_C, '\tdata[0x62]= 0;', '\tdata[0x63]= 0;'),
    (SOURCE_UNITS_UNITS_C, '\tmemcpy(data+0x24, global_identity4x3, 0x34);', '\tmemcpy(data+0x25, global_identity4x3, 0x34);'),
    (SOURCE_UNITS_UNITS_C, '\tmemcpy(data, global_zero_vector3d, 12);', '\tmemcpy(data, global_zero_vector3d, 13);'),
]
