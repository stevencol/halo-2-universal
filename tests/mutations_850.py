# mutations of milestone 850, written by make_mutations.py

TESTS = "test_small_850"

SOURCE_CSERIES_UNKNOWN_0051F40C_C = "source/cseries/unknown_0051f40c.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_INPUT_INPUT_XBOX_C = "source/input/input_xbox.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"

MUTATIONS = [
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\t}\n\tfor (index= 0; index<unknown_005233ec; index++)\n\t{', '\t}\n\tfor (index= 0; index<unknown_005233ec-1; index++)\n\t{'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\tif (player_index==NONE)', '\tif (player_index==0)'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\tif (unknown_0051f408 && !unknown_004850c8)', '\tif (unknown_0051f408 || !unknown_004850c8)'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\t\tunknown_005234c4= player_index;', ''),
    # taken out: for NONE the game reads past the table of tags, where the test has nothing (NULL either way)
    (SOURCE_PHYSICS_HAVOK_C, '\t\tlong tag= *(const long *)(*(const byte *const *)(graph+0x50)+index*0x20+0x04);', '\t\tlong tag= *(const long *)(*(const byte *const *)(graph+0x50)+index*0x20+0x5);'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tinherited= (const byte *)(uintptr_t)global_tag_instances[tag&0xFFFF].address;', '\t\t\tinherited= (const byte *)(uintptr_t)global_tag_instances[tag&0x10000].address;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tfor (index= 0; index<*(const long *)(graph+0x4C); index++)', '\tfor (index= 0; index<*(const long *)(graph+0x4d); index++)'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tcomponent_index= pair[index*2+1];', '\t\t\tcomponent_index= pair[index*3+1];'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tif (pair[index*2]==0x2001)', '\t\tif (pair[index*2]==0x2002)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tlong count= *(const long *)(unknown+0x34);', '\tlong count= *(const long *)(unknown+0x35);'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif ((*(const unsigned long *)(component+0x04)>>5)&1)', '\tif ((*(const unsigned long *)(component+0x5)>>5)&1)'),
    (SOURCE_INPUT_INPUT_XBOX_C, '\t\t\tif (state[0x10+index])', '\t\t\tif (state[0x11+index])'),
    (SOURCE_INPUT_INPUT_XBOX_C, '\t\t\t\tresult= TRUE;', ''),
    (SOURCE_INPUT_INPUT_XBOX_C, '\tboolean result= FALSE;', '\tboolean result= TRUE;'),
    (SOURCE_INPUT_INPUT_XBOX_C, '\tfor (slot= 0; slot<4; slot++)', '\tfor (slot= 1; slot<4; slot++)'),
    # taken out: the byte past the 12 is set after
    (SOURCE_OBJECTS_OBJECTS_C, '\tif ((*(const unsigned long *)(object+0x04)>>9)&1)', '\tif ((*(const unsigned long *)(object+0x04)>>10)&1)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tshort flags= 0;', '\tshort flags= 1;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tflags= (short)object_unknown_0016b430(object_index);', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tif (indices[index]!=0xFF && !geometry_cache_block_load(*(byte *const *)(data+0x28)+indices[index]*0x5C+0x38, 3))', '\t\tif (indices[index]!=0xFF && !geometry_cache_block_load(*(byte *const *)(data+0x28)+indices[index]*0x5C+0x39, 3))'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\t\tresult= FALSE;', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tconst byte *data= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;', '\tconst byte *data= (const byte *)(uintptr_t)global_tag_instances[tag_index&0x10000].address;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tboolean result= TRUE;', '\tboolean result= FALSE;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (*(const short *)(object+0x112)==NONE || *(const long *)((const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address+0x38)==NONE)', '\tif (*(const short *)(object+0x112)==NONE && *(const long *)((const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address+0x38)==NONE)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tobject+*(const short *)(object+0x10E), object+*(const short *)(object+0x112));', '\t\tobject+*(const short *)(object+0x10f), object+*(const short *)(object+0x112));'),
    (SOURCE_OBJECTS_OBJECTS_C, '\thavok_unknown_001cb5f0((long)((unsigned long)(long)*(const short *)(object+0x110)>>5), object+*(const short *)(object+0x12A), seconds,', '\thavok_unknown_001cb5f0((long)((unsigned long)(long)*(const short *)(object+0x110)>>5), object+*(const short *)(object+0x12b), seconds,'),
]
