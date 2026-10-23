# mutations of milestone 774: one or two lines of each function

TESTS = "test_small_774"

FILE = "source/game/game_unknown_001a1c8e.c"

MUTATIONS = [
    (FILE, "\tlong index= player->unknown2E8+player->value_count;", "\tlong index= player->value_count;"),
    (FILE, "\tif (index<GAME_UNKNOWN_001A1C8E_VALUES)", "\tif (index<=GAME_UNKNOWN_001A1C8E_VALUES)"),
    (FILE, "\t\tplayer->value_count++;", "\t\tplayer->unknown2E8++;"),
    (FILE, "game_state_malloc(sizeof(struct game_unknown_001a1c8e_globals));", "game_state_malloc(sizeof(struct game_unknown_001a1c8e_globals)-8);"),
    (FILE, "\treal first= values[0]<0.0f ? 0.0f : values[0]>64.0f ? 64.0f : values[0];", "\treal first= values[0]>64.0f ? 64.0f : values[0];"),
    (FILE, "\treal second= values[1]<0.0f ? 0.0f : values[1]>64.0f ? 64.0f : values[1];", "\treal second= values[1]<0.0f ? 0.0f : values[1]>63.0f ? 64.0f : values[1];"),
    (FILE, "\tunknown_0051e994->unknownBC4= 0;", "\tunknown_0051e994->unknownBC0= 0;"),
    (FILE, "\t\tplayer->entries[index].unknown40= 0;", "\t\tplayer->entries[index].unknown0[0]= 0;"),
    (FILE, "\tplayer->unknown2E8= 0;\n", ""),
    (FILE, "\tif (flags&2)", "\tif (flags&3)"),
    (FILE, "\treturn flags&8 ? 3 : NONE;", "\treturn flags&8 ? 3 : 0;"),
]
