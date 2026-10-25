# mutations of milestone 822, written by make_mutations.py

TESTS = "test_small_822"

SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"
SOURCE_RASTERIZER_RASTERIZER_XBOX_C = "source/rasterizer/rasterizer_xbox.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    # taken out: a difference that is no number is not less than 0.001 either, and an index of 0 is made 0 the same
    (SOURCE_MATH_REAL_MATH_C, '\treturn 0.001f>fabsf(difference);', '\treturn 0.001f>=fabsf(difference);'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\tindex= 0;', '\t\tindex= 1;'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\treturn unknown_00487288+(index<<7);', '\treturn unknown_00487288+(index<<8);'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\tindex= 16;', '\t\tindex= 17;'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(unit+0x13C)==NONE)', '\tif (*(const long *)(unit+0x13C)!=NONE)'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(unit+0x13C)==NONE)\n\t{\n\t\treturn TRUE;\n\t}\n\tif (*(const long *)(unit+0x14)==NONE)', '\tif (*(const long *)(unit+0x13C)==NONE)\n\t{\n\t\treturn FALSE;\n\t}\n\tif (*(const long *)(unit+0x14)==NONE)'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(unit+0x14)==NONE)', '\tif (*(const long *)(unit+0x15)==NONE)'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(unit+0x14)==NONE)\n\t{\n\t\treturn FALSE;\n\t}\n', '\tif (*(const long *)(unit+0x14)==NONE)\n\t{\n\t\treturn TRUE;\n\t}\n'),
]
