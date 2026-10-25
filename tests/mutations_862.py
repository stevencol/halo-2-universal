# mutations of milestone 862, written by make_mutations.py
# (three taken out by hand that change nothing: a movement of four reals, and >= instead of > for the maximum and the minimum, which only picks an equal value or -0 for +0)

TESTS = "test_small_862"

SOURCE_RASTERIZER_RASTERIZER_XBOX_C = "source/rasterizer/rasterizer_xbox.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"

MUTATIONS = [
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\t\t\tfor (index= 0; index<count; index++)', '\t\t\t\tfor (index= 1; index<count; index++)'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tmemset(unknown_00487288, 0, sizeof(unknown_00487288));', '\tmemset(unknown_00487288, 1, sizeof(unknown_00487288));'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\t\t\tfor (rows= 8; rows>0; rows--, base+= 128)', '\t\t\tfor (rows= 8; rows>0; rows--, base+= 129)'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tfor (count= 0; count<=16; count++)', '\tfor (count= 0; count<16; count++)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tscale= *(const real *)(channel+0x18)*30.0f;', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tframe= (long)*(const real *)(channel+0x1C);', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tvector[2]= movement[2]*scale;', '\tvector[2]= movement[3]*scale;'),
    (SOURCE_MATH_REAL_MATH_C, '\telse if (rgb[1]==maximum)', '\telse if (rgb[1]!=maximum)'),
    (SOURCE_MATH_REAL_MATH_C, '\t\thsv[0]= (rgb[1]-rgb[2])/delta;', '\t\thsv[0]= (rgb[2]-rgb[2])/delta;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (source!=1 && source!=0)', '\tif (source!=2 && source!=0)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (*(const short *)(definition+0x0A)==NONE)', '\tif (*(const short *)(definition+0x0A)==0)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tsize= *(const short *)(definition+0x0E);\n\tblock= (const byte *)global_scenario+*(const short *)(definition+0x0A);\n\tfor (index= 0; index<*(const long *)block; index++)', '\tsize= *(const short *)(definition+0x0E);\n\tfor (index= 0; index<*(const long *)block; index++)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tif (*(const long *)current==*(const long *)identifier && current[6]==(byte)type && current[7]==(byte)source', '\t\tif (*(const long *)current==*(const long *)identifier || current[6]==(byte)type && current[7]==(byte)source'),
]
