# mutations of milestone 839, written by make_mutations.py

TESTS = "test_small_839"

SOURCE_MAIN_MAIN_C = "source/main/main.c"
SOURCE_GAME_GAME_UNKNOWN_0016EFB0_C = "source/game/game_unknown_0016efb0.c"

MUTATIONS = [
    (SOURCE_MAIN_MAIN_C, '\tsum[2]= first[2]*weight+sum[2];', '\tsum[3]= first[2]*weight+sum[2];'),
    (SOURCE_MAIN_MAIN_C, '\tsum[6]= a*weight+sum[6];', '\tsum[7]= a*weight+sum[6];'),
    (SOURCE_MAIN_MAIN_C, '\tsum[8]= c*weight+sum[8];', '\tsum[8]= c*weight+sum[9];'),
    (SOURCE_MAIN_MAIN_C, '\tsum[3]= second[0]*weight+sum[3];', '\tsum[3]= second[0]*weight+sum[4];'),
    (SOURCE_GAME_GAME_UNKNOWN_0016EFB0_C, '\treturn *(const long *)(*(const byte *const *)((const byte *)global_scenario+0x34C)+index*0x10+0x08)==thing;', '\treturn *(const long *)(*(const byte *const *)((const byte *)global_scenario+0x34C)+index*0x10+0x9)==thing;'),
    (SOURCE_GAME_GAME_UNKNOWN_0016EFB0_C, '\tif (thing==NONE)', '\tif (thing!=NONE)'),
    (SOURCE_GAME_GAME_UNKNOWN_0016EFB0_C, '\tif (index==NONE)', '\tif (index==0)'),
    (SOURCE_GAME_GAME_UNKNOWN_0016EFB0_C, '\tif (value<0)', '\tif (value<=0)'),
]
