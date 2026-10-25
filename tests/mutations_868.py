# mutations of milestone 868, written by make_mutations.py
# (two taken out by hand that change nothing: the index a datum starts with, as object_unknown_000b7a40 always writes it when it finds the datum)

TESTS = "test_small_868"

SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    (SOURCE_OBJECTS_OBJECTS_C, '\textra_count= *(const short *)((const byte *)global_structure_bsp+0x140);', '\textra_count= *(const short *)((const byte *)global_structure_bsp+0x141);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tobject_iterator_new(&iterator, 0x80, 0);', '\tobject_iterator_new(&iterator, 0x80, 1);'),
    (SOURCE_UNITS_UNITS_C, '\t\tsound_index= *(const long *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(weapon_index))+0x278);', ''),
    (SOURCE_UNITS_UNITS_C, '\t\tif (weapon_index!=NONE)', '\t\tif (weapon_index==NONE)'),
    (SOURCE_UNITS_UNITS_C, '\tif (as_unit && *(const long *)(as_unit+0x13C)!=NONE)', '\tif (as_unit || *(const long *)(as_unit+0x13C)!=NONE)'),
    (SOURCE_UNITS_UNITS_C, '\t\tunit[0x241]= 0xFF;', '\t\tunit[0x242]= 0xFF;'),
]
