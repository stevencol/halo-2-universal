# mutations of milestone 863, written by make_mutations.py
# (one taken out by hand: without "sum= 0.0f" the sum starts with what the register had, which the compiler leaves at zero here, so no test can see it)

TESTS = "test_small_863"

SOURCE_MATH_RANDOM_MATH_C = "source/math/random_math.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    (SOURCE_MATH_RANDOM_MATH_C, '\tif (count==1)', '\tif (count==2)'),
    (SOURCE_MATH_RANDOM_MATH_C, '\tif (count==0)', '\tif (count==1)'),
    (SOURCE_MATH_RANDOM_MATH_C, '\t}\n\tfor (index= 0; index<count; index++)\n\t{', '\t}\n\tfor (index= 1; index<count; index++)\n\t{'),
    (SOURCE_UNITS_UNITS_C, '\t\tdata[index+0x6E]= whole>0xFF ? 0xFF : (byte)whole;', '\t\tdata[index+0x6E]= whole>0x100 ? 0xFF : (byte)whole;'),
    (SOURCE_UNITS_UNITS_C, '\tinverse= 1.0f-blend;\n\tfor (index= 0; index<13; index++)\n\t{', '\tinverse= 1.0f-blend;\n\tfor (index= 0; index<13-1; index++)\n\t{'),
    (SOURCE_UNITS_UNITS_C, '\t\t\tvalue= (*(const real *)(data+0x68)-value)*blend + value;', '\t\t\tvalue= (*(const real *)(data+0x69)-value)*blend + value;'),
    (SOURCE_UNITS_UNITS_C, '\tif (!(*(const real *)(data+0x60)>0.0f))\n\t{\n\t\treturn FALSE;\n\t}\n\t*(real *)(data+0x64)+= 1.0f;', '\tif (!(*(const real *)(data+0x60)>0.0f))\n\t{\n\t\treturn TRUE;\n\t}\n\t*(real *)(data+0x64)+= 1.0f;'),
]
