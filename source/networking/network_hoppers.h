/*
NETWORK_HOPPERS.H

A list of sixteen things that one of the blocks that are read from the
service of games fills (unknown_0047d8f4 of online_tasks.c): each has a kind,
from 1 to 5, and how many players it takes at least and at most. Most likely
they are the lists of games that the service makes matches for. Worked out
from halo2ship.exe; the name of the file is a guess, and so are the names of
its functions.
*/

#ifndef __NETWORK_HOPPERS_H
#define __NETWORK_HOPPERS_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	MAXIMUM_NETWORK_HOPPERS= 16,
	/* the kinds of one */
	_network_hopper_type1= 1,
	_network_hopper_type2,
	_network_hopper_type3,
	_network_hopper_type4,
	_network_hopper_type5,
	/* the most players that one can take */
	NETWORK_HOPPER_MAXIMUM_PLAYERS= 16
};

/* ---------- structures */

struct network_hopper
{
	long type;
	byte unknown4[0x5E8];
	/* how many players it takes, at least and at most */
	long minimum_players;
	long maximum_players;
	long unknown5F4;
	byte unknown5F8[0xC];
	/* one of the fourth kind has the two numbers after it */
	boolean unknown604;
	byte pad605[3];
	long unknown608;
	long unknown60C;
	byte unknown610[4];
};

typedef char verify_network_hopper_size[sizeof(struct network_hopper) == 0x614 ? 1 : -1];

/* ---------- prototypes/NETWORK_HOPPERS.C */

boolean network_hoppers_index_valid(long index);
struct network_hopper *network_hoppers_get(long index);
long network_hopper_get_unknown5F4(const struct network_hopper *hopper);
long network_hopper_get_maximum(const struct network_hopper *hopper);
boolean network_hopper_unknown_00193470(const struct network_hopper *hopper);
boolean network_hopper_unknown_00193490(const struct network_hopper *hopper);
boolean network_hopper_unknown_001934b0(const struct network_hopper *hopper);
boolean network_hopper_unknown_001934d0(const struct network_hopper *hopper);
boolean network_hopper_players_valid(const struct network_hopper *hopper);

/* ---------- globals */

extern struct network_hopper network_hoppers[MAXIMUM_NETWORK_HOPPERS];

#endif
