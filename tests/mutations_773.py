# mutations of milestone 773: one or two lines of each function

TESTS = "test_small_773"

RESULTS = "source/game/game_results.c"
LEVELS = "source/main/levels.c"
ONLINE = "source/networking/online_unknown_001a2f70.c"
MEMORY = "source/interface/user_interface_memory.c"

MUTATIONS = [
    (RESULTS, "\tif (!game_results_readable())\n\t{\n\t\treturn NONE;\n\t}\n\n\treturn game_results_get_statistic(player_index, team_index, statistic);", "\treturn game_results_get_statistic(player_index, team_index, statistic);"),
    (RESULTS, "\treturn game_results_get_statistic(player_index, team_index, statistic);\n}", "\treturn game_results_get_statistic(team_index, player_index, statistic);\n}"),
    (LEVELS, "\tif (campaign==NONE)\n\t{\n\t\tconst byte *level= levels_get_multiplayer_level(map);", "\tif (campaign==0)\n\t{\n\t\tconst byte *level= levels_get_multiplayer_level(map);"),
    (LEVELS, "\t\treturn level ? level+0xB4C : NULL;", "\t\treturn level ? level+0xB48 : NULL;"),
    (LEVELS, "\t\treturn level ? level+8 : NULL;", "\t\treturn level+8;"),
    (ONLINE, "\t*(long *)(data+0x610)= 0;", "\t*(long *)(data+0x60C)= 0;"),
    (ONLINE, "\t*(long *)(data+0x610)= 0;\n\tonline_unknown_001a2f70_end_task(data);", "\t*(long *)(data+0x610)= 0;"),
    (ONLINE, "\tname[0]= 0;\n\tif (title)", "\tif (title)"),
    (ONLINE, "\t\tonline_title_get_name(title, name, size);", "\t\tonline_title_get_name(title, name, size-1);"),
    (MEMORY, "\treturn user_interface_memory_allocate(size, 0, 0);", "\treturn user_interface_memory_allocate(size, 0, 1);"),
]
