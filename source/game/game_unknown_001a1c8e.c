/*
GAME_UNKNOWN_001A1C8E.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

A part of the game with something of 0x2F0 bytes for each of the four
local players, in the memory of the state of the game (0x0051e994), and
a few small functions near it. A function of the players clears the one of
a local player when it gets a unit. What it is for is not known, and the
name of the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "math/real_math.h"

#include "saved games/game_state.h"

/* ---------- constants */

enum
{
	MAXIMUM_GAME_UNKNOWN_001A1C8E_PLAYERS= 4,
	GAME_UNKNOWN_001A1C8E_VALUES= 16,
	GAME_UNKNOWN_001A1C8E_ENTRIES= 10
};

/* ---------- structures */

/* 0x44 bytes: only the last number is known */
struct game_unknown_001a1c8e_entry
{
	byte unknown0[0x40];
	long unknown40;
};

/* 0x2F0 bytes, one for each local player */
struct game_unknown_001a1c8e_player
{
	struct game_unknown_001a1c8e_entry entries[GAME_UNKNOWN_001A1C8E_ENTRIES];
	long values[GAME_UNKNOWN_001A1C8E_VALUES];
	/* the values are after these many */
	long unknown2E8;
	long value_count;
};

/* 0xBC8 bytes */
struct game_unknown_001a1c8e_globals
{
	struct game_unknown_001a1c8e_player players[MAXIMUM_GAME_UNKNOWN_001A1C8E_PLAYERS];
	long unknownBC0;
	long unknownBC4;
};

/* ---------- globals */

/* 0x0051e994: in the memory of the state of the game */
struct game_unknown_001a1c8e_globals *unknown_0051e994;

/* ---------- code */

/* 0x001a1c8e: a value goes after the others of a local player, if there is
room among the sixteen after the first unknown2E8 */
void __attribute__((stdcall)) game_unknown_001a1c8e_add_value(
	struct game_unknown_001a1c8e_player *player,
	long value)
{
	long index= player->unknown2E8+player->value_count;

	if (index<GAME_UNKNOWN_001A1C8E_VALUES)
	{
		player->values[index]= value;
		player->value_count++;
	}
}

/* 0x001a1d45: the memory of the state of the game for all of it */
void game_unknown_001a1c8e_initialize(
	void)
{
	unknown_0051e994= game_state_malloc(sizeof(struct game_unknown_001a1c8e_globals));
}

/* 0x001a1dfe: two reals from 0 to 64 as two bytes (what is less than 0 is
0, and what is more than 64 is 64) */
void game_unknown_001a1c8e_reals_to_bytes(
	const real *values,
	byte *bytes)
{
	real first= values[0]<0.0f ? 0.0f : values[0]>64.0f ? 64.0f : values[0];
	real second= values[1]<0.0f ? 0.0f : values[1]>64.0f ? 64.0f : values[1];

	bytes[0]= (byte)(long)first;
	bytes[1]= (byte)(long)second;
}

/* 0x001a221d: what there is of a local player is cleared: its counts and
the last number of each of its entries, and unknownBC4 of all */
void game_unknown_001a1c8e_reset_player(
	long player_index)
{
	struct game_unknown_001a1c8e_player *player= &unknown_0051e994->players[player_index];
	long index;

	unknown_0051e994->unknownBC4= 0;
	player->value_count= 0;
	player->unknown2E8= 0;
	for (index= 0; index<GAME_UNKNOWN_001A1C8E_ENTRIES; index++)
	{
		player->entries[index].unknown40= 0;
	}
}

/* 0x001a2c81: the first of four flags of 8 that something has (0 to 3);
NONE if it has none */
long game_unknown_001a1c8e_flag_index(
	const byte *data)
{
	word flags= *(const word *)(data+8);

	if (flags&1)
	{
		return 0;
	}
	if (flags&2)
	{
		return 1;
	}
	if (flags&4)
	{
		return 2;
	}

	return flags&8 ? 3 : NONE;
}
