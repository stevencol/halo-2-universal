# mutations of milestone 845, written by make_mutations.py

TESTS = "test_small_845"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"
SOURCE_INPUT_INPUT_ABSTRACTION_C = "source/input/input_abstraction.c"
SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\tresource= (struct cached_animation *)(*(byte *const *)(graph+0xB0)+*(const long *)(animation+0x24)*0x14);', '\tresource= (struct cached_animation *)(*(byte *const *)(graph+0xb1)+*(const long *)(animation+0x24)*0x14);'),
    # taken out: the branch to NULL only leads to reading through it, which the compiler may take away; the same code either way
    (SOURCE_PHYSICS_HAVOK_C, '\tif (!(animation[0x16]&0x10))', '\tif (!(animation[0x16]&0x11))'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tanimation_cache_predict(resource, TRUE);', '\t\tanimation_cache_predict(resource, FALSE);'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tanimation_cache_predict(resource, FALSE);', '\t\t\tanimation_cache_predict(resource, TRUE);'),
    (SOURCE_PHYSICS_HAVOK_C, '\tfor (index= 0; index<*(const long *)(graph+0xAC); index++)', '\tfor (index= 0; index<*(const long *)(graph+0xAC)-1; index++)'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tif (((const byte *)resource)[0x10])', '\t\tif (((const byte *)resource)[0x11])'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tstruct cached_animation *resource= (struct cached_animation *)(*(byte *const *)(graph+0xB0)+index*0x14);', '\t\tstruct cached_animation *resource= (struct cached_animation *)(*(byte *const *)(graph+0xB0)+index*0x15);'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (state[0x67]&2)', '\tif (state[0x67]&3)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tboolean result= unit_counter_update((signed char *)(state+0x64));', '\tboolean result= unit_counter_update((signed char *)(state+0x65));'),
    (SOURCE_PHYSICS_HAVOK_C, '\tresult|= unit_counter_update((signed char *)(state+0x60));', '\tresult|= unit_counter_update((signed char *)(state+0x61));'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tstate[0x65]= 0;', '\t\tstate[0x65]= 1;'),
    (SOURCE_MATH_REAL_MATH_C, '\treturn type==4 || type==5;', '\treturn type==4 || type!=5;'),
    (SOURCE_INPUT_INPUT_ABSTRACTION_C, '\t\tunknown_004e9264= 0;', '\t\tunknown_004e9264= 1;'),
    (SOURCE_INPUT_INPUT_ABSTRACTION_C, '\tif (unknown_004e9188 && unknown_004e925c)', '\tif (unknown_004e9188 || unknown_004e925c)'),
    (SOURCE_INPUT_INPUT_ABSTRACTION_C, '\t\tunknown_004e91a0= NULL;', ''),
    (SOURCE_INPUT_INPUT_ABSTRACTION_C, '\t\tunknown_004e925c= NULL;', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlong index= *(const long *)(system+0x30);\n\n\twhile (index!=NONE)\n\t{\n\t\tlong next= *(const long *)((const byte *)particle_location_data->data+(index&0xFFFF)*0x34+0x0C);', '\tlong index= *(const long *)(system+0x30);\n\n\twhile (index!=0)\n\t{\n\t\tlong next= *(const long *)((const byte *)particle_location_data->data+(index&0xFFFF)*0x34+0x0C);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tlong index= *(const long *)(system+0x30);', '\tlong index= *(const long *)(system+0x31);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tlong next= *(const long *)((const byte *)particle_location_data->data+(index&0xFFFF)*0x34+0x0C);', '\t\tlong next= *(const long *)((const byte *)particle_location_data->data+(index&0xFFFF)*0x35+0x0C);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tparticle_location_delete(index);\n\t\tindex= next;\n\t}', '\t\tparticle_location_delete(index);\n\t}'),
]
