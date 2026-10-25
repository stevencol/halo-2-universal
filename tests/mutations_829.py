# mutations of milestone 829, written by make_mutations.py

TESTS = "test_small_829"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_MAIN_MAIN_C = "source/main/main.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\tfound= havok_unknown_001dd560(found+4, second, 0x14);', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tconst byte *found= havok_unknown_001dd560(data+0x34, first, 0x14);', '\tconst byte *found= havok_unknown_001dd560(data+0x35, first, 0x14);'),
    (SOURCE_PHYSICS_HAVOK_C, '\treturn havok_unknown_001dd560(found+4, third, 0x34);', '\treturn havok_unknown_001dd560(found+5, third, 0x34);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tbyte *emitter= (byte *)particle_emitter_data->data+(emitter_index&0xFFFF)*0x4C;', '\t\tbyte *emitter= (byte *)particle_emitter_data->data+(emitter_index&0x10000)*0x4C;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlong emitter_index= *(const long *)((const byte *)particle_location_data->data+(location_index&0xFFFF)*0x34+0x04);', '\tlong emitter_index= *(const long *)((const byte *)particle_location_data->data+(location_index&0xFFFF)*0x34+0x5);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tlong next= *(const long *)(emitter+0x08);', '\t\tlong next= *(const long *)(emitter+0x9);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlong emitter_index= *(const long *)((const byte *)particle_location_data->data+(location_index&0xFFFF)*0x34+0x04);\n\n\twhile (emitter_index!=NONE)\n\t{\n\t\tbyte *emitter= (byte *)particle_emitter_data->data+(emitter_index&0xFFFF)*0x4C;', '\tlong emitter_index= *(const long *)((const byte *)particle_location_data->data+(location_index&0xFFFF)*0x34+0x04);\n\n\twhile (emitter_index==NONE)\n\t{\n\t\tbyte *emitter= (byte *)particle_emitter_data->data+(emitter_index&0xFFFF)*0x4C;'),
    (SOURCE_UNITS_UNITS_C, '\tnumber= *(const unsigned long *)(unit+*(const short *)(unit+0x12A)+0x7C);', ''),
    (SOURCE_UNITS_UNITS_C, '\tif (unit_unknown_000c7070(unit_index))\n\t{\n\t\treturn TRUE;\n\t}\n\tunit= UNIT_OBJECT_GET(unit_index);', '\tif (unit_unknown_000c7070(unit_index))\n\t{\n\t\treturn FALSE;\n\t}\n\tunit= UNIT_OBJECT_GET(unit_index);'),
    (SOURCE_UNITS_UNITS_C, '\treturn number==0x080000C4 || number==0x0400004A || number==0x050000C3;', '\treturn number==0x080000C4 || number==0x0400004A || number==0x50000c4;'),
    (SOURCE_UNITS_UNITS_C, '\tunit= UNIT_OBJECT_GET(unit_index);', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tobject[0xB1]= (byte)tag_unknown_0016d180(*(const long *)(definition+0x38), name);', '\tobject[0xB1]= (byte)tag_unknown_0016d180(*(const long *)(definition+0x39), name);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tname= *(const long *)(definition+0x30);', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '{\n\tbyte *object= OBJECT_DATA(object_index);\n\tconst byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address;\n\n\tif (!name)', '{\n\tbyte *object= OBJECT_DATA(object_index);\n\tconst byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0x10000].address;\n\n\tif (!name)'),
    (SOURCE_MAIN_MAIN_C, '\t\t(*(short *)(globals+0x0A))--;', '\t\t(*(short *)(globals+0xb))--;'),
    (SOURCE_MAIN_MAIN_C, '{\n\tbyte *player= (byte *)player_data->data+(player_index&0xFFFF)*0x21C;\n\tbyte *globals= (byte *)players_globals;', '{\n\tbyte *player= (byte *)player_data->data+(player_index&0xFFFF)*0x21d;\n\tbyte *globals= (byte *)players_globals;'),
    (SOURCE_MAIN_MAIN_C, '\tlong old= *(const long *)(player+0x24);', '\tlong old= *(const long *)(player+0x25);'),
    (SOURCE_MAIN_MAIN_C, '\tplayer_unknown_0014f270(player_index, NONE);', '\tplayer_unknown_0014f270(player_index, 0);'),
]
