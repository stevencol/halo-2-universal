# mutations of milestone 855, written by make_mutations.py

TESTS = "test_small_855"

SOURCE_GAME_GAME_UNKNOWN_001552E0_C = "source/game/game_unknown_001552e0.c"
SOURCE_MATH_RANDOM_MATH_C = "source/math/random_math.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_RASTERIZER_RASTERIZER_XBOX_C = "source/rasterizer/rasterizer_xbox.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_UNKNOWN_001552E0_C, '\tentry[0x108]= 0;', '\tentry[0x109]= 0;'),
    (SOURCE_GAME_GAME_UNKNOWN_001552E0_C, '\tif (*(const long *)(unknown_004e8c38+0x04)!=0)', '\tif (*(const long *)(unknown_004e8c38+0x04)!=1)'),
    (SOURCE_GAME_GAME_UNKNOWN_001552E0_C, '\tbyte *entry= unknown_004e8c38+0x0C+index*0x140;', '\tbyte *entry= unknown_004e8c38+0xd+index*0x140;'),
    (SOURCE_MATH_RANDOM_MATH_C, '\tindex= (short)(((*seed>>16)*1026)>>16);', '\tindex= (short)(((*seed>>16)*1027)>>16);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (*(const short *)(definition+0x0A)==NONE || *(const short *)(definition+0x0C)==NONE)', '\tif (*(const short *)(definition+0x0A)==NONE || *(const short *)(definition+0xd)==NONE)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tblock= (const byte *)global_scenario+*(const short *)(definition+0x0A);', '\tblock= (const byte *)global_scenario+*(const short *)(definition+0xb);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tcount= *(const long *)block;', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\treturn *(byte *const *)(block+4)+*(const short *)(definition+0x0E)*index;', '\treturn *(byte *const *)(block+4)+*(const short *)(definition+0xf)*index;'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\n\treturn TRUE;\n}', '\n\treturn FALSE;\n}'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\tunknown_00485af4[stage]= *(const short *)((const byte *)bitmap+4);', ''),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\tunknown_00485af4[4+stage]= *(const short *)((const byte *)bitmap+6);', '\t\tunknown_00485af4[5+stage]= *(const short *)((const byte *)bitmap+6);'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\tunknown_0051f3c8[4+stage]= (long)shell_bitmap_get_texture_blocking(bitmap, mip_bias);', ''),
]
