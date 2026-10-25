# mutations of milestone 879, written by make_mutations.py

TESTS = "test_small_879"

SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"

MUTATIONS = [
    (SOURCE_EFFECTS_EFFECTS_C, '\t\trestart_index= 0;', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\t(((*(const unsigned short *)((effect)+0x02)>>1)&1) || ((*(const unsigned short *)((effect)+0x02)>>9)&1))', '\t(((*(const unsigned short *)((effect)+0x02)>>1)&1) && ((*(const unsigned short *)((effect)+0x02)>>9)&1))'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tchance= *(const real *)(*(const byte *const *)(definition+0x18)+event_index*0x38+0x04);', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlong restart_index= *(const short *)(definition+0x04);', '\tlong restart_index= *(const short *)(definition+0x5);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t\tlocation.leaf_index= bsp3d_test_point(global_bsp3d, 0, (const real_point3d *)(particle_location+0x10));', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tif (*(const short *)(system+0x22)==structure_bsp_index)', '\t\tif (*(const short *)(system+0x22)!=structure_bsp_index)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tif (!(data->in_use_bit_vector[index>>5]&(1UL<<(index&31))))', '\t\tif (!(data->in_use_bit_vector[index>>6]&(1UL<<(index&31))))'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tif (*(const long *)(system+0x30)!=NONE && structure_bsp_index!=NONE)', '\t\tif (*(const long *)(system+0x30)==NONE && structure_bsp_index!=NONE)'),
]
