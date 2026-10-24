# mutations of milestone 807, written by make_mutations.py

TESTS = "test_small_807"

SOURCE_SOUND_SOUND_MANAGER_C = "source/sound/sound_manager.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_RASTERIZER_RASTERIZER_XBOX_C = "source/rasterizer/rasterizer_xbox.c"

MUTATIONS = [
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tdata[14]= unknown_0054e854;', '\tdata[15]= unknown_0054e854;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tdata[2]= point[2];', ''),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tdata[0]= point[0];', '\tdata[0]= point[1];'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tdata[7]= ((const unsigned long *)global_zero_vector3d)[2];', '\tdata[8]= ((const unsigned long *)global_zero_vector3d)[2];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (*(const long *)(first+0x04)==NONE || *(const long *)(first+0x0C)==NONE)', '\tif (*(const long *)(first+0x04)==0 || *(const long *)(first+0x0C)==NONE)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\treturn *(const long *)(second+0x74)>0 && *(const long *)(second+0x7C)>0;', '\treturn *(const long *)(second+0x75)>0 && *(const long *)(second+0x7C)>0;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (first_index==NONE)', '\tif (first_index!=NONE)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tsecond= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(first+0x04)&0xFFFF].address;', '\tsecond= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(first+0x5)&0xFFFF].address;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tdata[0x86]= 1;', '\tdata[0x87]= 1;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tdata[0x8A]= 2;', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tdata[0x82]= 1;', '\tdata[0x83]= 1;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tdata[0x92]= 1;', '\tdata[0x92]= 2;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (((1UL<<(header->type&0x1F))&2))', '\tif (((1UL<<(header->type&0x20))&2))'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tresult[3]= values[0x40/4];', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tif (((*(const unsigned long *)(definition+0x1EC)>>19)&1)', '\t\tif (((*(const unsigned long *)(definition+0x1ed)>>19)&1)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tresult[2]= values[0x3C/4];\n\tresult[3]= values[0x40/4];\n}', '\tresult[2]= values[0x3C/4];\n\tresult[3]= values[0x40/5];\n}'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\trasterizer_state_flags_00485b80|= rasterizer_flags_group2_004850bc|rasterizer_flags_group2_004850ac;', ''),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\trasterizer_state_flags_00485b88|= rasterizer_flags_group2_00485068|rasterizer_flags_group2_00485060;', ''),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\trasterizer_state_flags_00485b7c|= rasterizer_flags_group2_00485090|rasterizer_flags_group2_00485080;', ''),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\trasterizer_state_flags_00485b78|= rasterizer_flags_group2_0048508c|rasterizer_flags_group2_0048507c;', ''),
]
