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

/* 0x001934f0: whether one of the list is good, by its kind */
boolean unknown_001934f0(const struct network_hopper *hopper);

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
		return unknown_001934f0(&network_hoppers[index]);
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

		if (unknown_001934f0(hopper))
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
