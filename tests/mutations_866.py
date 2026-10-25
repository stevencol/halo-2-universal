# mutations of milestone 866, written by make_mutations.py

TESTS = "test_small_866"

SOURCE_EFFECTS_PARTICLE_SYSTEMS_C = "source/effects/particle_systems.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"

MUTATIONS = [
    (SOURCE_EFFECTS_PARTICLE_SYSTEMS_C, '\treturn particle_unknown_00246cd0(definition+0x08, values)*30.0f;', '\treturn particle_unknown_00246cd0(definition+0x9, values)*30.0f;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t(const byte *)(uintptr_t)global_tag_instances[*(const long *)(state+0x68)&0xFFFF].address, id, name);', '\t\t(const byte *)(uintptr_t)global_tag_instances[*(const long *)(state+0x68)&0x10000].address, id, name);'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\theader= (const byte *)data->data+data->size*index;', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tif (*(const short *)(header+0x04)==NONE || !((1UL<<(header[0x03]&31))&0x40) || (header[0x02]&0x10))', '\t\tif (*(const short *)(header+0x04)==NONE && !((1UL<<(header[0x03]&31))&0x40) || (header[0x02]&0x10))'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tif (scenario_index==NONE)', '\t\tif (scenario_index==0)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tfor (index= 0; index<data->first_unallocated; index++)', '\tfor (index= 0; index<data->first_unallocated-1; index++)'),
    (SOURCE_UNITS_UNITS_C, '\tmemset(control+0x5C, 0, 0x20);', '\tmemset(control+0x5C, 0, 0x21);'),
    (SOURCE_UNITS_UNITS_C, '\thavok_unknown_001dd0b0(UNIT_ANIMATION_GRAPH(unit), &id, 0x0800004D);', '\thavok_unknown_001dd0b0(UNIT_ANIMATION_GRAPH(unit), &id, 0x800004e);'),
    (SOURCE_UNITS_UNITS_C, '\tmemset(control+0x30, 0, 0x2C);', '\tmemset(control+0x31, 0, 0x2C);'),
    (SOURCE_UNITS_UNITS_C, '\thavok_unknown_001dd0b0(UNIT_ANIMATION_GRAPH(unit), &id, 0x08000145);', '\thavok_unknown_001dd0b0(UNIT_ANIMATION_GRAPH(unit), &id, 0x8000146);'),
    (SOURCE_UNITS_UNITS_C, '\thavok_unknown_001dd0b0(UNIT_ANIMATION_GRAPH(unit), &id, 0x0C00024D);', '\thavok_unknown_001dd0b0(UNIT_ANIMATION_GRAPH(unit), &id, 0xc00024e);'),
    (SOURCE_MATH_REAL_MATH_C, '\thsv[0]= first_hsv[0]*inverse + second_hsv[0]*t;', '\thsv[1]= first_hsv[0]*inverse + second_hsv[0]*t;'),
    (SOURCE_MATH_REAL_MATH_C, '\thsv[1]= first_hsv[1]*inverse + second_hsv[1]*t;', '\thsv[1]= first_hsv[2]*inverse + second_hsv[1]*t;'),
    (SOURCE_MATH_REAL_MATH_C, '\tif ((fabsl((long double)first_hsv[0]-second_hsv[0])>0.5L)!=((flags>>1)&1))', '\tif ((fabsl((long double)first_hsv[0]-second_hsv[0])>0.5L)!=((flags>>1)&2))'),
    (SOURCE_MATH_REAL_MATH_C, '\t\tresult[1]= first[1]*inverse + second[1]*t;', '\t\tresult[2]= first[1]*inverse + second[1]*t;'),
]
