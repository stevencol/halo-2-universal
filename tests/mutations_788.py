# mutations of milestone 788, written by make_mutations.py

TESTS = "test_small_788"

SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_UNITS_UNITS_C = "source/units/units.c"

MUTATIONS = [
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (index!=NONE && index>=0 && index<OBJECT_UNKNOWN11E_COUNT(object) && OBJECT_UNKNOWN11E(object)[index*8]==3)', '\tif (index!=NONE && index>=0 || index<OBJECT_UNKNOWN11E_COUNT(object) && OBJECT_UNKNOWN11E(object)[index*8]==3)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\treturn *(const long *)(OBJECT_UNKNOWN11E(object)+index*8+4);', '\t\treturn *(const long *)(OBJECT_UNKNOWN11E(object)+index*8+5);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\treturn *(const long *)(OBJECT_UNKNOWN11E(object)+index*8+4);\n\t}\n\n\treturn NONE;\n}\n\n/* 0x000fd460: that entry, if it is there, gets the kind 3 and a long */', '\t\treturn *(const long *)(OBJECT_UNKNOWN11E(object)+index*8+4);\n\t}\n\n\treturn 0;\n}\n\n/* 0x000fd460: that entry, if it is there, gets the kind 3 and a long */'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tconst byte *object= OBJECT_DATA(object_index);\n\tlong index= *(const long *)(object+0x14C);\n', '\tconst byte *object= OBJECT_DATA(object_index);\n\tlong index= *(const long *)(object+0x14d);\n'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (index!=NONE && index>=0 && index<OBJECT_UNKNOWN11E_COUNT(object))', '\tif (index!=NONE || index>=0 && index<OBJECT_UNKNOWN11E_COUNT(object))'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tOBJECT_UNKNOWN11E(object)[index*8]= 3;', '\t\tOBJECT_UNKNOWN11E(object)[index*9]= 3;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tbyte *object= OBJECT_DATA(object_index);\n\tlong index= *(const long *)(object+0x14C);\n', '\tbyte *object= OBJECT_DATA(object_index);\n\tlong index= *(const long *)(object+0x14d);\n'),
    # taken out: 0x81/4 and 0x171/4 are the same indices as 0x80/4 and 0x170/4
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (object[0x14/4]==NONE)', '\tif (object[0x14/4]==0)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tobject[0x84/4]= ((const unsigned long *)global_up3d)[2];', '\t\tobject[0x84/5]= ((const unsigned long *)global_up3d)[2];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tobject[0x74/4]= values[1];', '\t\tobject[0x74/5]= values[1];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tobject[0x170/4]= *(const unsigned long *)(definition+0x16C);', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif ((object[1]>>3)&1)', '\tif ((object[1]>>3)&2)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tconst byte *definition= (const byte *)(uintptr_t)global_tag_instances[object[0]&0xFFFF].address;', '\tconst byte *definition= (const byte *)(uintptr_t)global_tag_instances[object[0]&0x10000].address;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tlong sound_index= *(const long *)((const byte *)(uintptr_t)global_tag_instances[definition_index&0xFFFF].address+0x138);\n\n\tif (sound_index!=NONE)\n\t{\n\t\tsound_unknown_001896c0(sound_index, 1.0f);', '\tlong sound_index= *(const long *)((const byte *)(uintptr_t)global_tag_instances[definition_index&0xFFFF].address+0x138);\n\n\tif (sound_index==NONE)\n\t{\n\t\tsound_unknown_001896c0(sound_index, 1.0f);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tlong sound_index= *(const long *)((const byte *)(uintptr_t)global_tag_instances[definition_index&0xFFFF].address+0x138);', '\tlong sound_index= *(const long *)((const byte *)(uintptr_t)global_tag_instances[definition_index&0xFFFF].address+0x139);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tglobal_tag_instances[*(const long *)OBJECT_DATA(object_index)&0xFFFF].address+0x138);', '\t\tglobal_tag_instances[*(const long *)OBJECT_DATA(object_index)&0x10000].address+0x138);'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tglobal_tag_instances[*(const long *)OBJECT_DATA(object_index)&0xFFFF].address+0x138);\n\n\tif (sound_index!=NONE)\n\t{\n\t\tsound_unknown_001896c0(sound_index, 1.0f);', '\t\tglobal_tag_instances[*(const long *)OBJECT_DATA(object_index)&0xFFFF].address+0x138);\n\n\tif (sound_index==NONE)\n\t{\n\t\tsound_unknown_001896c0(sound_index, 1.0f);'),
    (SOURCE_UNITS_UNITS_C, '\treturn (flags>>1)&1 && !((flags>>2)&1);', '\treturn (flags>>1)&2 && !((flags>>2)&1);'),
    (SOURCE_UNITS_UNITS_C, '\tshort weapon= (signed char)unit[0x212+hand];', '\tshort weapon= (signed char)unit[0x213+hand];'),
    (SOURCE_UNITS_UNITS_C, '\tif (weapon==NONE || (weapon_index= ((const long *)(unit+UNIT_WEAPONS_OFFSET))[weapon])==NONE)\n\t{\n\t\treturn FALSE;\n\t}\n\tflags= UNIT_OBJECT_GET(weapon_index)[0x16C];', '\tif (weapon==NONE || (weapon_index= ((const long *)(unit+UNIT_WEAPONS_OFFSET))[weapon])==NONE)\n\t{\n\t\treturn TRUE;\n\t}\n\tflags= UNIT_OBJECT_GET(weapon_index)[0x16C];'),
    (SOURCE_UNITS_UNITS_C, '\tif (weapon==NONE || (weapon_index= ((const long *)(unit+UNIT_WEAPONS_OFFSET))[weapon])==NONE)', '\tif (weapon==0 || (weapon_index= ((const long *)(unit+UNIT_WEAPONS_OFFSET))[weapon])==NONE)'),
    (SOURCE_UNITS_UNITS_C, '\tfor (index= 0; index<16; index++)', '\tfor (index= 0; index<16-1; index++)'),
    (SOURCE_UNITS_UNITS_C, '\tkind= *(const short *)(definition+0x1F0);', ''),
    (SOURCE_UNITS_UNITS_C, '\treturn (kind==4 && *(const short *)(definition+0x218)==1) || kind==5;', '\treturn (kind==4 && *(const short *)(definition+0x218)==1) || kind==6;'),
    (SOURCE_UNITS_UNITS_C, '\tif (!((object[0x10A]>>2)&1))', '\tif (!((object[0x10b]>>2)&1))'),
    (SOURCE_UNITS_UNITS_C, '\tif (!((object[0x10A]>>2)&1))\n\t{\n\t\treturn FALSE;\n\t}\n\tkind= *(const short *)(definition+0x1F0);', '\tif (!((object[0x10A]>>2)&1))\n\t{\n\t\treturn TRUE;\n\t}\n\tkind= *(const short *)(definition+0x1F0);'),
    (SOURCE_UNITS_UNITS_C, '\tcount= *(const long *)(*(const byte *const *)(data+0x04)+0x44);', '\tcount= *(const long *)(*(const byte *const *)(data+0x04)+0x45);'),
    (SOURCE_UNITS_UNITS_C, '\tunit[0x387]= 0;', ''),
    (SOURCE_UNITS_UNITS_C, '\t\tif (data[index*0xD8+0xB90])', '\t\tif (data[index*0xd9+0xB90])'),
    (SOURCE_UNITS_UNITS_C, '\tfor (index= 0; index<count; index++)', '\tfor (index= 1; index<count; index++)'),
    (SOURCE_UNITS_UNITS_C, '\tif (bit<0 || bit>=32)', '\tif (bit<0 || bit>=33)'),
    (SOURCE_UNITS_UNITS_C, '\tif (bit<0 || bit>=32)\n\t{\n\t\treturn FALSE;\n\t}\n\tif (set)', '\tif (bit<0 || bit>=32)\n\t{\n\t\treturn TRUE;\n\t}\n\tif (set)'),
    (SOURCE_UNITS_UNITS_C, '\tunsigned long *flags= (unsigned long *)(UNIT_OBJECT_GET(unit_index)+0x3B4);', '\tunsigned long *flags= (unsigned long *)(UNIT_OBJECT_GET(unit_index)+0x3b5);'),
    (SOURCE_UNITS_UNITS_C, '\t}\n\treturn TRUE;\n}', '\t}\n\treturn FALSE;\n}'),
]
