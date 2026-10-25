# mutations of milestone 813, written by make_mutations.py

TESTS = "test_small_813"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_SCENARIO_SCENARIO_C = "source/scenario/scenario.c"
SOURCE_MAIN_MAIN_C = "source/main/main.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\thavok_unknown_clear(data);\n\thavok_unknown_clear(data+0x20);\n\thavok_unknown_clear(data+0x40);\n}\n', '\thavok_unknown_clear(data);\n\thavok_unknown_clear(data+0x20);\n\thavok_unknown_clear(data+0x41);\n}\n'),
    (SOURCE_PHYSICS_HAVOK_C, '{\n\thavok_unknown_clear(data);\n\thavok_unknown_clear(data+0x20);\n\thavok_unknown_clear(data+0x40);\n}', '{\n\thavok_unknown_clear(data);\n\thavok_unknown_clear(data+0x21);\n\thavok_unknown_clear(data+0x40);\n}'),
    (SOURCE_SCENARIO_SCENARIO_C, '\tif (!tag || *(const long *)(tag+0x04)==NONE || !(tag[0x10]&0x20))', '\tif (!tag || *(const long *)(tag+0x5)==NONE || !(tag[0x10]&0x20))'),
    (SOURCE_SCENARIO_SCENARIO_C, '\tif (*index==NONE && (scenario[0x12]&2) && count>0)', '\tif (*index!=NONE && (scenario[0x12]&2) && count>0)'),
    (SOURCE_SCENARIO_SCENARIO_C, '\tif (cluster_index==NONE)', '\tif (cluster_index==0)'),
    (SOURCE_SCENARIO_SCENARIO_C, '\twhich= (short)*index;\n\tif (which>=0 && which<count)\n\t{', '\twhich= (short)*index;\n\tif (which>=0 || which<count)\n\t{'),
    (SOURCE_MAIN_MAIN_C, '\tif ((long)(game_time_globals->time-*(const long *)(player+0x190))>=game_time_globals->ticks_per_second*10)', '\tif ((long)(game_time_globals->time-*(const long *)(player+0x190))>game_time_globals->ticks_per_second*10)'),
    (SOURCE_MAIN_MAIN_C, '\t\t|| !other[0xC4] || *(const short *)(other+0x28)!=NONE || (other[2]&2))', '\t\t|| !other[0xC4] || *(const short *)(other+0x28)!=NONE || (other[3]&2))'),
    (SOURCE_MAIN_MAIN_C, '\tif (!(*(const unsigned short *)(player+2)&0x1000))', '\tif (!(*(const unsigned short *)(player+3)&0x1000))'),
    (SOURCE_MAIN_MAIN_C, '\t\tplayer[3]&= ~0x10;', '\t\tplayer[3]&= ~0x11;'),
    (SOURCE_UNITS_UNITS_C, '\tif (unit_index==NONE || !UNIT_OBJECT_IS_UNIT(UNIT_OBJECT_GET(unit_index)) || !unit_has_two_weapons_in_hand(unit_index))', '\tif (unit_index==0 || !UNIT_OBJECT_IS_UNIT(UNIT_OBJECT_GET(unit_index)) || !unit_has_two_weapons_in_hand(unit_index))'),
    (SOURCE_UNITS_UNITS_C, '\tlong unit_index= *(const long *)(UNIT_OBJECT_GET(object_index)+0x14);', '\tlong unit_index= *(const long *)(UNIT_OBJECT_GET(object_index)+0x15);'),
    (SOURCE_UNITS_UNITS_C, '\treturn object_index==unit_get_weapon(*(const long *)(UNIT_OBJECT_GET(object_index)+0x14), (signed char)unit[0x213]);', '\treturn object_index!=unit_get_weapon(*(const long *)(UNIT_OBJECT_GET(object_index)+0x14), (signed char)unit[0x213]);'),
    (SOURCE_UNITS_UNITS_C, '\tif (unit_index==NONE || !UNIT_OBJECT_IS_UNIT(UNIT_OBJECT_GET(unit_index)) || !unit_has_two_weapons_in_hand(unit_index))\n\t{\n\t\treturn FALSE;\n\t}\n\tunit= UNIT_OBJECT_GET(*(const long *)(UNIT_OBJECT_GET(object_index)+0x14));', '\tif (unit_index==NONE || !UNIT_OBJECT_IS_UNIT(UNIT_OBJECT_GET(unit_index)) || !unit_has_two_weapons_in_hand(unit_index))\n\t{\n\t\treturn TRUE;\n\t}\n\tunit= UNIT_OBJECT_GET(*(const long *)(UNIT_OBJECT_GET(object_index)+0x14));'),
]
