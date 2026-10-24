# mutations of milestone 801, written by make_mutations.py

TESTS = "test_small_801"

SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"

MUTATIONS = [
    (SOURCE_PHYSICS_HAVOK_C, '\tfor (index= 0; index<longs; index++)', '\tfor (index= 0; index<longs-1; index++)'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tresult[index]= bits[index]&mask[index];', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\tlong longs= (count+0x1F)>>5;', '\tlong longs= (count+0x1F)>>6;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tif (!((1UL<<(kind&0x1F))&0x1883))', '\tif (!((1UL<<(kind&0x20))&0x1883))'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\treturn *(const long *)unknown_0051e9a0<0x200;', '\t\treturn *(const long *)unknown_0051e9a0<=0x200;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tbyte kind= *(const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;', '\tbyte kind= *(const byte *)(uintptr_t)global_tag_instances[tag_index&0x10000].address;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\treturn TRUE;', '\t\treturn FALSE;'),
    (SOURCE_PHYSICS_HAVOK_C, '\theader[0]= 0x62706170;', '\theader[0]= 0x62706171;'),
    # taken out: header[1]= 0 instead of header[0]= 0 does the same (header[0] is written again after)
    (SOURCE_PHYSICS_HAVOK_C, '\t\t((byte *)header)[9]= data[5];', '\t\t((byte *)header)[10]= data[5];'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t((byte *)header)[11]= data[7];', '\t\t((byte *)header)[12]= data[7];'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\theader[10]= data[6];', '\t\theader[11]= data[6];'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\theader[8]= data[4];', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\t\theader[9]= data[5];', '\t\theader[10]= data[5];'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tmemset(header, 0, 0x0F);', '\t\tmemset(header, 0, 0x10);'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tbyte *entry= data+index*0x20;', '\t\tbyte *entry= data+index*0x21;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\tentry[0x10]= 0;', '\t\tentry[0x10]= 1;'),
    (SOURCE_PHYSICS_HAVOK_C, '\tfor (index= 2; index>=0; index--)', '\tfor (index= 2; index>=1; index--)'),
]
