# mutations of milestone 784, written by make_mutations.py

TESTS = "test_small_784"

SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"
SOURCE_NETWORKING_ONLINE_UNKNOWN_000B36C0_C = "source/networking/online_unknown_000b36c0.c"
SOURCE_CSERIES_CSERIES_C = "source/cseries/cseries.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_NETWORKING_SIMULATION_GAMESTATE_ENTITIES_C = "source/networking/simulation_gamestate_entities.c"

MUTATIONS = [
    # taken out: without the checks of the bits of the angles (or their memcpy) an infinite or NaN angle fails the comparisons that follow all the same
    (SOURCE_MATH_REAL_MATH_C, '\treturn angles[0]>=0.0f && 6.2831855f>=angles[0] && angles[1]>=-1.5707964f && 1.5707964f>=angles[1];', '\treturn angles[0]>=0.0f && 6.2831855f>=angles[0] && angles[1]>=-1.5707964f && 1.5707964f>=angles[2];'),
    (SOURCE_NETWORKING_ONLINE_UNKNOWN_000B36C0_C, '{\n\tunknown_0054761c= FALSE;\n\tonline_unknown_000b36c0_unregister();', '{\n\tunknown_0054761c= TRUE;\n\tonline_unknown_000b36c0_unregister();'),
    (SOURCE_NETWORKING_ONLINE_UNKNOWN_000B36C0_C, '\t\tonline_task_close(unknown_00510584);\n\t\tunknown_00510584= NONE;\n\t}', '\t\tonline_task_close(unknown_00510584);\n\t\tunknown_00510584= 0;\n\t}'),
    (SOURCE_NETWORKING_ONLINE_UNKNOWN_000B36C0_C, '\tonline_unknown_000b36c0_unregister();\n\tif (unknown_00510584!=NONE)\n\t{', '\tonline_unknown_000b36c0_unregister();\n\tif (unknown_00510584==NONE)\n\t{'),
    (SOURCE_NETWORKING_ONLINE_UNKNOWN_000B36C0_C, '\t}\n\tunknown_00510580= FALSE;\n}', '\t}\n}'),
    (SOURCE_NETWORKING_ONLINE_UNKNOWN_000B36C0_C, '\tif (unknown_00547620>=count*3)', '\tif (unknown_00547620>=count*4)'),
    (SOURCE_NETWORKING_ONLINE_UNKNOWN_000B36C0_C, '\t\tunknown_0054761c= FALSE;', '\t\tunknown_0054761c= TRUE;'),
    (SOURCE_NETWORKING_ONLINE_UNKNOWN_000B36C0_C, '\t\t\tunknown_00510584= NONE;', '\t\t\tunknown_00510584= 0;'),
    (SOURCE_NETWORKING_ONLINE_UNKNOWN_000B36C0_C, '\t\tcount= unknown_005107d8;', ''),
    (SOURCE_CSERIES_CSERIES_C, '\t\tif (!first[1])\n\t\t{\n\t\t\treturn TRUE;\n\t\t}\n\t\tfirst+= 2;', '\t\tif (!first[1])\n\t\t{\n\t\t\treturn FALSE;\n\t\t}\n\t\tfirst+= 2;'),
    (SOURCE_CSERIES_CSERIES_C, '\t\tsecond+= 2;', '\t\tsecond+= 3;'),
    (SOURCE_CSERIES_CSERIES_C, '\t\tif (first[0]!=second[0])', '\t\tif (first[0]==second[0])'),
    (SOURCE_CSERIES_CSERIES_C, '\t\tif (first[1]!=second[1])\n\t\t{\n\t\t\treturn FALSE;\n\t\t}\n\t\tif (!first[1])', '\t\tif (first[1]!=second[1])\n\t\t{\n\t\t\treturn TRUE;\n\t\t}\n\t\tif (!first[1])'),
    (SOURCE_CSERIES_CSERIES_C, '\t\t\treturn length+4;', '\t\t\treturn length+5;'),
    (SOURCE_CSERIES_CSERIES_C, '\t\tif (!string[1])', '\t\tif (!string[2])'),
    (SOURCE_CSERIES_CSERIES_C, '\t\tif (length>=0xFF)', '\t\tif (length>0xFF)'),
    (SOURCE_CSERIES_CSERIES_C, '\t\tlength+= 5;', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\t*reference_index= collideable_object_cluster_partition.unknown0[cluster_index];\n\tif (*reference_index==NONE)\n\t{', '\t*reference_index= collideable_object_cluster_partition.unknown0[cluster_index];\n\tif (*reference_index==0)\n\t{'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t*reference_index= collideable_object_cluster_partition.unknown0[cluster_index];\n\tif (*reference_index==NONE)\n\t{\n\t\treturn NONE;\n\t}\n\n\treturn object_cluster_reference_next(&collideable_object_cluster_partition, reference_index, data);', '\t*reference_index= collideable_object_cluster_partition.unknown0[cluster_index];\n\tif (*reference_index==NONE)\n\t{\n\t\treturn 0;\n\t}\n\n\treturn object_cluster_reference_next(&collideable_object_cluster_partition, reference_index, data);'),
    (SOURCE_OBJECTS_OBJECTS_C, '{\n\tif (*reference_index==NONE)\n\t{\n\t\treturn NONE;\n\t}\n\n\treturn object_cluster_reference_next(&collideable_object_cluster_partition, reference_index, data);', '{\n\tif (*reference_index==NONE)\n\t{\n\t\treturn 0;\n\t}\n\n\treturn object_cluster_reference_next(&collideable_object_cluster_partition, reference_index, data);'),
    (SOURCE_OBJECTS_OBJECTS_C, '{\n\tif (*reference_index==NONE)\n\t{', '{\n\tif (*reference_index==0)\n\t{'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t*reference_index= noncollideable_object_cluster_partition.unknown0[cluster_index];\n\tif (*reference_index==NONE)\n\t{\n\t\treturn NONE;\n\t}\n\n\treturn object_cluster_reference_next(&noncollideable_object_cluster_partition, reference_index, data);', '\t*reference_index= noncollideable_object_cluster_partition.unknown0[cluster_index];\n\tif (*reference_index==NONE)\n\t{\n\t\treturn 0;\n\t}\n\n\treturn object_cluster_reference_next(&noncollideable_object_cluster_partition, reference_index, data);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t*reference_index= noncollideable_object_cluster_partition.unknown0[cluster_index];\n\tif (*reference_index==NONE)\n\t{', '\t*reference_index= noncollideable_object_cluster_partition.unknown0[cluster_index];\n\tif (*reference_index!=NONE)\n\t{'),
    (SOURCE_OBJECTS_OBJECTS_C, '\treference= (const byte *)references->data+(iterator->reference_index&0xFFFF)*references->size;', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (iterator->reference_index==NONE)', '\tif (iterator->reference_index==0)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\titerator->reference_index= *(const long *)(reference+0x08);', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\treturn *(const short *)(reference+0x04);', '\treturn *(const short *)(reference+0x5);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tlong havok_component_index= *(const long *)(object+0xB4);\n\n\tif (havok_component_index!=NONE)\n\t{\n\t\tunknown_001d1260(HAVOK_COMPONENT_GET(havok_component_index));', '\tlong havok_component_index= *(const long *)(object+0xB4);\n\n\tif (havok_component_index==NONE)\n\t{\n\t\tunknown_001d1260(HAVOK_COMPONENT_GET(havok_component_index));'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tobject[0xC0]&= ~0x40;', '\tobject[0xC0]&= ~0x41;'),
    (SOURCE_OBJECTS_OBJECTS_C, 'void object_unknown_000b8840(\n\tlong object_index)\n{\n\tbyte *object= OBJECT_BYTES(object_index);\n\tlong havok_component_index= *(const long *)(object+0xB4);\n\n\tif (havok_component_index!=NONE)\n\t{\n\t\tunknown_001d1260(HAVOK_COMPONENT_GET(havok_component_index));', 'void object_unknown_000b8840(\n\tlong object_index)\n{\n\tbyte *object= OBJECT_BYTES(object_index);\n\tlong havok_component_index= *(const long *)(object+0xb5);\n\n\tif (havok_component_index!=NONE)\n\t{\n\t\tunknown_001d1260(HAVOK_COMPONENT_GET(havok_component_index));'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tlong havok_component_index= *(const long *)(object+0xB4);\n\n\tif (havok_component_index!=NONE)\n\t{\n\t\tunknown_001d1540(HAVOK_COMPONENT_GET(havok_component_index));', '\tlong havok_component_index= *(const long *)(object+0xB4);\n\n\tif (havok_component_index!=0)\n\t{\n\t\tunknown_001d1540(HAVOK_COMPONENT_GET(havok_component_index));'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tobject[0xC0]|= 0x40;', '\tobject[0xc1]|= 0x40;'),
    (SOURCE_OBJECTS_OBJECTS_C, 'void __attribute__((stdcall)) object_unknown_000b8890(\n\tlong object_index)\n{\n\tbyte *object= OBJECT_BYTES(object_index);\n\tlong havok_component_index= *(const long *)(object+0xB4);\n\n\tif (havok_component_index!=NONE)\n\t{\n\t\tunknown_001d1540(HAVOK_COMPONENT_GET(havok_component_index));', 'void __attribute__((stdcall)) object_unknown_000b8890(\n\tlong object_index)\n{\n\tbyte *object= OBJECT_BYTES(object_index);\n\tlong havok_component_index= *(const long *)(object+0xb5);\n\n\tif (havok_component_index!=NONE)\n\t{\n\t\tunknown_001d1540(HAVOK_COMPONENT_GET(havok_component_index));'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\treturn *(const long *)(*(const byte *const *)(tag+0x98)+index*0x18+0x08);', '\t\treturn *(const long *)(*(const byte *const *)(tag+0x98)+index*0x18+0x9);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\treturn *(const long *)(*(const byte *const *)(tag+0x98)+index*0x18+0x08);\n\t}\n\n\treturn NONE;\n}\n', '\t\treturn *(const long *)(*(const byte *const *)(tag+0x98)+index*0x18+0x08);\n\t}\n\n\treturn 0;\n}\n'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (index>=0 && index<*(const long *)(tag+0x94))', '\tif (index>=0 && index<=*(const long *)(tag+0x94))'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tglobal_tag_instances[*(const long *)OBJECT_BYTES(object_index)&0xFFFF].address;', '\t\tglobal_tag_instances[*(const long *)OBJECT_BYTES(object_index)&0x10000].address;'),
    (SOURCE_NETWORKING_SIMULATION_GAMESTATE_ENTITIES_C, '\t\t\tchild= *(const long *)(object+0x0C);', '\t\t\tchild= *(const long *)(object+0xd);'),
    (SOURCE_NETWORKING_SIMULATION_GAMESTATE_ENTITIES_C, '\t\t\t\t&& found==NONE)', '\t\t\t\t|| found==NONE)'),
    (SOURCE_NETWORKING_SIMULATION_GAMESTATE_ENTITIES_C, '\t\t\tif (*(const long *)object==*(const long *)(data+0x04) && *(const long *)(object+0xD4)==NONE', '\t\t\tif (*(const long *)object==*(const long *)(data+0x04) && *(const long *)(object+0xD4)!=NONE'),
    (SOURCE_NETWORKING_SIMULATION_GAMESTATE_ENTITIES_C, '\t\t\tconst byte *object= (const byte *)headers[child&0xFFFF].datum;', '\t\t\tconst byte *object= (const byte *)headers[child&0x10000].datum;'),
    # by hand: where the rest of a reference starts, and the five characters at a time
    (SOURCE_OBJECTS_OBJECTS_C, '\t*data= reference+0x0C;', '\t*data= reference+0x08;'),
    (SOURCE_CSERIES_CSERIES_C, '\t\tstring+= 5;\n\t\tlength+= 5;', '\t\tstring+= 4;\n\t\tlength+= 4;'),
]
