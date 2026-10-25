# mutations of milestone 825, written by make_mutations.py

TESTS = "test_small_825"

SOURCE_MAIN_MAIN_C = "source/main/main.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_MAIN_MAIN_GAME_C = "source/main/main_game.c"

MUTATIONS = [
    (SOURCE_MAIN_MAIN_C, '\tfor (index= 0; index<8; index++)', '\tfor (index= 1; index<8; index++)'),
    (SOURCE_MAIN_MAIN_C, '\t\tunknown_00470a3c[index]= index+1;', '\t\tunknown_00470a3c[index]= index+2;'),
    (SOURCE_MAIN_MAIN_C, '\tboolean result= (player[2]>>5)&1 ? *(const short *)(data+0x10)!=1 : *(const short *)(data+0x10)!=2;', '\tboolean result= (player[3]>>5)&1 ? *(const short *)(data+0x10)!=1 : *(const short *)(data+0x10)!=2;'),
    (SOURCE_MAIN_MAIN_C, '\tif (*(const short *)((const byte *)players_globals+0x08)!=1)', '\tif (*(const short *)((const byte *)players_globals+0x08)==1)'),
    (SOURCE_MAIN_MAIN_C, '\tif (kind==1 && number!=1)', '\tif (kind==1 && number==1)'),
    (SOURCE_MAIN_MAIN_C, '\t\treturn FALSE;\n\t}\n\n\treturn TRUE;\n}\n', '\t\treturn FALSE;\n\t}\n\n\treturn FALSE;\n}\n'),
    (SOURCE_OBJECTS_OBJECTS_C, '\treturn *(const long *)(definition+0x04)!=NONE;', '\treturn *(const long *)(definition+0x04)!=0;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tdefinition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)OBJECT_DATA(object_index)&0xFFFF].address;', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tdefinition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)OBJECT_DATA(object_index)&0xFFFF].address;\n\tdefinition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(definition+0x38)&0xFFFF].address;\n', '\tdefinition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)OBJECT_DATA(object_index)&0xFFFF].address;\n\tdefinition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(definition+0x38)&0x10000].address;\n'),
    (SOURCE_PHYSICS_HAVOK_C, '\ttag= *(const long *)(*(const byte *const *)(definition+0x50)+index*0x20+0x04);', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tconst byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)reference&0xFFFF].address;', '\tconst byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)reference&0x10000].address;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (index>=*(const long *)(definition+0x4C))', '\tif (index>*(const long *)(definition+0x4C))'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (tag==NONE)', '\tif (tag!=NONE)'),
    (SOURCE_PHYSICS_HAVOK_C, '\tlong double k= vector[2];', '\tlong double k= vector[3];'),
    (SOURCE_PHYSICS_HAVOK_C, '\tconst real *vector= (const real *)(data+0x28);', '\tconst real *vector= (const real *)(data+0x29);'),
    (SOURCE_PHYSICS_HAVOK_C, '\tlong double i= vector[0];', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tlong double j= vector[1];', '\tlong double j= vector[2];'),
    (SOURCE_MAIN_MAIN_GAME_C, '\t\t((byte *)unknown_00502104->data+(*(const long *)(thing+0x08)&0xFFFF)*0x10)[4]--;', '\t\t((byte *)unknown_00502104->data+(*(const long *)(thing+0x08)&0xFFFF)*0x11)[4]--;'),
    (SOURCE_MAIN_MAIN_GAME_C, '\twhile ((signed char)(voice[2]<<5)>0)', '\twhile ((signed char)(data[3]<<5)>0)'),
    (SOURCE_MAIN_MAIN_GAME_C, '\t\tvoice[2]= (byte)(((voice[2]&~7)|((count-1)&7)));', '\t\tdata[2]= (byte)(((data[3]&~7)|((count-1)&7)));'),
    (SOURCE_MAIN_MAIN_GAME_C, '\t\tconst byte *thing= *(const byte *const *)(voice+0x18+count*4);', '\t\tconst byte *thing= *(const byte *const *)(data+0x19+count*4);'),
]
