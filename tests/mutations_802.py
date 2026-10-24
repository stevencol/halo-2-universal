# mutations of milestone 802, written by make_mutations.py

TESTS = "test_small_802"

SOURCE_CACHE_TAG_UNKNOWN_0016D180_C = "source/cache/tag_unknown_0016d180.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\tfor (index= 0; index<*(const long *)(tag+0x58); index++)', '\tfor (index= 1; index<*(const long *)(tag+0x58); index++)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\tif (tag_index==NONE || name==NONE || !name)', '\tif (tag_index==NONE && name==NONE || !name)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\t\tif (*(const long *)(*(const byte *const *)(tag+0x5C)+index*0x0C)==name)', '\t\tif (*(const long *)(*(const byte *const *)(tag+0x5d)+index*0x0C)==name)'),
    (SOURCE_CACHE_TAG_UNKNOWN_0016D180_C, '\tif (tag_index==NONE || name==NONE || !name)\n\t{\n\t\treturn NONE;\n\t}\n\ttag= TAG_ADDRESS(tag_index);', '\tif (tag_index==NONE || name==NONE || !name)\n\t{\n\t\treturn 0;\n\t}\n\ttag= TAG_ADDRESS(tag_index);'),
    (SOURCE_UNITS_UNITS_C, '\t\t\tcounter[0]= 0;\n\t\t\tcounter[3]|= 2;\n\t\t}', '\t\t\tcounter[0]= 0;\n\t\t\tcounter[3]|= 3;\n\t\t}'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (counter[0]<=0)', '\t\tif (counter[0]<=1)'),
    (SOURCE_UNITS_UNITS_C, '\t\t\tcounter[0]= maximum;', '\t\t\tcounter[1]= maximum;'),
    (SOURCE_UNITS_UNITS_C, '\t\t}\n\t}\n\n\treturn TRUE;\n}\n\n/* 0x001d9430: what is left of it, in seconds: up to the end going up, or', '\t\t}\n\t}\n\n\treturn FALSE;\n}\n\n/* 0x001d9430: what is left of it, in seconds: up to the end going up, or'),
    (SOURCE_UNITS_UNITS_C, '\t\treturn (real)(counter[1]-counter[0])*game_time_globals->seconds_per_tick;', '\t\treturn (real)(counter[1]-counter[1])*game_time_globals->seconds_per_tick;'),
    (SOURCE_UNITS_UNITS_C, '\t\treturn 0.0f;\n\t}\n\tif (counter[3]&1)\n\t{\n\t\treturn (real)(counter[1]-counter[0])*game_time_globals->seconds_per_tick;', '\t\treturn 0.0f;\n\t}\n\tif (counter[3]&2)\n\t{\n\t\treturn (real)(counter[1]-counter[0])*game_time_globals->seconds_per_tick;'),
    (SOURCE_UNITS_UNITS_C, '\treturn (real)counter[0]*game_time_globals->seconds_per_tick;', '\treturn (real)counter[1]*game_time_globals->seconds_per_tick;'),
    (SOURCE_UNITS_UNITS_C, '\tif (!counter[1])', '\tif (!counter[2])'),
    (SOURCE_UNITS_UNITS_C, '\tconst struct object_header_datum *header= (const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);', '\tconst struct object_header_datum *header= (const struct object_header_datum *)object_header_data->data+(object_index&0x10000);'),
    (SOURCE_UNITS_UNITS_C, '\treturn !UNIT_KIND0_STATE6(&headers[first_index&0xFFFF]) && !UNIT_KIND0_STATE6(&headers[second_index&0xFFFF]);', '\treturn !UNIT_KIND0_STATE6(&headers[first_index&0xFFFF]) && !UNIT_KIND0_STATE6(&headers[second_index&0x10000]);'),
    (SOURCE_UNITS_UNITS_C, '\tconst unsigned long *point= source[0x1D] ? (const unsigned long *)(source+0x10) : (const unsigned long *)global_up3d;', '\tconst unsigned long *point= source[0x1e] ? (const unsigned long *)(source+0x10) : (const unsigned long *)global_up3d;'),
    (SOURCE_UNITS_UNITS_C, '\t((unsigned long *)(data+0xAC))[0]= point[0];', '\t((unsigned long *)(data+0xAC))[1]= point[0];'),
    (SOURCE_UNITS_UNITS_C, '\t((unsigned long *)(data+0xAC))[1]= point[1];', ''),
    (SOURCE_UNITS_UNITS_C, '\t((unsigned long *)(data+0xAC))[2]= point[2];', '\t((unsigned long *)(data+0xad))[2]= point[2];'),
    (SOURCE_UNITS_UNITS_C, '\tdata[0xBC]= value;', ''),
    (SOURCE_UNITS_UNITS_C, '\t((unsigned long *)(data+0xCC))[2]= second[2];', '\t((unsigned long *)(data+0xcd))[2]= second[2];'),
    (SOURCE_UNITS_UNITS_C, '\t((unsigned long *)(data+0xC0))[1]= first[1];', '\t((unsigned long *)(data+0xC0))[2]= first[1];'),
    (SOURCE_UNITS_UNITS_C, '\t((unsigned long *)(data+0xC0))[0]= first[0];', '\t((unsigned long *)(data+0xc1))[0]= first[0];'),
    (SOURCE_UNITS_UNITS_C, '\t\twhich= 3;', ''),
    (SOURCE_UNITS_UNITS_C, '\tif (entries[4]<oldest)', '\tif (entries[5]<oldest)'),
    (SOURCE_UNITS_UNITS_C, '\t\toldest= entries[2];', '\t\toldest= entries[3];'),
    (SOURCE_UNITS_UNITS_C, '\tif (entries[6]<oldest)', '\tif (entries[7]<oldest)'),
    (SOURCE_UNITS_UNITS_C, '\t\tbits[0]|= mask;', '\t\tbits[1]|= mask;'),
    (SOURCE_UNITS_UNITS_C, '\t\tbits[1]|= mask;', '\t\tbits[2]|= mask;'),
    (SOURCE_UNITS_UNITS_C, '\tif ((value>>1)&1)', '\tif ((value>>1)&2)'),
    (SOURCE_UNITS_UNITS_C, '\t\tbits[0]&= ~mask;', ''),
]
