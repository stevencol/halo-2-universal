# mutations of milestone 746: one or two lines of each function

TESTS = "test_small_746"

PLAYERS = "source/networking/network_recent_players.c"

MUTATIONS = [
    (PLAYERS, "\tif (!unknown_004cf8e0)\n\t{\n\t\treturn;\n\t}", "\tif (unknown_004cf8e0)\n\t{\n\t\treturn;\n\t}"),
    (PLAYERS, "\tunknown_004cf95d= TRUE;", "\tunknown_004cf95d= FALSE;"),
    (PLAYERS, "\tunknown_004cf960= network_recent_players_time();", "\tunknown_004cf960= GetTickCount();"),
    (PLAYERS, "\tmemset(data, 0, NETWORK_RECENT_PLAYER_DATA_SIZE);", "\tmemset(data, 0, NETWORK_RECENT_PLAYER_DATA_SIZE-4);"),
    (PLAYERS, "\tmemcpy(data, identifier, 0xC);", "\tmemcpy(data, identifier, 8);"),
    (PLAYERS, "\tmemcpy(data+0xC, unknown, 0x40);", "\tmemcpy(data+0xC, unknown, 0x3C);"),
    (PLAYERS, "\t*(long *)(data+0x4C)= *(const long *)(unknown+0x40);", "\t*(long *)(data+0x4C)= *(const long *)(unknown+0x44);"),
    (PLAYERS, "\tdata[0x53]= unknown[0x47];", "\tdata[0x53]= unknown[0x46];"),
    (PLAYERS, "\tif (unknown7F==0xFF && other)", "\tif (unknown7F==0xFF)"),
    (PLAYERS, "\t\tdata[NETWORK_RECENT_PLAYER_DATA_UNKNOWN56_OFFSET]= other[NETWORK_RECENT_PLAYER_DATA_UNKNOWN56_OFFSET];", "\t\tdata[NETWORK_RECENT_PLAYER_DATA_UNKNOWN56_OFFSET]= other[0x55];"),
    (PLAYERS, "\tif (unknown_004cf97c==index)\n\t{\n\t\tunknown_004cf97c= *(const short *)(player+NETWORK_RECENT_PLAYER_FIRST_NEXT_OFFSET);", "\tif (unknown_004cf97c==index)\n\t{\n\t\tunknown_004cf97c= *(const short *)(player+NETWORK_RECENT_PLAYER_FIRST_PREVIOUS_OFFSET);"),
    (PLAYERS, "\t\tunknown_004cf980= *(const short *)(player+NETWORK_RECENT_PLAYER_FIRST_PREVIOUS_OFFSET);", "\t\tunknown_004cf980= NONE;"),
    (PLAYERS, "\t\t*(short *)(network_recent_player_get(next)+NETWORK_RECENT_PLAYER_FIRST_PREVIOUS_OFFSET)=", "\t\t*(short *)(network_recent_player_get(next)+NETWORK_RECENT_PLAYER_FIRST_NEXT_OFFSET)="),
    (PLAYERS, "\t\t*(short *)(network_recent_player_get(previous)+NETWORK_RECENT_PLAYER_SECOND_NEXT_OFFSET)=", "\t\t*(short *)(network_recent_player_get(previous)+NETWORK_RECENT_PLAYER_FIRST_NEXT_OFFSET)="),
    (PLAYERS, "\tif (unknown_004cf988==index)", "\tif (unknown_004cf984==index)"),
    (PLAYERS, "\tnetwork_recent_player_remove_first(index);", "\tnetwork_recent_player_remove_second(index);"),
    (PLAYERS, "\t*(short *)(player+NETWORK_RECENT_PLAYER_FIRST_NEXT_OFFSET)= (short)unknown_004cf97c;", "\t*(short *)(player+NETWORK_RECENT_PLAYER_FIRST_NEXT_OFFSET)= NONE;"),
    (PLAYERS, "\tif (unknown_004cf984!=NONE)", "\tif (unknown_004cf984!=0)"),
    (PLAYERS, "\tunknown_004cf984= index;\n}", "}"),
]
