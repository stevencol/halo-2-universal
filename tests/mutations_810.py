# mutations of milestone 810, written by make_mutations.py

TESTS = "test_small_810"

SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"
SOURCE_AI_ACTORS_C = "source/ai/actors.c"
SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C = "source/game/game_unknown_0002b4a0.c"

MUTATIONS = [
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tif ((object[0x348]>>3)&1)', '\t\tif ((object[0x348]>>4)&1)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tresult|= 0x40;', '\t\tresult|= 0x41;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\telse if (kind==11 && ((*(const unsigned long *)(object+0x12C)>>1)&1))', '\telse if (kind==11 && ((*(const unsigned long *)(object+0x12d)>>1)&1))'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif ((header->flags&0x10) || (object[4]&1))', '\tif ((header->flags&0x10) && (object[4]&1))'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tconst byte *element= *(const byte *const *)(tag+0x54)+(signed char)object[0xB1]*0x38;', '\t\tconst byte *element= *(const byte *const *)(tag+0x55)+(signed char)object[0xB1]*0x38;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tconst byte *object= OBJECT_DATA(object_index);\n\tconst byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address;\n\tconst byte *tag= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(definition+0x38)&0xFFFF].address;', '\tconst byte *object= OBJECT_DATA(object_index);\n\tconst byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0x10000].address;\n\tconst byte *tag= (const byte *)(uintptr_t)global_tag_instances[*(const long *)(definition+0x38)&0xFFFF].address;'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif ((signed char)object[0xB1]==NONE || kind!=1)', '\tif ((signed char)object[0xB1]==NONE || kind!=2)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tvalues[index]= entry==NONE ? 0 : *(const short *)(*(const byte *const *)(element+0x18)+entry*0x14+0x10);', '\t\tvalues[index]= entry!=NONE ? 0 : *(const short *)(*(const byte *const *)(element+0x18)+entry*0x14+0x10);'),
    (SOURCE_AI_ACTORS_C, '\t\treal y= point[1]-points[index*2+1];', '\t\treal y= point[2]-points[index*2+1];'),
    (SOURCE_AI_ACTORS_C, '\t\t\tif (cross>0.0f && cross*cross>length*squared)', '\t\t\tif (cross>0.0f || cross*cross>length*squared)'),
    (SOURCE_AI_ACTORS_C, '\t\treal x= point[0]-points[index*2];', '\t\treal x= point[0]-points[index*3];'),
    (SOURCE_AI_ACTORS_C, '\t{\n\t\tlong next= index+1>=count ? 0 : index+1;\n\t\treal x= point[0]-points[index*2];', '\t{\n\t\tlong next= index+2>=count ? 0 : index+1;\n\t\treal x= point[0]-points[index*2];'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\t\tunknown_004ba028= 0.7f;', ''),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\t\tunknown_004ba01c= 3;', ''),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\telse if (kind==4)', '\telse if (kind!=4)'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\t\tunknown_004670e4= 0.3f;', ''),
]
