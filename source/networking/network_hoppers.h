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
	NETWORK_HOPPER_MAXIMUM_PLAYERS= 16,
	NUMBER_OF_NETWORK_HOPPER_UNKNOWN5A0= 19,
	NUMBER_OF_NETWORK_HOPPER_UNKNOWN51C= 128,
	NUMBER_OF_NETWORK_HOPPER_UNKNOWN4= 4,
	NETWORK_HOPPER_UNKNOWN8_MAXIMUM= 100,
	/* what network_hoppers_get_unknown4 says of one that is not there */
	NETWORK_HOPPER_UNKNOWN4_NONE= 3,
	/* what network_hopper_get_players_value says of a number of players
	that one does not take */
	NETWORK_HOPPER_PLAYERS_VALUE_NONE= 0x7FFFFFFF
};

/* ---------- structures */

struct network_hopper
{
	long type;
	/* from 0 to 3, and from 1 to 100 */
	long unknown4;
	unsigned long unknown8;
	byte unknownC[0x510];
	/* each is at least its place in the list */
	char unknown51C[NUMBER_OF_NETWORK_HOPPER_UNKNOWN51C];
	byte unknown59C[4];
	/* a number for each number of players that it can have */
	long unknown5A0[NUMBER_OF_NETWORK_HOPPER_UNKNOWN5A0];
	/* of the first two kinds: how many players it takes, at least and at
	most. Of the others: how many players a team has, and how many teams it
	takes, at least and at most */
	long minimum_players;
	long maximum_players;
	long unknown5F4;
	/* of the third kind, a byte; of the fourth and fifth, a number */
	long unknown5F8;
	/* of the fourth kind: two numbers of teams, the first no more than the
	second, and the second no more than the most teams */
	long unknown5FC;
	long unknown600;
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
long network_hopper_get_players_value(const struct network_hopper *hopper, long player_count);
long network_hopper_unknown_00193250(const struct network_hopper *hopper);
long network_hopper_get_minimum_players(const struct network_hopper *hopper);
long network_hopper_get_maximum_players(const struct network_hopper *hopper);
long network_hopper_get_minimum_teams(const struct network_hopper *hopper);
long network_hopper_get_maximum_teams(const struct network_hopper *hopper);
long network_hopper_unknown_00193400(const struct network_hopper *hopper);
long network_hopper_get_team_size(const struct network_hopper *hopper);
boolean network_hopper_valid(const struct network_hopper *hopper);
boolean network_hopper_header_valid(const struct network_hopper *hopper);
boolean network_hopper_type4_valid(const struct network_hopper *hopper);
boolean network_hopper_teams_valid(const struct network_hopper *hopper);
long network_hopper_unknown_001931a0(long number, const struct network_hopper *hopper);
long network_hoppers_get_unknown4(long index);

/* ---------- globals */

extern struct network_hopper network_hoppers[MAXIMUM_NETWORK_HOPPERS];

#endif
