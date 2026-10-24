# mutations of milestone 787, written by make_mutations.py

TESTS = "test_small_787"

SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\tif (bit&0x199F)', '\tif (bit&0x19a0)'),
    (SOURCE_UNITS_UNITS_C, '\tdefinition= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;', ''),
    (SOURCE_UNITS_UNITS_C, '\treturn *(const long *)(tag+0x60)>0;', '\treturn *(const long *)(tag+0x60)>=0;'),
    # taken out: bit&0x41 (the bit 0x01 already returned TRUE), data[0x85] (the memset already cleared both), and 0x35d/4, 0x365/4, 0x351/4 (the same index as 0x35C/4, 0x364/4, 0x350/4)
    (SOURCE_UNITS_UNITS_C, '\t{\n\tcase 0:\n\t\tbreak;', '\t{\n\tcase 1:\n\t\tbreak;'),
    (SOURCE_UNITS_UNITS_C, '\tcase 2:', '\tcase 3:'),
    (SOURCE_UNITS_UNITS_C, '\tcase 1:\n\t\treturn 0;\n\tcase 2:', '\tcase 1:\n\t\treturn 1;\n\tcase 2:'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (*(const short *)(data+0x2C)==NONE || (1UL<<(data[0x2E]&0x1F))&0x181F)', '\t\tif (*(const short *)(data+0x2C)==0 || (1UL<<(data[0x2E]&0x1F))&0x181F)'),
    (SOURCE_UNITS_UNITS_C, '\ttag= UNIT_DEFINITION_UNKNOWN38(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index)));', ''),
    (SOURCE_UNITS_UNITS_C, '\tif (!tag || *(const long *)(tag+0x60)<=0)', '\tif (!tag || *(const long *)(tag+0x61)<=0)'),
    (SOURCE_UNITS_UNITS_C, '\tif (object_index==NONE)\n\t{\n\t\treturn 0;\n\t}\n\ttag= UNIT_DEFINITION_UNKNOWN38(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index)));', '\tif (object_index==NONE)\n\t{\n\t\treturn 1;\n\t}\n\ttag= UNIT_DEFINITION_UNKNOWN38(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index)));'),
    (SOURCE_UNITS_UNITS_C, '\tconst byte *tag;\n\n\tif (object_index==NONE)\n\t{\n\t\treturn 0;', '\tconst byte *tag;\n\n\tif (object_index==0)\n\t{\n\t\treturn 0;'),
    (SOURCE_UNITS_UNITS_C, '\tmemset(data, 0, 0x88);', '\tmemset(data, 1, 0x88);'),
    (SOURCE_UNITS_UNITS_C, '\t\tif ((*(const byte *const *)(element+index*0x60+0x40))[0x40])', '\t\tif ((*(const byte *const *)(element+index*0x61+0x40))[0x40])'),
    (SOURCE_UNITS_UNITS_C, '\t\tif ((*(const byte *const *)(element+index*0x60+0x40))[0x40])\n\t\t{\n\t\t\treturn TRUE;\n\t\t}\n\t}', '\t\tif ((*(const byte *const *)(element+index*0x60+0x40))[0x40])\n\t\t{\n\t\t\treturn FALSE;\n\t\t}\n\t}'),
    (SOURCE_UNITS_UNITS_C, '\tfor (index= 0; index<*(const long *)(data+0x74); index++)', '\tfor (index= 0; index<*(const long *)(data+0x74)-1; index++)'),
    (SOURCE_UNITS_UNITS_C, '\tconst byte *element= *(const byte *const *)(data+0x70);', '\tconst byte *element= *(const byte *const *)(data+0x71);'),
    (SOURCE_UNITS_UNITS_C, '\t\t&& (tag= UNIT_DEFINITION_UNKNOWN38(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index)))))', '\t\t|| (tag= UNIT_DEFINITION_UNKNOWN38(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index)))))'),
    (SOURCE_UNITS_UNITS_C, '\n\tif (object_index!=NONE\n\t\t&& (tag= UNIT_DEFINITION_UNKNOWN38(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index)))))', '\n\tif (object_index!=0\n\t\t&& (tag= UNIT_DEFINITION_UNKNOWN38(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(object_index)))))'),
    (SOURCE_UNITS_UNITS_C, '\tcase 0x0A000017:', '\tcase 0xa000018:'),
    (SOURCE_UNITS_UNITS_C, '\tcase 0x09000015:', '\tcase 0x9000016:'),
    (SOURCE_UNITS_UNITS_C, '\tcase 0x0A000014:', '\tcase 0xa000015:'),
    (SOURCE_UNITS_UNITS_C, '\tcase 0x0A00000F:', '\tcase 0xa000010:'),
    (SOURCE_UNITS_UNITS_C, '\tlong index= *(const long *)(unit+0x354);', '\tlong index= *(const long *)(unit+0x355);'),
    (SOURCE_UNITS_UNITS_C, '\treturn (*(const byte *const *)((const byte *)global_collision_bsp+0x2C))[index*8+4]&1;', '\treturn (*(const byte *const *)((const byte *)global_collision_bsp+0x2d))[index*8+4]&1;'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(unit+0x35C)!=NONE || index==NONE || *(const long *)(unit+0x360)!=NONE)', '\tif (*(const long *)(unit+0x35C)!=NONE || index!=NONE || *(const long *)(unit+0x360)!=NONE)'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(unit+0x35C)!=NONE || index==NONE || *(const long *)(unit+0x360)!=NONE)\n\t{\n\t\treturn FALSE;\n\t}\n', '\tif (*(const long *)(unit+0x35C)!=NONE || index==NONE || *(const long *)(unit+0x360)!=NONE)\n\t{\n\t\treturn TRUE;\n\t}\n'),
    (SOURCE_UNITS_UNITS_C, '\tunit[0x358/4]= NONE;', '\tunit[0x358/5]= NONE;'),
    (SOURCE_UNITS_UNITS_C, '\treturn unit[0x3DC]==6 && unit[0x3F8] && (*(const unsigned short *)(unit+0x348)>>13)&1;', '\treturn unit[0x3DC]==7 && unit[0x3F8] && (*(const unsigned short *)(unit+0x348)>>13)&1;'),
    (SOURCE_UNITS_UNITS_C, '\treturn value!=NONE && ((unit[0xC0]>>1)&1 || (unit[0xC0]>>2)&1 || value==*(const long *)(unit+0x3E4));', '\treturn value!=NONE && ((unit[0xC0]>>1)&1 || (unit[0xC0]>>2)&1 || value==*(const long *)(unit+0x3e5));'),
    (SOURCE_UNITS_UNITS_C, '\tlong value= *(const long *)(unit+0x3E8);', '\tlong value= *(const long *)(unit+0x3e9);'),
    (SOURCE_UNITS_UNITS_C, '\treturn ((*(const unsigned long *)(UNIT_DEFINITION_GET(object)+0x264)>>4)&1)+1;', '\treturn ((*(const unsigned long *)(UNIT_DEFINITION_GET(object)+0x264)>>4)&2)+1;'),
    (SOURCE_UNITS_UNITS_C, '\t\treturn 3;', '\t\treturn 4;'),
    (SOURCE_UNITS_UNITS_C, '\tconst byte *object= UNIT_OBJECT_GET(object_index);\n\n\tif ((object[0x10A]>>2)&1)\n\t{\n\t\treturn 3;', '\tconst byte *object= UNIT_OBJECT_GET(object_index);\n\n\tif ((object[0x10A]>>3)&1)\n\t{\n\t\treturn 3;'),
    (SOURCE_UNITS_UNITS_C, '\treturn !((data[0x12]&1) && !(data[0x11]&9));', '\treturn !((data[0x12]&2) && !(data[0x11]&9));'),
    (SOURCE_UNITS_UNITS_C, '\treturn (bits[bit>>5]&(1UL<<(bit&0x1F)))!=0;', '\treturn (bits[bit>>6]&(1UL<<(bit&0x1F)))!=0;'),
    (SOURCE_UNITS_UNITS_C, '\tconst unsigned long *bits= (const unsigned long *)(UNIT_UNKNOWN346(UNIT_OBJECT_GET(unit_index))+0x04);', '\tconst unsigned long *bits= (const unsigned long *)(UNIT_UNKNOWN346(UNIT_OBJECT_GET(unit_index))+0x5);'),
]
