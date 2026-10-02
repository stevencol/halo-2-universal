/*
PLAYER_PROFILE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The profiles of the players. The names of the file and of the function are
guesses. Not here: the rest, from 0x001a03a0 to 0x001a08d0.
*/

#include "player_profile.h"

#include <string.h>

/* ---------- code */

/* 0x001a0750: a profile as the game gives it to a player who has none */
void player_profile_set_defaults(
	struct player_profile *profile)
{
	memset(profile, 0, sizeof(*profile));
	profile->unknown118= 10;
	profile->unknown119= 0;
	profile->unknown11A= 0;
	profile->unknown11B= 10;
	profile->unknown11D= 0;
	profile->unknown11E= 0;
	profile->flags|= 1;
	profile->unknown102= 3;
	profile->unknownFC= 0;
	profile->unknown100= 0;
	profile->unknown101= 0;
}
