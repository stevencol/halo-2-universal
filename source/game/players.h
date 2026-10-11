/*
PLAYERS.H

The players of a game. Worked out from halo2ship.exe; the names are guesses.
*/

#ifndef __PLAYERS_H
#define __PLAYERS_H

#include "cseries/cseries.h"

/* ---------- constants */

enum
{
	/* who a player is, in twelve bytes: a number of 64 bits and this */
	PLAYER_IDENTIFIER_SIZE= 12,
	PLAYER_IDENTIFIER_LOCAL= 0xBAD00000
};

/* ---------- prototypes/PLAYERS.C */

void __attribute__((stdcall)) player_identifier_from_controller(byte *identifier, long controller_index);
long players_first_local_player(void);
long players_next_local_player(long local_player_index);

#endif
