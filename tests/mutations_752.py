# mutations of milestone 752: one or two lines of each function

TESTS = "test_small_752"

BALANCE = "source/networking/network_team_balance.c"

MUTATIONS = [
    (BALANCE, "\tmemcpy(&improvement->teams, teams, sizeof(improvement->teams));", "\tmemcpy(&improvement->teams, teams, sizeof(improvement->teams)-4);"),
    (BALANCE, "\t\timproved= TRUE;\n\t\twhile", "\t\timproved= FALSE;\n\t\twhile"),
    (BALANCE, "\t\twhile (network_team_balance_improvement_pass(improvement))\n\t\t{\n\t\t}\n", ""),
    (BALANCE, "\treturn (long)(7*(unsigned long)network_team_balance_skill_difference(teams)+", "\treturn (long)(1*(unsigned long)network_team_balance_skill_difference(teams)+"),
    (BALANCE, "\t\t3*(unsigned long)network_team_balance_place_difference(teams));", "\t\t0*(unsigned long)network_team_balance_place_difference(teams));"),
    (BALANCE, "\t\tdifference= (long)(0-(unsigned long)difference);\n", ""),
    (BALANCE, "\t\t\tif (first_group->unknown0!=second_group->unknown0)", "\t\t\tif (first_group->unknown0==second_group->unknown0)"),
    (BALANCE, "\t\t\t\t2*((unsigned long)first_group->player_count", "\t\t\t\t1*((unsigned long)first_group->player_count"),
    (BALANCE, "\t\t\t\tswapped= (long)(0-(unsigned long)swapped);\n", ""),
    (BALANCE, "\t\t\tif (swapped<=difference)", "\t\t\tif (swapped<difference)"),
    (BALANCE, "network_team_balance_team_add_group(&trial.teams[first_index], second_group);", "network_team_balance_team_add_group(&trial.teams[first_index], first_group);"),
    (BALANCE, "\t\t\t\treference= improvement->improved ? &improvement->best : &improvement->teams;", "\t\t\t\treference= &improvement->teams;"),
    (BALANCE, "\t\t\t\tif (trial_players<reference_players ||", "\t\t\t\tif (trial_players<=reference_players ||"),
    (BALANCE, "network_team_balance_score(&trial)<network_team_balance_score(reference)", "network_team_balance_score(&trial)<=network_team_balance_score(reference)"),
    (BALANCE, "\t\t\t\t\timprovement->improved= TRUE;", "\t\t\t\t\timprovement->improved= FALSE;"),
    (BALANCE, "\timprovement->improved= FALSE;\n\tfor (first_index", "\timprovement->improved= TRUE;\n\tfor (first_index"),
    (BALANCE, "first_index<improvement->teams.team_count-1;", "first_index<improvement->teams.team_count-2;"),
    (BALANCE, "for (second_index= first_index+1;", "for (second_index= first_index+2;"),
    # (copying 4 bytes less survived: they are the identifier of the last
    # player of the fourth team, which only counts with four teams and 16
    # players in the last one)
    (BALANCE, "\t\tmemcpy(&improvement->teams, &improvement->best, sizeof(improvement->teams));", "\t\tmemcpy(&improvement->best, &improvement->teams, sizeof(improvement->teams));"),
]
