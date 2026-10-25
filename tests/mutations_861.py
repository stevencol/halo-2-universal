# mutations of milestone 861, written by make_mutations.py
# (two taken out by hand that change nothing: with > the graph at the count is still none, as unknown4C_get_tag gives NULL for it; and the zeros of memset are all written over by havok_unknown_001ddb40)

TESTS = "test_small_861"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_GAME_GAME_UNKNOWN_00134D90_C = "source/game/game_unknown_00134d90.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\tif ((datum[0x04]&1) && !(force&1))', '\tif ((datum[0x04]&1) && !(force&2))'),
    (SOURCE_UNITS_UNITS_C, '\t\treturn (*(const unsigned short *)(datum+0x26)&(1UL<<(global_structure_bsp_index&31)))!=0;', '\t\treturn (*(const unsigned short *)(datum+0x26)&(1UL<<(global_structure_bsp_index&31)))==0;'),
    (SOURCE_UNITS_UNITS_C, '\tnamed= *(const short *)(datum+0x02)!=NONE || unit_unknown_000d59f0(datum);', '\tnamed= *(const short *)(datum+0x02)!=NONE && unit_unknown_000d59f0(datum);'),
    (SOURCE_UNITS_UNITS_C, '\tplaced= *(const short *)(datum+0x2C)==NONE || *(const short *)(datum+0x2C)==global_structure_bsp_index;', '\tplaced= *(const short *)(datum+0x2C)==NONE && *(const short *)(datum+0x2C)==global_structure_bsp_index;'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tvalue+= *(const real *)(state+0x18);', ''),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\t\treturn unknown_0013bb90(ramp+0x0C, value, 0.0f);', '\t\treturn unknown_0013bb90(ramp+0xd, value, 0.0f);'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tif (*(const short *)(ramps+scale_index*24+0x16)!=NONE)', '\tif (*(const short *)(ramps+scale_index*24+0x16)==NONE)'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\t\tresult= 0.0f;', ''),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tlong index)\n{\n\tconst byte *state= unknown_004e6740+index*32+4;\n\n\tif (!state)', '\tlong index)\n{\n\tconst byte *state= unknown_004e6740+index*32+5;\n\n\tif (!state)'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tfor (index= 0; index<*(const long *)(current+0x2C); index++)', '\t\tfor (index= 1; index<*(const long *)(current+0x2C); index++)'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tif (*(const long *)(*(const byte *const *)(current+0x30)+index*0x6C)==name)', '\t\t\tif (*(const long *)(*(const byte *const *)(current+0x30)+index*0x6C)!=name)'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\t\tunsigned long found= (unsigned short)graph_index|(unsigned long)index<<16;', '\t\t\t\tunsigned long found= (unsigned short)graph_index|(unsigned long)index<<17;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tvector[2]= 0.0f;', '\tvector[3]= 0.0f;'),
    (SOURCE_PHYSICS_HAVOK_C, '\thavok_unknown_001ddb40(data,\n\t\t(const byte *)(uintptr_t)global_tag_instances[*(const long *)channel&0xFFFF].address,\n\t\t*(const unsigned long *)(channel+0x04));', '\thavok_unknown_001ddb40(data,\n\t\t(const byte *)(uintptr_t)global_tag_instances[*(const long *)channel&0x10000].address,\n\t\t*(const unsigned long *)(channel+0x04));'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (*(const short *)(channel+0x06)==NONE)', '\tif (*(const short *)(channel+0x06)!=NONE)'),
]
