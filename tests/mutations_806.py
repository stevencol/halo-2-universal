# mutations of milestone 806, written by make_mutations.py

TESTS = "test_small_806"

SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C = "source/networking/network_life_cycle.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\tif (state<=2 || state>8)\n\t{\n\t\treturn FALSE;\n\t}\n\tif (value4C)', '\tif (state<=2 || state>8)\n\t{\n\t\treturn TRUE;\n\t}\n\tif (value4C)'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\tif (state<=2 || state>8)', '\tif (state<2 || state>8)'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\tlong state= *(const long *)(session+0x741C);', '\tlong state= *(const long *)(session+0x741d);'),
    (SOURCE_NETWORKING_NETWORK_LIFE_CYCLE_C, '\t}\n\treturn TRUE;\n}', '\t}\n\treturn FALSE;\n}'),
    (SOURCE_UNITS_UNITS_C, '\t\t&& *(const long *)(data+0x68)!=NONE && *(const long *)data!=NONE && *(const short *)(data+0x06)!=NONE', '\t\t&& *(const long *)(data+0x68)==NONE && *(const long *)data!=NONE && *(const short *)(data+0x06)!=NONE'),
    (SOURCE_UNITS_UNITS_C, '\t\t&& ((signed char)unit[0x1F6]!=NONE || (signed char)unit[0x1F7]!=NONE);', '\t\t&& ((signed char)unit[0x1F6]==NONE || (signed char)unit[0x1F7]!=NONE);'),
    (SOURCE_UNITS_UNITS_C, '\t\t&& ((signed char)unit[0x149]<0 || (unit[0x137]&1))', '\t\t&& ((signed char)unit[0x14a]<0 || (unit[0x137]&1))'),
    (SOURCE_UNITS_UNITS_C, '\treturn !((*(const unsigned long *)(UNIT_DEFINITION_GET(unit)+0xBC)>>11)&1)', '\treturn !((*(const unsigned long *)(UNIT_DEFINITION_GET(unit)+0xbd)>>11)&1)'),
    (SOURCE_UNITS_UNITS_C, '\tcase 0x1D0005FD:', '\tcase 0x1d0005fe:'),
    (SOURCE_UNITS_UNITS_C, '\t\tbreak;\n\t}\n\n\treturn TRUE;\n}\n', '\t\tbreak;\n\t}\n\n\treturn FALSE;\n}\n'),
    (SOURCE_UNITS_UNITS_C, '\tcase 0x180005FC:', '\tcase 0x180005fd:'),
    (SOURCE_UNITS_UNITS_C, '\tcase 0x0D0005FF:', '\tcase 0xd000600:'),
]
