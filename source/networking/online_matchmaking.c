/*
ONLINE_MATCHMAKING.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A session that this console offers to the matchmaking of the service of
games, and the attributes that it says of it (seven kinds, each with a
number of the libraries). It is in the zone from 0x0008ee00 to 0x0008fa80,
which calls functions of the libraries that the game does not name yet
(0x003a9d8e, 0x003a9db8, 0x003a9dc3). The name of the file is a guess, and so
are the names of its functions.
*/

#include "cseries/cseries.h"
#include "input/input.h"

#include <string.h>

/* ---------- constants */

enum
{
	NUMBER_OF_ONLINE_MATCHMAKING_ATTRIBUTES= 7,
	/* in what the game knows of the player of a gamepad: eight bytes */
	UNKNOWN_0054E8E0_UNKNOWN4E0_OFFSET= 0x4E0
};

/* ---------- structures */

struct online_matchmaking_unknown_pair
{
	unsigned long low;
	unsigned long high;
};

/* ---------- code */

/* 0x0008f390: if what is given has 2 at 4, the eight bytes at 0x4E0 of what
the game knows of the player of a gamepad; 0 otherwise. The game copies
0x6A2 bytes from there to its stack first, if the eight are not 0 */
struct online_matchmaking_unknown_pair __attribute__((stdcall)) online_matchmaking_unknown_0008f390(
	long gamepad_index,
	const byte *data)
{
	struct online_matchmaking_unknown_pair pair= {0, 0};

	if (*(const long *)(data+4)==2)
	{
		const byte *unknown= unknown_0054e8e0[gamepad_index]+UNKNOWN_0054E8E0_UNKNOWN4E0_OFFSET;

		memcpy(&pair, unknown, sizeof(pair));
	}

	return pair;
}

/* 0x0008f3f0: the number by which the libraries know each kind of
attribute; NONE for one that there is not */
long online_matchmaking_attribute_identifier(
	long kind)
{
	switch (kind)
	{
	case 0:
		return 0x9C1;
	case 1:
		return 0x3C2;
	case 2:
		return 0x4C3;
	case 3:
		return 0x6C4;
	case 4:
		return 0x4C5;
	case 5:
		return 0x581;
	case 6:
		return 0x681;
	default:
		return NONE;
	}
}

/* 0x0008f450: whether a value is good for a kind of attribute: something,
2, 4, something even, 4, 8, or something even */
boolean online_matchmaking_attribute_valid(
	long kind,
	unsigned long value)
{
	switch (kind)
	{
	case 0:
		return value>0;
	case 1:
		return value==2;
	case 2:
	case 4:
		return value==4;
	case 3:
	case 6:
		return value>0 && !(value&1);
	case 5:
		return value==8;
	default:
		return FALSE;
	}
}

/* 0x0008ebc0: the text at 0xC of something; an empty one if there is
nothing */
const char *online_matchmaking_unknown_0008ebc0(
	const byte *unknown)
{
	return unknown ? (const char *)(unknown+0xC) : "";
}
