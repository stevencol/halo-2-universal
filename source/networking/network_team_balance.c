/*
NETWORK_TEAM_BALANCE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Up to four teams, each of up to sixteen groups of players that go
together, and each group of up to sixteen players with a number each (the
groups are kept sorted by it, from the highest). Each team has besides a
group with all its players, to know how many it has and the sum of their
numbers. It looks like what the matchmaking of the online service does to
make even teams, but the name of the file is a guess, and so are the names
of its functions and of what they use.
*/

#include "cseries/cseries.h"

#include <string.h>

/* ---------- constants */

enum
{
	MAXIMUM_NETWORK_TEAM_BALANCE_TEAMS= 4,
	MAXIMUM_NETWORK_TEAM_BALANCE_GROUPS= 16,
	MAXIMUM_NETWORK_TEAM_BALANCE_GROUP_PLAYERS= 16
};

/* ---------- structures */

/* 8 bytes: the number that says how good the player is (it seems), and
something else of it */
struct network_team_balance_player
{
	long skill;
	long identifier;
};

/* 0x90 bytes */
struct network_team_balance_group
{
	boolean unknown0;
	long identifier;
	long player_count;
	long skill_total;
	struct network_team_balance_player players[MAXIMUM_NETWORK_TEAM_BALANCE_GROUP_PLAYERS];
};

/* 0x998 bytes */
struct network_team_balance_team
{
	long group_count;
	/* how many of its groups have unknown0 */
	long unknown4;
	struct network_team_balance_group groups[MAXIMUM_NETWORK_TEAM_BALANCE_GROUPS];
	/* all its players */
	struct network_team_balance_group total;
};

struct network_team_balance_teams
{
	long team_count;
	struct network_team_balance_team teams[MAXIMUM_NETWORK_TEAM_BALANCE_TEAMS];
};

/* what puts the groups in the teams */
struct network_team_balance
{
	/* that no team has more players than it can */
	boolean valid;
	struct network_team_balance_teams teams;
	long maximum_players;
};

/* what makes the teams better by moving groups from one to another */
struct network_team_balance_improvement
{
	struct network_team_balance_teams teams;
	/* that something better than the teams was found, and what */
	boolean improved;
	struct network_team_balance_teams best;
};

/* ---------- prototypes */

/* 0x00020670 (cseries.c) */
void __attribute__((stdcall)) array_construct_elements(void *array, unsigned long size, long count,
	void *(__attribute__((thiscall)) *constructor)(void *element));

/* ---------- code */

/* 0x00091530 (not in Ghidra's list): what makes each player of a group */
static void *__attribute__((thiscall)) network_team_balance_player_construct(
	void *element)
{
	struct network_team_balance_player *player= element;

	player->skill= NONE;
	player->identifier= NONE;

	return player;
}

/* 0x00091540: a group is left with no players: NONE in all of their eight
bytes, and NONE as its identifier */
void network_team_balance_group_reset(
	struct network_team_balance_group *group)
{
	long index;

	for (index= 0; index<MAXIMUM_NETWORK_TEAM_BALANCE_GROUP_PLAYERS; index++)
	{
		group->players[index].skill= NONE;
		group->players[index].identifier= NONE;
	}
	group->player_count= 0;
	group->skill_total= 0;
	group->unknown0= FALSE;
	group->identifier= NONE;
}

/* 0x00091580: a player goes into a group, after those whose number is not
lower than its own; the sum of the numbers grows with it. The game does
not look whether the group has room */
void network_team_balance_group_add_player(
	struct network_team_balance_group *group,
	const struct network_team_balance_player *player)
{
	long index= group->player_count;

	while (index>0 && group->players[index-1].skill<player->skill)
	{
		group->players[index]= group->players[index-1];
		index--;
	}
	group->players[index]= *player;
	group->player_count++;
	group->skill_total= (long)((unsigned long)group->skill_total+(unsigned long)player->skill);
}

/* 0x000915d0: a player goes out of a group: the one with the same
identifier (the game does not look whether there is one), and the sum of
the numbers goes down with the number of the one that it is given */
void network_team_balance_group_remove_player(
	struct network_team_balance_group *group,
	const struct network_team_balance_player *player)
{
	long index= 0;

	while (group->players[index].identifier!=player->identifier)
	{
		index++;
	}
	group->player_count--;
	for (; index<group->player_count; index++)
	{
		group->players[index]= group->players[index+1];
	}
	group->skill_total= (long)((unsigned long)group->skill_total-(unsigned long)player->skill);
}

/* 0x00091620: a team is made, with no groups. As in
network_team_balance_reset, the identifier of the group with all the
players is left at 0 */
struct network_team_balance_team *__attribute__((stdcall)) network_team_balance_team_construct(
	struct network_team_balance_team *team)
{
	long index;

	for (index= 0; index<MAXIMUM_NETWORK_TEAM_BALANCE_GROUPS; index++)
	{
		array_construct_elements(team->groups[index].players, sizeof(team->groups[index].players[0]),
			MAXIMUM_NETWORK_TEAM_BALANCE_GROUP_PLAYERS, network_team_balance_player_construct);
		team->groups[index].player_count= 0;
		team->groups[index].skill_total= 0;
		team->groups[index].unknown0= FALSE;
		team->groups[index].identifier= NONE;
	}
	network_team_balance_group_reset(&team->total);
	team->group_count= 0;
	team->unknown4= 0;
	team->total.identifier= 0;

	return team;
}

/* 0x00091690: a group goes into a team, after the others, and its players
into the group of all the players of the team */
void __attribute__((stdcall)) network_team_balance_team_add_group(
	struct network_team_balance_team *team,
	const struct network_team_balance_group *group)
{
	long index;

	memcpy(&team->groups[team->group_count], group, sizeof(*group));
	team->group_count++;
	for (index= 0; index<group->player_count; index++)
	{
		network_team_balance_group_add_player(&team->total, &group->players[index]);
	}
	if (group->unknown0)
	{
		team->unknown4++;
	}
}

/* 0x000916e0: a group goes out of a team: the one with the same identifier
(the game does not look whether there is one), and its players out of the
group of all the players of the team. If the group that it is given is one
of the team, those are the players of what is there after the others moved
down */
void __attribute__((stdcall)) network_team_balance_team_remove_group(
	struct network_team_balance_team *team,
	const struct network_team_balance_group *group)
{
	long index= 0;

	while (team->groups[index].identifier!=group->identifier)
	{
		index++;
	}
	team->group_count--;
	for (; index<team->group_count; index++)
	{
		memcpy(&team->groups[index], &team->groups[index+1], sizeof(team->groups[index]));
	}
	for (index= 0; index<group->player_count; index++)
	{
		network_team_balance_group_remove_player(&team->total, &group->players[index]);
	}
	if (group->unknown0)
	{
		team->unknown4--;
	}
}

/* 0x00091760: the group of a team with the most players (and of those, the
one with the highest sum of numbers; the first if there are several) is
given and goes out of the team. The first is taken even if the team has no
groups */
void __attribute__((stdcall)) network_team_balance_team_take_largest_group(
	struct network_team_balance_team *team,
	struct network_team_balance_group *group)
{
	const struct network_team_balance_group *largest= &team->groups[0];
	long index;

	for (index= 1; index<team->group_count; index++)
	{
		const struct network_team_balance_group *other= &team->groups[index];

		if (other->player_count>largest->player_count ||
			(other->player_count==largest->player_count && other->skill_total>largest->skill_total))
		{
			largest= other;
		}
	}
	memcpy(group, largest, sizeof(*group));
	network_team_balance_team_remove_group(team, group);
}

/* 0x000917c0: there are to be so many teams, and each has no groups. Of
the group with all the players only the count, the sum, unknown0 and the
identifier (0, not NONE) are cleared */
void network_team_balance_reset(
	struct network_team_balance_teams *teams,
	long team_count)
{
	long index;

	teams->team_count= team_count;
	for (index= 0; index<team_count; index++)
	{
		struct network_team_balance_team *team= &teams->teams[index];

		team->group_count= 0;
		team->unknown4= 0;
		team->total.skill_total= 0;
		team->total.player_count= 0;
		team->total.unknown0= FALSE;
		team->total.identifier= 0;
	}
}

/* 0x00091800: how far apart are the sums of the numbers of the team that
has the highest and the one that has the lowest */
long network_team_balance_skill_difference(
	const struct network_team_balance_teams *teams)
{
	long highest= teams->teams[0].total.skill_total;
	long lowest= highest;
	long index;

	for (index= 1; index<teams->team_count; index++)
	{
		long skill_total= teams->teams[index].total.skill_total;

		if (highest<=skill_total)
		{
			highest= skill_total;
		}
		if (lowest>skill_total)
		{
			lowest= skill_total;
		}
	}

	return (long)((unsigned long)highest-(unsigned long)lowest);
}

/* 0x00091860: how many more players the largest team has than the
smallest */
long network_team_balance_player_difference(
	const struct network_team_balance_teams *teams)
{
	long highest= teams->teams[0].total.player_count;
	long lowest= highest;
	long index;

	for (index= 1; index<teams->team_count; index++)
	{
		long player_count= teams->teams[index].total.player_count;

		if (highest<=player_count)
		{
			highest= player_count;
		}
		if (lowest>player_count)
		{
			lowest= player_count;
		}
	}

	return (long)((unsigned long)highest-(unsigned long)lowest);
}

/* 0x000918c0: the players of each team are in order, from the highest
number; for each place, as far as the team with the most players goes,
how far apart are the highest and the lowest number of the teams there (0
for a team that has no player there), all added */
long __attribute__((stdcall)) network_team_balance_place_difference(
	const struct network_team_balance_teams *teams)
{
	long team_count= teams->team_count;
	long most= teams->teams[0].total.player_count;
	unsigned long sum= 0;
	long index;
	long place;

	for (index= 1; index<team_count; index++)
	{
		if (most<=teams->teams[index].total.player_count)
		{
			most= teams->teams[index].total.player_count;
		}
	}
	for (place= 0; place<most; place++)
	{
		const struct network_team_balance_group *first= &teams->teams[0].total;
		long highest= place<first->player_count ? first->players[place].skill : 0;
		long lowest= highest;

		for (index= 1; index<team_count; index++)
		{
			const struct network_team_balance_group *total= &teams->teams[index].total;
			long skill= place<total->player_count ? total->players[place].skill : 0;

			if (highest<=skill)
			{
				highest= skill;
			}
			if (lowest>skill)
			{
				lowest= skill;
			}
		}
		sum+= (unsigned long)highest-(unsigned long)lowest;
	}

	return (long)sum;
}

/* 0x000919a0: the team with the fewest players, and of those the one with
the lowest sum of numbers; the first of them if there are several */
long network_team_balance_smallest_team(
	const struct network_team_balance_teams *teams)
{
	long smallest= 0;
	long index;

	for (index= 1; index<teams->team_count; index++)
	{
		const struct network_team_balance_group *total= &teams->teams[index].total;
		const struct network_team_balance_group *smallest_total= &teams->teams[smallest].total;

		if (total->player_count<smallest_total->player_count ||
			(total->player_count==smallest_total->player_count && total->skill_total<smallest_total->skill_total))
		{
			smallest= index;
		}
	}

	return smallest;
}

/* 0x00091a00: what puts the groups in the teams is made, with no teams */
struct network_team_balance *network_team_balance_construct(
	struct network_team_balance *balance)
{
	long index;

	for (index= 0; index<MAXIMUM_NETWORK_TEAM_BALANCE_TEAMS; index++)
	{
		network_team_balance_team_construct(&balance->teams.teams[index]);
	}
	balance->teams.team_count= 0;
	balance->valid= FALSE;

	return balance;
}

/* 0x00091a30: the groups of a team (what it is given has them, and its
group of all the players) are put in so many teams: the one with the most
players first, each in the team that has the fewest players then. It is
good if no team ends with more players than it can have; whether it is */
boolean __attribute__((stdcall)) network_team_balance_distribute(
	struct network_team_balance *balance,
	long team_count,
	const struct network_team_balance_team *groups,
	long maximum_players)
{
	struct network_team_balance_teams *teams= &balance->teams;
	struct network_team_balance_team pending;
	long most;
	long index;

	balance->maximum_players= maximum_players;
	balance->valid= FALSE;
	network_team_balance_reset(teams, team_count);
	memcpy(&pending, groups, sizeof(pending));
	while (pending.group_count>0)
	{
		struct network_team_balance_group group;

		network_team_balance_group_reset(&group);
		network_team_balance_team_take_largest_group(&pending, &group);
		network_team_balance_team_add_group(&teams->teams[network_team_balance_smallest_team(teams)], &group);
	}
	most= teams->teams[0].total.player_count;
	for (index= 1; index<teams->team_count; index++)
	{
		long player_count= teams->teams[index].total.player_count;

		if (most<=player_count)
		{
			most= player_count;
		}
	}
	balance->valid= most<=maximum_players;

	return balance->valid;
}

/* 0x00091b30: what makes the teams better is made, with no teams in either
of the two that it has */
struct network_team_balance_improvement *network_team_balance_improvement_construct(
	struct network_team_balance_improvement *improvement)
{
	long index;

	for (index= 0; index<MAXIMUM_NETWORK_TEAM_BALANCE_TEAMS; index++)
	{
		network_team_balance_team_construct(&improvement->teams.teams[index]);
	}
	improvement->teams.team_count= 0;
	for (index= 0; index<MAXIMUM_NETWORK_TEAM_BALANCE_TEAMS; index++)
	{
		network_team_balance_team_construct(&improvement->best.teams[index]);
	}
	improvement->best.team_count= 0;
	improvement->improved= FALSE;

	return improvement;
}

boolean network_team_balance_improvement_pass(struct network_team_balance_improvement *improvement);

/* 0x00091b90: from some teams, swaps are tried until no swap makes them
better. Whether some did */
boolean network_team_balance_improve(
	struct network_team_balance_improvement *improvement,
	const struct network_team_balance_teams *teams)
{
	boolean improved= FALSE;

	memcpy(&improvement->teams, teams, sizeof(improvement->teams));
	if (network_team_balance_improvement_pass(improvement))
	{
		improved= TRUE;
		while (network_team_balance_improvement_pass(improvement))
		{
		}
	}

	return improved;
}

/* how good some teams are, the lower the better: seven times how far apart
the sums of their numbers are and three times how far apart they are place
by place. The game has this in network_team_balance_improvement_try_swaps */
static long network_team_balance_score(
	const struct network_team_balance_teams *teams)
{
	return (long)(7*(unsigned long)network_team_balance_skill_difference(teams)+
		3*(unsigned long)network_team_balance_place_difference(teams));
}

/* 0x00091bd0: each group of a team is tried in the place of each group of
another team that has the same unknown0, if that does not take the players
of the two teams further apart. The teams with the swap are better than
the best found until then (or than the teams, if none was found yet) if
they have the players less far apart, or as far apart and a lower score;
if so they are the best found */
void __attribute__((stdcall)) network_team_balance_improvement_try_swaps(
	long first_index,
	long second_index,
	struct network_team_balance_improvement *improvement)
{
	const struct network_team_balance_team *first= &improvement->teams.teams[first_index];
	const struct network_team_balance_team *second= &improvement->teams.teams[second_index];
	long difference= (long)((unsigned long)first->total.player_count-(unsigned long)second->total.player_count);
	long first_group_index;

	if (difference<0)
	{
		difference= (long)(0-(unsigned long)difference);
	}
	for (first_group_index= 0; first_group_index<first->group_count; first_group_index++)
	{
		const struct network_team_balance_group *first_group= &first->groups[first_group_index];
		long second_group_index;

		for (second_group_index= 0; second_group_index<second->group_count; second_group_index++)
		{
			const struct network_team_balance_group *second_group= &second->groups[second_group_index];
			long swapped;

			if (first_group->unknown0!=second_group->unknown0)
			{
				continue;
			}
			swapped= (long)((unsigned long)first->total.player_count-
				2*((unsigned long)first_group->player_count-(unsigned long)second_group->player_count)-
				(unsigned long)second->total.player_count);
			if (swapped<0)
			{
				swapped= (long)(0-(unsigned long)swapped);
			}
			if (swapped<=difference)
			{
				struct network_team_balance_teams trial;
				const struct network_team_balance_teams *reference;
				long trial_players;
				long reference_players;

				memcpy(&trial, &improvement->teams, sizeof(trial));
				network_team_balance_team_remove_group(&trial.teams[first_index], first_group);
				network_team_balance_team_remove_group(&trial.teams[second_index], second_group);
				network_team_balance_team_add_group(&trial.teams[first_index], second_group);
				network_team_balance_team_add_group(&trial.teams[second_index], first_group);
				reference= improvement->improved ? &improvement->best : &improvement->teams;
				trial_players= network_team_balance_player_difference(&trial);
				reference_players= network_team_balance_player_difference(reference);
				if (trial_players<reference_players ||
					(trial_players==reference_players && network_team_balance_score(&trial)<network_team_balance_score(reference)))
				{
					improvement->improved= TRUE;
					memcpy(&improvement->best, &trial, sizeof(trial));
				}
			}
		}
	}
}

/* 0x00091dd0: swaps are tried between each two teams; if some were better,
the best found are the teams from then on. Whether there were */
boolean network_team_balance_improvement_pass(
	struct network_team_balance_improvement *improvement)
{
	long first_index;

	improvement->improved= FALSE;
	for (first_index= 0; first_index<improvement->teams.team_count-1; first_index++)
	{
		long second_index;

		for (second_index= first_index+1; second_index<improvement->teams.team_count; second_index++)
		{
			network_team_balance_improvement_try_swaps(first_index, second_index, improvement);
		}
	}
	if (improvement->improved)
	{
		memcpy(&improvement->teams, &improvement->best, sizeof(improvement->teams));
	}

	return improvement->improved;
}

boolean __attribute__((stdcall)) network_team_balance_assign(long team_count, long maximum_players,
	boolean mark_groups, long minimum_group_players, long maximum_group_players, boolean grouped,
	long player_count, const long *player_groups, const long *player_skills, boolean improve,
	long *player_teams);

/* 0x00091e30: the players are put in teams as network_team_balance_assign
does, all with the number 0 and with no swaps; how many more players the
largest team has than the smallest is given. FALSE if they cannot be put */
boolean __attribute__((stdcall)) network_team_balance_assign_difference(
	long player_count,
	const long *player_groups,
	long team_count,
	long maximum_players,
	boolean mark_groups,
	long minimum_group_players,
	long maximum_group_players,
	boolean grouped,
	long *difference)
{
	long counts[MAXIMUM_NETWORK_TEAM_BALANCE_GROUP_PLAYERS];
	long player_teams[MAXIMUM_NETWORK_TEAM_BALANCE_GROUP_PLAYERS];
	boolean assigned;

	memset(counts, 0, sizeof(counts));
	assigned= network_team_balance_assign(team_count, maximum_players, mark_groups, minimum_group_players,
		maximum_group_players, grouped, player_count, player_groups, counts, FALSE, player_teams);
	if (assigned)
	{
		long highest;
		long lowest;
		long index;

		memset(counts, 0, sizeof(counts));
		for (index= 0; index<player_count; index++)
		{
			counts[player_teams[index]]++;
		}
		highest= counts[0];
		lowest= highest;
		for (index= 1; index<team_count; index++)
		{
			if (lowest>counts[index])
			{
				lowest= counts[index];
			}
			if (highest<=counts[index])
			{
				highest= counts[index];
			}
		}
		*difference= highest-lowest;
	}

	return assigned;
}

/* 0x00091f00: whether some players fit in so many teams: those with a group
(from 0) and the others, all in a group after those. The players that the
teams are short of do not count against the difference in players, which
has to be no more than the one that is given */
boolean __attribute__((stdcall)) network_team_balance_fits(
	long grouped_player_count,
	long other_player_count,
	const long *player_groups,
	long team_count,
	long maximum_players,
	boolean mark_groups,
	long minimum_group_players,
	long maximum_group_players,
	boolean grouped,
	long maximum_difference)
{
	long groups[MAXIMUM_NETWORK_TEAM_BALANCE_GROUP_PLAYERS];
	long player_count= grouped_player_count+other_player_count;
	long missing= (long)((unsigned long)team_count*(unsigned long)maximum_players-(unsigned long)player_count);
	long group_count= 0;
	long difference= NONE;
	long index;

	for (index= 0; index<grouped_player_count; index++)
	{
		long group= player_groups[index];

		if (group_count<=group+1)
		{
			group_count= group+1;
		}
		groups[index]= group;
	}
	for (index= grouped_player_count; index<player_count; index++)
	{
		groups[index]= group_count;
	}
	if (!network_team_balance_assign_difference(player_count, groups, team_count, maximum_players, mark_groups,
		minimum_group_players, maximum_group_players, grouped, &difference))
	{
		return FALSE;
	}
	difference= (long)((unsigned long)difference-(unsigned long)missing);
	if (difference<0)
	{
		difference= 0;
	}

	return difference<=maximum_difference;
}

/* 0x00091fc0: so many players, each with its number and (if they are
grouped) its group, are put in teams: the groups are distributed and, if
asked, swaps make the teams better. The groups with so many players (from
the fewest to the most that are given) have unknown0, if asked; only
groups with the same unknown0 are swapped. The team of each player is
given. FALSE if the players do not fit */
boolean __attribute__((stdcall)) network_team_balance_assign(
	long team_count,
	long maximum_players,
	boolean mark_groups,
	long minimum_group_players,
	long maximum_group_players,
	boolean grouped,
	long player_count,
	const long *player_groups,
	const long *player_skills,
	boolean improve,
	long *player_teams)
{
	struct network_team_balance_group groups[MAXIMUM_NETWORK_TEAM_BALANCE_GROUPS];
	struct network_team_balance_team team;
	struct network_team_balance balance;
	struct network_team_balance_improvement improvement;
	const struct network_team_balance_teams *teams;
	long index;

	for (index= 0; index<MAXIMUM_NETWORK_TEAM_BALANCE_GROUPS; index++)
	{
		array_construct_elements(groups[index].players, sizeof(groups[index].players[0]), MAXIMUM_NETWORK_TEAM_BALANCE_GROUP_PLAYERS,
			network_team_balance_player_construct);
		groups[index].player_count= 0;
		groups[index].skill_total= 0;
		groups[index].unknown0= FALSE;
		groups[index].identifier= NONE;
	}
	for (index= 0; index<player_count; index++)
	{
		struct network_team_balance_player player;

		player.skill= player_skills[index];
		player.identifier= index;
		network_team_balance_group_add_player(&groups[grouped ? player_groups[index] : index], &player);
	}
	network_team_balance_team_construct(&team);
	for (index= 0; index<MAXIMUM_NETWORK_TEAM_BALANCE_GROUPS; index++)
	{
		struct network_team_balance_group *group= &groups[index];

		if (group->player_count>0)
		{
			group->identifier= index;
			if (mark_groups && group->player_count>=minimum_group_players && group->player_count<=maximum_group_players)
			{
				group->unknown0= TRUE;
			}
			network_team_balance_team_add_group(&team, group);
		}
	}
	network_team_balance_construct(&balance);
	if (!network_team_balance_distribute(&balance, team_count, &team, maximum_players))
	{
		return FALSE;
	}
	network_team_balance_improvement_construct(&improvement);
	teams= &balance.teams;
	if (improve && network_team_balance_improve(&improvement, &balance.teams))
	{
		teams= &improvement.teams;
	}
	for (index= 0; index<teams->team_count; index++)
	{
		const struct network_team_balance_group *total= &teams->teams[index].total;
		long player_index;

		for (player_index= 0; player_index<total->player_count; player_index++)
		{
			player_teams[total->players[player_index].identifier]= index;
		}
	}

	return TRUE;
}
