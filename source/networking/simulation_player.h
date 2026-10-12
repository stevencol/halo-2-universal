/*
SIMULATION_PLAYER.H

What the game in the network has of a player of the game: one of the
sixteen that the thing has (simulation_world.h). The name of the file is a
guess, and so are the names of its functions.
*/

#ifndef __SIMULATION_PLAYER_H
#define __SIMULATION_PLAYER_H

#include "cseries/cseries.h"
#include "math/real_math.h"

/* ---------- constants */

enum
{
	/* of one of them: three numbers that it is given; the three numbers that
	the player is known by; the six bytes that its console is known by; the
	thing that it is of; two bytes; when what it does last came; and what it
	does */
	SIMULATION_PLAYER_UNKNOWN0_OFFSET= 0x0,
	SIMULATION_PLAYER_INDEX_OFFSET= 0x4,
	SIMULATION_PLAYER_STATE_OFFSET= 0x8,
	SIMULATION_PLAYER_IDENTIFIER_OFFSET= 0xC,
	SIMULATION_PLAYER_IDENTIFIER_LENGTH= 12,
	SIMULATION_PLAYER_MACHINE_OFFSET= 0x18,
	SIMULATION_PLAYER_MACHINE_LENGTH= 6,
	SIMULATION_PLAYER_WORLD_OFFSET= 0x20,
	SIMULATION_PLAYER_UNKNOWN24_OFFSET= 0x24,
	SIMULATION_PLAYER_UNKNOWN25_OFFSET= 0x25,
	SIMULATION_PLAYER_ACTION_TIME_OFFSET= 0x28,
	SIMULATION_PLAYER_ACTION_OFFSET= 0x2C,
	/* the states of one whose doings come from the view of its console, and
	the one of one that is never given them; and what such a view has for it,
	at 0x5098 of what it has at 0x8, and the size of what that gives */
	_simulation_player_state2= 2,
	_simulation_player_state4= 4,
	_simulation_player_state5= 5,
	SIMULATION_PLAYER_VIEW_UNKNOWN8_UNKNOWN5098_OFFSET= 0x5098,
	SIMULATION_PLAYER_PACKED_ACTION_SIZE= 0x34,
	/* of what a view has for the players: the bits of the players whose
	action it has, and where the actions are */
	SIMULATION_PLAYER_ACTIONS_MASK_OFFSET= 0x14,
	SIMULATION_PLAYER_ACTIONS_OFFSET= 0x18,
	/* and the bits of the players of which it has something else, of 0x40
	bytes, and where those are */
	SIMULATION_PLAYER_ACTIONS_OTHER_MASK_OFFSET= 0x71C,
	SIMULATION_PLAYER_ACTIONS_OTHER_OFFSET= 0x720,
	SIMULATION_PLAYER_ACTIONS_OTHER_SIZE= 0x40,
	/* of what comes of a player: which it is, and where its two things
	are, each if there is one */
	SIMULATION_PLAYER_UPDATE_INDEX_OFFSET= 0x4,
	SIMULATION_PLAYER_UPDATE_ACTION_OFFSET= 0x44,
	SIMULATION_PLAYER_UPDATE_OTHER_OFFSET= 0x4C,
	/* of what a player does (0x5C bytes): bits, sixteen bytes that go as
	they are, four bytes and two that take the place of those of the unit
	of the player, and what is made small apart. Of what it is made into
	(0x34 bytes): the sixteen bytes, four bits, the four bytes (the third
	and fourth of them, NONE when they are the same), the two, and a byte
	of the unit. Of a unit: its four bytes and a byte */
	SIMULATION_PLAYER_ACTION_FLAGS_OFFSET= 0x0,
	SIMULATION_PLAYER_ACTION_UNKNOWN4_OFFSET= 0x4,
	SIMULATION_PLAYER_ACTION_UNKNOWN4_SIZE= 0x10,
	SIMULATION_PLAYER_ACTION_UNKNOWN1E_OFFSET= 0x1E,
	SIMULATION_PLAYER_ACTION_UNKNOWN24_OFFSET= 0x24,
	SIMULATION_PLAYER_ACTION_UNKNOWN34_OFFSET= 0x34,
	SIMULATION_PLAYER_PACKED_ACTION_FLAGS_OFFSET= 0x10,
	SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN12_OFFSET= 0x12,
	SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN14_OFFSET= 0x14,
	SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN15_OFFSET= 0x15,
	SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN16_OFFSET= 0x16,
	SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN32_OFFSET= 0x32,
	SIMULATION_PLAYER_UNIT_UNKNOWN210_OFFSET= 0x210,
	SIMULATION_PLAYER_UNIT_UNKNOWN240_OFFSET= 0x240,
	/* more of what it is made into: four bytes and two more that are each
	a bit of what the player does, two reals, a number that is made into
	another, and a number. And where those go in what the player does */
	SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN18_OFFSET= 0x18,
	SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN20_OFFSET= 0x20,
	SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN24_OFFSET= 0x24,
	SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN28_OFFSET= 0x28,
	SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN2C_OFFSET= 0x2C,
	SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN30_OFFSET= 0x30,
	SIMULATION_PLAYER_PACKED_ACTION_UNKNOWN31_OFFSET= 0x31,
	SIMULATION_PLAYER_ACTION_UNKNOWN38_OFFSET= 0x38,
	SIMULATION_PLAYER_ACTION_UNKNOWN50_OFFSET= 0x50,
	SIMULATION_PLAYER_ACTION_UNKNOWN54_OFFSET= 0x54,
	SIMULATION_PLAYER_ACTION_UNKNOWN58_OFFSET= 0x58,
	/* of an object: what it is attached to and its type. Of a unit: the
	object that it is of, and the places of the two weapons that it has
	in hand */
	SIMULATION_PLAYER_OBJECT_PARENT_OFFSET= 0x14,
	SIMULATION_PLAYER_OBJECT_TYPE_OFFSET= 0xAA,
	SIMULATION_PLAYER_UNIT_UNKNOWN24C_OFFSET= 0x24C,
	SIMULATION_PLAYER_UNIT_WEAPON_INDICES_OFFSET= 0x212,
	/* of a player of the game: its unit. Of a unit: the unit that rides
	it. Of where a unit is, for a message (0x40 bytes): the number of the
	unit in the message, where it is, how it faces (two vectors) and how
	it moves (two more) */
	SIMULATION_PLAYER_PLAYER_UNIT_OFFSET= 0x2C,
	SIMULATION_PLAYER_PLAYER_DATUM_SIZE= 0x21C,
	SIMULATION_PLAYER_UNIT_UNKNOWN248_OFFSET= 0x248,
	SIMULATION_PLAYER_UNIT_STATE_SIZE= 0x40,
	SIMULATION_PLAYER_UNIT_STATE_ORIGIN_OFFSET= 0x4,
	SIMULATION_PLAYER_UNIT_STATE_FORWARD_OFFSET= 0x10,
	SIMULATION_PLAYER_UNIT_STATE_UP_OFFSET= 0x1C,
	SIMULATION_PLAYER_UNIT_STATE_VELOCITY_OFFSET= 0x28,
	SIMULATION_PLAYER_UNIT_STATE_ANGULAR_VELOCITY_OFFSET= 0x34
};

/* ---------- prototypes/SIMULATION_PLAYER.C */

void __attribute__((stdcall)) simulation_player_new(byte *player, const long *identifier, const word *machine,
	byte *world, long unknown0, long index, long state);
boolean __attribute__((stdcall)) simulation_player_get_action(byte *player, byte *action);
void __attribute__((stdcall)) simulation_player_action_pack(byte *packed, const byte *action, long unit_index);
void simulation_player_action_unpack(const byte *packed, byte *action);
void simulation_player_action_unpack_object(const long *packed, long *unknown34);
void simulation_player_action_pack_object(const long *unknown34, long *packed);
void __attribute__((stdcall)) simulation_player_get_unit_weapons(long *weapons, long object_index);
boolean simulation_player_get_unit_state(long player_index, byte *state);

/* ---------- globals */

extern real unknown_004ced44;

#endif
