# mutations of milestone 786, written by make_mutations.py

TESTS = "test_small_786"

SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    (SOURCE_UNITS_UNITS_C, '\tfor (entry= unknown_00467430; entry<unknown_00467430+5; entry++)', '\tfor (entry= unknown_00467430; entry<unknown_00467430+5-1; entry++)'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (*entry==name)\n\t\t{\n\t\t\treturn TRUE;\n\t\t}\n\t}', '\t\tif (*entry==name)\n\t\t{\n\t\t\treturn FALSE;\n\t\t}\n\t}'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (*entry==name)', '\t\tif (*entry!=name)'),
    (SOURCE_UNITS_UNITS_C, '\t\t}\n\t}\n\n\treturn FALSE;\n}\n\n/* 0x000c8f60: the first child of an object that is a unit with the short', '\t\t}\n\t}\n\n\treturn TRUE;\n}\n\n/* 0x000c8f60: the first child of an object that is a unit with the short'),
    (SOURCE_UNITS_UNITS_C, '\t\tchild_index= *(const long *)(child+0x0C);', ''),
    (SOURCE_UNITS_UNITS_C, '\t\tif (UNIT_OBJECT_IS_UNIT(child) && *(const short *)(child+0x1FC)==number)', '\t\tif (UNIT_OBJECT_IS_UNIT(child) || *(const short *)(child+0x1FC)==number)'),
    (SOURCE_UNITS_UNITS_C, '\twhile (child_index!=NONE)', '\twhile (child_index==NONE)'),
    (SOURCE_UNITS_UNITS_C, '\tlong child_index= *(const long *)(UNIT_OBJECT_GET(object_index)+0x10);', '\tlong child_index= *(const long *)(UNIT_OBJECT_GET(object_index)+0x11);'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (index!=(signed char)unit[0x212] && index!=(signed char)unit[0x213]', '\t\tif (index!=(signed char)unit[0x212] && index==(signed char)unit[0x213]'),
    (SOURCE_UNITS_UNITS_C, '\tlong index;\n\n\tfor (index= 0; index<4; index++)\n\t{\n\t\tif (index!=(signed char)unit[0x212] && index!=(signed char)unit[0x213]', '\tlong index;\n\n\tfor (index= 1; index<4; index++)\n\t{\n\t\tif (index!=(signed char)unit[0x212] && index!=(signed char)unit[0x213]'),
    (SOURCE_UNITS_UNITS_C, '\t\t}\n\t}\n\n\treturn NONE;\n}\n\n/* 0x000cc3c0: whether what a unit has at its short at 0x12A is set and has', '\t\t}\n\t}\n\n\treturn 0;\n}\n\n/* 0x000cc3c0: whether what a unit has at its short at 0x12A is set and has'),
    (SOURCE_UNITS_UNITS_C, '\t\t\t&& ((const long *)(unit+UNIT_WEAPONS_OFFSET))[index]!=NONE)', '\t\t\t&& ((const long *)(unit+UNIT_WEAPONS_OFFSET))[index]==NONE)'),
    (SOURCE_UNITS_UNITS_C, '\treturn UNIT_UNKNOWN12A_IS(UNIT_UNKNOWN12A(UNIT_OBJECT_GET(unit_index)), 0x0600008C);', '\treturn UNIT_UNKNOWN12A_IS(UNIT_UNKNOWN12A(UNIT_OBJECT_GET(unit_index)), 0x600008d);'),
    (SOURCE_UNITS_UNITS_C, '\treturn UNIT_UNKNOWN12A_IS(data, 0x0D000042) || UNIT_UNKNOWN12A_IS(data, 0x0C000043);', '\treturn UNIT_UNKNOWN12A_IS(data, 0x0D000042) || UNIT_UNKNOWN12A_IS(data, 0xc000044);'),
    (SOURCE_UNITS_UNITS_C, '\treturn index>=0 && index<*(const long *)(definition+0x1C8) && (UNIT_DEFINITION_UNKNOWN1C8(definition, index)>>2)&1;', '\treturn index>=0 && index<*(const long *)(definition+0x1C8) && (UNIT_DEFINITION_UNKNOWN1C8(definition, index)>>2)&2;'),
    (SOURCE_UNITS_UNITS_C, '\treturn index>=0 && index<*(const long *)(definition+0x1C8) && (UNIT_DEFINITION_UNKNOWN1C8(definition, index)>>3)&1;', '\treturn index>=0 && index<*(const long *)(definition+0x1C8) && (UNIT_DEFINITION_UNKNOWN1C8(definition, index)>>4)&1;'),
    (SOURCE_UNITS_UNITS_C, '\tunit[0x23C]= (signed char)index;', '\tunit[0x23d]= (signed char)index;'),
    (SOURCE_UNITS_UNITS_C, '\tunit[index+0x23E]+= amount;', '\tunit[index+0x23f]+= amount;'),
    (SOURCE_UNITS_UNITS_C, '\tsigned char *unit= (signed char *)UNIT_OBJECT_GET(unit_index);', ''),
    (SOURCE_UNITS_UNITS_C, '\treturn unit[index+0x23E];', '\treturn unit[index+0x23f];'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (weapons[index]!=NONE && *(const long *)UNIT_OBJECT_GET(weapons[index])==tag_index)', '\t\tif (weapons[index]==NONE && *(const long *)UNIT_OBJECT_GET(weapons[index])==tag_index)'),
    (SOURCE_UNITS_UNITS_C, '\tlong index;\n\n\tfor (index= 0; index<4; index++)\n\t{\n\t\tif (weapons[index]!=NONE && *(const long *)UNIT_OBJECT_GET(weapons[index])==tag_index)', '\tlong index;\n\n\tfor (index= 0; index<4-1; index++)\n\t{\n\t\tif (weapons[index]!=NONE && *(const long *)UNIT_OBJECT_GET(weapons[index])==tag_index)'),
    (SOURCE_UNITS_UNITS_C, '\t\tif (weapons[index]!=NONE && *(const long *)UNIT_OBJECT_GET(weapons[index])==tag_index)\n\t\t{\n\t\t\treturn TRUE;\n\t\t}\n\t}', '\t\tif (weapons[index]!=NONE && *(const long *)UNIT_OBJECT_GET(weapons[index])==tag_index)\n\t\t{\n\t\t\treturn FALSE;\n\t\t}\n\t}'),
    (SOURCE_UNITS_UNITS_C, '\t\t}\n\t}\n\n\treturn FALSE;\n}\n\n/* 0x000cfe40: the first weapon of a unit whose definition has the bit 3 of', '\t\t}\n\t}\n\n\treturn TRUE;\n}\n\n/* 0x000cfe40: the first weapon of a unit whose definition has the bit 3 of'),
    (SOURCE_UNITS_UNITS_C, '\t\t\t&& (*(const unsigned long *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(weapons[index]))+0x12C)>>3)&1)', '\t\t\t&& (*(const unsigned long *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(weapons[index]))+0x12d)>>3)&1)'),
    (SOURCE_UNITS_UNITS_C, '\t{\n\t\tif (weapons[index]!=NONE\n\t\t\t&& (*(const unsigned long *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(weapons[index]))+0x12C)>>3)&1)', '\t{\n\t\tif (weapons[index]!=0\n\t\t\t&& (*(const unsigned long *)(UNIT_DEFINITION_GET(UNIT_OBJECT_GET(weapons[index]))+0x12C)>>3)&1)'),
    (SOURCE_UNITS_UNITS_C, '\t\t}\n\t}\n\n\treturn NONE;\n}\n\n/* 0x000d05c0: the next of a list of objects (each has the next at 0x0C;', '\t\t}\n\t}\n\n\treturn 0;\n}\n\n/* 0x000d05c0: the next of a list of objects (each has the next at 0x0C;'),
    (SOURCE_UNITS_UNITS_C, '\tshort index;\n\n\tfor (index= 0; index<4; index++)\n\t{\n\t\tif (weapons[index]!=NONE', '\tshort index;\n\n\tfor (index= 1; index<4; index++)\n\t{\n\t\tif (weapons[index]!=NONE'),
    (SOURCE_UNITS_UNITS_C, '\t\tlong object_index= *(const long *)(iterator+0x0C);', '\t\tlong object_index= *(const long *)(iterator+0xd);'),
    (SOURCE_UNITS_UNITS_C, '\twhile (*(const long *)(iterator+0x0C)!=NONE)', '\twhile (*(const long *)(iterator+0x0C)==NONE)'),
    # taken out: 0x181/4 is the same index as 0x180/4
    (SOURCE_UNITS_UNITS_C, '\tunit[0x15C/4+2]= point[2];', '\tunit[0x15C/5+2]= point[2];'),
    (SOURCE_UNITS_UNITS_C, '\tunit[0x180/4+2]= point[2];', '\tunit[0x180/4+2]= point[3];'),
    (SOURCE_UNITS_UNITS_C, '\tunit[0x15C/4]= point[0];', '\tunit[0x15C/4]= point[1];'),
    (SOURCE_UNITS_UNITS_C, '\tif (!UNIT_OBJECT_IS_UNIT(parent))\n\t{\n\t\treturn FALSE;\n\t}\n', '\tif (!UNIT_OBJECT_IS_UNIT(parent))\n\t{\n\t\treturn TRUE;\n\t}\n'),
    (SOURCE_UNITS_UNITS_C, '\n\tif (object_index==NONE)\n\t{', '\n\tif (object_index!=NONE)\n\t{'),
    (SOURCE_UNITS_UNITS_C, '\tif (object_index==NONE)\n\t{\n\t\treturn FALSE;\n\t}\n\tobject= UNIT_OBJECT_GET(object_index);', '\tif (object_index==NONE)\n\t{\n\t\treturn TRUE;\n\t}\n\tobject= UNIT_OBJECT_GET(object_index);'),
    (SOURCE_UNITS_UNITS_C, '\tif (*(const long *)(object+0x14)==NONE)', '\tif (*(const long *)(object+0x14)!=NONE)'),
]
