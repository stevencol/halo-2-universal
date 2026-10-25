# mutations of milestone 867, written by make_mutations.py

TESTS = "test_small_867"

SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C = "source/game/game_unknown_0003f2b0.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_GAME_GAME_UNKNOWN_00134D90_C = "source/game/game_unknown_00134d90.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\t\t&& (object= object_try_and_get_and_verify_type(object_index, NONE)) && (object[0x07]&1))', '\t\t\t&& (object= object_try_and_get_and_verify_type(object_index, 0)) && (object[0x07]&1))'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\tif (instance==0x7FF)', '\t\tif (instance!=0x7FF)'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\tif (!lightmaps || *(const long *)(lightmaps+0x80)<=0 || !bsp', '\tif (!lightmaps || *(const long *)(lightmaps+0x80)<=1 || !bsp'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\t+*(const unsigned short *)(*(const byte *const *)(lightmap+0x54)+instance*12+2)*0x38+0x0C), 3))', '\t\t+*(const unsigned short *)(*(const byte *const *)(lightmap+0x54)+instance*12+2)*0x38+0x0C), 4))'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (((1UL<<(object[0xAA]&31))&0x1C) && ((object[0x12C]>>3)&1))\n\t{\n\t\treturn TRUE;\n\t}\n\n\treturn FALSE;\n}\n', '\tif (((1UL<<(object[0xAA]&31))&0x1C) && ((object[0x12C]>>3)&1))\n\t{\n\t\treturn TRUE;\n\t}\n\n\treturn TRUE;\n}\n'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tconst byte *parent_unit= object_try_and_get_and_verify_type(index, 3);', '\t\tconst byte *parent_unit= object_try_and_get_and_verify_type(index, 4);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tfor (index= *(const long *)(object+0x10); index!=NONE; )', '\tfor (index= *(const long *)(object+0x10); index!=0; )'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tif (object_unknown_000beb30(index))\n\t\t{\n\t\t\treturn TRUE;\n\t\t}\n\t\tindex= *(const long *)(child+0x0C);', '\t\tif (object_unknown_000beb30(index))\n\t\t{\n\t\t\treturn FALSE;\n\t\t}\n\t\tindex= *(const long *)(child+0x0C);'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\t\t\t(const real_matrix4x3 *)(*(const byte *const *)(model+0x4C)+node_index*0x60+0x28), (real_matrix4x3 *)matrix);', '\t\t\t(const real_matrix4x3 *)(*(const byte *const *)(model+0x4C)+node_index*0x60+0x29), (real_matrix4x3 *)matrix);'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\t\tout[10]= matrix[9]*matrix[0];', ''),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tconst byte *model= (const byte *)(uintptr_t)global_tag_instances[model_index&0xFFFF].address;', '\tconst byte *model= (const byte *)(uintptr_t)global_tag_instances[model_index&0x10000].address;'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\t\tout[6]= matrix[8]*matrix[0];', ''),
]
