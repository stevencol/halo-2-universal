# mutations of milestone 791, written by make_mutations.py

TESTS = "test_small_791"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_SCENARIO_SCENARIO_C = "source/scenario/scenario.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\treturn name==0x0A000066 || (name>=0x09000008 && name<=0x09000009) || name==0x0F00067F;', '\treturn name==0x0A000066 || (name>=0x09000008 || name<=0x09000009) || name==0x0F00067F;'),
    (SOURCE_UNITS_UNITS_C, '\tlong name= *(const long *)(UNIT_UNKNOWN33E(UNIT_OBJECT_GET(object_index))+0xA4);', '\tlong name= *(const long *)(UNIT_UNKNOWN33E(UNIT_OBJECT_GET(object_index))+0xa5);'),
    (SOURCE_UNITS_UNITS_C, '\treturn !data[0x89] || (!(data[0x8B]&1) && (data[0x8B]&2));', '\treturn !data[0x89] || (!(data[0x8c]&1) && (data[0x8B]&2));'),
    (SOURCE_UNITS_UNITS_C, '\treturn data[0x89] && (data[0x8B]&1) && (data[0x8B]&2);', '\treturn data[0x89] && (data[0x8B]&1) && (data[0x8B]&3);'),
    (SOURCE_UNITS_UNITS_C, '\treturn *(const long *)((const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address+0x14);', '\treturn *(const long *)((const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address+0x15);'),
    (SOURCE_UNITS_UNITS_C, '\tif (tag_index==NONE)', '\tif (tag_index!=NONE)'),
    (SOURCE_UNITS_UNITS_C, '\tif (tag_index==NONE)\n\t{\n\t\treturn 0;\n\t}\n', '\tif (tag_index==NONE)\n\t{\n\t\treturn 1;\n\t}\n'),
    (SOURCE_UNITS_UNITS_C, '\tlong tag_index= *(const long *)(object+*(const short *)(object+0x342));', '\tlong tag_index= *(const long *)(object+*(const short *)(object+0x343));'),
    (SOURCE_UNITS_UNITS_C, '\tdata[9]= ((const unsigned long *)global_up3d)[0];', '\tdata[10]= ((const unsigned long *)global_up3d)[0];'),
    (SOURCE_UNITS_UNITS_C, '\tdata[10]= ((const unsigned long *)global_up3d)[1];', '\tdata[10]= ((const unsigned long *)global_up3d)[2];'),
    (SOURCE_UNITS_UNITS_C, '\tdata[0]= 0x06000086;', '\tdata[0]= 0x6000087;'),
    (SOURCE_UNITS_UNITS_C, '\tdata[8]= ((const unsigned long *)global_forward3d)[2];', '\tdata[9]= ((const unsigned long *)global_forward3d)[2];'),
    (SOURCE_UNITS_UNITS_C, '\treturn !((object[0x10A]>>2)&1) && ((object[0x12C]>>5)&1 || (object[0x12C]>>4)&1);', '\treturn !((object[0x10A]>>2)&1) && ((object[0x12C]>>5)&1 || (object[0x12C]>>5)&1);'),
    (SOURCE_UNITS_UNITS_C, '{\n\tif (unit_index==NONE)\n\t{', '{\n\tif (unit_index!=NONE)\n\t{'),
    (SOURCE_UNITS_UNITS_C, '\treturn UNIT_UNKNOWN12A_IS(UNIT_UNKNOWN12A(UNIT_OBJECT_GET(unit_index)), 0x0E0000C2);', '\treturn UNIT_UNKNOWN12A_IS(UNIT_UNKNOWN12A(UNIT_OBJECT_GET(unit_index)), 0xe0000c3);'),
    (SOURCE_UNITS_UNITS_C, '\tif (unit_index==NONE)\n\t{\n\t\treturn FALSE;\n\t}\n', '\tif (unit_index==NONE)\n\t{\n\t\treturn TRUE;\n\t}\n'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t}\n\tlocation->cluster_index= NONE;\n\tlocation->structure_bsp_index= global_structure_bsp_index;', '\t}\n\tlocation->structure_bsp_index= global_structure_bsp_index;'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\tlocation->cluster_index= *(const short *)(*(const byte *const *)((const byte *)global_structure_bsp+0x30)+leaf_index*8);', ''),
    (SOURCE_SCENARIO_SCENARIO_C, '\tlocation->leaf_index= leaf_index;', ''),
    (SOURCE_SCENARIO_SCENARIO_C, '\tlocation->leaf_index= leaf_index;\n\tif (leaf_index!=NONE)\n\t{', '\tlocation->leaf_index= leaf_index;\n\tif (leaf_index!=0)\n\t{'),
    (SOURCE_SCENARIO_SCENARIO_C, '\tif (tag_index==NONE)', '\tif (tag_index!=NONE)'),
    (SOURCE_SCENARIO_SCENARIO_C, '\tif (index==NONE || index>=*(const long *)(bsp+0xD4))', '\tif (index==NONE && index>=*(const long *)(bsp+0xD4))'),
    (SOURCE_SCENARIO_SCENARIO_C, '\treturn *(const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address&1;', '\treturn *(const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address&2;'),
    (SOURCE_SCENARIO_SCENARIO_C, '\ttag_index= *(const long *)(*(const byte *const *)(bsp+0xD8)+index*0x64+0x24);', ''),
    (SOURCE_SCENARIO_SCENARIO_C, '\treturn (bits[((*(const long *)(bsp+0x9C)+0x1F)>>5)*row+(column>>5)]&(1UL<<(column&0x1F)))!=0;', '\treturn (bits[((*(const long *)(bsp+0x9C)+0x1F)>>5)*row+(column>>6)]&(1UL<<(column&0x1F)))!=0;'),
    (SOURCE_SCENARIO_SCENARIO_C, '\tconst unsigned long *bits= *(const unsigned long *const *)(bsp+0x58);', '\tconst unsigned long *bits= *(const unsigned long *const *)(bsp+0x59);'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\tif (*entry==*other)', '\t\tif (*entry!=*other)'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\twhile (*entry==*other && *entry)', '\t\twhile (*entry==*other || *entry)'),
    (SOURCE_SCENARIO_SCENARIO_C, '\tfor (index= 0; index<*(const long *)(data+0x48); index++)', '\tfor (index= 1; index<*(const long *)(data+0x48); index++)'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\tconst char *entry= (const char *)(*(const byte *const *)(data+0x4C)+index*0x24);', '\t\tconst char *entry= (const char *)(*(const byte *const *)(data+0x4C)+index*0x25);'),
]
