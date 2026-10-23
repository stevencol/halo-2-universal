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

#include "cache/cache_files.h"
#include "objects/objects.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"

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

/* 0x00510c94 (main.c): what has, at 0x294, how fast something has to go */
struct unknown_00510c94;
extern struct unknown_00510c94 *unknown_00510c94;

/* 0x0051e994: in the memory of the state of the game */
struct game_unknown_001a1c8e_globals *unknown_0051e994;

/* ---------- code */

/* 0x001a1c3e: a value goes after the first unknown2E8 values of a local
player, if they are not sixteen: the first of the others goes after them
to make room (or the last of them is lost, if there is no room for it) */
void __attribute__((stdcall)) game_unknown_001a1c8e_add_first_value(
	struct game_unknown_001a1c8e_player *player,
	long value)
{
	long first_count= player->unknown2E8;

	if (first_count>=GAME_UNKNOWN_001A1C8E_VALUES)
	{
		return;
	}
	if (player->value_count)
	{
		if (first_count+player->value_count<GAME_UNKNOWN_001A1C8E_VALUES)
		{
			player->values[first_count+player->value_count]= player->values[first_count];
		}
		else
		{
			player->value_count--;
		}
	}
	player->values[player->unknown2E8]= value;
	player->unknown2E8++;
}

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

/* 0x001a1cb5: the value of a place goes out of those of a local player:
the last of its part takes its place (and, if it is of the first part, the
last of the others takes the place of that one) */
void game_unknown_001a1c8e_remove_value(
	struct game_unknown_001a1c8e_player *player,
	long index)
{
	long first_count;
	long total;

	if (index<0)
	{
		return;
	}
	first_count= player->unknown2E8;
	total= first_count+player->value_count;
	if (index>=total)
	{
		return;
	}
	if (index<first_count)
	{
		if (index!=first_count-1)
		{
			player->values[index]= player->values[first_count-1];
		}
		if (player->value_count)
		{
			player->values[player->unknown2E8-1]= player->values[player->unknown2E8+player->value_count-1];
		}
		player->unknown2E8--;
	}
	else
	{
		if (index!=total-1)
		{
			player->values[index]= player->values[total-1];
		}
		player->value_count--;
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

/* 0x001a1d55: whether an object goes as fast as something has to (its
velocity, with a third of what it has upwards; 0 for an object with the
first flag of 0xC1): for a biped or a vehicle with something at 0x13C, a
bit more than a number of the globals of the game; for anything else, the
number of 0x294 of unknown_00510c94 */
boolean __attribute__((stdcall)) game_unknown_001a1c8e_object_fast_enough(
	long object_index)
{
	const byte *object= (const byte *)((const struct object_header_datum *)object_header_data->data)[object_index&0xFFFF].datum;
	real speed= 0.0f;
	real minimum;

	if (!(object[0xC1]&1))
	{
		real_vector3d velocity;

		object_get_velocities(object_index, &velocity, NULL);
		velocity.k*= 0.33f;
		speed= magnitude3d(&velocity);
	}
	if (((1<<object[0xAA])&3) && *(const long *)(object+0x13C)!=NONE)
	{
		const byte *element= *(const byte *const *)((const byte *)global_game_globals+0x134);

		minimum= *(const real *)(element+0x3C)*1.05f;
	}
	else
	{
		minimum= *(const real *)((const byte *)unknown_00510c94+0x294);
	}

	return speed>=minimum;
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

/* 0x001a2084: whether an object can be (it seems) taken hold of: a biped
or a vehicle with nothing at 0x13C, or something of the thirteenth kind
whose definition does not have the seventh flag of 0xBC, and in both cases
without the third flag of 0x10A */
boolean game_unknown_001a1c8e_object_unknown_001a2084(
	long object_index)
{
	const byte *object= object_try_and_get_and_verify_type(object_index, NONE);
	unsigned long type_bit;

	if (!object)
	{
		return FALSE;
	}
	type_bit= 1u<<object[0xAA];
	if (type_bit&3)
	{
		if (*(const long *)(object+0x13C)!=NONE)
		{
			return FALSE;
		}
	}
	else if (type_bit&0x1000)
	{
		const byte *definition= (const byte *)(uintptr_t)global_tag_instances[*(const long *)object&0xFFFF].address;

		if (*(const unsigned long *)(definition+0xBC)>>6&1)
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	return !(object[0x10A]>>2&1);
}
