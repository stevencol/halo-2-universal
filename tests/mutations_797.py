# mutations of milestone 797, written by make_mutations.py

TESTS = "test_small_797"

SOURCE_SCENARIO_SCENARIO_C = "source/scenario/scenario.c"
SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"

MUTATIONS = [
    (SOURCE_SCENARIO_SCENARIO_C, '\tlong longs= (clusters+0x1F)>>5;', '\tlong longs= (clusters+0x20)>>5;'),
    (SOURCE_SCENARIO_SCENARIO_C, '\treturn *(unsigned long *const *)(bsp+0x58)+((clusters+0x1F)>>5)*row;', '\treturn *(unsigned long *const *)(bsp+0x58)+((clusters+0x20)>>5)*row;'),
    (SOURCE_SCENARIO_SCENARIO_C, '\tif ((unsigned long)(square*4)<*(const unsigned long *)(bsp+0x54))', '\tif ((unsigned long)(square*4)<*(const unsigned long *)(bsp+0x55))'),
    (SOURCE_SCENARIO_SCENARIO_C, '\tlong clusters= *(const long *)(bsp+0x9C);', '\tlong clusters= *(const long *)(bsp+0x9d);'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\tconst byte *entry= (const byte *)(uintptr_t)block[1]+which*0x48;', '\t\tconst byte *entry= (const byte *)(uintptr_t)block[2]+which*0x48;'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\tif (first<=index && first+*(const unsigned short *)(entry+0x08)>index)', '\t\tif (first<=index && first+*(const unsigned short *)(entry+0x08)>=index)'),
    (SOURCE_SCENARIO_SCENARIO_C, '\tfor (which= 0; which<block[0]; which++)', '\tfor (which= 1; which<block[0]; which++)'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\tlong first= *(const unsigned short *)(entry+0x06);', '\t\tlong first= *(const unsigned short *)(entry+0x7);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t((short *)(data+0x440))[value]= NONE;', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\t((short *)(data+0x440))[value]= ((const short *)(data+0x40))[index];\n\t((short *)(data+0x40))[index]= value;\n}', '\t((short *)(data+0x440))[value]= ((const short *)(data+0x40))[index];\n\t((short *)(data+0x41))[index]= value;\n}'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t((short *)(data+0x440))[value]= ((const short *)(data+0x40))[index];', '\t((short *)(data+0x441))[value]= ((const short *)(data+0x40))[index];'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t((short *)(data+0x40))[index]= value;', '\t\t((short *)(data+0x41))[index]= value;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tdata[3]= down[0];', '\tdata[4]= down[0];'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tdata[10]= up[0];', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\tdata[8]= point[1];', '\tdata[8]= point[2];'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tdata[11]= up[1];', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t((unsigned long *)(data+0x3C))[0]= vector[0];', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\t((unsigned long *)(data+0x24))[2]= color[2];', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t((unsigned long *)(data+0x3C))[1]= vector[1];', '\t\t((unsigned long *)(data+0x3d))[1]= vector[1];'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t((unsigned long *)(data+0x3C))[2]= vector[2];', '\t\t((unsigned long *)(data+0x3C))[3]= vector[2];'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tvalues[index]= value;', ''),
    # taken out: one long more is written past the buffer of the test, which is not compared
]
