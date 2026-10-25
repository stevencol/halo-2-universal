# mutations of milestone 860, written by make_mutations.py

TESTS = "test_small_860"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_SOUND_SOUND_MANAGER_C = "source/sound/sound_manager.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\tif (!unit_unknown_000e4050(biped_index) && !unit_unknown_000e0dc0(biped_index))', '\tif (!unit_unknown_000e4050(biped_index) || !unit_unknown_000e0dc0(biped_index))'),
    (SOURCE_UNITS_UNITS_C, '\tif ((*(const unsigned long *)(definition+0x264)>>1)&1)', '\tif ((*(const unsigned long *)(definition+0x264)>>2)&1)'),
    (SOURCE_UNITS_UNITS_C, '\tif ((*(const unsigned long *)(definition+0x264)>>1)&1)\n\t{\n\t\treturn FALSE;\n\t}\n\tif (!unit_unknown_000e4050(biped_index) && !unit_unknown_000e0dc0(biped_index))', '\tif ((*(const unsigned long *)(definition+0x264)>>1)&1)\n\t{\n\t\treturn TRUE;\n\t}\n\tif (!unit_unknown_000e4050(biped_index) && !unit_unknown_000e0dc0(biped_index))'),
    (SOURCE_UNITS_UNITS_C, '\tif (change==0.0f)\n\t{\n\t\treturn FALSE;\n\t}\n\t*height_change= (*(const real *)(definition+0x268)-*(const real *)(definition+0x26C))', '\tif (change==0.0f)\n\t{\n\t\treturn TRUE;\n\t}\n\t*height_change= (*(const real *)(definition+0x268)-*(const real *)(definition+0x26C))'),
    (SOURCE_PHYSICS_HAVOK_C, '\twhile (child_index!=NONE)', '\twhile (child_index==NONE)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tconst byte *object= (const byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;', '\tconst byte *object= (const byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0x10000].datum;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tchild_index= *(const long *)(child+0x0C);', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (*(const long *)(object+0xB4)==NONE)', '\tif (*(const long *)(object+0xB4)!=NONE)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tfor (listener_index= 0; listener_index<4; listener_index++, listener+= 0x48)', '\tfor (listener_index= 1; listener_index<4; listener_index++, listener+= 0x48)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tmemset(unknown_0054e8a0, 0, ((*(const long *)(structure_bsp+0x9C)+31)>>5)*4);', '\tmemset(unknown_0054e8a0, 0, ((*(const long *)(structure_bsp+0x9C)+31)>>6)*4);'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\t\t\tunknown_0054e8a0[cluster_index>>5]|= 1UL<<(cluster_index&31);', '\t\t\t\tunknown_0054e8a0[cluster_index>>5]|= 1UL<<(cluster_index&32);'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tif (!listener[0x02] || *(const short *)listener==NONE)', '\t\tif (!listener[0x02] || *(const short *)listener==0)'),
]
