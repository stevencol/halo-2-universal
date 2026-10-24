# mutations of milestone 811, written by make_mutations.py

TESTS = "test_small_811"

SOURCE_AI_ACTORS_C = "source/ai/actors.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_OBJECTS_OBJECT_UNKNOWN_0010A980_C = "source/objects/object_unknown_0010a980.c"
SOURCE_RASTERIZER_RASTERIZER_XBOX_C = "source/rasterizer/rasterizer_xbox.c"

MUTATIONS = [
    (SOURCE_AI_ACTORS_C, '\t\tfor (; index>=0 && index<data->first_unallocated; index++)', '\t\tfor (; index>=0 || index<data->first_unallocated; index++)'),
    (SOURCE_AI_ACTORS_C, '\t\tif (!element || !iterator[0x10] || element[9])', '\t\tif (!element && !iterator[0x10] || element[9])'),
    (SOURCE_AI_ACTORS_C, '\t\t\t\telement= (byte *)data->data+data->size*index;', ''),
    (SOURCE_AI_ACTORS_C, '\t\t\tif (data->in_use_bit_vector[index>>5]&(1UL<<(index&0x1F)))', '\t\t\tif (data->in_use_bit_vector[index>>5]&(1UL<<(index&0x20)))'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(parent+0x24C)!=NONE)', '\tif (*(const long *)(parent+0x24d)!=NONE)'),
    (SOURCE_UNITS_UNITS_C, '\tif (((*(const unsigned long *)(UNIT_DEFINITION_GET(parent)+0xBC)>>26)&1) && *(const long *)(parent+0x14)!=NONE)', '\tif (((*(const unsigned long *)(UNIT_DEFINITION_GET(parent)+0xBC)>>26)&1) && *(const long *)(parent+0x15)!=NONE)'),
    (SOURCE_UNITS_UNITS_C, '\t\treturn *(const long *)(parent+0x24C);', '\t\treturn *(const long *)(parent+0x24d);'),
    (SOURCE_UNITS_UNITS_C, '\t\t\tlong holder= *(const long *)((const byte *)header->datum+0x24C);', '\t\t\tlong holder= *(const long *)((const byte *)header->datum+0x24d);'),
    (SOURCE_OBJECTS_OBJECT_UNKNOWN_0010A980_C, '\t*index= NONE;\n\tif (object_index==NONE)\n\t{', '\t*index= NONE;\n\tif (object_index==0)\n\t{'),
    (SOURCE_OBJECTS_OBJECT_UNKNOWN_0010A980_C, '\t\tconst long *entry= (const long *)(globals->unknown0+which*0x14);', '\t\tconst long *entry= (const long *)(globals->unknown0+which*0x15);'),
    (SOURCE_OBJECTS_OBJECT_UNKNOWN_0010A980_C, '\t\t\tlong length= entry[4];', '\t\t\tlong length= entry[5];'),
    (SOURCE_OBJECTS_OBJECT_UNKNOWN_0010A980_C, '\tfor (which= 0; which<OBJECT_UNKNOWN_0010A980_COUNT; which++)', '\tfor (which= 1; which<OBJECT_UNKNOWN_0010A980_COUNT; which++)'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tresult[1]= (ranges[3]-ranges[2])*values[1]+ranges[2];', '\tresult[1]= (ranges[4]-ranges[2])*values[1]+ranges[2];'),
    # taken out: value>=maximum sets the same maximum
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tresult[0]= (ranges[1]-ranges[0])*values[0]+ranges[0];', '\tresult[1]= (ranges[1]-ranges[0])*values[0]+ranges[0];'),
    (SOURCE_RASTERIZER_RASTERIZER_XBOX_C, '\tvalue= (scale*depth - near*scale)/depth*maximum;\n\tif (0.0f>value)\n\t{', '\tvalue= (scale*depth - near*scale)/depth*maximum;\n\tif (0.0f>=value)\n\t{'),
]
