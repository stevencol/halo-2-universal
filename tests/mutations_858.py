# mutations of milestone 858, written by make_mutations.py
# (one taken out by hand that changes nothing: with id>>17 in havok_unknown_001c6440,
# havok_unknown_001daea0 still sees there is no animation and gives NULL)

TESTS = "test_small_858"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_RASTERIZER_RASTERIZER_XBOX_C = "source/rasterizer/rasterizer_xbox.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\t}\n\tanimation= *(const byte *const *)(graph+0x30)+index*0x6C;\n\tblend_screen= (signed char)animation[0x12];', '\t}\n\tblend_screen= (signed char)animation[0x12];'),
    (SOURCE_PHYSICS_HAVOK_C, '\thavok_unknown_001dd9d0(graph, id);\n\tif ((short)id!=NONE)\n\t{', '\thavok_unknown_001dd9d0(graph, id);\n\tif ((short)id!=0)\n\t{'),
    (SOURCE_PHYSICS_HAVOK_C, '\treturn *(byte *const *)(graph+0x28)+blend_screen*0x1C+0x04;', '\treturn *(byte *const *)(graph+0x28)+blend_screen*0x1d+0x04;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tunsigned long id= *(const unsigned long *)(channel+0x04);', '\tunsigned long id= *(const unsigned long *)(channel+0x5);'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t(const byte *)(uintptr_t)global_tag_instances[*(const long *)channel&0xFFFF].address, id);', '\t\t(const byte *)(uintptr_t)global_tag_instances[*(const long *)channel&0x10000].address, id);'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (*(const long *)channel==NONE || *(const short *)(channel+0x06)==NONE)', '\tif (*(const long *)channel==0 || *(const short *)(channel+0x06)==NONE)'),
    (SOURCE_PHYSICS_HAVOK_C, '\treturn !(havok_unknown_001c6440(channel)[0x18]&1);', '\treturn !(havok_unknown_001c6440(channel)[0x18]&2);'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (*(const long *)channel==NONE || *(const short *)(channel+0x06)==NONE)\n\t{\n\t\treturn TRUE;\n\t}\n', '\tif (*(const long *)channel==NONE || *(const short *)(channel+0x06)==NONE)\n\t{\n\t\treturn FALSE;\n\t}\n'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tif (!bitmap)\n\t{\n\t\treturn FALSE;\n\t}\n\tunknown_0051f3c8[4+(short)stage]= (long)shell_bitmap_get_texture_blocking(bitmap, mip_bias);', '\tif (!bitmap)\n\t{\n\t\treturn TRUE;\n\t}\n\tunknown_0051f3c8[4+(short)stage]= (long)shell_bitmap_get_texture_blocking(bitmap, mip_bias);'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tif (tag_index==NONE)\n\t{\n\t\ttag_index= *(const long *)((uintptr_t)unknown_00485a80+0x64);\n\t\tindex= 0;\n\t\tif (tag_index==NONE)', '\tif (tag_index==NONE)\n\t{\n\t\tindex= 0;\n\t\tif (tag_index==NONE)'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tbitmap= bitmap_group_get_bitmap(tag_index, index);', ''),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tif (!group || *(const long *)(group+0x44)<=0)', '\tif (!group || *(const long *)(group+0x45)<=0)'),
]
