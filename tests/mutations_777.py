# mutations of milestone 777: one or two lines of each function

TESTS = "test_small_777"

PLAYERS = "source/networking/network_recent_players.c"
SEARCH = "source/networking/network_unknown_000b2e30.c"

MUTATIONS = [
    (PLAYERS, "\t\tif (*(const long *)(thing+0xC)==0x61 && *(const long *)(thing+0x10)==4 &&", "\t\tif (*(const long *)(thing+0xC)==0x61 &&"),
    (PLAYERS, "\t\t\tif (*(const long *)(more+0x4)==4)", "\t\t\tif (*(const long *)(more+0x4)==1)"),
    (PLAYERS, "\t\t\tif (*(const long *)(more+0x34)==1)", "\t\t\tif (*(const long *)(more+0x30)==1)"),
    (PLAYERS, "(network_recent_player_get(player_index)+0x60)&= ~1u;", "(network_recent_player_get(player_index)+0x60)&= ~2u;"),
    (SEARCH, "\tif (!search->active && unknown_004d8b18 && unknown_004d8b19)", "\tif (!search->active && unknown_004d8b18)"),
    (SEARCH, "\t\tsearch->unknown4= 0;\n", ""),
    (SEARCH, "\t\tmemset(entries, 0, entry_count*NETWORK_UNKNOWN_000B2E30_ENTRY_SIZE);", "\t\tmemset(entries, 0, NETWORK_UNKNOWN_000B2E30_ENTRY_SIZE);"),
    (SEARCH, "\t\tsearch->task_index= NONE;", "\t\tsearch->task_index= 0;"),
    (SEARCH, "\tif (search->qos_index!=NONE)", "\tif (TRUE)"),
    (SEARCH, "\tsearch->unknown48= 0;\n", ""),
    (SEARCH, "\tentry[0]= TRUE;", "\tentry[1]= TRUE;"),
    (SEARCH, "= unknown_00510548 ? unknown_0051054c : GetTickCount();", "= GetTickCount();"),
    (SEARCH, "\tmemcpy(entry+0x20, found+0x18, 0x24);", "\tmemcpy(entry+0x20, found+0x14, 0x24);"),
    (SEARCH, "\tif (index<0 || index>=search->entry_count)", "\tif (index<0 || index>search->entry_count)"),
    (SEARCH, "\tif (!entry[0] || *(const short *)(entry+NETWORK_UNKNOWN_000B2E30_ENTRY_UNKNOWN72_OFFSET))", "\tif (!entry[0])"),
    (SEARCH, "\tconst struct network_unknown_000b2e30 *search= &unknown_004d8eb4[1];", "\tconst struct network_unknown_000b2e30 *search= &unknown_004d8eb4[0];"),
]
