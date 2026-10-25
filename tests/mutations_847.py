# mutations of milestone 847, written by make_mutations.py

TESTS = "test_small_847"

SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C = "source/game/game_unknown_0003f2b0.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\tlong index= bsp3d_test_point(*(const struct bsp3d *const *)(bsp+0x18), 0, point);', '\tlong index= bsp3d_test_point(*(const struct bsp3d *const *)(bsp+0x19), 0, point);'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\tif (*leaf_index<0 || *leaf_index>=*(const long *)(bsp+0x2C) || *cluster_index<0 || *cluster_index>=*(const long *)(bsp+0x9C))', '\tif (*leaf_index<0 || *leaf_index>=*(const long *)(bsp+0x2C) || *cluster_index<1 || *cluster_index>=*(const long *)(bsp+0x9C))'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\tif (index!=NONE)', '\tif (index!=0)'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\t*cluster_index= *(const short *)(*(const byte *const *)(bsp+0x30)+index*8);\n\t\treturn TRUE;\n\t}', '\t\t*cluster_index= *(const short *)(*(const byte *const *)(bsp+0x30)+index*8);\n\t\treturn FALSE;\n\t}'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)((const byte *)main_game_globals+0x08)!=2)', '\tif (*(const long *)((const byte *)main_game_globals+0x08)!=3)'),
    (SOURCE_UNITS_UNITS_C, '\tplayer_index= *(const long *)(UNIT_OBJECT_GET(unit_index)+0x13C);', ''),
    (SOURCE_UNITS_UNITS_C, '\tif (!(weapon[0x12C]&1) || (unit_index= *(const long *)(weapon+0x154))==NONE)', '\tif (!(weapon[0x12C]&1) || (unit_index= *(const long *)(weapon+0x155))==NONE)'),
    (SOURCE_UNITS_UNITS_C, '\t}\n\tweapon= UNIT_OBJECT_GET(weapon_index);\n\tif (!(weapon[0x12C]&1) || (unit_index= *(const long *)(weapon+0x154))==NONE)', '\t}\n\tif (!(weapon[0x12C]&1) || (unit_index= *(const long *)(weapon+0x154))==NONE)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tmemset(header->datum, 0, size);', '\tmemset(header->datum, 1, size);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (index==NONE)', '\tif (index!=NONE)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\theader= &((struct object_header_datum *)object_header_data->data)[index&0xFFFF];', '\theader= &((struct object_header_datum *)object_header_data->data)[index&0x10000];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (index==NONE)\n\t{\n\t\treturn NONE;\n\t}\n\theader= &((struct object_header_datum *)object_header_data->data)[index&0xFFFF];', '\tif (index==NONE)\n\t{\n\t\treturn 0;\n\t}\n\theader= &((struct object_header_datum *)object_header_data->data)[index&0xFFFF];'),
]
