# mutations of milestone 842, written by make_mutations.py

TESTS = "test_small_842"

SOURCE_GAME_GAME_ENGINE_C = "source/game/game_engine.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tif (object_index==NONE)', '\t\tif (object_index==0)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tshort kept= 0;', '\tshort kept= 1;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tobject= (const byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;', '\t\tobject= (const byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0x10000].datum;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tif (*(const long *)(definition+0x74)<=0)', '\t\tif (*(const long *)(definition+0x74)<0)'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tsecond= 3;', '\t\tsecond= 4;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (unknown_0047989c && *(const long *)((const byte *)unknown_0047989c+0x20)>0)', '\tif (unknown_0047989c && *(const long *)((const byte *)unknown_0047989c+0x20)>=0)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (unknown_004798a0 && *(const long *)((const byte *)unknown_004798a0+0x20)>0)', '\tif (unknown_004798a0 || *(const long *)((const byte *)unknown_004798a0+0x20)>0)'),
    # taken out: exactly 0.85 is not a value the test can make
]
