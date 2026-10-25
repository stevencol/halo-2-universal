# mutations of milestone 874, written by make_mutations.py

TESTS = "test_small_874"

SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\tif (((flags>>26)&1) || ((flags>>27)&1))', '\tif (((flags>>26)&1) && ((flags>>27)&1))'),
    (SOURCE_UNITS_UNITS_C, '\t\tsigned char weapon= (signed char)unit[0x212+index];', ''),
    (SOURCE_UNITS_UNITS_C, '\tstatic const long bits[]= { 0x16, 8, 0x12, 0, 1, 0x0A, 0x0B };', ''),
    (SOURCE_UNITS_UNITS_C, '\t\tif (weapon!=NONE && *(const long *)(unit+0x218+weapon*4)!=NONE', '\t\tif (weapon==NONE && *(const long *)(unit+0x218+weapon*4)!=NONE'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (other_index==NONE || other_index==weapon_index)', '\t\tif (other_index==0 || other_index==weapon_index)'),
    (SOURCE_UNITS_UNITS_C, '\t\tlong other_index= *(const long *)(UNIT_OBJECT_GET(*(const long *)(weapon+0x154))+0x218+index*4);', '\t\tlong other_index= *(const long *)(UNIT_OBJECT_GET(*(const long *)(weapon+0x154))+0x218+index*5);'),
    (SOURCE_UNITS_UNITS_C, '\t\t\trounds+= weapon_magazine_rounds(other, magazine_index, infinite);', ''),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(weapon+0x154)==NONE || weapon_only)', '\tif (*(const long *)(weapon+0x154)==NONE && weapon_only)'),
]
