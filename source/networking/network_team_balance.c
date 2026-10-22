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

/* ---------- code */

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
