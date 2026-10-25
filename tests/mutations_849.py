# mutations of milestone 849, written by make_mutations.py

TESTS = "test_small_849"

SOURCE_GAME_GAME_UNKNOWN_0013BF00_C = "source/game/game_unknown_0013bf00.c"
SOURCE_SOUND_SOUND_MANAGER_C = "source/sound/sound_manager.c"
SOURCE_PHYSICS_HAVOK_C = "source/physics/havok.c"

MUTATIONS = [
    (SOURCE_GAME_GAME_UNKNOWN_0013BF00_C, '\t\t\t\ttitle[1]= NONE;', ''),
    (SOURCE_GAME_GAME_UNKNOWN_0013BF00_C, '\t\t\tif ((real)title[1]*time->seconds_per_tick>=definition[8]+definition[7])', '\t\t\tif ((real)title[1]*time->seconds_per_tick>definition[8]+definition[7])'),
    (SOURCE_GAME_GAME_UNKNOWN_0013BF00_C, '\t\t\t\tfade= 0.0f;', ''),
    (SOURCE_GAME_GAME_UNKNOWN_0013BF00_C, '\t\t\t\tfade= 1.0f;', ''),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tplayer[0xB5]= 0;', ''),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tplayer[0xB6]= 0;', '\tplayer[0xb7]= 0;'),
    (SOURCE_SOUND_SOUND_MANAGER_C, '\tplayer[0xB4]= 1;', '\tplayer[0xb5]= 1;'),
    # taken out: the byte past the 0xAC is set after
    (SOURCE_PHYSICS_HAVOK_C, '\tlong index= *(const short *)(iterator+0x08)+1;', '\tlong index= *(const short *)(iterator+0x08)+2;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tentry= mode_entry ? havok_unknown_001dd560(mode_entry+4, weapon_class, 0x14) : NULL;', ''),
    (SOURCE_PHYSICS_HAVOK_C, '\t\t\tlong weapon_class= current==0 || current==2 ? *(const long *)(iterator+0x10) : 0x030000D9;', '\t\t\tlong weapon_class= current==0 && current==2 ? *(const long *)(iterator+0x10) : 0x030000D9;'),
    (SOURCE_PHYSICS_HAVOK_C, '\t\twhile (*step<4 && !entry)', '\t\twhile (*step<=4 && !entry)'),
]
