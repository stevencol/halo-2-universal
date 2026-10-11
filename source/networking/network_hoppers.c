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

/* 0x00193560: whether one of the list is good, whatever its kind.
0x00193630, 0x001936a0: whether one of the fourth kind is, and one of the
third or the fifth */
boolean unknown_00193560(const struct network_hopper *hopper);
boolean unknown_00193630(const struct network_hopper *hopper);
boolean unknown_001936a0(const struct network_hopper *hopper);

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
	if (!unknown_00193560(hopper))
	{
		return FALSE;
	}
	switch (hopper->type)
	{
	case _network_hopper_type1:
	case _network_hopper_type2:
		return network_hopper_players_valid(hopper)!=0;
	case _network_hopper_type4:
		return unknown_00193630(hopper)!=0;
	default:
		return unknown_001936a0(hopper)!=0;
	}
}
