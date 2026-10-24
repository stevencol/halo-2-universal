# mutations of milestone 790, written by make_mutations.py

TESTS = "test_small_790"

SOURCE_UNITS_UNITS_C = "source/units/units.c"
SOURCE_OBJECTS_OBJECT_EARLY_MOVERS_C = "source/objects/object_early_movers.c"
SOURCE_OBJECTS_OBJECT_UNKNOWN_0010A980_C = "source/objects/object_unknown_0010a980.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\tif (index<0 || index>=2)', '\tif (index<1 || index>=2)'),
    (SOURCE_UNITS_UNITS_C, '\tname= index ? 0x110000DC : 0x0F0000DB;', '\tname= index ? 0x110000dd : 0x0F0000DB;'),
    (SOURCE_UNITS_UNITS_C, '\tdefinition= UNIT_DEFINITION_GET(UNIT_OBJECT_GET(weapon_index));', ''),
    (SOURCE_UNITS_UNITS_C, '\tif (index<0 || index>=2)\n\t{\n\t\treturn 0;\n\t}\n\tdefinition= UNIT_DEFINITION_GET(UNIT_OBJECT_GET(weapon_index));', '\tif (index<0 || index>=2)\n\t{\n\t\treturn 1;\n\t}\n\tdefinition= UNIT_DEFINITION_GET(UNIT_OBJECT_GET(weapon_index));'),
    (SOURCE_UNITS_UNITS_C, '\t}\n\tfor (index= 0; index<4; index++)\n\t{', '\t}\n\tfor (index= 0; index<4-1; index++)\n\t{'),
    (SOURCE_UNITS_UNITS_C, '\t\t}\n\t}\n\n\treturn NONE;\n}\n\n/* 0x00106400: whether the first element (0x40 bytes) of the block at 0x2C8', '\t\t}\n\t}\n\n\treturn 0;\n}\n\n/* 0x00106400: whether the first element (0x40 bytes) of the block at 0x2C8'),
    (SOURCE_UNITS_UNITS_C, '\tif (unit_index==NONE)', '\tif (unit_index!=NONE)'),
    (SOURCE_UNITS_UNITS_C, '\tlong unit_index= (weapon[0x12C]&1) ? *(const long *)(weapon+0x154) : NONE;', '\tlong unit_index= (weapon[0x12C]&2) ? *(const long *)(weapon+0x154) : NONE;'),
    (SOURCE_UNITS_UNITS_C, '\treturn (always || weapon[0x20C]==2) && *(const short *)(element+0x06)==2 && *(const short *)(element+0x18)==1;', '\treturn (always || weapon[0x20C]==2) && *(const short *)(element+0x06)==3 && *(const short *)(element+0x18)==1;'),
    (SOURCE_UNITS_UNITS_C, '\telement= *(const byte *const *)(definition+0x2CC);', '\telement= *(const byte *const *)(definition+0x2cd);'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(definition+0x2C8)<=0)', '\tif (*(const long *)(definition+0x2c9)<=0)'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(definition+0x2C8)<=0)\n\t{\n\t\treturn FALSE;\n\t}\n\telement= *(const byte *const *)(definition+0x2CC);', '\tif (*(const long *)(definition+0x2C8)<=0)\n\t{\n\t\treturn TRUE;\n\t}\n\telement= *(const byte *const *)(definition+0x2CC);'),
    (SOURCE_UNITS_UNITS_C, '\treturn *(const short *)(data+0x36)!=NONE && (enough || kind);', '\treturn *(const short *)(data+0x36)!=NONE || (enough || kind);'),
    (SOURCE_UNITS_UNITS_C, '\tboolean kind= data[0x02]>0 && data[0x02]<7;', '\tboolean kind= data[0x02]>0 && data[0x02]<8;'),
    (SOURCE_UNITS_UNITS_C, '\tboolean enough= *(const real *)(data+0x40)>=0.0001f;', '\tboolean enough= *(const real *)(data+0x40)>0.0001f;'),
    (SOURCE_UNITS_UNITS_C, '\t\t}\n\t}\n\n\treturn FALSE;\n}\n\n/* whether three things from an offset of what an object has at its short at', '\t\t}\n\t}\n\n\treturn TRUE;\n}\n\n/* whether three things from an offset of what an object has at its short at'),
    (SOURCE_UNITS_UNITS_C, '\tfor (index= 0; index<13; index++)', '\tfor (index= 0; index<14; index++)'),
    (SOURCE_UNITS_UNITS_C, '\tif ((short)(*(const unsigned long *)(data+0x5C)>>16)==NONE)\n\t{\n\t\treturn FALSE;\n\t}\n\tfor (index= 0; index<13; index++)', '\tif ((short)(*(const unsigned long *)(data+0x5C)>>16)==NONE)\n\t{\n\t\treturn TRUE;\n\t}\n\tfor (index= 0; index<13; index++)'),
    (SOURCE_UNITS_UNITS_C, '\tif ((short)(*(const unsigned long *)(data+0x5C)>>16)==NONE)', '\tif ((short)(*(const unsigned long *)(data+0x5C)>>17)==NONE)'),
    (SOURCE_UNITS_UNITS_C, '\treturn UNIT_UNKNOWN33E_SET(data, 0x9C) || UNIT_UNKNOWN33E_SET(data, 0xBC);', '\treturn UNIT_UNKNOWN33E_SET(data, 0x9C) && UNIT_UNKNOWN33E_SET(data, 0xBC);'),
    (SOURCE_UNITS_UNITS_C, '\treturn UNIT_UNKNOWN33E_SET(data, 0xDC) && (data[0]>>7)&1;', '\treturn UNIT_UNKNOWN33E_SET(data, 0xDC) || (data[0]>>7)&1;'),
    (SOURCE_UNITS_UNITS_C, '\t\t*second= *(const long *)(data+0x7C);\n\t\treturn TRUE;\n\t}', '\t\t*second= *(const long *)(data+0x7C);\n\t\treturn FALSE;\n\t}'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(data+0x68)!=NONE && *(const long *)data!=NONE && *(const short *)(data+0x06)!=NONE)', '\tif (*(const long *)(data+0x68)!=NONE && *(const long *)data!=NONE && *(const short *)(data+0x06)==NONE)'),
    (SOURCE_UNITS_UNITS_C, '\t*second= NONE;\n\treturn FALSE;\n}', '\t*second= NONE;\n\treturn TRUE;\n}'),
    (SOURCE_UNITS_UNITS_C, '\t\t+ *(const long *)(*(const byte *const *)(data+0x04)+0x0C) + *(const signed char *)*(const byte *const *)(data+0x04));', '\t\t+ *(const long *)(*(const byte *const *)(data+0x04)+0xd) + *(const signed char *)*(const byte *const *)(data+0x04));'),
    (SOURCE_UNITS_UNITS_C, '\tlong place= ((((long)data[0x08]+0x1F)>>3&~3L) + (bit>>5)*4 + *(const short *)(*(const byte *const *)(data+0x04)+0x06)', '\tlong place= ((((long)data[0x08]+0x1F)>>3&~3L) + (bit>>6)*4 + *(const short *)(*(const byte *const *)(data+0x04)+0x06)'),
    (SOURCE_UNITS_UNITS_C, '\treturn (*(const unsigned long *)(*(const byte *const *)data+place)&(1UL<<(bit&0x1F)))!=0;', '\treturn (*(const unsigned long *)(*(const byte *const *)data+place)&(1UL<<(bit&0x1F)))!=1;'),
    (SOURCE_OBJECTS_OBJECT_EARLY_MOVERS_C, '\t\t\tresult[1]= values[1];', '\t\t\tresult[1]= values[2];'),
    (SOURCE_OBJECTS_OBJECT_EARLY_MOVERS_C, '\t\t\tresult[2]= values[2];', ''),
    (SOURCE_OBJECTS_OBJECT_EARLY_MOVERS_C, '\t\t\tresult[0]= values[0];', '\t\t\tresult[1]= values[0];'),
    (SOURCE_OBJECTS_OBJECT_EARLY_MOVERS_C, '\tif (object_index==NONE)', '\tif (object_index!=NONE)'),
    (SOURCE_OBJECTS_OBJECT_UNKNOWN_0010A980_C, '\tif (!((*(const unsigned short *)(object+0xC0)>>9)&1))', '\tif (!((*(const unsigned short *)(object+0xC0)>>10)&1))'),
    (SOURCE_OBJECTS_OBJECT_UNKNOWN_0010A980_C, '\tobject[0xC1]&= ~0x02;', ''),
    (SOURCE_OBJECTS_OBJECT_UNKNOWN_0010A980_C, '\tobject= (byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;', '\tobject= (byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0x10000].datum;'),
    (SOURCE_OBJECTS_OBJECT_UNKNOWN_0010A980_C, '\t}\n\tfor (index= 0; index<OBJECT_UNKNOWN_0010A980_COUNT; index++)\n\t{', '\t}\n\tfor (index= 0; index<OBJECT_UNKNOWN_0010A980_COUNT-1; index++)\n\t{'),
]
