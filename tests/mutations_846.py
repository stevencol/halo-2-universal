# mutations of milestone 846, written by make_mutations.py

TESTS = "test_small_846"

SOURCE_SOUND_SOUND_MANAGER_C = "source/sound/sound_manager.c"
SOURCE_GAME_GAME_UNKNOWN_00134D90_C = "source/game/game_unknown_00134d90.c"
SOURCE_CSERIES_UNKNOWN_0051F40C_C = "source/cseries/unknown_0051f40c.c"
SOURCE_OBJECTS_OBJECTS_C = "source/objects/objects.c"

MUTATIONS = [
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tentry[4]|= 1;', '\tentry[5]|= 1;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tindex= sound_unknown_00188f90(tag_index, object_index, NONE, 0);', ''),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t}\n\tentry= (byte *)unknown_004ed28c->data+(index&0xFFFF)*0x18;\n\tentry[4]|= 1;', '\t}\n\tentry= (byte *)unknown_004ed28c->data+(index&0x10000)*0x18;\n\tentry[4]|= 1;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tbyte *entry;\n\n\tif (tag_index==NONE)\n\t{\n\t\treturn NONE;', '\tbyte *entry;\n\n\tif (tag_index!=NONE)\n\t{\n\t\treturn NONE;'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tgroup= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;', '\tgroup= (const byte *)(uintptr_t)global_tag_instances[tag_index&0x10000].address;'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tif (index==NONE)', '\tif (index==0)'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\tif (index<0 || index>=*(const long *)(group+0x44))', '\tif (index<1 || index>=*(const long *)(group+0x44))'),
    (SOURCE_GAME_GAME_UNKNOWN_00134D90_C, '\treturn *(byte *const *)(group+0x48)+index*0x74;', '\treturn *(byte *const *)(group+0x49)+index*0x74;'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\t\tif ((flags&0x10) && ((flags>>6)&0xFFF)==(unsigned long)absolute_index)', '\t\tif ((flags&0x11) && ((flags>>6)&0xFFF)==(unsigned long)absolute_index)'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\tif (object_index==NONE)', '\tif (object_index==0)'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\tabsolute_index= object_index&0xFFFF;', '\tabsolute_index= object_index&0x10000;'),
    (SOURCE_CSERIES_UNKNOWN_0051F40C_C, '\t\tunsigned long flags= unknown_0051f40c[8*index];', '\t\tunsigned long flags= unknown_0051f40c[9*index];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tcase 3:', '\tcase 4:'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tcase 0:', '\tcase 1:'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (definition_index==NONE)', '\tif (definition_index==0)'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tcase 2:', '\tcase 3:'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tobject_iterator_new(&iterator, 0xFFFFFFFF, 0);', '\tobject_iterator_new(&iterator, 0xFFFFFFFF, 1);'),
    # taken out: the bit 0 is already set there
    (SOURCE_OBJECTS_OBJECTS_C, '\t\tobject_index= object_get_ultimate_parent(object_index);', ''),
    (SOURCE_OBJECTS_OBJECTS_C, '\t\theader= &((struct object_header_datum *)object_header_data->data)[object_index&0xFFFF];', '\t\theader= &((struct object_header_datum *)object_header_data->data)[object_index&0x10000];'),
    (SOURCE_OBJECTS_OBJECTS_C, '\tif (header->flags&0x80)', '\tif (header->flags&0x81)'),
]
