# mutations of milestone 852, written by make_mutations.py

TESTS = "test_small_852"

SOURCE_HS_HS_RUNTIME_C = "source/hs/hs_runtime.c"
SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C = "source/game/game_unknown_0002b4a0.c"
SOURCE_SOUND_SOUND_MANAGER_C = "source/sound/sound_manager.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    (SOURCE_HS_HS_RUNTIME_C, '\tlong absolute_index= data_next_absolute_index(data, 0);', '\tlong absolute_index= data_next_absolute_index(data, 1);'),
    (SOURCE_HS_HS_RUNTIME_C, '\t\tabsolute_index= data_next_absolute_index(data, absolute_index+1);', '\t\tabsolute_index= data_next_absolute_index(data, absolute_index+2);'),
    (SOURCE_HS_HS_RUNTIME_C, '\t\tbyte *node= (byte *)data->data+absolute_index*0x14;', '\t\tbyte *node= (byte *)data->data+absolute_index*0x15;'),
    (SOURCE_HS_HS_RUNTIME_C, '\twhile (absolute_index!=NONE)', '\twhile (absolute_index==NONE)'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\t\tif (load && !geometry_cache_block_load(section+0x38, 3))', '\t\tif (load && !geometry_cache_block_load(section+0x39, 3))'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\tif (load && !geometry_cache_block_load(section+0x28, 3))', '\tif (load && !geometry_cache_block_load(section+0x28, 4))'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\t\tsection= *(byte *const *)(bsp+0x13C)+definition_index*0xC8;', ''),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\t\tsection= *(byte *const *)(definition+0x28)+((place>>9)&0x1FF)*0x5C;', '\t\tsection= *(byte *const *)(definition+0x28)+((place>>9)&0x1FF)*0x5d;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\t\tshort section= *(const short *)(*(const byte *const *)(group+0x0C)+part*0x10+0x04);', '\t\t\tshort section= *(const short *)(*(const byte *const *)(group+0xd)+part*0x10+0x04);'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\t\tboolean predicted= geometry_cache_block_predict(*(byte *const *)(definition+0x28)+section*0x5C+0x38);', '\t\t\tboolean predicted= geometry_cache_block_predict(*(byte *const *)(definition+0x29)+section*0x5C+0x38);'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tfor (index= 0; index<*(const long *)(definition+0x1C); index++)', '\tfor (index= 0; index<*(const long *)(definition+0x1C)-1; index++)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\t\tboolean predicted= geometry_cache_block_predict(*(byte *const *)(definition+0x28)+index*0x5C+0x38);', '\t\t\tboolean predicted= geometry_cache_block_predict(*(byte *const *)(definition+0x28)+index*0x5d+0x38);'),
    (SOURCE_UNITS_UNITS_C, '\t\tlong double square= i*i+j*j+k*k;', ''),
    (SOURCE_UNITS_UNITS_C, '\tif (state==3)', '\tif (state!=3)'),
    (SOURCE_UNITS_UNITS_C, '\tstate= biped[0x3DC];', '\tstate= biped[0x3dd];'),
    (SOURCE_UNITS_UNITS_C, '\tif (state==4 || state==5 || unit_unknown_000e4050(biped_index))', '\tif (state==4 && state==5 || unit_unknown_000e4050(biped_index))'),
]
