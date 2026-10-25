# mutations of milestone 870, written by make_mutations.py
# (two taken out by hand that change nothing: a marker buffer one byte longer, and an eighth pair at 0xFC, which is the first of the three at 0x118, NONE as well)

TESTS = "test_small_870"

SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\tobject_get_origin(unit_index, (real_point3d *)(matrix+10));', '\tobject_get_origin(unit_index, (real_point3d *)(matrix+11));'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (!unknown_000b8d30(FALSE, marker, parent_index, *(const long *)(seat+0x08), 1))', '\t\tif (!unknown_000b8d30(TRUE, marker, parent_index, *(const long *)(seat+0x08), 1))'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (!unknown_000b8d30(FALSE, marker, parent_index, *(const long *)(seat+0x08), 1))\n\t\t{\n\t\t\treturn FALSE;\n\t\t}\n\t\tmemcpy(matrix, marker+0x38, 13*sizeof(real));', '\t\tif (!unknown_000b8d30(FALSE, marker, parent_index, *(const long *)(seat+0x08), 1))\n\t\t{\n\t\t\treturn TRUE;\n\t\t}\n\t\tmemcpy(matrix, marker+0x38, 13*sizeof(real));'),
    (SOURCE_UNITS_UNITS_C, '\tstate[0x00]&= ~2;', ''),
    (SOURCE_UNITS_UNITS_C, '\t}\n\tfor (index= 0; index<3; index++)\n\t{', '\t}\n\tfor (index= 0; index<3-1; index++)\n\t{'),
    (SOURCE_UNITS_UNITS_C, '\tstate[0x03]= 0;', '\tstate[0x4]= 0;'),
]
