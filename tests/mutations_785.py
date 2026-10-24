# mutations of milestone 785, written by make_mutations.py

TESTS = "test_small_785"

SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_OBJECTS_LIQUID_C = "source/objects/liquid.c"

MUTATIONS = [
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tparent_index= object_index;\n\t\tobject_index= *(const long *)(OBJECT_DATA(object_index)+0x14);\n\t}', '\t\tparent_index= object_index;\n\t\tobject_index= *(const long *)(OBJECT_DATA(object_index)+0x15);\n\t}'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tconst struct object_header_datum *headers= (const struct object_header_datum *)object_header_data->data;\n\tlong parent_index= NONE;\n\n\twhile (object_index!=NONE)\n\t{\n\t\tparent_index= object_index;\n\t\tobject_index= *(const long *)(OBJECT_DATA(object_index)+0x14);', '\tconst struct object_header_datum *headers= (const struct object_header_datum *)object_header_data->data;\n\tlong parent_index= NONE;\n\n\twhile (object_index!=0)\n\t{\n\t\tparent_index= object_index;\n\t\tobject_index= *(const long *)(OBJECT_DATA(object_index)+0x14);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\treturn (headers[parent_index&0xFFFF].flags>>6)&1;', '\treturn (headers[parent_index&0x10000].flags>>6)&1;'),
    # taken out: parent_index= 0 differs only for NONE, which the game reads as the header 0xFFFF; number>=0x7FFE and number<=0 set the same value (0x7FFE, 0)
    (SOURCE_OBJECTS_OBJECTS_C, '\tbyte *data= object+*(const short *)(object+0x11A);', '\tbyte *data= object+*(const short *)(object+0x11b);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tresult[0]= entry[0];', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tresult[1]= entry[1];', '\tresult[2]= entry[1];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tresult[2]= entry[2];', '\tresult[2]= entry[3];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (index<0 || index>=count)', '\tif (index<0 || index>count)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\treturn other_index!=NONE && previous_index==other_index;', '\treturn other_index!=0 && previous_index==other_index;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\twhile (object_index!=NONE && previous_index!=other_index)', '\twhile (object_index==NONE && previous_index!=other_index)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tprevious_index= object_index;', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tlong previous_index= NONE;', '\tlong previous_index= 0;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (object_index!=NONE)', '\tif (object_index==NONE)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tunsigned long *flags= (unsigned long *)(OBJECT_DATA(object_index)+0x04);', '\t\tunsigned long *flags= (unsigned long *)(OBJECT_DATA(object_index)+0x5);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tlink= (long *)(object+0x0C);', '\t\tlink= (long *)(object+0xd);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\n\tif (*link==NONE)\n\t{', '\n\tif (*link==0)\n\t{'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tif (*link==NONE)', '\t\tif (*link!=NONE)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tif (*link==object_index)', '\t\tif (*link!=object_index)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (unknown_004de300[index&0xFFFF]==unknown_004de2fc)', '\tif (unknown_004de300[index&0xFFFF]!=unknown_004de2fc)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (unknown_004de300[index&0xFFFF]==unknown_004de2fc)\n\t{\n\t\treturn FALSE;\n\t}\n\tunknown_004de300[index&0xFFFF]= unknown_004de2fc;', '\tif (unknown_004de300[index&0xFFFF]==unknown_004de2fc)\n\t{\n\t\treturn TRUE;\n\t}\n\tunknown_004de300[index&0xFFFF]= unknown_004de2fc;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tunknown_004de300[index&0xFFFF]= unknown_004de2fc;\n\treturn TRUE;\n}', '\tunknown_004de300[index&0xFFFF]= unknown_004de2fc;\n\treturn FALSE;\n}'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tunknown_004de300[index&0xFFFF]= unknown_004de2fc;', '\tunknown_004de300[index&0x10000]= unknown_004de2fc;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tnumber= 0x7FFE;', '\t\tnumber= 0x7fff;'),
    (SOURCE_UNITS_UNITS_C, '\t\t\tunknown_004de2d0[index]= NONE;', ''),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const short *)(object+0xAC)==NONE)', '\tif (*(const short *)(object+0xad)==NONE)'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (unknown_004de2d0[index]==object_index)', '\t\tif (unknown_004de2d0[index]!=object_index)'),
    (SOURCE_UNITS_UNITS_C, '\tbyte *object= (byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;', '\tbyte *object= (byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0x10000].datum;'),
    (SOURCE_OBJECTS_LIQUID_C, '\tlong which)\n{\n\tif (!liquid_data || !*((const byte *)liquid_data+0x29) || liquid_index==NONE)\n\t{\n\t\treturn NULL;', '\tlong which)\n{\n\tif (!liquid_data || !*((const byte *)liquid_data+0x29) || liquid_index!=NONE)\n\t{\n\t\treturn NULL;'),
    (SOURCE_OBJECTS_LIQUID_C, '\treturn (byte *)liquid_data->data+(liquid_index&0xFFFF)*0x110+0x20+which*0x50;', '\treturn (byte *)liquid_data->data+(liquid_index&0xFFFF)*0x110+0x20+which*0x51;'),
    (SOURCE_OBJECTS_LIQUID_C, '\t\treturn NONE;', '\t\treturn 0;'),
    (SOURCE_OBJECTS_LIQUID_C, '\tlong liquid_index)\n{\n\tif (!liquid_data || !*((const byte *)liquid_data+0x29) || liquid_index==NONE)\n\t{\n\t\treturn NONE;', '\tlong liquid_index)\n{\n\tif (!liquid_data || !*((const byte *)liquid_data+0x29) || liquid_index!=NONE)\n\t{\n\t\treturn NONE;'),
    (SOURCE_OBJECTS_LIQUID_C, '\treturn *(const long *)((const byte *)liquid_data->data+(liquid_index&0xFFFF)*0x110+0x0C);', '\treturn *(const long *)((const byte *)liquid_data->data+(liquid_index&0xFFFF)*0x111+0x0C);'),
]
