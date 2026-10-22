# mutations of milestone 750: one or two lines of each function

TESTS = "test_small_750"

BALANCE = "source/networking/network_team_balance.c"

MUTATIONS = [
    (BALANCE, "\t\tgroup->players[index]= group->players[index+1];", "\t\tgroup->players[index]= group->players[index];"),
    (BALANCE, "(unsigned long)group->skill_total-(unsigned long)player->skill", "(unsigned long)group->skill_total+(unsigned long)player->skill"),
    (BALANCE, "\tgroup->player_count--;", "\tgroup->player_count-= 2;"),
    (BALANCE, "\tmemcpy(&team->groups[team->group_count], group, sizeof(*group));", "\tmemcpy(&team->groups[team->group_count], group, sizeof(*group)-4);"),
    (BALANCE, "\tteam->group_count++;", "\tteam->group_count+= 2;"),
    (BALANCE, "\tif (group->unknown0)\n\t{\n\t\tteam->unknown4++;", "\tif (!group->unknown0)\n\t{\n\t\tteam->unknown4++;"),
    (BALANCE, "&team->groups[index], &team->groups[index+1],", "&team->groups[index], &team->groups[index],"),
    (BALANCE, "\tfor (index= 0; index<group->player_count; index++)\n\t{\n\t\tnetwork_team_balance_group_remove_player", "\tfor (index= 1; index<group->player_count; index++)\n\t{\n\t\tnetwork_team_balance_group_remove_player"),
    (BALANCE, "\t\tteam->unknown4--;", "\t\tteam->unknown4++;"),
    (BALANCE, "\t\tif (other->player_count>largest->player_count ||", "\t\tif (other->player_count<largest->player_count ||"),
    (BALANCE, "other->skill_total>largest->skill_total))", "other->skill_total>=largest->skill_total))"),
    (BALANCE, "\tfor (index= 1; index<team->group_count; index++)", "\tfor (index= 2; index<team->group_count; index++)"),
    (BALANCE, "\t\tif (most<=teams->teams[index].total.player_count)", "\t\tif (most>teams->teams[index].total.player_count)"),
    (BALANCE, "first->players[place].skill : 0;", "first->players[place].skill : 1;"),
    (BALANCE, "\t\t\tlong skill= place<total->player_count ?", "\t\t\tlong skill= place<=total->player_count ?"),
    (BALANCE, "\t\tsum+= (unsigned long)highest-(unsigned long)lowest;", "\t\tsum= (unsigned long)highest-(unsigned long)lowest;"),
]
