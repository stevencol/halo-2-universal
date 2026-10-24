# mutations of milestone 808, written by make_mutations.py

TESTS = "test_small_808"

SOURCE_AI_ACTORS_C = "source/ai/actors.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_MAIN_MAIN_C = "source/main/main.c"

MUTATIONS = [
    (SOURCE_AI_ACTORS_C, '\tshort index;\n\n\tfor (index= 0; index<count; index++)\n\t{\n\t\tlong next= index+1>=count ? 0 : index+1;', '\tshort index;\n\n\tfor (index= 1; index<count; index++)\n\t{\n\t\tlong next= index+1>=count ? 0 : index+1;'),
    (SOURCE_AI_ACTORS_C, '\t\treal cross= (point[1]-points[index*2+1])*(points[next*2]-points[index*2])', '\t\treal cross= (point[1]-points[index*2+2])*(points[next*2]-points[index*2])'),
    (SOURCE_AI_ACTORS_C, '\t\t\t- (point[0]-points[index*2])*(points[next*2+1]-points[index*2+1]);', '\t\t\t- (point[0]-points[index*2])*(points[next*2+1]-points[index*2+2]);'),
    (SOURCE_AI_ACTORS_C, '\t\t}\n\t}\n\n\treturn TRUE;\n}\n\n/* 0x001db070: the long at 0x10 of the entry (0x14 bytes) that matches a long', '\t\t}\n\t}\n\n\treturn FALSE;\n}\n\n/* 0x001db070: the long at 0x10 of the entry (0x14 bytes) that matches a long'),
    (SOURCE_AI_ACTORS_C, '\t\tif (*(const long *)(element+0x04)==value04 && (char)element[0x0A]==value0A && (char)element[0x0B]==value0B)', '\t\tif (*(const long *)(element+0x04)==value04 && (char)element[0x0A]==value0A && (char)element[0x0B]!=value0B)'),
    (SOURCE_AI_ACTORS_C, '\t\t\t\tif (*(const long *)(entry+0x04)==entry04 && *(const long *)(entry+0x08)==entry08', '\t\t\t\tif (*(const long *)(entry+0x04)==entry04 || *(const long *)(entry+0x08)==entry08'),
    (SOURCE_AI_ACTORS_C, '\t\t\tfor (other= 0; other<*(const long *)(element+0x0C); other++)', '\t\t\tfor (other= 1; other<*(const long *)(element+0x0C); other++)'),
    (SOURCE_AI_ACTORS_C, '\t\t\t\t\t&& (char)entry[0x0E]==entry0E && (char)entry[0x0F]==entry0F && (char)entry[0x0C]==entry0C)', '\t\t\t\t\t&& (char)entry[0x0E]==entry0E && (char)entry[0x0F]==entry0F || (char)entry[0x0C]==entry0C)'),
    # taken out: 0x89/4 and 0x9d/4 are the same indices as 0x88/4 and 0x9C/4
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tobject[0x98/4]= second[1];', '\t\tobject[0x98/5]= second[1];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (!quiet && flags && (long)object[0xD4/4]!=NONE)', '\tif (!quiet || flags && (long)object[0xD4/4]!=NONE)'),
    (SOURCE_MAIN_MAIN_C, '\tobject_index= *(const long *)(globals+0x11FC);', ''),
    (SOURCE_MAIN_MAIN_C, '\tif (((*(const unsigned long *)(object+4)>>8)&1) && *(const short *)(object+0x2C)!=NONE)', '\tif (((*(const unsigned long *)(object+4)>>8)&2) && *(const short *)(object+0x2C)!=NONE)'),
    (SOURCE_MAIN_MAIN_C, '\tif (!object_try_and_get_and_verify_type(object_index, 0xFFFFFFFF))', '\tif (!object_try_and_get_and_verify_type(object_index, 0x100000000))'),
    (SOURCE_MAIN_MAIN_C, '\t\t*(short *)(globals+0x11FA)= 0;\n\t\treturn NONE;\n\t}', '\t\t*(short *)(globals+0x11FA)= 0;\n\t\treturn 0;\n\t}'),
]
