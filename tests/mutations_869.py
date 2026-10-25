# mutations of milestone 869, written by make_mutations.py

TESTS = "test_small_869"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_RASTERIZER_RASTERIZER_XBOX_C = "source/rasterizer/rasterizer_xbox.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\t\t*timer= NONE;\n\t}\n\n\treturn TRUE;\n}\n', '\t\t*timer= NONE;\n\t}\n\n\treturn FALSE;\n}\n'),
    (SOURCE_UNITS_UNITS_C, '\tif (*timer==NONE)\n\t{\n\t\treturn FALSE;\n\t}\n\tif (definition)', '\tif (*timer==NONE)\n\t{\n\t\treturn TRUE;\n\t}\n\tif (definition)'),
    (SOURCE_UNITS_UNITS_C, '\t\tfirst_delay= *(const real *)(definition+0x30);', ''),
    (SOURCE_UNITS_UNITS_C, '\tif ((reset || *first==0.0f) && *second==0.0f)', '\tif ((reset || *first==0.0f) && *second!=0.0f)'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\tif (*(const real *)(fog+0x2C)>=distance)', '\t\tif (*(const real *)(fog+0x2d)>=distance)'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\tlayer= (long *)(fog+0x34);\n\t\tresult= 5;\n\t}', '\t\tlayer= (long *)(fog+0x34);\n\t}'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\treturn *(const real *)(fog+0x58)>0.0f ? 2 : 0;', '\t\treturn *(const real *)(fog+0x59)>0.0f ? 2 : 0;'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tlayer[4]= *(const long *)(fog+0x110);', '\tlayer[4]= *(const long *)(fog+0x111);'),
]
