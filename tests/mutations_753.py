# mutations of milestone 753: one or two lines of each function

TESTS = "test_small_753"

BALANCE = "source/networking/network_team_balance.c"

MUTATIONS = [
    # (the sum of the numbers of the groups, which starts at 0, cannot be
    # broken: it is only compared between groups, and all of them would have
    # the same more)
    (BALANCE, "\t\tplayer.skill= player_skills[index];", "\t\tplayer.skill= 0;"),
    (BALANCE, "network_team_balance_group_add_player(&groups[grouped ? player_groups[index] : index], &player);", "network_team_balance_group_add_player(&groups[index], &player);"),
    (BALANCE, "\t\t\tgroup->identifier= index;", "\t\t\tgroup->identifier= 0;"),
    (BALANCE, "group->player_count>=minimum_group_players", "group->player_count>minimum_group_players"),
    (BALANCE, "\t\t\tnetwork_team_balance_team_add_group(&team, group);\n", ""),
    (BALANCE, "\tif (!network_team_balance_distribute(&balance, team_count, &team, maximum_players))\n\t{\n\t\treturn FALSE;", "\tif (!network_team_balance_distribute(&balance, team_count, &team, maximum_players))\n\t{\n\t\treturn TRUE;"),
    (BALANCE, "\tif (improve && network_team_balance_improve(&improvement, &balance.teams))", "\tif (network_team_balance_improve(&improvement, &balance.teams))"),
    (BALANCE, "\t\t\tplayer_teams[total->players[player_index].identifier]= index;", "\t\t\tplayer_teams[total->players[player_index].identifier]= 0;"),
    (BALANCE, "\t\t\tcounts[player_teams[index]]++;", "\t\t\tcounts[player_teams[index]]+= 2;"),
    (BALANCE, "\t\t\tif (lowest>counts[index])", "\t\t\tif (lowest<counts[index])"),
    (BALANCE, "\t\t*difference= highest-lowest;", "\t\t*difference= highest;"),
    (BALANCE, "\t\tif (group_count<=group+1)", "\t\tif (group_count<group)"),
    (BALANCE, "\t\tgroups[index]= group_count;", "\t\tgroups[index]= 0;"),
    (BALANCE, "(unsigned long)team_count*(unsigned long)maximum_players-(unsigned long)player_count", "(unsigned long)team_count*(unsigned long)maximum_players+(unsigned long)player_count"),
    (BALANCE, "\t\tdifference= 0;\n\t}\n\n\treturn difference<=maximum_difference;", "\t\tdifference= 1;\n\t}\n\n\treturn difference<=maximum_difference;"),
    (BALANCE, "\treturn difference<=maximum_difference;", "\treturn difference<maximum_difference;"),
]
