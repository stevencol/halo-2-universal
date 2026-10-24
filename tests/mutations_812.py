# mutations of milestone 812, written by make_mutations.py

TESTS = "test_small_812"

SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_SCENARIO_SCENARIO_C = "source/scenario/scenario.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    (SOURCE_OBJECTS_OBJECTS_C, '\t\t\t\t(*(short *)(list+4))++;', '\t\t\t\t(*(short *)(list+5))++;'),
    (SOURCE_OBJECTS_OBJECTS_C, '{\n\tconst struct object_header_datum *header= (const struct object_header_datum *)object_header_data->data+(object_index&0xFFFF);\n\tbyte *object= (byte *)header->datum;', '{\n\tconst struct object_header_datum *header= (const struct object_header_datum *)object_header_data->data+(object_index&0x10000);\n\tbyte *object= (byte *)header->datum;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tlong *link= (long *)(list+8);', '\t\tlong *link= (long *)(list+9);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tif (!((flags>>14)&1) && !((flags>>15)&1))', '\t\tif (!((flags>>14)&1) && !((flags>>16)&1))'),
    (SOURCE_SCENARIO_SCENARIO_C, '\tif (other==NONE || other<0 || other>=count || entries[other*2+1]==NONE)', '\tif (other!=NONE || other<0 || other>=count || entries[other*2+1]==NONE)'),
    (SOURCE_SCENARIO_SCENARIO_C, '\tlong count= *(const long *)(scenario+0x08);', '\tlong count= *(const long *)(scenario+0x9);'),
    (SOURCE_SCENARIO_SCENARIO_C, '\t\tif (which!=NONE && which>=0 && which<count && entries[which*2+1]!=NONE', '\t\tif (which!=NONE && which>=0 && which<count && entries[which*3+1]!=NONE'),
    (SOURCE_SCENARIO_SCENARIO_C, '\tif (other==NONE || other<0 || other>=count || entries[other*2+1]==NONE)\n\t{\n\t\treturn NONE;\n\t}\n\t*index= other;', '\tif (other==NONE || other<0 || other>=count || entries[other*2+1]==NONE)\n\t{\n\t\treturn 0;\n\t}\n\t*index= other;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tdata[0x60]= 0;', '\tdata[0x60]= 1;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tdata[0x65]= 0;', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tdata[0x63]= 0;', '\tdata[0x64]= 0;'),
    (SOURCE_PHYSICS_HAVOK_C, '\thavok_unknown_clear(data);\n\thavok_unknown_clear(data+0x20);\n\thavok_unknown_clear(data+0x40);', '\thavok_unknown_clear(data);\n\thavok_unknown_clear(data+0x21);\n\thavok_unknown_clear(data+0x40);'),
    (SOURCE_UNITS_UNITS_C, '\t\t\tlength= *(const float *)(data+0x14);', '\t\t\tlength= *(const float *)(data+0x15);'),
    (SOURCE_UNITS_UNITS_C, '\t\t\t\tlength= *(const float *)(data+0x1C);', ''),
    (SOURCE_UNITS_UNITS_C, '\t\t\t\tnew_distance= step*new_speed + new_distance;', ''),
    (SOURCE_UNITS_UNITS_C, '\tif (!result && time>0.0f)', '\tif (!result || time>0.0f)'),
]
