# mutations of milestone 805, written by make_mutations.py

TESTS = "test_small_805"

SOURCE_EFFECTS_EFFECTS_C = "source/effects/effects.c"
SOURCE_MATH_REAL_MATH_C = "source/math/real_math.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"

MUTATIONS = [
    (SOURCE_EFFECTS_EFFECTS_C, '\t\tif (*(const long *)(entries+index*8+4)==effect_index && entries[index*8]==2)', '\t\tif (*(const long *)(entries+index*8+5)==effect_index && entries[index*8]==2)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\teffect= (const byte *)effect_data->data+(effect_index&0xFFFF)*effect_data->size;', ''),
    (SOURCE_EFFECTS_EFFECTS_C, '\tif (effect_index==NONE || (effect_index&0xFFFF)>=effect_data->first_unallocated)', '\tif (effect_index!=NONE || (effect_index&0xFFFF)>=effect_data->first_unallocated)'),
    (SOURCE_EFFECTS_EFFECTS_C, '\tcount= (unsigned long)(long)*(const short *)(object+0x11C)>>3;', '\tcount= (unsigned long)(long)*(const short *)(object+0x11C)>>4;'),
    (SOURCE_MATH_REAL_MATH_C, '\t\tif ((bits&0x7F800000)==0x7F800000 || !(point[index]>=-50000.0f) || !(50000.0f>=point[index]))', '\t\tif ((bits&0x7F800000)==0x7F800000 || !(point[index]>=-50000.0f) || !(50000.0f>point[index]))'),
    # taken out: return 2 is the same TRUE as return 1
    (SOURCE_MATH_REAL_MATH_C, '\tlong index;\n\n\tfor (index= 0; index<3; index++)\n\t{\n\t\tunsigned long bits;', '\tlong index;\n\n\tfor (index= 1; index<3; index++)\n\t{\n\t\tunsigned long bits;'),
    (SOURCE_MATH_REAL_MATH_C, '\t\tif ((bits&0x7F800000)==0x7F800000 || !(point[index]>=-50000.0f) || !(50000.0f>=point[index]))\n\t\t{\n\t\t\treturn 0;\n\t\t}\n\t}', '\t\tif ((bits&0x7F800000)==0x7F800000 || !(point[index]>=-50000.0f) || !(50000.0f>=point[index]))\n\t\t{\n\t\t\treturn 1;\n\t\t}\n\t}'),
    (SOURCE_PHYSICS_HAVOK_C, '\tdata[0x10]= (byte)(flags>>14)&2;\n\treturn TRUE;\n}', '\tdata[0x10]= (byte)(flags>>14)&2;\n\treturn FALSE;\n}'),
    (SOURCE_PHYSICS_HAVOK_C, '\tdata[0x10]= (byte)(flags>>14)&2;', '\tdata[0x10]= (byte)(flags>>14)&3;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tdata[0x0C]= value0C;', '\tdata[0xd]= value0C;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tdata[0x10]= 0;\n\tdata[0x11]= 0;\n\t*(short *)(data+0x14)= 0;', '\tdata[0x10]= 0;\n\t*(short *)(data+0x14)= 0;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tboolean result= FALSE;', '\tboolean result= TRUE;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif ((signed char)(byte)(1UL<<(header->type&0x1F))<0', '\tif ((signed char)(byte)(1UL<<(header->type&0x1F))<1'),
    (SOURCE_OBJECTS_OBJECTS_C, '\treturn (cluster_bits[header->cluster_index>>5]&(1UL<<(header->cluster_index&0x1F)))!=0;', '\treturn (cluster_bits[header->cluster_index>>5]&(1UL<<(header->cluster_index&0x20)))!=0;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (header->cluster_index==NONE)', '\tif (header->cluster_index==0)'),
]
