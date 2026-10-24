# mutations of milestone 796, written by make_mutations.py

TESTS = "test_small_796"

SOURCE_GAME_GAME_ENGINE_C = "source/game/game_engine.c"
SOURCE_CACHE_TAG_UNKNOWN_0016D180_C = "source/cache/tag_unknown_0016d180.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_ENGINE_C, '\tif (list->count>=list->maximum_count-1)', '\tif (list->count>list->maximum_count-1)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif (list->count>=list->maximum_count-1)\n\t{\n\t\treturn FALSE;\n\t}\n\tlist->longsC[list->count]= valueC;', '\tif (list->count>=list->maximum_count-1)\n\t{\n\t\treturn TRUE;\n\t}\n\tlist->longsC[list->count]= valueC;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tlist->shorts14[list->count]= value14;', ''),
    (SOURCE_GAME_GAME_ENGINE_C, '\tlist->longs10[list->count]= value10;', ''),
    (SOURCE_GAME_GAME_ENGINE_C, '\tfor (index= 0; index<list->count; index++)', '\tfor (index= 0; index<list->count-1; index++)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tif (list->longsC[index]==valueC)', '\t\tif (list->longsC[index]!=valueC)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\t}\n\t}\n\n\treturn NONE;\n}\n\n/* 0x00163d50: two of a list change places */', '\t\t}\n\t}\n\n\treturn 0;\n}\n\n/* 0x00163d50: two of a list change places */'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tother= list->shorts8[first];', ''),
    (SOURCE_GAME_GAME_ENGINE_C, '\tlist->longsC[first]= list->longsC[second];', ''),
    (SOURCE_GAME_GAME_ENGINE_C, '\tlist->longsC[second]= value;', ''),
    (SOURCE_GAME_GAME_ENGINE_C, '\tlist->shorts8[second]= other;', ''),
    (SOURCE_GAME_GAME_ENGINE_C, '\tdata[0]= 0;', '\tdata[1]= 0;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tdata[1]= 0;', ''),
    (SOURCE_GAME_GAME_ENGINE_C, '\tbyte *data= unknown_004e9bc8+index*0x20CC+part*0x1010+0x78;', '\tbyte *data= unknown_004e9bc8+index*0x20cd+part*0x1010+0x78;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tdata[3]= 0;', '\tdata[4]= 0;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\treturn (flags>>23)&0x3F;', '\treturn (flags>>24)&0x3F;'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\tlong name)\n{\n\tconst byte *tag;\n\tlong index;\n\n\tif (tag_index==NONE)\n\t{\n\t\treturn NONE;\n\t}\n\ttag= TAG_ADDRESS(tag_index);\n\tif (!name)', '\tlong name)\n{\n\tconst byte *tag;\n\tlong index;\n\n\tif (tag_index!=NONE)\n\t{\n\t\treturn NONE;\n\t}\n\ttag= TAG_ADDRESS(tag_index);\n\tif (!name)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\tfor (index= 0; index<*(const long *)(tag+0x50); index++)', '\tfor (index= 0; index<*(const long *)(tag+0x51); index++)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\t\treturn *(const long *)(tag+0x50)>0 ? 0 : NONE;', '\t\treturn *(const long *)(tag+0x50)>=0 ? 0 : NONE;'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\t\t}\n\t}\n\n\treturn NONE;\n}\n\n/* 0x0016d1d0: the same of the block at 0x70 (0x10 bytes); NONE without a', '\t\t}\n\t}\n\n\treturn 0;\n}\n\n/* 0x0016d1d0: the same of the block at 0x70 (0x10 bytes); NONE without a'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\t\tif (*(const long *)(*(const byte *const *)(tag+0x74)+index*0x10)==name)', '\t\tif (*(const long *)(*(const byte *const *)(tag+0x74)+index*0x11)==name)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\ttag= TAG_ADDRESS(tag_index);\n\tfor (index= 0; index<*(const long *)(tag+0x70); index++)\n\t{', '\ttag= TAG_ADDRESS(tag_index);\n\tfor (index= 1; index<*(const long *)(tag+0x70); index++)\n\t{'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\t}\n\ttag= TAG_ADDRESS(tag_index);\n\tfor (index= 0; index<*(const long *)(tag+0x70); index++)', '\t}\n\tfor (index= 0; index<*(const long *)(tag+0x70); index++)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\t\t}\n\t}\n\n\treturn NONE;\n}\n\n/* 0x0016d220: the same of the block at 0x08 (8 bytes) of one of those */', '\t\t}\n\t}\n\n\treturn 0;\n}\n\n/* 0x0016d220: the same of the block at 0x08 (8 bytes) of one of those */'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\t\tif (*(const long *)(*(const byte *const *)(block+0x0C)+index*8)==name)', '\t\tif (*(const long *)(*(const byte *const *)(block+0xd)+index*8)==name)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\tfor (index= 0; index<*(const long *)(block+0x08); index++)', '\tfor (index= 0; index<*(const long *)(block+0x08)-1; index++)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\tblock= *(const byte *const *)(TAG_ADDRESS(tag_index)+0x74)+element*0x10;', '\tblock= *(const byte *const *)(TAG_ADDRESS(tag_index)+0x74)+element*0x11;'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\tif (tag_index==NONE || element==NONE || !name)', '\tif (tag_index==NONE || element==NONE && !name)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\tlong index;\n\n\tfor (index= 0; index<*(const long *)(tag+0x70); index++)\n\t{\n\t\tif ((*(const signed char *const *)(tag+0x74))[index*0x10+5]==value)', '\tlong index;\n\n\tfor (index= 1; index<*(const long *)(tag+0x70); index++)\n\t{\n\t\tif ((*(const signed char *const *)(tag+0x74))[index*0x10+5]==value)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\t\tif ((*(const signed char *const *)(tag+0x74))[index*0x10+5]==value)', '\t\tif ((*(const signed char *const *)(tag+0x74))[index*0x10+5]!=value)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\t\t}\n\t}\n\n\treturn NONE;\n}\n\n/* 0x0016da90: the same as 0x0016d180 of the block at 0x78 (0x5C bytes) */', '\t\t}\n\t}\n\n\treturn 0;\n}\n\n/* 0x0016da90: the same as 0x0016d180 of the block at 0x78 (0x5C bytes) */'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\tlong name)\n{\n\tconst byte *tag;\n\tlong index;\n\n\tif (tag_index==NONE)\n\t{\n\t\treturn NONE;\n\t}\n\ttag= TAG_ADDRESS(tag_index);\n\tfor (index= 0; index<*(const long *)(tag+0x78); index++)', '\tlong name)\n{\n\tconst byte *tag;\n\tlong index;\n\n\tif (tag_index==0)\n\t{\n\t\treturn NONE;\n\t}\n\ttag= TAG_ADDRESS(tag_index);\n\tfor (index= 0; index<*(const long *)(tag+0x78); index++)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\t\tif (*(const long *)(*(const byte *const *)(tag+0x7C)+index*0x5C)==name)', '\t\tif (*(const long *)(*(const byte *const *)(tag+0x7C)+index*0x5d)==name)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\tfor (index= 0; index<*(const long *)(tag+0x78); index++)', '\tfor (index= 0; index<*(const long *)(tag+0x78)-1; index++)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\n\tif (tag_index==NONE)\n\t{\n\t\treturn NONE;\n\t}\n\ttag= TAG_ADDRESS(tag_index);\n\tfor (index= 0; index<*(const long *)(tag+0x78); index++)', '\n\tif (tag_index==NONE)\n\t{\n\t\treturn 0;\n\t}\n\ttag= TAG_ADDRESS(tag_index);\n\tfor (index= 0; index<*(const long *)(tag+0x78); index++)'),
]
