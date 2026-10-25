# mutations of milestone 856, written by make_mutations.py
# (two taken out by hand that change nothing: clamping to 1 with >= instead of >, and to 0 with >= instead of >, which only turns -0 into +0, and the harness takes them as equal)

TESTS = "test_small_856"

SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_RASTERIZER_RASTERIZER_XBOX_C = "source/rasterizer/rasterizer_xbox.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_SOUND_SOUND_MANAGER_C = "source/sound/sound_manager.c"

MUTATIONS = [
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (*(const short *)datum==NONE || type<0 || type>12)', '\tif (*(const short *)datum==NONE && type<0 || type>12)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (index<0 || index>*(const long *)block-1)', '\tif (index<=0 || index>*(const long *)block-1)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (offset==NONE)', '\tif (offset!=NONE)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\toffset= *(const short *)(object_type_definitions[type]+0x0C);', ''),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tcount= *(const long *)((const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address+0x44);', ''),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\trasterizer_unknown_00014390((short)stage, bitmap, 0.0f);\n\n\treturn FALSE;\n}\n', '\trasterizer_unknown_00014390((short)stage, bitmap, 0.0f);\n\n\treturn TRUE;\n}\n'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tif (tag_index==NONE)', '\tif (tag_index!=NONE)'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tif (count<=0)\n\t{\n\t\treturn FALSE;\n\t}\n\tbitmap= bitmap_group_get_bitmap(tag_index, (short)((short)index%count));', '\tif (count<=0)\n\t{\n\t\treturn TRUE;\n\t}\n\tbitmap= bitmap_group_get_bitmap(tag_index, (short)((short)index%count));'),
    (SOURCE_PHYSICS_HAVOK_C, '\tanimation= *(byte *const *)(owner+0x30)+index*0x6C;', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\t\towner= unknown4C_get_tag((short)id, graph);', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\t}\n\tif ((short)id!=NONE)\n\t{', '\t}\n\tif ((short)id==NONE)\n\t{'),
    (SOURCE_PHYSICS_HAVOK_C, '\tbyte *animation;\n\n\tif (index==NONE)\n\t{\n\t\treturn NULL;', '\tbyte *animation;\n\n\tif (index==0)\n\t{\n\t\treturn NULL;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tstruct cached_animation *resource= (struct cached_animation *)(*(byte *const *)(graph+0xB0)+*(const long *)(animation+0x24)*0x14);', '\t\tstruct cached_animation *resource= (struct cached_animation *)(*(byte *const *)(graph+0xB0)+*(const long *)(animation+0x24)*0x15);'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tgraph= unknown4C_get_tag((short)id, graph);', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\treturn *(byte *const *)(animation+0x38);', '\treturn *(byte *const *)(animation+0x39);'),
    (SOURCE_PHYSICS_HAVOK_C, '\tconst byte *animation;\n\n\tif ((short)id!=NONE)\n\t{\n\t\tgraph= unknown4C_get_tag((short)id, graph);', '\tconst byte *animation;\n\n\tif ((short)id!=0)\n\t{\n\t\tgraph= unknown4C_get_tag((short)id, graph);'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tif (data[1]&0xF0)', '\tif (data[1]&0xf1)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\treturn (*(const real *)(data+8)-*(const real *)(data+4))*value + *(const real *)(data+4);', '\treturn (*(const real *)(data+8)-*(const real *)(data+4))*value + *(const real *)(data+5);'),
]
