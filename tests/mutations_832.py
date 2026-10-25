# mutations of milestone 832, written by make_mutations.py

TESTS = "test_small_832"

SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_CSERIES_CSERIES_C = "source/cseries/cseries.c"
SOURCE_EFFECTS_PARTICLE_SYSTEMS_C = "source/effects/particle_systems.c"

MUTATIONS = [
    (SOURCE_MATH_REAL_MATH_C, '\treturn real_math_unknown_000a0200(up[2]*forward[2] + up[1]*forward[1] + forward[0]*up[0], 0.0f) ? 1 : 0;', '\treturn real_math_unknown_000a0200(up[2]*forward[2] + up[1]*forward[1] + forward[0]*up[1], 0.0f) ? 1 : 0;'),
    (SOURCE_MATH_REAL_MATH_C, '\treal difference= forward[0]*forward[0]+forward[1]*forward[1]+forward[2]*forward[2]-1.0f;', '\treal difference= forward[1]*forward[0]+forward[1]*forward[1]+forward[2]*forward[2]-1.0f;'),
    # taken out: exactly 0.001 is not a square of a vector the test makes; and a pair only gives 0.267 when the first is none of the three of 0.1335, so it changes nothing
    (SOURCE_MATH_REAL_MATH_C, '\tif ((bits&0x7F800000)==0x7F800000 || !(0.001f>fabsf(difference)))\n\t{\n\t\treturn 0;\n\t}\n\tif (!real_math_unknown_000a0190(up))', '\tif ((bits&0x7F800000)==0x7F800000 || !(0.001f>fabsf(difference)))\n\t{\n\t\treturn 1;\n\t}\n\tif (!real_math_unknown_000a0190(up))'),
    (SOURCE_UNITS_UNITS_C, '\tif (first==0x09000020 || first==0x0900001F || first==0x090006B2)', '\tif (first==0x09000020 || first==0x0900001F && first==0x090006B2)'),
    (SOURCE_CSERIES_CSERIES_C, '\tfor (index= 0; index<9; index++)', '\tfor (index= 0; index<9-1; index++)'),
    (SOURCE_CSERIES_CSERIES_C, '\t\tif (codes[index].language==language)', '\t\tif (codes[index].language!=language)'),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\tmemcpy(entry+0x0C, data+0xA0, 16);', '\tmemcpy(entry+0x0C, data+0xa1, 16);'),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\tmemcpy(table+0x6C08+first*8, data+0x19C, (unsigned long)count*8);', '\tmemcpy(table+0x6C08+first*9, data+0x19C, (unsigned long)count*8);'),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\tbyte *entry= table+4+index*0x6C;', '\tbyte *entry= table+4+index*0x6d;'),
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\tmemset(entry+0x2C, 0, 0x40);', '\tmemset(entry+0x2d, 0, 0x40);'),
]
