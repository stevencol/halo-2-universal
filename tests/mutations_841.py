# mutations of milestone 841, written by make_mutations.py

TESTS = "test_small_841"

SOURCE_NETWORKING_GAME_VOICE_C = "source/networking/game_voice.c"
SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C = "source/game/game_unknown_0002b4a0.c"
SOURCE_SOUND_SOUND_MANAGER_C = "source/sound/sound_manager.c"

MUTATIONS = [
    (SOURCE_NETWORKING_GAME_VOICE_C, '\t\treturn 4;', '\t\treturn 5;'),
    (SOURCE_NETWORKING_GAME_VOICE_C, '\tif (player_index==NONE)', '\tif (player_index!=NONE)'),
    (SOURCE_NETWORKING_GAME_VOICE_C, '\tif (unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==1 && *(const unsigned long *)(entry+0x09)==0xBAD00000)', '\tif (unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==1 && *(const unsigned long *)(entry+0x09)!=0xBAD00000)'),
    (SOURCE_NETWORKING_GAME_VOICE_C, '\tif (unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==1 && *(const unsigned long *)(entry+0x09)==0xBAD00000)\n\t{\n\t\treturn 0;\n\t}\n\tstate= *(const long *)(session+0x741C);', '\tif (unknown_00467214!=NONE && online_tasks_get_logon_state(unknown_00467214)==1 && *(const unsigned long *)(entry+0x09)==0xBAD00000)\n\t{\n\t\treturn 1;\n\t}\n\tstate= *(const long *)(session+0x741C);'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\tstate= (byte *)cached_object_render_state_data->data+((state_index&0xFFFF)<<8);', '\tstate= (byte *)cached_object_render_state_data->data+((state_index&0x10000)<<8);'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\tstate[0xB8]= 0;', ''),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\tif (!always && !(((const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address)[0x1C]&1))', '\tif (!always || !(((const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address)[0x1C]&1))'),
    (SOURCE_GAME_GAME_UNKNOWN_0002B4A0_C, '\tif (state_index==NONE)', '\tif (state_index==0)'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tconst byte *entry= (const byte *)data->data+absolute_index*0x18;', '\t\tconst byte *entry= (const byte *)data->data+absolute_index*0x19;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tif (*(const long *)(entry+0x0C)==thing && ((entry[4]>>5)&1))', '\t\tif (*(const long *)(entry+0x0C)==thing && ((entry[5]>>5)&1))'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\t\tabsolute_index= data_next_absolute_index(data, absolute_index+1);', '\t\tabsolute_index= data_next_absolute_index(data, absolute_index+2);'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tlong absolute_index= data_next_absolute_index(data, 0);', '\tlong absolute_index= data_next_absolute_index(data, 1);'),
]
