# mutations of milestone 747: one or two lines of each function

TESTS = "test_small_747"

PLAYERS = "source/networking/network_recent_players.c"

MUTATIONS = [
    (PLAYERS, "\tunknown_004cf970= 8;", "\tunknown_004cf970= 0;"),
    (PLAYERS, "\tunknown_004cf978= 0;\n\tunknown_004cf970", "\tunknown_004cf970"),
    (PLAYERS, "\t\tplayer[NETWORK_RECENT_PLAYER_DATA_UNKNOWN56_OFFSET]= 0xFF;", "\t\tplayer[NETWORK_RECENT_PLAYER_DATA_UNKNOWN56_OFFSET]= 0;"),
    (PLAYERS, "\t\t*(short *)(player+NETWORK_RECENT_PLAYER_SECOND_NEXT_OFFSET)= NONE;\n\t\t*(long *)(player+0x60)= 0;", "\t\t*(long *)(player+0x60)= 0;"),
    (PLAYERS, "\t\t*(long *)(player+0x64)= 0;", "\t\t*(long *)(player+0x64)= 1;"),
    (PLAYERS, "\tlong last= unknown_004cf978-1;", "\tlong last= unknown_004cf978;"),
    (PLAYERS, "\t\telse if (difference<0)\n\t\t{\n\t\t\tlast= middle-1;", "\t\telse if (difference<0)\n\t\t{\n\t\t\tlast= middle;"),
    (PLAYERS, "\t\t\tmiddle++;\n", ""),
    (PLAYERS, "\tif (index_reference)\n\t{\n\t\t*index_reference= middle;", "\tif (index_reference)\n\t{\n\t\t*index_reference= found;"),
    (PLAYERS, "\t\tdifference= memcmp(identifier, network_recent_player_get(middle), 0xC);", "\t\tdifference= memcmp(identifier, network_recent_player_get(middle), 8);"),
    (PLAYERS, "\t\tif (!(flags&4) && (flags&0x10) && (*(const unsigned long *)player|high) &&", "\t\tif ((flags&0x10) && (*(const unsigned long *)player|high) &&"),
    (PLAYERS, "\t\t\t(high>>16)!=0xFEFE && !(player[8]&3) && *(const short *)(player+0xC)!=0)", "\t\t\t!(player[8]&3) && *(const short *)(player+0xC)!=0)"),
    (PLAYERS, "\t\t\t(high>>16)!=0xFEFE && !(player[8]&3) && *(const short *)(player+0xC)!=0)", "\t\t\t(high>>16)!=0xFEFE && !(player[8]&1) && *(const short *)(player+0xC)!=0)"),
    (PLAYERS, "\t\t\t\tmemcpy(data, player, NETWORK_RECENT_PLAYER_DATA_SIZE);", "\t\t\t\tmemcpy(data, player, 0x54);"),
    (PLAYERS, "\t\t*index_reference= next;", "\t\t*index_reference= index;"),
    (PLAYERS, "\t\tif (next==NONE)\n\t\t{\n\t\t\treturn FALSE;", "\t\tif (next==NONE)\n\t\t{\n\t\t\treturn TRUE;"),
]
