# mutations of milestone 878, written by make_mutations.py

TESTS = "test_small_878"

SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"

MUTATIONS = [
    (SOURCE_EFFECTS_EFFECTS_C, '\teffect[0x02]&= ~1;', '\teffect[0x3]&= ~1;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (*(const real *)(event+0x08)==*(const real *)(event+0x0C))', '\tif (*(const real *)(event+0x08)!=*(const real *)(event+0x0C))'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (event_index<0 || event_index>=*(const long *)(definition+0x14))', '\tif (event_index<0 || event_index>*(const long *)(definition+0x14))'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t}\n\tdefinition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(effect+0x04)&0xFFFF].address;\n\tif (event_index<0 || event_index>=*(const long *)(definition+0x14))', '\t}\n\tdefinition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(effect+0x5)&0xFFFF].address;\n\tif (event_index<0 || event_index>=*(const long *)(definition+0x14))'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (!location || global_structure_bsp_index==NONE)', '\tif (!location || global_structure_bsp_index!=NONE)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\treturn *(const short *)(effect+0x20)!=NONE;', '\treturn *(const short *)(effect+0x21)!=NONE;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t: *(const short *)(*(const byte *const *)((const byte *)global_structure_bsp+0x30)+*(const long *)(effect+0x1C)*8);', '\t\t: *(const short *)(*(const byte *const *)((const byte *)global_structure_bsp+0x30)+*(const long *)(effect+0x1C)*9);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tconst byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(effect+0x04)&0xFFFF].address;', '\tconst byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(effect+0x5)&0xFFFF].address;'),
]
