/*
PLAYER_PROFILE.H

What a player keeps of how he plays: his name, his controls, his looks. Only
what the code uses; the names are guesses.
*/

#ifndef __PLAYER_PROFILE_H
#define __PLAYER_PROFILE_H

#include "cseries/cseries.h"
#include "text/unicode.h"

/* ---------- constants */

enum
{
	PLAYER_PROFILE_SIZE= 0x1E0,
	PLAYER_PROFILE_NAME_LENGTH= 31
};

/* ---------- structures */

struct player_profile
{
	byte unknown0[4];
	unsigned long flags;
	wchar name[PLAYER_PROFILE_NAME_LENGTH+1];
	byte unknown48[0xB4];
	long unknownFC;
	byte unknown100;
	byte unknown101;
	byte unknown102;
	byte unknown103[0x15];
	byte unknown118;
	byte unknown119;
	byte unknown11A;
	byte unknown11B;
	byte unknown11C;
	byte unknown11D;
	byte unknown11E;
	byte unknown11F[0xC1];
};

typedef char verify_player_profile_size[
	sizeof(struct player_profile) == PLAYER_PROFILE_SIZE ? 1 : -1];
typedef char verify_player_profile_unknown118_offset[
	offsetof(struct player_profile, unknown118) == 0x118 ? 1 : -1];

/* ---------- prototypes/PLAYER_PROFILE.C */

void player_profile_set_defaults(struct player_profile *profile);

#endif
