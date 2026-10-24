# mutations of milestone 783, written by make_mutations.py

TESTS = "test_small_783"

SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C = "source/game/game_unknown_0003f2b0.c"
SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"
SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"
SOURCE_GAME_GAME_ENGINE_C = "source/game/game_engine.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\tunknown_004c56c0[bit>>5]|= 1UL<<(bit&0x1F);', '\t\tunknown_004c56c0[bit>>5]|= 1UL<<(bit&0x20);'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\tfor (index= 0; index<*(const short *)(data+0xA6C); index++)', '\tfor (index= 0; index<*(const short *)(data+0xA6C)-1; index++)'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\tshort bit= *(const short *)(data+index*0x1A+0xA6E);', '\t\tshort bit= *(const short *)(data+index*0x1A+0xa6f);'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\tfor (index= 0; index<6; index++)', '\tfor (index= 1; index<6; index++)'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\tif (plane->n.i*center->x + plane->n.k*center->z + plane->n.j*center->y - plane->d>=distance)', '\t\tif (plane->n.i*center->x + plane->n.k*center->z + plane->n.j*center->y - plane->d>distance)'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\t}\n\t}\n\n\treturn FALSE;\n}\n', '\t\t}\n\t}\n\n\treturn TRUE;\n}\n'),
    (SOURCE_GAME_GAME_UNKNOWN_0003F2B0_C, '\t\t\treturn TRUE;', '\t\t\treturn FALSE;'),
    (SOURCE_MATH_REAL_MATH_C, '\treturn radius*radius>=x*x + z*z + y*y;', '\treturn radius*radius>x*x + z*z + y*y;'),
    (SOURCE_MATH_REAL_MATH_C, '\tsign= bits>>31;', ''),
    (SOURCE_MATH_REAL_MATH_C, '\tshift= (0x96-((bits>>23)&0xFF))&0x1F;', ''),
    (SOURCE_MATH_REAL_MATH_C, '\tif (!sign && bits && (exponent<0 || (((1UL<<shift)-1)&(unsigned long)bits&0x7FFFFF)))', '\tif (!sign || bits && (exponent<0 || (((1UL<<shift)-1)&(unsigned long)bits&0x7FFFFF)))'),
    (SOURCE_MATH_REAL_MATH_C, '\texponent= ((bits>>23)&0xFF)-0x7F;', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\tunsigned long index)\n{\n\tif (tag_index!=NONE)\n\t{\n\t\tconst byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;', '\tunsigned long index)\n{\n\tif (tag_index!=0)\n\t{\n\t\tconst byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t\t&& *(const long *)(*(const byte *const *)(tag+0x08)+index*0x1C+0x0C)>>4!=0)', '\t\t\t&& *(const long *)(*(const byte *const *)(tag+0x08)+index*0x1C+0x0C)>>4==0)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (tag_index!=NONE)\n\t{\n\t\tconst byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;\n\n\t\tif (index<*(const unsigned long *)(tag+0x04)', '\tif (tag_index!=NONE)\n\t{\n\t\tconst byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0x10000].address;\n\n\t\tif (index<*(const unsigned long *)(tag+0x04)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t\t&& *(const long *)(*(const byte *const *)(tag+0x08)+index*0x1C+0x0C)>>4!=0)\n\t\t{\n\t\t\treturn TRUE;\n\t\t}\n\t}', '\t\t\t&& *(const long *)(*(const byte *const *)(tag+0x08)+index*0x1C+0x0C)>>4!=0)\n\t\t{\n\t\t\treturn FALSE;\n\t\t}\n\t}'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tvalue= *(const unsigned short *const *)(entry+0x08);', '\t\tvalue= *(const unsigned short *const *)(entry+0x9);'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t\t((long *)(text+0x04))[1]= *value*3;', '\t\t\t((long *)(text+0x04))[1]= *value*4;'),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\t\t((long *)(text+0x04))[0]= *value*3;', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tentry= *(const byte *const *)(data+0x54)+index*0x0C;', '\t\tentry= *(const byte *const *)(data+0x54)+index*0xd;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tresult[2]= value[2];', '\tresult[2]= value[3];'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif (tag_index!=NONE)', '\tif (tag_index!=0)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tresult[1]= value[1];', '\tresult[1]= value[2];'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tconst byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;', '\t\tconst byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0x10000].address;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tlong value= game_engine_unknown_000a4950(flags)+2;', '\tlong value= game_engine_unknown_000a4950(flags)+3;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif ((bits&0x20) && value>=11)', '\tif ((bits&0x21) && value>=11)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tvalue= 11;', '\t\tvalue= 12;'),
    # taken out: value>23 instead of value>=23 does the same (23 is set to 23)
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tvalue= 0x3E;', ''),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tif ((bits&(1UL<<(index+6))) && value>=0x20)', '\t\tif ((bits&(1UL<<(index+7))) && value>=0x20)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tfor (index= 0; index<16; index++)', '\tfor (index= 0; index<16-1; index++)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif ((bits&0x20) && value>=0x3E)', '\tif ((bits&0x21) && value>=0x3E)'),
    (SOURCE_GAME_GAME_ENGINE_C, '\t\tvalue= 0x16;', '\t\tvalue= 0x17;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tlong value= game_engine_unknown_000a4950(flags)+1;', '\tlong value= game_engine_unknown_000a4950(flags)+2;'),
    (SOURCE_GAME_GAME_ENGINE_C, '\tif ((*(const byte *)flags&0x20) && value>=0x16)', '\tif ((*(const byte *)flags&0x21) && value>=0x16)'),
    # by hand: the result of 0x0009a150 one more, and the size of each element of 0x0004cbb0
    (SOURCE_GAME_GAME_ENGINE_C, '	*result= game_engine_unknown_000a4950(flags);', '	*result= game_engine_unknown_000a4950(flags)+1;'),
    (SOURCE_EFFECTS_EFFECTS_C, '(tag+0x08)+index*0x1C+0x0C)>>4!=0)', '(tag+0x08)+index*0x18+0x0C)>>4!=0)'),
]
