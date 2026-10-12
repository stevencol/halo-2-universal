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
#include "units/units.h"
#include "main/main.h"
#include "main/main_game.h"
#include "networking/simulation_view.h"
#include "networking/simulation_world.h"

#include <string.h>

/* ---------- prototypes */

/* 0x0008b7f0: what a view has for the players gives what one of them does,
packed; whether it has it. 0x000825e0: that is unpacked */
boolean __attribute__((stdcall)) simulation_player_actions_take(long unknown0, byte *unknown5098, byte *packed);

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
something of it (simulation_player_actions_take), it is unpacked (simulation_player_action_unpack) and
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

				simulation_player_action_unpack(packed, unpacked);
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

/* 0x000a58d0: a number that came in a message, as the one that the game has
for it; NONE if it has none */
long unknown_000a58d0(long unknown);

/* a bit of what a player does is set or clear */
static inline __attribute__((always_inline)) void simulation_player_action_set_flag(
	byte *action,
	unsigned long flag,
	boolean set)
{
	unsigned long flags= *(const unsigned long *)(action+SIMULATION_PLAYER_ACTION_FLAGS_OFFSET);

	*(unsigned long *)(action+SIMULATION_PLAYER_ACTION_FLAGS_OFFSET)= set ? flags|flag : flags&~flag;
}

/* 0x000825e0: what a player does is made whole again from the 0x34 bytes
of a message: the inverse of simulation_player_action_pack. It starts as
nothing (player_action_clear); the sixteen bytes, the four bits, the four
bytes and the two are put back; six bytes are each a bit (0x40000, 0x80000,
0x800000, 0x1000000, 0x40000000 and 0x80000000); and the last byte goes to
0x58. If one of the two reals is more than nothing and the number with them
is one that the game has (unknown_000a58d0), that, the number after it and
the two reals go too */
void simulation_player_action_unpack(
	const byte *packed,
	byte *action)
{
	byte packed_flags;

	player_action_clear(action);
	memcpy(action+SIMULATION_PLAYER_ACTION_UNKNOWN4_OFFSET, packed, SIMULATION_PLAYER_ACTION_UNKNOWN4_SIZE);
	packed_flags= packed[SIMULATION_PLAYER_PACKED_ACTION_FLAGS_OFFSET];
	simulation_player_action_set_flag(action, 1, (packed_flags&1)!=0);
	simulation_player_action_set_flag(action, 2, (packed_flags&2)!=0);
	simulation_player_action_set_flag(action, 0x800, (packed_flags&4)!=0);
	simulation_player_action_set_flag(action, 0x4000, (packed_flags&8)!=0);
	*(long *)(action+SIMULATION_PLAYER_ACTION_UNKNOWN1E_OFFSET)=
		*(const long *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN12_OFFSET);
	*(short *)(action+SIMULATION_PLAYER_ACTION_UNKNOWN24_OFFSET)=
		*(const short *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN16_OFFSET);
	simulation_player_action_set_flag(action, 0x40000, packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN18_OFFSET]!=0);
	simulation_player_action_set_flag(action, 0x80000, packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN18_OFFSET+1]!=0);
	simulation_player_action_set_flag(action, 0x800000, packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN18_OFFSET+2]!=0);
	simulation_player_action_set_flag(action, 0x1000000, packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN18_OFFSET+3]!=0);
	if ((*(const real *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN20_OFFSET)>0.0f ||
			*(const real *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN24_OFFSET)>0.0f) &&
		*(const long *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN28_OFFSET)!=NONE)
	{
		long unknown= unknown_000a58d0(*(const long *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN28_OFFSET));

		if (unknown!=NONE)
		{
			*(long *)(action+SIMULATION_PLAYER_ACTION_UNKNOWN34_OFFSET)= unknown;
			*(long *)(action+SIMULATION_PLAYER_ACTION_UNKNOWN38_OFFSET)=
				*(const long *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN2C_OFFSET);
			*(long *)(action+SIMULATION_PLAYER_ACTION_UNKNOWN50_OFFSET)=
				*(const long *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN20_OFFSET);
			*(long *)(action+SIMULATION_PLAYER_ACTION_UNKNOWN54_OFFSET)=
				*(const long *)(packed+SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN24_OFFSET);
		}
	}
	simulation_player_action_set_flag(action, 0x40000000, packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN30_OFFSET]!=0);
	simulation_player_action_set_flag(action, 0x80000000, packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN31_OFFSET]!=0);
	action[SIMULATION_PLAYER_ACTION_UNKNOWN58_OFFSET]= packed[SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN32_OFFSET];
}

/* 0x000a5930, 0x000a5980: the number that goes in a message for an object,
when the simulation rules the game and when it does not; NONE if it has
none */
long unknown_000a5930(long object_index);
long unknown_000a5980(long object_index);

/* 0x00082b30: the part of what a player does that names an object, from a
message: two reals, the number of the object in the message and one more
number. If one of the reals is more than nothing and the game has an object
for the number (unknown_000a58d0), the object, the number and the two reals
are put where the player has them. The game has it written again in
simulation_player_action_unpack */
void simulation_player_action_unpack_object(
	const long *packed,
	long *unknown34)
{
	if ((*(const real *)&packed[0]>0.0f || *(const real *)&packed[1]>0.0f) && packed[2]!=NONE)
	{
		long object_index= unknown_000a58d0(packed[2]);

		if (object_index!=NONE)
		{
			unknown34[0]= object_index;
			unknown34[1]= packed[3];
			unknown34[7]= packed[0];
			unknown34[8]= packed[1];
		}
	}
}

/* 0x00082ac0: the other way: if one of the reals is more than nothing, the
object is still there (object_try_and_get_and_verify_type, of any type) and
there is a number for it to go in a message (unknown_000a5930 if the thing
that is going is not in its third state nor its fifth, unknown_000a5980 if
it is), that number, the other and the two reals go to the message */
void simulation_player_action_pack_object(
	const long *unknown34,
	long *packed)
{
	if (*(const real *)&unknown34[7]>0.0f || *(const real *)&unknown34[8]>0.0f)
	{
		long object_index= unknown34[0];

		if (object_index!=NONE && object_try_and_get_and_verify_type(object_index, 0xFFFFFFFF))
		{
			long state= *(const long *)(unknown_004cf770.unknownC+SIMULATION_WORLD_STATE_OFFSET);
			long number= state!=3 && state!=5 ? unknown_000a5930(object_index) : unknown_000a5980(object_index);

			if (number!=NONE)
			{
				packed[2]= number;
				packed[3]= unknown34[1];
				packed[0]= unknown34[7];
				packed[1]= unknown34[8];
			}
		}
	}
}

/* 0x00082b70: the two weapons that the unit of an object has in hand. The
unit is what the object is attached to in the end (object_get_ultimate_parent,
written here); it has to be a biped or a vehicle, and to have the object at
0x24C. With no object, or a unit that is not so, nothing is written */
void __attribute__((stdcall)) simulation_player_get_unit_weapons(
	long *weapons,
	long object_index)
{
	const struct object_header_datum *headers;
	const byte *unit;
	long unit_index;
	long parent;

	if (object_index==NONE)
	{
		return;
	}
	headers= (const struct object_header_datum *)object_header_data->data;
	parent= object_index;
	do
	{
		unit_index= parent;
		parent= *(const long *)((const byte *)headers[unit_index&0xFFFF].datum+SIMULATION_PLAYER_OBJECT_PARENT_OFFSET);
	} while (parent!=NONE);
	if (unit_index==NONE)
	{
		return;
	}
	unit= (const byte *)headers[unit_index&0xFFFF].datum;
	if (!((1u<<unit[SIMULATION_PLAYER_OBJECT_TYPE_OFFSET])&3) ||
		*(const long *)(unit+SIMULATION_PLAYER_UNIT_UNKNOWN24C_OFFSET)!=object_index)
	{
		return;
	}
	weapons[0]= unit_get_weapon(unit_index, *(const signed char *)(unit+SIMULATION_PLAYER_UNIT_WEAPON_INDICES_OFFSET));
	unit= (const byte *)((const struct object_header_datum *)object_header_data->data)[unit_index&0xFFFF].datum;
	weapons[1]= unit_get_weapon(unit_index, *(const signed char *)(unit+SIMULATION_PLAYER_UNIT_WEAPON_INDICES_OFFSET+1));
}

/* 0x000828b0: where the unit of a player is, for a message: the number that
the unit has in a message (unknown_000a5980), where it is, how it faces and
how it moves. If the unit is attached to something, that has to be a biped
or a vehicle that has the unit at 0x248, and it is told of in its place.
FALSE, with nothing written, with no unit, one attached to anything else,
or one with no number */
boolean simulation_player_get_unit_state(
	long player_index,
	byte *state)
{
	const struct object_header_datum *headers= (const struct object_header_datum *)object_header_data->data;
	const byte *player= (const byte *)player_data->data+(player_index&0xFFFF)*SIMULATION_PLAYER_PLAYER_DATUM_SIZE;
	long unit_index= *(const long *)(player+SIMULATION_PLAYER_PLAYER_UNIT_OFFSET);
	long parent_index;
	long number;

	if (unit_index==NONE)
	{
		return FALSE;
	}
	parent_index= *(const long *)((const byte *)headers[unit_index&0xFFFF].datum+SIMULATION_PLAYER_OBJECT_PARENT_OFFSET);
	if (parent_index!=NONE)
	{
		const byte *parent= (const byte *)headers[parent_index&0xFFFF].datum;

		if (!((1u<<parent[SIMULATION_PLAYER_OBJECT_TYPE_OFFSET])&3) ||
			*(const long *)(parent+SIMULATION_PLAYER_UNIT_UNKNOWN248_OFFSET)!=unit_index)
		{
			return FALSE;
		}
		unit_index= parent_index;
	}
	if (unit_index==NONE)
	{
		return FALSE;
	}
	number= unknown_000a5980(unit_index);
	if (number==NONE)
	{
		return FALSE;
	}
	memset(state, 0, SIMULATION_PLAYER_UNIT_STATE_SIZE);
	*(long *)state= number;
	object_get_origin(unit_index, (real_point3d *)(state+SIMULATION_PLAYER_UNIT_STATE_ORIGIN_OFFSET));
	object_get_orientation(unit_index, (real_vector3d *)(state+SIMULATION_PLAYER_UNIT_STATE_FORWARD_OFFSET),
		(real_vector3d *)(state+SIMULATION_PLAYER_UNIT_STATE_UP_OFFSET));
	object_get_velocities(unit_index, (real_vector3d *)(state+SIMULATION_PLAYER_UNIT_STATE_VELOCITY_OFFSET),
		(real_vector3d *)(state+SIMULATION_PLAYER_UNIT_STATE_ANGULAR_VELOCITY_OFFSET));

	return TRUE;
}

/* 0x000c7100: the object that a unit is of */
long unknown_000c7100(long unit_index);

/* 0x00082c10: the weapons in hand of the units that ride with an object.
What the object is attached to in the end is looked at
(object_get_ultimate_parent, written here), and each thing attached to
that: a biped or a vehicle that has something at 0x24C and is of the object
(unknown_000c7100) gives its two weapons in hand, each only where none was
written yet. The game has unit_get_weapon written here */
void __attribute__((stdcall)) simulation_player_get_rider_weapons(
	long object_index,
	long *weapons)
{
	long ultimate_index;
	long parent;
	long child;

	if (object_index==NONE)
	{
		return;
	}
	parent= object_index;
	do
	{
		ultimate_index= parent;
		parent= *(const long *)((const byte *)((const struct object_header_datum *)object_header_data->data)
			[ultimate_index&0xFFFF].datum+SIMULATION_PLAYER_OBJECT_PARENT_OFFSET);
	} while (parent!=NONE);
	if (ultimate_index==NONE)
	{
		return;
	}
	child= *(const long *)((const byte *)((const struct object_header_datum *)object_header_data->data)
		[ultimate_index&0xFFFF].datum+SIMULATION_PLAYER_OBJECT_FIRST_CHILD_OFFSET);
	while (child!=NONE)
	{
		const byte *unit= (const byte *)((const struct object_header_datum *)object_header_data->data)[child&0xFFFF].datum;

		if (((1u<<unit[SIMULATION_PLAYER_OBJECT_TYPE_OFFSET])&3) &&
			*(const long *)(unit+SIMULATION_PLAYER_UNIT_UNKNOWN24C_OFFSET)!=NONE && unknown_000c7100(child)==object_index)
		{
			if (weapons[0]==NONE)
			{
				weapons[0]= unit_get_weapon(child, *(const signed char *)((const byte *)
					((const struct object_header_datum *)object_header_data->data)[child&0xFFFF].datum+
					SIMULATION_PLAYER_UNIT_WEAPON_INDICES_OFFSET));
			}
			if (weapons[1]==NONE)
			{
				weapons[1]= unit_get_weapon(child, *(const signed char *)((const byte *)
					((const struct object_header_datum *)object_header_data->data)[child&0xFFFF].datum+
					SIMULATION_PLAYER_UNIT_WEAPON_INDICES_OFFSET+1));
			}
		}
		child= *(const long *)(unit+SIMULATION_PLAYER_OBJECT_NEXT_OFFSET);
	}
}

/* 0x000a9f70: an object is put at a place, as a number says; what it gives
goes on to 0x000aa260, which gives the object how it moves, how it faces and
how it turns */
long __attribute__((stdcall)) unknown_000a9f70(long object_index, long unknown, const byte *origin);
void __attribute__((stdcall)) unknown_000aa260(long unknown0, const byte *velocity, long object_index, long unknown,
	const byte *origin, const byte *forward, const byte *up, const byte *angular_velocity);

/* 0x00082980: where a unit is, taken from a message: the inverse of
simulation_player_get_unit_state. The unit is the one that the game has for
the number of the message (unknown_000a58d0), a biped or a vehicle that is
attached to nothing. It has to be moved by the one that a number says (its
number at 0x13C, with the third bit of its byte at 0x10A clear), or to be
ridden (0x248) by a unit that is. Then it is put where the message says
(unknown_000a9f70, unknown_000aa260) */
void simulation_player_apply_unit_state(
	const byte *state,
	long unknown)
{
	const struct object_header_datum *headers;
	const byte *unit;
	long unit_index= unknown_000a58d0(*(const long *)state);
	long placed;

	if (unit_index==NONE)
	{
		return;
	}
	headers= (const struct object_header_datum *)object_header_data->data;
	unit= (const byte *)headers[unit_index&0xFFFF].datum;
	if (!((1u<<unit[SIMULATION_PLAYER_OBJECT_TYPE_OFFSET])&3))
	{
		return;
	}
	if (*(const long *)(unit+SIMULATION_PLAYER_UNIT_UNKNOWN13C_OFFSET)!=unknown ||
		((unit[SIMULATION_PLAYER_UNIT_UNKNOWN10A_OFFSET]>>2)&1))
	{
		long rider_index= *(const long *)(unit+SIMULATION_PLAYER_UNIT_UNKNOWN248_OFFSET);

		if (rider_index==NONE ||
			*(const long *)((const byte *)headers[rider_index&0xFFFF].datum+SIMULATION_PLAYER_UNIT_UNKNOWN13C_OFFSET)!=unknown)
		{
			return;
		}
	}
	if (*(const long *)(unit+SIMULATION_PLAYER_OBJECT_PARENT_OFFSET)!=NONE)
	{
		return;
	}
	placed= unknown_000a9f70(unit_index, 2, state+SIMULATION_PLAYER_UNIT_STATE_ORIGIN_OFFSET);
	unknown_000aa260(placed, state+SIMULATION_PLAYER_UNIT_STATE_VELOCITY_OFFSET, unit_index, 2,
		state+SIMULATION_PLAYER_UNIT_STATE_ORIGIN_OFFSET, state+SIMULATION_PLAYER_UNIT_STATE_FORWARD_OFFSET,
		state+SIMULATION_PLAYER_UNIT_STATE_UP_OFFSET, state+SIMULATION_PLAYER_UNIT_STATE_ANGULAR_VELOCITY_OFFSET);
}
