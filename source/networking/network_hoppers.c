/*
NETWORK_HOPPERS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A list of sixteen things that one of the blocks of the service of games
fills. The names are guesses.
*/

#include "network_hoppers.h"

#include "networking/network_blob.h"

/* ---------- prototypes */


/* 0x00194660: something is done with one of the list whose first number
is nothing */
void __attribute__((stdcall)) unknown_00194660(long index);

/* 0x001943d0: the games that one of the list can play; whether there are
any. 0x0019c5f0: something of a map, by its number, if the console has the
map; NULL if not */
boolean __attribute__((stdcall)) unknown_001943d0(long hopper_index, struct network_hopper_game_set *set);
const byte *__attribute__((stdcall)) levels_get_multiplayer_level(long map);

/* ---------- globals */

/* 0x00551ae8 */
struct network_hopper network_hoppers[MAXIMUM_NETWORK_HOPPERS];

/* 0x0047d8f4, of online_tasks.c: what has the block that fills the list */
extern struct network_blob unknown_0047d8f4;

/* ---------- code */

/* whether there is a list: its block is of a kind and is good */
static inline __attribute__((always_inline)) boolean network_hoppers_available(
	void)
{
	return unknown_0047d8f4.kind!=0 && (unknown_0047d8f4.flags&_network_blob_good_bit);
}

/* 0x00192db0: whether one of the list is there and is good */
boolean network_hoppers_index_valid(
	long index)
{
	if (index>=0 && index<MAXIMUM_NETWORK_HOPPERS && network_hoppers_available())
	{
		return network_hopper_valid(&network_hoppers[index]);
	}

	return FALSE;
}

/* 0x00192e60: one of the list, if it is there and is good; NULL if not */
struct network_hopper *network_hoppers_get(
	long index)
{
	if (index>=0 && index<MAXIMUM_NETWORK_HOPPERS && network_hoppers_available())
	{
		struct network_hopper *hopper= &network_hoppers[index];

		if (network_hopper_valid(hopper))
		{
			return hopper;
		}
	}

	return NULL;
}

/* 0x001933a0: a number of one of the fifth kind, or of one of the fourth
that has it; nothing for the others */
long network_hopper_get_unknown5F4(
	const struct network_hopper *hopper)
{
	if (hopper->type==_network_hopper_type5)
	{
		return hopper->unknown5F4;
	}
	if (hopper->type==_network_hopper_type4 && hopper->unknown604)
	{
		return hopper->unknown60C;
	}

	return 0;
}

/* 0x001933d0: the most players that one of the fifth kind takes, or the
number of one of the fourth that has it; nothing for the others */
long network_hopper_get_maximum(
	const struct network_hopper *hopper)
{
	if (hopper->type==_network_hopper_type5)
	{
		return hopper->maximum_players;
	}
	if (hopper->type==_network_hopper_type4 && hopper->unknown604)
	{
		return hopper->unknown608;
	}

	return 0;
}

/* 0x00193470: whether one is of the second, fourth or fifth kind */
boolean network_hopper_unknown_00193470(
	const struct network_hopper *hopper)
{
	return hopper->type==_network_hopper_type5 || hopper->type==_network_hopper_type2 ||
		hopper->type==_network_hopper_type4;
}

/* 0x00193490: whether one is of the first or third kind */
boolean network_hopper_unknown_00193490(
	const struct network_hopper *hopper)
{
	return hopper->type==_network_hopper_type1 || hopper->type==_network_hopper_type3;
}

/* 0x001934b0: whether one is of the third, fourth or fifth kind */
boolean network_hopper_unknown_001934b0(
	const struct network_hopper *hopper)
{
	return hopper->type==_network_hopper_type5 || hopper->type==_network_hopper_type3 ||
		hopper->type==_network_hopper_type4;
}

/* 0x001934d0: whether one is of the fifth kind, or of the fourth and has
its two numbers */
boolean network_hopper_unknown_001934d0(
	const struct network_hopper *hopper)
{
	return (hopper->type==_network_hopper_type4 && hopper->unknown604) || hopper->type==_network_hopper_type5;
}

/* 0x00193610: whether the players that one takes are right: one at least,
sixteen at most, and no more at least than at most */
boolean network_hopper_players_valid(
	const struct network_hopper *hopper)
{
	return hopper->minimum_players>=1 && hopper->maximum_players<=NETWORK_HOPPER_MAXIMUM_PLAYERS &&
		hopper->minimum_players<=hopper->maximum_players;
}

/* 0x001932c0: the fewest players that one takes: of the first two kinds,
what it says; of the others, its fewest teams by the players of a team. The
game does not look at whether the kind is one of the five, here or in the
functions after this one */
long network_hopper_get_minimum_players(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return hopper->minimum_players;
	default:
		return hopper->maximum_players*hopper->minimum_players;
	}
}

/* 0x00193300: the most players that one takes: of the first two kinds,
what it says; of the others, its most teams by the players of a team */
long network_hopper_get_maximum_players(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return hopper->maximum_players;
	default:
		return hopper->unknown5F4*hopper->minimum_players;
	}
}

/* 0x001931d0: the number that one has for a number of players: nothing for
the most that it takes, and a very big number for what it does not take */
long network_hopper_get_players_value(
	const struct network_hopper *hopper,
	long player_count)
{
	long maximum= network_hopper_get_maximum_players(hopper);
	long minimum= network_hopper_get_minimum_players(hopper);

	if (player_count<minimum || player_count>maximum)
	{
		return NETWORK_HOPPER_PLAYERS_VALUE_NONE;
	}
	if (player_count==maximum)
	{
		return 0;
	}

	return hopper->unknown5A0[player_count];
}

/* 0x00193250: a number of one, by its kind: its most players for the first
and the third; 1 for the second; and two numbers of its own for the fourth
and the fifth */
long network_hopper_unknown_00193250(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
		return hopper->maximum_players;
	case _network_hopper_type2:
		return 1;
	case _network_hopper_type3:
		return hopper->unknown5F4*hopper->minimum_players;
	case _network_hopper_type4:
		return hopper->unknown600;
	default:
		return hopper->unknown5F4;
	}
}

/* 0x00193340: the fewest teams that one takes; 1 for the first two kinds */
long network_hopper_get_minimum_teams(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return 1;
	default:
		return hopper->maximum_players;
	}
}

/* 0x00193370: the most teams that one takes; 1 for the first two kinds */
long network_hopper_get_maximum_teams(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return 1;
	default:
		return hopper->unknown5F4;
	}
}

/* 0x00193400: something of one: nothing for the first two kinds; whether
its byte is set, for the third; and its number, for the others */
long network_hopper_unknown_00193400(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return 0;
	case _network_hopper_type3:
		return (byte)hopper->unknown5F8 ? 1 : 0;
	default:
		return hopper->unknown5F8;
	}
}

/* 0x00193440: how many players a team of one has; NONE for the first two
kinds, which have no teams */
long network_hopper_get_team_size(
	const struct network_hopper *hopper)
{
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return NONE;
	default:
		return hopper->minimum_players;
	}
}

/* 0x001934f0: whether one of the list is good: as all are, and as the ones
of its kind are */
boolean network_hopper_valid(
	const struct network_hopper *hopper)
{
	if (!network_hopper_header_valid(hopper))
	{
		return FALSE;
	}
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return network_hopper_players_valid(hopper)!=0;
	case _network_hopper_type4:
		return network_hopper_type4_valid(hopper)!=0;
	default:
		return network_hopper_teams_valid(hopper)!=0;
	}
}

/* 0x00193560: whether one of the list is good, whatever its kind: the kind
is one of the five, its two numbers are what they can be, and each of its
128 bytes is at least its place among them */
boolean network_hopper_header_valid(
	const struct network_hopper *hopper)
{
	boolean valid= FALSE;
	long index;

	if ((hopper->type==_network_hopper_type2 || hopper->type==_network_hopper_type1 ||
		hopper->type==_network_hopper_type4 || hopper->type==_network_hopper_type3 ||
		hopper->type==_network_hopper_type5) &&
		hopper->unknown4>=0 && hopper->unknown4<NUMBER_OF_NETWORK_HOPPER_UNKNOWN4 &&
		hopper->unknown8>0 && hopper->unknown8<=NETWORK_HOPPER_UNKNOWN8_MAXIMUM)
	{
		valid= TRUE;
	}
	for (index= 0; index<NUMBER_OF_NETWORK_HOPPER_UNKNOWN51C; index++)
	{
		if (hopper->unknown51C[index]<index)
		{
			valid= FALSE;
		}
	}

	return valid;
}

/* 0x00193630: whether one of the fourth kind is good: as the ones with
teams are, but with two teams at least; and its three other numbers of teams
are right */
boolean network_hopper_type4_valid(
	const struct network_hopper *hopper)
{
	long team_size= hopper->minimum_players;
	long minimum_teams= hopper->maximum_players;
	long maximum_teams= hopper->unknown5F4;

	return team_size>1 && team_size<=NETWORK_HOPPER_MAXIMUM_PLAYERS && minimum_teams>1 &&
		maximum_teams<=NETWORK_HOPPER_MAXIMUM_PLAYERS/team_size && minimum_teams<=maximum_teams &&
		hopper->unknown5FC>0 && hopper->unknown600<=maximum_teams && hopper->unknown5FC<=hopper->unknown600 &&
		hopper->unknown5F8>=0 && hopper->unknown5F8<=maximum_teams-minimum_teams;
}

/* 0x001936a0: whether one with teams is good: from two to sixteen players
in a team, one team at least, no more teams than make sixteen players, and
no more at least than at most */
boolean network_hopper_teams_valid(
	const struct network_hopper *hopper)
{
	long team_size= hopper->minimum_players;
	long minimum_teams= hopper->maximum_players;
	long maximum_teams= hopper->unknown5F4;

	return team_size>1 && team_size<=NETWORK_HOPPER_MAXIMUM_PLAYERS && minimum_teams>=1 &&
		maximum_teams<=NETWORK_HOPPER_MAXIMUM_PLAYERS/team_size && minimum_teams<=maximum_teams;
}

/* 0x001931a0: from a number down, the first place of the 128 bytes of one
whose byte before it is less than the number; nothing if there is none, and
the number itself if it is not more than nothing */
long network_hopper_unknown_001931a0(
	long number,
	const struct network_hopper *hopper)
{
	long index= number;

	while (index>0 && hopper->unknown51C[index-1]>=number)
	{
		index--;
	}

	return index;
}

/* 0x00192df0: the first number of one of the list; nothing for NONE, and 3
for one that is not there or is not good. Something is done with one whose
number is nothing (unknown_00194660). The game asks three times whether it
is good */
long network_hoppers_get_unknown4(
	long index)
{
	if (index==NONE)
	{
		return 0;
	}
	if (index>=0 && index<MAXIMUM_NETWORK_HOPPERS && network_hoppers_available())
	{
		const struct network_hopper *hopper= &network_hoppers[index];

		if (network_hopper_valid(hopper))
		{
			if (network_hopper_valid(hopper) && hopper->unknown4==0)
			{
				unknown_00194660(index);
			}
			if (network_hopper_valid(hopper))
			{
				return hopper->unknown4;
			}
		}
	}

	return NETWORK_HOPPER_UNKNOWN4_NONE;
}

/* 0x00192eb0: the game that one of the list plays next: one of its games,
taken at random by how much each weighs (from 1 to 1000), of the ones whose
map the console has. Its map and its rules; whether there is one */
boolean __attribute__((stdcall)) network_hopper_choose_game(
	long hopper_index,
	long *map,
	struct game_variant *variant)
{
	struct network_hopper_game_set set;
	long weights[MAXIMUM_NETWORK_HOPPER_GAMES];
	unsigned long seed;
	long chosen= NONE;
	long total= 0;
	long running= 0;
	long index;
	long pick;

	if (!unknown_001943d0(hopper_index, &set))
	{
		return FALSE;
	}
	if (set.count<=0)
	{
		return FALSE;
	}
	index= 0;
	do
	{
		long weight= 0;

		if (levels_get_multiplayer_level(set.games[index].map))
		{
			weight= set.games[index].weight;
			if (weight<NETWORK_HOPPER_GAME_WEIGHT_MINIMUM)
			{
				weight= NETWORK_HOPPER_GAME_WEIGHT_MINIMUM;
			}
			else if (weight>NETWORK_HOPPER_GAME_WEIGHT_MAXIMUM)
			{
				weight= NETWORK_HOPPER_GAME_WEIGHT_MAXIMUM;
			}
		}
		total+= weight;
		weights[index]= weight;
		index++;
	} while (index<set.count);
	if (total<=0)
	{
		return FALSE;
	}
	seed= global_random_seed_reference[1]*0x19660D+0x3C6EF35F;
	global_random_seed_reference[1]= seed;
	pick= (short)(((unsigned long)((long)(short)(total+1)-1)*(seed>>16)>>16)+1);
	for (index= 0; index<set.count && chosen==NONE; index++)
	{
		running+= weights[index];
		if (running>=pick)
		{
			chosen= index;
		}
	}
	if (chosen<0 || chosen>=set.count)
	{
		return FALSE;
	}
	*map= set.games[chosen].map;
	*variant= set.games[chosen].variant;

	return TRUE;
}
