/*
SIMULATION_PLAYER.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the game in the network has of a player of the game. The name of the
file is a guess, and so are the names of its functions.
*/

#include "simulation_player.h"

#include "game/game.h"
#include "game/player_action.h"
#include "objects/objects.h"
#include "networking/simulation_view.h"
#include "networking/simulation_world.h"

#include <string.h>

/* ---------- prototypes */

/* 0x0008b7f0: what a view has for the players gives what one of them does,
packed; whether it has it. 0x000825e0: that is unpacked */
boolean __attribute__((stdcall)) simulation_player_actions_take(long unknown0, byte *unknown5098, byte *packed);
void unknown_000825e0(const byte *packed, byte *action);

/* ---------- globals */

/* 0x004ced44: for how long what a player did goes on being what it does, in
seconds */
real unknown_004ced44;

/* ---------- code */

/* 0x00084750: what the thing has of a player is new: its three numbers, the
numbers that the player is known by, the bytes of its console, and the
thing. Its first byte is clear; its second is set from the start in a thing
that follows another console (its third state or its fifth), which does not
wait for anyone to know of the player. What it does is nothing
(player_action_clear), and has not come */
void __attribute__((stdcall)) simulation_player_new(
	byte *player,
	const long *identifier,
	const word *machine,
	byte *world,
	long unknown0,
	long index,
	long state)
{
	long world_state;

	*(long *)(player+SIMULATION_PLAYER_UNKNOWN0_OFFSET)= unknown0;
	*(long *)(player+SIMULATION_PLAYER_INDEX_OFFSET)= index;
	*(long *)(player+SIMULATION_PLAYER_STATE_OFFSET)= state;
	memcpy(player+SIMULATION_PLAYER_IDENTIFIER_OFFSET, identifier, SIMULATION_PLAYER_IDENTIFIER_LENGTH);
	memcpy(player+SIMULATION_PLAYER_MACHINE_OFFSET, machine, SIMULATION_PLAYER_MACHINE_LENGTH);
	player[SIMULATION_PLAYER_UNKNOWN24_OFFSET]= FALSE;
	*(byte **)(player+SIMULATION_PLAYER_WORLD_OFFSET)= world;
	world_state= *(const long *)(world+SIMULATION_WORLD_STATE_OFFSET);
	player[SIMULATION_PLAYER_UNKNOWN25_OFFSET]= world_state==3 || world_state==5;
	*(long *)(player+SIMULATION_PLAYER_ACTION_TIME_OFFSET)= NONE;
	player_action_clear(player+SIMULATION_PLAYER_ACTION_OFFSET);
}

/* 0x00084990: what a player of the thing does, for the game; whether there
is something. One in its fifth state or its sixth has it from the view of
its console (the one that the thing has with its bytes, or the first of the
first kind or the third in a thing that follows another): if that has
something of it (simulation_player_actions_take), it is unpacked (unknown_000825e0) and
kept, with the time of the game. Then what is kept is given, if the player
is not in its third state, something came, less time than the game has for
it has gone by since, and it is good (player_action_valid) */
boolean __attribute__((stdcall)) simulation_player_get_action(
	byte *player,
	byte *action)
{
	long state= *(const long *)(player+SIMULATION_PLAYER_STATE_OFFSET);
	long time;

	if (state>=_simulation_player_state4 && state<=_simulation_player_state5)
	{
		byte *world= *(byte *const *)(player+SIMULATION_PLAYER_WORLD_OFFSET);
		long world_state= *(const long *)(world+SIMULATION_WORLD_STATE_OFFSET);
		const byte *view;

		if (world_state!=3 && world_state!=5)
		{
			view= simulation_world_get_view_by_unknown14(world, (const word *)(player+SIMULATION_PLAYER_MACHINE_OFFSET));
		}
		else
		{
			view= simulation_world_unknown_0006acb0(world);
		}
		if (view)
		{
			byte packed[SIMULATION_PLAYER_PACKED_ACTION_SIZE];

			if (simulation_player_actions_take(*(const long *)(player+SIMULATION_PLAYER_UNKNOWN0_OFFSET),
				*(byte *const *)(view+SIMULATION_VIEW_UNKNOWN8_OFFSET)+SIMULATION_PLAYER_VIEW_UNKNOWN8_UNKNOWN5098_OFFSET, packed))
			{
				byte unpacked[PLAYER_ACTION_SIZE];

				unknown_000825e0(packed, unpacked);
				memcpy(player+SIMULATION_PLAYER_ACTION_OFFSET, unpacked, PLAYER_ACTION_SIZE);
				*(unsigned long *)(player+SIMULATION_PLAYER_ACTION_TIME_OFFSET)= game_time_globals->time;
			}
		}
	}
	time= *(const long *)(player+SIMULATION_PLAYER_ACTION_TIME_OFFSET);
	if (*(const long *)(player+SIMULATION_PLAYER_STATE_OFFSET)!=_simulation_player_state2 && time!=NONE &&
		unknown_004ced44>(real)(long)(game_time_globals->time-(unsigned long)time)*game_time_globals->seconds_per_tick &&
		player_action_valid(player+SIMULATION_PLAYER_ACTION_OFFSET))
	{
		memcpy(action, player+SIMULATION_PLAYER_ACTION_OFFSET, PLAYER_ACTION_SIZE);

		return TRUE;
	}

	return FALSE;
}

/* 0x0008b7f0: what a view has for the players gives what one of them does,
packed, and does not have it any more; whether it had it */
boolean __attribute__((stdcall)) simulation_player_actions_take(
	long player_index,
	byte *actions,
	byte *packed)
{
	unsigned long bit= 1<<player_index;
	unsigned long *mask= (unsigned long *)(actions+SIMULATION_PLAYER_ACTIONS_MASK_OFFSET);

	if (!(*mask&bit))
	{
		return FALSE;
	}
	memcpy(packed, actions+SIMULATION_PLAYER_ACTIONS_OFFSET+player_index*SIMULATION_PLAYER_PACKED_ACTION_SIZE,
		SIMULATION_PLAYER_PACKED_ACTION_SIZE);
	*mask&= ~bit;

	return TRUE;
}

/* 0x0008bf10: what a view has for the players takes what comes of one of
them: what it does, packed, and something else, each if it comes */
void __attribute__((stdcall)) simulation_player_actions_add(
	byte *actions,
	const byte *update)
{
	long player_index= *(const long *)(update+SIMULATION_PLAYER_UPDATE_INDEX_OFFSET);
	const byte *packed= *(const byte *const *)(update+SIMULATION_PLAYER_UPDATE_ACTION_OFFSET);
	const byte *other= *(const byte *const *)(update+SIMULATION_PLAYER_UPDATE_OTHER_OFFSET);

	if (packed)
	{
		memcpy(actions+SIMULATION_PLAYER_ACTIONS_OFFSET+player_index*SIMULATION_PLAYER_PACKED_ACTION_SIZE, packed,
			SIMULATION_PLAYER_PACKED_ACTION_SIZE);
		*(unsigned long *)(actions+SIMULATION_PLAYER_ACTIONS_MASK_OFFSET)|= 1<<player_index;
	}
	if (other)
	{
		memcpy(actions+SIMULATION_PLAYER_ACTIONS_OTHER_OFFSET+player_index*SIMULATION_PLAYER_ACTIONS_OTHER_SIZE, other,
			SIMULATION_PLAYER_ACTIONS_OTHER_SIZE);
		*(unsigned long *)(actions+SIMULATION_PLAYER_ACTIONS_OTHER_MASK_OFFSET)|= 1<<player_index;
	}
}

/* 0x001a1d55: a byte for a unit. 0x00082d20: the rest of what a player does
is made small, for a unit or for none */
byte __attribute__((stdcall)) unknown_001a1d55(long unit_index);
void __attribute__((stdcall)) unknown_00082d20(long unit_index, const byte *unknown34, byte *packed);

/* 0x000823d0: what a player does is made small for a message, 0x34 bytes
that start clear. Sixteen bytes go as they are; of its bits, the first two
and those of 0x800 and 0x4000 are four bits. With a unit, the four bytes
and the byte that the unit has go too, and what unknown_001a1d55 says of
it; with none, NONE. The four bytes that the player has take the place of
those of the unit if the first two of them are the same, and its two bytes
that of the unit's unless they are NONE; and of the third and fourth of
the four, the fourth is NONE if it is the same as the third. Then the rest
(unknown_00082d20) */
void __attribute__((stdcall)) simulation_player_action_pack(
	byte *packed,
	const byte *action,
	long unit_index)
{
	unsigned long flags;
	byte *packed_flags= packed+SIMULATION_PLAYER_PACKED_ACTION_FLAGS_OFFSET;

	memset(packed, 0, SIMULATION_PLAYER_PACKED_ACTION_SIZE);
	memcpy(packed, action+SIMULATION_PLAYER_ACTION_UNKNOWN4_OFFSET, SIMULATION_PLAYER_ACTION_UNKNOWN4_SIZE);
	flags= *(const unsigned long *)(action+SIMULATION_PLAYER_ACTION_FLAGS_OFFSET);
	*packed_flags= (byte)((*packed_flags&~0xF) | (flags&1 ? 1 : 0) | (flags&2 ? 2 : 0) | (flags&0x800 ? 4 : 0) |
		(flags&0x4000 ? 8 : 0));
	if (unit_index!=NONE)
	{
		const struct object_header_datum *header=
			(const struct object_header_datum *)object_header_data->data+(unit_index&0xFFFF);
		const byte *unit= (const byte *)header->datum;

		*(long *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN12_OFFSET)=
			*(const long *)(unit+SIMULATION_PLAYER_UNIT_UNKNOWN210_OFFSET);
		*(short *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN16_OFFSET)=
			*(const signed char *)(unit+SIMULATION_PLAYER_UNIT_UNKNOWN240_OFFSET);
		packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN32_OFFSET]= unknown_001a1d55(unit_index);
	}
	else
	{
		*(short *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN12_OFFSET)= NONE;
		packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN14_OFFSET]= 0xFF;
		packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN15_OFFSET]= 0xFF;
		*(short *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN16_OFFSET)= NONE;
	}
	if (*(const short *)(action+SIMULATION_PLAYER_ACTION_UNKNOWN1E_OFFSET)==
		*(const short *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN12_OFFSET))
	{
		*(long *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN12_OFFSET)=
			*(const long *)(action+SIMULATION_PLAYER_ACTION_UNKNOWN1E_OFFSET);
	}
	if (*(const short *)(action+SIMULATION_PLAYER_ACTION_UNKNOWN24_OFFSET)!=NONE)
	{
		*(short *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN16_OFFSET)=
			*(const short *)(action+SIMULATION_PLAYER_ACTION_UNKNOWN24_OFFSET);
	}
	if (packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN14_OFFSET]!=0xFF &&
		packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN14_OFFSET]==packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN15_OFFSET])
	{
		packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN15_OFFSET]= 0xFF;
	}
	unknown_00082d20(unit_index, action+SIMULATION_PLAYER_ACTION_UNKNOWN34_OFFSET, packed);
}
