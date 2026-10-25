# mutations of milestone 872, written by make_mutations.py

TESTS = "test_small_872"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C = "source/game/game_unknown_0003f2b0.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tset= frame>=lower && upper>frame;', '\t\t\tset= frame>=lower && upper>=frame;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tposition= *(const real *)(channel+0x1C);', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tset= frame>lower && upper>=frame;', '\t\t\tset= frame>lower && upper>frame;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tanimation= havok_unknown_001daea0(\n\t\t(const byte *)(uintptr_t)global_tag_instances[*(const long *)channel&0xFFFF].address,\n\t\t*(const unsigned long *)(channel+0x04));', '\tanimation= havok_unknown_001daea0(\n\t\t(const byte *)(uintptr_t)global_tag_instances[*(const long *)channel&0x10000].address,\n\t\t*(const unsigned long *)(channel+0x04));'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\t\t\t\t+*(const short *)(variant+0x04+level*2)*0x5C+0x38);', '\t\t\t\t\t+*(const short *)(variant+0x5+level*2)*0x5C+0x38);'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\tconst byte *group= *(const byte *const *)(definition+0x20)+index*0x10;', '\t\tconst byte *group= *(const byte *const *)(definition+0x21)+index*0x10;'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\t{\n\t\t\tsections[index]= NONE;\n\t\t}', '\t\t{\n\t\t\tsections[index]= 0;\n\t\t}'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\tif (selection<*(const long *)(group+0x08))', '\t\tif (selection<=*(const long *)(group+0x08))'),
]
