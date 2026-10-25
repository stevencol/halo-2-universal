# mutations of milestone 843, written by make_mutations.py

TESTS = "test_small_843"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\tfound= havok_unknown_001db120(data, first, second, third);', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tfor (index= 0; index<*(const long *)(found+0x24); index++)', '\t\tfor (index= 0; index<*(const long *)(found+0x25); index++)'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tstruct cached_animation *animation= (struct cached_animation *)(*(byte *const *)(data+0xB0)+(*(const long *const *)(found+0x30))[index]*0x14);', '\t\t\tstruct cached_animation *animation= (struct cached_animation *)(*(byte *const *)(data+0xB0)+(*(const long *const *)(found+0x30))[index]*0x15);'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\t\tanimation_cache_predict(animation, TRUE);', '\t\t\t\tanimation_cache_predict(animation, FALSE);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tstruct object_header_datum *header= &((struct object_header_datum *)object_header_data->data)[object_index&0xFFFF];', '\tstruct object_header_datum *header= &((struct object_header_datum *)object_header_data->data)[object_index&0x10000];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\told_size= (unsigned short)header->data_size;', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tmask= (1UL<<alignment_bits)-1;', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\t*(short *)reference= 0;\n\t\treturn TRUE;\n\t}', '\t\t*(short *)reference= 0;\n\t\treturn FALSE;\n\t}'),
]
