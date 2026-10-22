# mutations of milestone 749: one or two lines of each function

TESTS = "test_small_749"

BALANCE = "source/networking/network_team_balance.c"

MUTATIONS = [
    (BALANCE, "\t\tgroup->players[index].skill= NONE;", "\t\tgroup->players[index].skill= 0;"),
    (BALANCE, "\tgroup->identifier= NONE;", "\tgroup->identifier= 0;"),
    (BALANCE, "\twhile (index>0 && group->players[index-1].skill<player->skill)", "\twhile (index>0 && group->players[index-1].skill<=player->skill)"),
    (BALANCE, "\tgroup->player_count++;", "\tgroup->player_count+= 2;"),
    (BALANCE, "(unsigned long)group->skill_total+(unsigned long)player->skill", "(unsigned long)group->skill_total-(unsigned long)player->skill"),
    (BALANCE, "\t\tteam->unknown4= 0;\n", ""),
    (BALANCE, "\t\tteam->total.identifier= 0;", "\t\tteam->total.identifier= NONE;"),
    (BALANCE, "\tfor (index= 0; index<team_count; index++)", "\tfor (index= 0; index<team_count-1; index++)"),
    (BALANCE, "\t\tif (highest<=skill_total)", "\t\tif (highest>=skill_total)"),
    (BALANCE, "\t\tif (lowest>skill_total)", "\t\tif (lowest<skill_total)"),
    (BALANCE, "\tfor (index= 1; index<teams->team_count; index++)\n\t{\n\t\tlong skill_total", "\tfor (index= 2; index<teams->team_count; index++)\n\t{\n\t\tlong skill_total"),
    (BALANCE, "\tlong highest= teams->teams[0].total.player_count;", "\tlong highest= teams->teams[1].total.player_count;"),
    (BALANCE, "\t\tif (highest<=player_count)", "\t\tif (highest>player_count)"),
    (BALANCE, "\t\tif (total->player_count<smallest_total->player_count ||", "\t\tif (total->player_count>smallest_total->player_count ||"),
    (BALANCE, "total->skill_total<smallest_total->skill_total))", "total->skill_total<=smallest_total->skill_total))"),
]
