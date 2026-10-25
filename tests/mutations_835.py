# mutations of milestone 835, written by make_mutations.py

TESTS = "test_small_835"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_SOUND_SOUND_MANAGER_C = "source/sound/sound_manager.c"
SOURCE_GAME_GAME_UNKNOWN_0016EFB0_C = "source/game/game_unknown_0016efb0.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\tbyte *counters= unit+*(const short *)(unit+0x33E);', '\tbyte *counters= unit+*(const short *)(unit+0x33f);'),
    (SOURCE_UNITS_UNITS_C, '\tresult|= unit_counter_update(counters+0x90);', ''),
    (SOURCE_UNITS_UNITS_C, '\tresult|= unit_counter_update(counters+0x8C);', '\tresult|= unit_counter_update(counters+0x8d);'),
    (SOURCE_UNITS_UNITS_C, '\tresult|= unit_counter_update(counters+0x84);', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (*(const long *)(OBJECT_DATA(object_index)+0xD4)!=NONE)', '\tif (*(const long *)(OBJECT_DATA(object_index)+0xD4)==NONE)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tobject= OBJECT_DATA(object_index);', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (((const byte *)main_game_globals)[0x0C]==4)', '\tif (((const byte *)main_game_globals)[0x0C]==5)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tunknown_000b58c0(*(const long *)(OBJECT_DATA(object_index)+0xD4), 0x2000);', '\t\tunknown_000b58c0(*(const long *)(OBJECT_DATA(object_index)+0xD4), 0x2001);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tchild_index= *(const long *)(OBJECT_DATA(child_index)+0x0C);', '\t\tchild_index= *(const long *)(OBJECT_DATA(child_index)+0xd);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tchild_index= *(const long *)((const byte *)header->datum+0x10);', '\tchild_index= *(const long *)((const byte *)header->datum+0x11);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tconst struct object_header_datum *header= &((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF];', '\tconst struct object_header_datum *header= &((const struct object_header_datum *)object_header_data->data)[object_index&0x10000];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif ((1UL<<(header->type&0x1F))&0x1003)', '\tif ((1UL<<(header->type&0x1F))&0x1004)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tentry[3]= 0;', '\t\tentry[3]= 1;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tentry[5]|= 1;', '\tentry[6]|= 1;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tentry= (byte *)unknown_004ed28c->data+(index&0xFFFF)*0x18;', '\tentry= (byte *)unknown_004ed28c->data+(index&0xFFFF)*0x19;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tentry[3]= 1;', ''),
    (SOURCE_GAME_GAME_UNKNOWN_0016EFB0_C, '\t\treturn FALSE;', '\t\treturn TRUE;'),
    (SOURCE_GAME_GAME_UNKNOWN_0016EFB0_C, '\t\t&& real_is_in_range_0054e860(*(const float *)(camera+0x24))', '\t\t|| real_is_in_range_0054e860(*(const float *)(camera+0x24))'),
    (SOURCE_GAME_GAME_UNKNOWN_0016EFB0_C, '\t\t&& valid_world_real_point3d((const real *)(camera+0x10))', '\t\t&& valid_world_real_point3d((const real *)(camera+0x11))'),
    (SOURCE_GAME_GAME_UNKNOWN_0016EFB0_C, '\t\t&& real_is_in_range_0054e858(*(const float *)(camera+0x28))', '\t\t|| real_is_in_range_0054e858(*(const float *)(camera+0x28))'),
]
