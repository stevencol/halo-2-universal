# mutations of milestone 751: one or two lines of each function

TESTS = "test_small_751"

BALANCE = "source/networking/network_team_balance.c"

MUTATIONS = [
    (BALANCE, "\tplayer->skill= NONE;", "\tplayer->skill= 0;"),
    (BALANCE, "\t\t\tMAXIMUM_NETWORK_TEAM_BALANCE_GROUP_PLAYERS, network_team_balance_player_construct);", "\t\t\tMAXIMUM_NETWORK_TEAM_BALANCE_GROUP_PLAYERS-1, network_team_balance_player_construct);"),
    (BALANCE, "\t\tteam->groups[index].skill_total= 0;", "\t\tteam->groups[index].skill_total= 1;"),
    (BALANCE, "\t\tteam->groups[index].identifier= NONE;", "\t\tteam->groups[index].identifier= 0;"),
    (BALANCE, "\tnetwork_team_balance_group_reset(&team->total);\n", ""),
    (BALANCE, "\tteam->total.identifier= 0;\n\n\treturn team;", "\tteam->total.identifier= NONE;\n\n\treturn team;"),
    (BALANCE, "\t\tnetwork_team_balance_team_construct(&balance->teams.teams[index]);", "\t\tnetwork_team_balance_team_construct(&balance->teams.teams[0]);"),
    (BALANCE, "\tbalance->teams.team_count= 0;", "\tbalance->teams.team_count= 1;"),
    (BALANCE, "\tbalance->maximum_players= maximum_players;\n", ""),
    (BALANCE, "\tnetwork_team_balance_reset(teams, team_count);", "\tnetwork_team_balance_reset(teams, 4);"),
    (BALANCE, "\twhile (pending.group_count>0)", "\twhile (pending.group_count>1)"),
    (BALANCE, "network_team_balance_team_add_group(&teams->teams[network_team_balance_smallest_team(teams)], &group);", "network_team_balance_team_add_group(&teams->teams[0], &group);"),
    (BALANCE, "\t\tif (most<=player_count)", "\t\tif (most>player_count)"),
    (BALANCE, "\tbalance->valid= most<=maximum_players;", "\tbalance->valid= most<maximum_players;"),
    (BALANCE, "\t\tnetwork_team_balance_team_construct(&improvement->best.teams[index]);", "\t\tnetwork_team_balance_team_construct(&improvement->best.teams[0]);"),
    (BALANCE, "\timprovement->best.team_count= 0;", "\timprovement->best.team_count= 1;"),
    (BALANCE, "\timprovement->improved= FALSE;", "\timprovement->improved= TRUE;"),
]
