# mutations of milestone 816, written by make_mutations.py

TESTS = "test_small_816"

SOURCE_GAME_GAME_ENGINE_C = "source/game/game_engine.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t\t\tlist->longsC[list->count]= first[index];\n\t\t\t\tlist->shorts8[list->count]= 0;\n\t\t\t\tlist->longs10[list->count]= 0;\n\t\t\t\tlist->shorts14[list->count]= NONE;\n\t\t\t\tlist->count++;', '\t\t\t\tlist->longsC[list->count]= first[index];\n\t\t\t\tlist->shorts8[list->count]= 0;\n\t\t\t\tlist->shorts14[list->count]= NONE;\n\t\t\t\tlist->count++;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t{\n\t\tlist= *(struct game_engine_list **)(lists+0x14);\n\t\tif (!keep)', '\t{\n\t\tlist= *(struct game_engine_list **)(lists+0x15);\n\t\tif (!keep)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t\t\tlist->longsC[list->count]= second[index];\n\t\t\t\tlist->shorts8[list->count]= 0;\n\t\t\t\tlist->longs10[list->count]= 0;\n\t\t\t\tlist->shorts14[list->count]= NONE;\n\t\t\t\tlist->count++;', '\t\t\t\tlist->longsC[list->count]= second[index];\n\t\t\t\tlist->shorts8[list->count]= 0;\n\t\t\t\tlist->longs10[list->count]= 1;\n\t\t\t\tlist->shorts14[list->count]= NONE;\n\t\t\t\tlist->count++;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tfor (index= 0; index<first_count; index++)', '\t\tfor (index= 1; index<first_count; index++)'),
    (SOURCE_UNITS_UNITS_C, '\t\tunsigned long flags= *(const unsigned long *)(*(const byte *const *)(definition+0x1CC)+*(const short *)(unit+0x1FC)*0xB0);', '\t\tunsigned long flags= *(const unsigned long *)(*(const byte *const *)(definition+0x1CC)+*(const short *)(unit+0x1fd)*0xB0);'),
    (SOURCE_UNITS_UNITS_C, '\tshort result= 0;', '\tshort result= 1;'),
    (SOURCE_UNITS_UNITS_C, '\t\tif ((flags>>4)&1)', '\t\tif ((flags>>5)&1)'),
    (SOURCE_UNITS_UNITS_C, '\tif ((1UL<<(parent[0xAA]&0x1F))&3)', '\tif ((1UL<<(parent[0xAA]&0x1F))&4)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tmaximum= *(const unsigned long *)(definition+0x48);', '\t\tmaximum= *(const unsigned long *)(definition+0x49);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tif (maximum>0x40)', '\t\tif (maximum>0x41)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tconst byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address;', '\t\tconst byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0x10000].address;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tbyte *entry= unknown_004c1bd8+index*0xD0C;', '\t\tbyte *entry= unknown_004c1bd8+index*0xd0d;'),
]
