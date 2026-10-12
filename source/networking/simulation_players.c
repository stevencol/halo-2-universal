/*
SIMULATION_PLAYERS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the simulation of a game keeps of its players and of the consoles that
they are on. The names are guesses.
*/

#include "simulation_players.h"

#include "cache/cache_files.h"
#include "game/game.h"
#include "main/main.h"
#include "main/main_game.h"
#include "memory/data.h"
#include "networking/network_life_cycle.h"
#include "networking/network_observer.h"
#include "networking/network_session.h"
#include "networking/simulation.h"
#include "networking/simulation_world.h"

#include <string.h>

/* ---------- prototypes */

/* 0x00152df0: a player of the game has left. 0x0014c540: something is done
with a player that had left. 0x0014be90: what the others are told of a
player becomes something else */
void __attribute__((stdcall)) unknown_00152df0(long player_index);
void unknown_0014c540(long player_index);
void __attribute__((stdcall)) unknown_0014be90(const byte *properties, long player_index);

/* 0x0014c320: something is done with two places of the players of the
game */
void __attribute__((stdcall)) unknown_0014c320(long absolute_index, long other_absolute_index);

/* ---------- code */

/* the player of the game that another console says something of: the one in
the place that it says, if it is who it says; NULL if not */
static inline __attribute__((always_inline)) const byte *simulation_players_update_get_player(
	const byte *update)
{
	const byte *datum= (const byte *)datum_try_and_get_absolute(player_data, *(const long *)update);

	if (datum && memcmp(datum+PLAYER_DATUM_IDENTIFIER_OFFSET, update+SIMULATION_PLAYERS_UPDATE_IDENTIFIER_OFFSET,
		SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE)==0)
	{
		return datum;
	}

	return NULL;
}

/* 0x00082fa0: the thing starts with nothing: no consoles, no players, and
its four numbers as NONE */
void simulation_players_reset(
	byte *players)
{
	*(long *)(players+SIMULATION_PLAYERS_UNKNOWN10_OFFSET)= NONE;
	*(long *)(players+SIMULATION_PLAYERS_UNKNOWN14_OFFSET)= NONE;
	*(long *)(players+SIMULATION_PLAYERS_UNKNOWN18_OFFSET)= NONE;
	*(long *)(players+SIMULATION_PLAYERS_UNKNOWN20_OFFSET)= NONE;
	*(unsigned long *)(players+SIMULATION_PLAYERS_MACHINE_MASK_OFFSET)= 0;
	memset(players+SIMULATION_PLAYERS_MACHINES_OFFSET, 0,
		MAXIMUM_SIMULATION_PLAYERS_MACHINES*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE);
	players[SIMULATION_PLAYERS_CHANGED_OFFSET]= FALSE;
	simulation_players_collection_clear(players+SIMULATION_PLAYERS_COLLECTION_OFFSET);
	*(long *)(players+SIMULATION_PLAYERS_UNKNOWNBCC_OFFSET)= 0;
	memset(players+SIMULATION_PLAYERS_UNKNOWNBD0_OFFSET, 0, SIMULATION_PLAYERS_UNKNOWNBD0_SIZE);
	players[SIMULATION_PLAYERS_UNKNOWNC30_OFFSET]= TRUE;
}

/* 0x00083510: what the thing has of the players is what the game has of
them now, and so is what it copies of players_globals; and the players of its
world are told (simulation_world_players_clear_unknown25), if the world is
not at 3 or at 5 */
void simulation_players_refresh(
	byte *players)
{
	byte *world;

	simulation_players_collection_clear(players+SIMULATION_PLAYERS_COLLECTION_OFFSET);
	simulation_players_collection_build(players+SIMULATION_PLAYERS_COLLECTION_OFFSET);
	*(long *)(players+SIMULATION_PLAYERS_UNKNOWNBCC_OFFSET)= players_globals->unknown2C;
	memcpy(players+SIMULATION_PLAYERS_UNKNOWNBD0_OFFSET, players_globals->unknown30, SIMULATION_PLAYERS_UNKNOWNBD0_SIZE);
	world= *(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET);
	*(long *)(players+SIMULATION_PLAYERS_UNKNOWN18_OFFSET)= NONE;
	players[SIMULATION_PLAYERS_UNKNOWNC30_OFFSET]= TRUE;
	if (*(const long *)(world+SIMULATION_PLAYERS_WORLD_STATE_OFFSET)!=3 &&
		*(const long *)(world+SIMULATION_PLAYERS_WORLD_STATE_OFFSET)!=5)
	{
		simulation_world_players_clear_unknown25(world);
	}
}

/* 0x00083570: a player of the world of the thing gets its byte of 0x24, and
loses the one after it; not if the world is at 3 or at 5, nor if there is no
such player. The game looks for the player by the low sixteen bits of its
number, and writes by all of it */
void simulation_players_mark_player(
	const byte *players,
	long player_index)
{
	byte *world= *(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET);
	long state= *(const long *)(world+SIMULATION_PLAYERS_WORLD_STATE_OFFSET);
	long absolute_index= player_index&0xFFFF;

	if (state!=3 && state!=5 && absolute_index<MAXIMUM_SIMULATION_PLAYERS &&
		*(const long *)(world+SIMULATION_PLAYERS_WORLD_PLAYERS_OFFSET+
			absolute_index*SIMULATION_PLAYERS_WORLD_PLAYER_SIZE)!=NONE)
	{
		byte *player= world+SIMULATION_PLAYERS_WORLD_PLAYERS_OFFSET+player_index*SIMULATION_PLAYERS_WORLD_PLAYER_SIZE;

		if (player[SIMULATION_PLAYERS_WORLD_PLAYER_UNKNOWN25_OFFSET])
		{
			player[SIMULATION_PLAYERS_WORLD_PLAYER_UNKNOWN25_OFFSET]= FALSE;
		}
		player[SIMULATION_PLAYERS_WORLD_PLAYER_UNKNOWN24_OFFSET]= TRUE;
	}
}

/* 0x000835c0: whether something of the thing changed: its byte says so, or
its session is established and plays a game and has another number than the
one that the thing copied. Not without a world, nor with one at 3 */
boolean simulation_players_changed(
	const byte *players)
{
	const byte *world= *(const byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET);
	const byte *session;
	boolean changed= FALSE;
	long state;

	if (!world || *(const long *)(world+SIMULATION_PLAYERS_WORLD_STATE_OFFSET)==3)
	{
		return FALSE;
	}
	if (players[SIMULATION_PLAYERS_CHANGED_OFFSET])
	{
		changed= TRUE;
	}
	session= *(const byte *const *)(players+SIMULATION_PLAYERS_SESSION_OFFSET);
	if (!session)
	{
		return changed;
	}
	state= *(const long *)(session+NETWORK_SESSION_STATE_OFFSET);
	if (state>=_network_session_state3 && state<=_network_session_state8 &&
		*(const long *)(session+SIMULATION_PLAYERS_SESSION_UNKNOWN4980_OFFSET)==4 &&
		*(const long *)(players+SIMULATION_PLAYERS_UNKNOWN18_OFFSET)!=
			*(const long *)(session+SIMULATION_PLAYERS_SESSION_UNKNOWN7618_OFFSET))
	{
		changed= TRUE;
	}

	return changed;
}

/* 0x00083b80: which of the consoles of the thing is known by six bytes: the
last one that is, or NONE */
long __attribute__((stdcall)) simulation_players_find_machine(
	const byte *players,
	const byte *identifier)
{
	unsigned long mask= *(const unsigned long *)(players+SIMULATION_PLAYERS_MACHINE_MASK_OFFSET);
	long found= NONE;
	long index;

	for (index= 0; index<MAXIMUM_SIMULATION_PLAYERS_MACHINES; index++)
	{
		if ((mask&(1<<index)) &&
			memcmp(players+SIMULATION_PLAYERS_MACHINES_OFFSET+index*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE,
				identifier, SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE)==0)
		{
			found= index;
		}
	}

	return found;
}

/* 0x00083bd0: whether the thing has a player in a place, who is someone,
and whose byte of 0xC is not set */
boolean __attribute__((stdcall)) simulation_players_has_player(
	long player_index,
	const byte *players,
	const byte *identifier)
{
	if (*(const unsigned long *)(players+SIMULATION_PLAYERS_COLLECTION_OFFSET)&(1<<player_index))
	{
		const byte *player= players+SIMULATION_PLAYERS_PLAYERS_OFFSET+player_index*SIMULATION_PLAYERS_PLAYER_SIZE;

		if (memcmp(identifier, player, SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE)==0 &&
			!player[SIMULATION_PLAYERS_PLAYER_UNKNOWNC_OFFSET])
		{
			return TRUE;
		}
	}

	return FALSE;
}

/* 0x00083c20: whether what there is of a player is not what another console
says of the player in its place: there is none there, or it is someone else,
or on another console, or one of its two numbers is another */
boolean __attribute__((stdcall)) simulation_players_player_differs(
	long player_index,
	const byte *player,
	const byte *machines,
	unsigned long mask,
	const byte *updates)
{
	if (mask&(1<<player_index))
	{
		const byte *update= updates+player_index*SIMULATION_PLAYERS_UPDATE_SIZE;

		if (memcmp(player, update, SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE)==0 &&
			memcmp(player+SIMULATION_PLAYERS_NEW_MACHINE_OFFSET,
				machines+*(const long *)(update+SIMULATION_PLAYERS_UPDATE_MACHINE_OFFSET)*
					SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE,
				SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE)==0 &&
			*(const long *)(player+SIMULATION_PLAYERS_NEW_UNKNOWN1C_OFFSET)==
				*(const long *)(update+SIMULATION_PLAYERS_UPDATE_UNKNOWN10_OFFSET) &&
			*(const long *)(player+SIMULATION_PLAYERS_NEW_UNKNOWN1C_OFFSET+4)==
				*(const long *)(update+SIMULATION_PLAYERS_UPDATE_UNKNOWN10_OFFSET+4))
		{
			return FALSE;
		}
	}

	return TRUE;
}

/* 0x00084a90: what there is of the players is nothing: no player, and each
place with its three numbers as NONE */
void simulation_players_collection_clear(
	byte *collection)
{
	long index;

	memset(collection, 0, SIMULATION_PLAYERS_COLLECTION_SIZE);
	for (index= 0; index<MAXIMUM_SIMULATION_PLAYERS; index++)
	{
		byte *player= collection+SIMULATION_PLAYERS_COLLECTION_PLAYERS_OFFSET+index*SIMULATION_PLAYERS_PLAYER_SIZE;

		player[SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET]= FALSE;
		*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN10_OFFSET)= NONE;
		*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN20_OFFSET)= NONE;
		*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN1C_OFFSET)= NONE;
	}
}

/* 0x00084ad0: what there is of the players takes each player of the game,
in its place: who it is; and of one that left, a number of it; of one that
did not, its console, two numbers, and what the others are told of it */
void __attribute__((stdcall)) simulation_players_collection_build(
	byte *collection)
{
	long absolute_index= NONE;

	for (;;)
	{
		const byte *datum;
		byte *player;
		long player_index;

		absolute_index= data_next_absolute_index(player_data, absolute_index+1);
		if (absolute_index==NONE)
		{
			break;
		}
		datum= (const byte *)player_data->data+absolute_index*player_data->size;
		player_index= ((*(const short *)datum<<16)|absolute_index)&0xFFFF;
		player= collection+SIMULATION_PLAYERS_COLLECTION_PLAYERS_OFFSET+player_index*SIMULATION_PLAYERS_PLAYER_SIZE;
		*(unsigned long *)collection|= 1<<player_index;
		memcpy(player, datum+PLAYER_DATUM_IDENTIFIER_OFFSET, SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE);
		if (datum[PLAYER_DATUM_FLAGS_OFFSET]&FLAG(_player_datum_left_bit))
		{
			player[SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET]= TRUE;
			*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN10_OFFSET)=
				*(const long *)(datum+PLAYER_DATUM_UNKNOWN10_OFFSET);
			memset(player+SIMULATION_PLAYERS_PLAYER_MACHINE_OFFSET, 0, SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE);
			*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN20_OFFSET)= NONE;
			*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN1C_OFFSET)= NONE;
		}
		else
		{
			player[SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET]= FALSE;
			*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN10_OFFSET)= NONE;
			memcpy(player+SIMULATION_PLAYERS_PLAYER_MACHINE_OFFSET, datum+PLAYER_DATUM_MACHINE_OFFSET,
				SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE);
			*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN20_OFFSET)=
				*(const long *)(datum+PLAYER_DATUM_UNKNOWN20_OFFSET);
			*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN1C_OFFSET)=
				*(const short *)(datum+PLAYER_DATUM_UNKNOWN1C_OFFSET);
			memcpy(player+SIMULATION_PLAYERS_PLAYER_PROPERTIES_OFFSET, datum+PLAYER_DATUM_PROPERTIES_OFFSET,
				SIMULATION_PLAYERS_PLAYER_PROPERTIES_SIZE);
		}
	}
}

/* 0x00084be0: the players of which there is something and that have not
left, as bits */
unsigned long simulation_players_collection_get_active_mask(
	const byte *collection)
{
	unsigned long mask= *(const unsigned long *)collection;
	unsigned long active= 0;
	long index;

	for (index= 0; index<MAXIMUM_SIMULATION_PLAYERS; index++)
	{
		unsigned long bit= 1<<index;

		if (mask&bit)
		{
			const byte *player= collection+SIMULATION_PLAYERS_COLLECTION_PLAYERS_OFFSET+
				index*SIMULATION_PLAYERS_PLAYER_SIZE;

			if (!player[SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET])
			{
				active|= bit;
			}
			else
			{
				active&= ~bit;
			}
		}
	}

	return active;
}

/* 0x00084f30: another console says that a player left: the player of the
game in that place leaves, if it is who the console says and has not left
yet. Whether it did */
boolean simulation_players_update_remove_player(
	const byte *update)
{
	long absolute_index= *(const long *)update;
	const byte *datum= (const byte *)datum_try_and_get_absolute(player_data, absolute_index);

	if (!datum)
	{
		return FALSE;
	}
	if (memcmp(datum+PLAYER_DATUM_IDENTIFIER_OFFSET, update+SIMULATION_PLAYERS_UPDATE_IDENTIFIER_OFFSET,
		SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE)!=0 ||
		(datum[PLAYER_DATUM_FLAGS_OFFSET]&FLAG(_player_datum_left_bit)))
	{
		return FALSE;
	}
	unknown_00152df0(datum_absolute_index_to_index(player_data, absolute_index));

	return TRUE;
}

/* 0x000850c0: another console says something of a player that left: the
player of the game in that place, if it is who the console says and has left,
has unknown_0014c540 done with it. Whether it had */
boolean simulation_players_update_unknown_000850c0(
	const byte *update)
{
	const byte *datum= simulation_players_update_get_player(update);

	if (!datum || !(datum[PLAYER_DATUM_FLAGS_OFFSET]&FLAG(_player_datum_left_bit)))
	{
		return FALSE;
	}
	unknown_0014c540(datum_absolute_index_to_index(player_data, *(const long *)update));

	return TRUE;
}

/* 0x000854c0: another console says what the others are told of a player:
the player of the game in that place takes it, if it is who the console says.
Whether it did */
boolean __attribute__((stdcall)) simulation_players_update_set_properties(
	const byte *update)
{
	const byte *datum= simulation_players_update_get_player(update);

	if (!datum)
	{
		return FALSE;
	}
	unknown_0014be90(update+SIMULATION_PLAYERS_UPDATE_PROPERTIES_OFFSET,
		datum_absolute_index_to_index(player_data, *(const long *)update));

	return TRUE;
}

/* 0x00084fb0: another console says something of two places of players at
once: unknown_0014c320 is done with them, if in each there is who the console
says (no one, where it says no one) and every player that there is in them
has left; and there is one in one of them at least. Whether it was done */
boolean __attribute__((stdcall)) simulation_players_update_unknown_00084fb0(
	const byte *update)
{
	long absolute_index= *(const long *)update;
	long other_absolute_index= *(const long *)(update+SIMULATION_PLAYERS_UPDATE_OTHER_INDEX_OFFSET);
	const byte *datum= (const byte *)datum_try_and_get_absolute(player_data, absolute_index);
	const byte *other_datum= (const byte *)datum_try_and_get_absolute(player_data, other_absolute_index);
	byte identifier[SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE];
	byte other_identifier[SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE];

	if (!datum && !other_datum)
	{
		return FALSE;
	}
	if (datum && !(datum[PLAYER_DATUM_FLAGS_OFFSET]&FLAG(_player_datum_left_bit)))
	{
		return FALSE;
	}
	if (other_datum && !(other_datum[PLAYER_DATUM_FLAGS_OFFSET]&FLAG(_player_datum_left_bit)))
	{
		return FALSE;
	}
	memset(identifier, 0, sizeof(identifier));
	if (datum)
	{
		memcpy(identifier, datum+PLAYER_DATUM_IDENTIFIER_OFFSET, sizeof(identifier));
	}
	memset(other_identifier, 0, sizeof(other_identifier));
	if (other_datum)
	{
		memcpy(other_identifier, other_datum+PLAYER_DATUM_IDENTIFIER_OFFSET, sizeof(other_identifier));
	}
	if (memcmp(identifier, update+SIMULATION_PLAYERS_UPDATE_IDENTIFIER_OFFSET, sizeof(identifier))!=0 ||
		memcmp(other_identifier, update+SIMULATION_PLAYERS_UPDATE_OTHER_IDENTIFIER_OFFSET, sizeof(other_identifier))!=0)
	{
		return FALSE;
	}
	unknown_0014c320(absolute_index, other_absolute_index);

	return TRUE;
}

/* 0x00084c90: what there is of the players takes what is said of one of
them, by its kind. One that left is left with who it was and when it left.
Two that change places change everything but who they are, which is as it is
said. The game does not look at whether the kind is one of the five */
void __attribute__((stdcall)) simulation_players_collection_apply_update(
	byte *collection,
	const byte *update)
{
	long player_index= *(const long *)update;
	byte *player= collection+SIMULATION_PLAYERS_COLLECTION_PLAYERS_OFFSET+player_index*SIMULATION_PLAYERS_PLAYER_SIZE;
	unsigned long *mask= (unsigned long *)collection;
	unsigned long bit= 1<<player_index;

	switch (*(const long *)(update+SIMULATION_PLAYERS_UPDATE_TYPE_OFFSET))
	{
	case _simulation_players_update_left:
		player[SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET]= TRUE;
		*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN10_OFFSET)= game_time_globals->time;
		memset(player+SIMULATION_PLAYERS_PLAYER_MACHINE_OFFSET, 0, SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE);
		*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN1C_OFFSET)= NONE;
		*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN20_OFFSET)= NONE;
		break;
	case _simulation_players_update_swap:
		{
			long other_index= *(const long *)(update+SIMULATION_PLAYERS_UPDATE_OTHER_INDEX_OFFSET);
			byte *other= collection+SIMULATION_PLAYERS_COLLECTION_PLAYERS_OFFSET+
				other_index*SIMULATION_PLAYERS_PLAYER_SIZE;
			unsigned long other_bit= 1<<other_index;
			boolean left= player[SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET];
			boolean other_left= other[SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET];
			long unknown10= *(const long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN10_OFFSET);
			long other_unknown10= *(const long *)(other+SIMULATION_PLAYERS_PLAYER_UNKNOWN10_OFFSET);
			boolean there= (*mask&bit)!=0;

			if (*mask&other_bit)
			{
				*mask|= bit;
			}
			else
			{
				*mask&= ~bit;
			}
			memcpy(player, update+SIMULATION_PLAYERS_UPDATE_OTHER_IDENTIFIER_OFFSET,
				SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE);
			player[SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET]= other_left;
			*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN10_OFFSET)= other_unknown10;
			if (there)
			{
				*mask|= other_bit;
			}
			else
			{
				*mask&= ~other_bit;
			}
			memcpy(other, update+SIMULATION_PLAYERS_UPDATE_IDENTIFIER_OFFSET, SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE);
			other[SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET]= left;
			*(long *)(other+SIMULATION_PLAYERS_PLAYER_UNKNOWN10_OFFSET)= unknown10;
		}
		break;
	case _simulation_players_update_remove:
		*mask&= ~bit;
		memset(player, 0, SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE);
		player[SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET]= FALSE;
		*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN10_OFFSET)= NONE;
		break;
	case _simulation_players_update_add:
		if (*mask&bit)
		{
			player[SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET]= FALSE;
			*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN10_OFFSET)= NONE;
		}
		else
		{
			*mask|= bit;
			memcpy(player, update+SIMULATION_PLAYERS_UPDATE_IDENTIFIER_OFFSET,
				SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE);
			if (update[SIMULATION_PLAYERS_UPDATE_NEW_LEFT_OFFSET])
			{
				player[SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET]= TRUE;
				*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN10_OFFSET)= game_time_globals->time;
			}
		}
		memcpy(player+SIMULATION_PLAYERS_PLAYER_MACHINE_OFFSET, update+SIMULATION_PLAYERS_UPDATE_NEW_MACHINE_OFFSET,
			SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE);
		*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN1C_OFFSET)=
			*(const long *)(update+SIMULATION_PLAYERS_UPDATE_NEW_UNKNOWN1C_OFFSET);
		*(long *)(player+SIMULATION_PLAYERS_PLAYER_UNKNOWN20_OFFSET)=
			*(const long *)(update+SIMULATION_PLAYERS_UPDATE_NEW_UNKNOWN20_OFFSET);
		memcpy(player+SIMULATION_PLAYERS_PLAYER_PROPERTIES_OFFSET, update+SIMULATION_PLAYERS_UPDATE_PROPERTIES_OFFSET,
			SIMULATION_PLAYERS_PLAYER_PROPERTIES_SIZE);
		break;
	case _simulation_players_update_properties:
		memcpy(player+SIMULATION_PLAYERS_PLAYER_PROPERTIES_OFFSET, update+SIMULATION_PLAYERS_UPDATE_PROPERTIES_OFFSET,
			SIMULATION_PLAYERS_PLAYER_PROPERTIES_SIZE);
		break;
	}
}

/* 0x00084e90: two players of what there is change places: what says so is
made, with who is in each place (no one, where there is no one), and is
taken */
void __attribute__((stdcall)) simulation_players_collection_swap(
	byte *update,
	long player_index,
	byte *collection,
	long other_player_index)
{
	unsigned long mask= *(const unsigned long *)collection;

	*(long *)(update+SIMULATION_PLAYERS_UPDATE_TYPE_OFFSET)= _simulation_players_update_swap;
	*(long *)update= player_index;
	if (mask&(1<<player_index))
	{
		memcpy(update+SIMULATION_PLAYERS_UPDATE_IDENTIFIER_OFFSET,
			collection+SIMULATION_PLAYERS_COLLECTION_PLAYERS_OFFSET+player_index*SIMULATION_PLAYERS_PLAYER_SIZE,
			SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE);
	}
	else
	{
		memset(update+SIMULATION_PLAYERS_UPDATE_IDENTIFIER_OFFSET, 0, SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE);
	}
	*(long *)(update+SIMULATION_PLAYERS_UPDATE_OTHER_INDEX_OFFSET)= other_player_index;
	if (mask&(1<<other_player_index))
	{
		memcpy(update+SIMULATION_PLAYERS_UPDATE_OTHER_IDENTIFIER_OFFSET,
			collection+SIMULATION_PLAYERS_COLLECTION_PLAYERS_OFFSET+other_player_index*SIMULATION_PLAYERS_PLAYER_SIZE,
			SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE);
	}
	else
	{
		memset(update+SIMULATION_PLAYERS_UPDATE_OTHER_IDENTIFIER_OFFSET, 0, SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE);
	}
	simulation_players_collection_apply_update(collection, update);
}

/* 0x00083a40: what there is to tell another console of the players, if
something of it changed since it was last taken (the byte at 0xC30), or
always: the consoles and which of them there are, which players there are
and which of them have not left (simulation_players_collection_get_active_mask),
and the twelve bytes that each player that there is is known by, with
nothing for the others. It is taken: the byte is clear. FALSE, with nothing
written, if there is nothing to tell. The game has the sixteen players
written four by four */
boolean __attribute__((stdcall)) simulation_players_take_update(
	byte *identifiers,
	byte *machines,
	boolean always,
	byte *players,
	long *machine_mask,
	long *player_mask,
	long *active_mask)
{
	const unsigned long *collection;
	long index;

	if (!always && !players[SIMULATION_PLAYERS_UNKNOWNC30_OFFSET])
	{
		return FALSE;
	}
	*machine_mask= *(const long *)(players+SIMULATION_PLAYERS_MACHINE_MASK_OFFSET);
	memcpy(machines, players+SIMULATION_PLAYERS_MACHINES_OFFSET,
		MAXIMUM_SIMULATION_PLAYERS_MACHINES*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE);
	collection= (const unsigned long *)(players+SIMULATION_PLAYERS_COLLECTION_OFFSET);
	*player_mask= *collection;
	*active_mask= simulation_players_collection_get_active_mask((byte *)collection);
	memset(identifiers, 0, MAXIMUM_SIMULATION_PLAYERS*SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE);
	for (index= 0; index<MAXIMUM_SIMULATION_PLAYERS; index++)
	{
		if (*collection&(1u<<index))
		{
			memcpy(identifiers+index*SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE,
				players+SIMULATION_PLAYERS_PLAYERS_OFFSET+index*SIMULATION_PLAYERS_PLAYER_SIZE,
				SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE);
		}
	}
	players[SIMULATION_PLAYERS_UNKNOWNC30_OFFSET]= FALSE;

	return TRUE;
}

/* 0x0014c630: the rest of the game is told which consoles there are, and
their six bytes. 0x0014c880: and which of them this one is, by its six
bytes, or that it is none */
void __attribute__((stdcall)) unknown_0014c630(unsigned long machine_mask, const byte *machines);
void __attribute__((stdcall)) unknown_0014c880(const byte *machine);

/* 0x004d8ba0, of main.c, and 0x00527330, of main_game.c: there is a game in
the network. 0x00527378: something of it, which is given this */
extern boolean unknown_004d8ba0;
extern boolean unknown_00527330;
byte *unknown_00527378;

/* the thing that is going gets the six bytes of this console and which
console it is: the game has it written in each of the three places */
static inline __attribute__((always_inline)) void simulation_players_set_world_machine(
	byte *players)
{
	byte *world= *(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET);

	memcpy(world+SIMULATION_PLAYERS_WORLD_MACHINE_OFFSET,
		players+SIMULATION_PLAYERS_MACHINES_OFFSET+
			*(const long *)(players+SIMULATION_PLAYERS_UNKNOWN20_OFFSET)*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE,
		SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE);
	world[SIMULATION_PLAYERS_WORLD_HAS_MACHINE_OFFSET]= TRUE;
	*(long *)(*(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET)+SIMULATION_PLAYERS_WORLD_MACHINE_INDEX_OFFSET)=
		*(const long *)(players+SIMULATION_PLAYERS_UNKNOWN20_OFFSET);
}

/* 0x00083000: what the simulation has of the players, for a new map. In a
game of the first mode or the second with a game in the network, it gets
the session of that (network_life_cycle_get_any_session), if it is in some
state and has its consoles. With a session: something of the game in the
network gets this; the consoles are those of the session, this one being
the one that the session says, and it has changed, for the rest of the game
and for the other consoles. With none: the consoles are those of the
options of the game, and this one is the last of them that has the six
bytes that the options say, if they say any. In both, the thing that is
going gets this console (which the game does with none found, too, in the
first) */
void __attribute__((stdcall)) simulation_players_initialize_for_new_map(
	byte *players)
{
	struct main_game_globals *game= main_game_globals;
	byte *session;
	long index;

	if (game->options.game_mode==1 || game->options.game_mode==2)
	{
		session= NULL;
		if (unknown_004d8ba0 && network_life_cycle_get_any_session(&session) &&
			*(const long *)(session+SIMULATION_PLAYERS_SESSION_STATE_OFFSET)!=0 &&
			*(const long *)(session+SIMULATION_PLAYERS_SESSION_MEMBERSHIP_OFFSET)!=NONE)
		{
			*(byte **)(players+SIMULATION_PLAYERS_SESSION_OFFSET)= session;
		}
	}
	session= *(byte *const *)(players+SIMULATION_PLAYERS_SESSION_OFFSET);
	if (session)
	{
		const byte *membership= NULL;
		long local_peer= NONE;
		byte *unknown;
		long count;
		unsigned long machine_mask;
		byte machines[MAXIMUM_SIMULATION_PLAYERS_MACHINES*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE];

		if (*(const long *)(session+SIMULATION_PLAYERS_SESSION_STATE_OFFSET)!=0 &&
			*(const long *)(session+SIMULATION_PLAYERS_SESSION_MEMBERSHIP_OFFSET)!=NONE)
		{
			local_peer= *(const long *)(session+SIMULATION_PLAYERS_SESSION_LOCAL_PEER_OFFSET);
			membership= session+SIMULATION_PLAYERS_SESSION_MEMBERSHIP_OFFSET;
		}
		unknown= unknown_00527330 ? unknown_00527378 : NULL;
		*(byte **)(players+SIMULATION_PLAYERS_UNKNOWN8_OFFSET)= unknown;
		*(byte **)(unknown+SIMULATION_PLAYERS_UNKNOWN8_PLAYERS_OFFSET)= players;
		*(long *)(unknown+SIMULATION_PLAYERS_UNKNOWN8_UNKNOWN84_OFFSET)= NONE;
		*(long *)(unknown+SIMULATION_PLAYERS_UNKNOWN8_UNKNOWN88_OFFSET)= NONE;
		count= *(const long *)(membership+SIMULATION_PLAYERS_MEMBERSHIP_PEER_COUNT_OFFSET);
		machine_mask= (1u<<count)-1;
		memset(machines, 0, sizeof(machines));
		for (index= 0; index<count; index++)
		{
			memcpy(machines+index*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE,
				membership+SIMULATION_PLAYERS_MEMBERSHIP_PEER_MACHINE_OFFSET+index*SIMULATION_PLAYERS_MEMBERSHIP_PEER_SIZE,
				SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE);
		}
		*(long *)(players+SIMULATION_PLAYERS_UNKNOWN10_OFFSET)=
			*(const long *)(*(byte *const *)(players+SIMULATION_PLAYERS_SESSION_OFFSET)+SIMULATION_PLAYERS_SESSION_UNKNOWN7618_OFFSET);
		*(unsigned long *)(players+SIMULATION_PLAYERS_MACHINE_MASK_OFFSET)= machine_mask;
		*(long *)(players+SIMULATION_PLAYERS_UNKNOWN14_OFFSET)= *(const long *)membership;
		*(long *)(players+SIMULATION_PLAYERS_UNKNOWN20_OFFSET)= local_peer;
		memcpy(players+SIMULATION_PLAYERS_MACHINES_OFFSET, machines, sizeof(machines));
		simulation_players_set_world_machine(players);
		players[SIMULATION_PLAYERS_CHANGED_OFFSET]= TRUE;
		players[SIMULATION_PLAYERS_UNKNOWNC30_OFFSET]= TRUE;
	}
	else
	{
		*(unsigned long *)(players+SIMULATION_PLAYERS_MACHINE_MASK_OFFSET)= game->options.machine_valid_mask;
		memcpy(players+SIMULATION_PLAYERS_MACHINES_OFFSET, game->options.machine_identifiers,
			MAXIMUM_SIMULATION_PLAYERS_MACHINES*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE);
		players[SIMULATION_PLAYERS_CHANGED_OFFSET]= FALSE;
		*(long *)(players+SIMULATION_PLAYERS_UNKNOWN20_OFFSET)= NONE;
		if (game->options.local_machine_exists)
		{
			for (index= 0; index<MAXIMUM_SIMULATION_PLAYERS_MACHINES; index++)
			{
				if ((*(const unsigned long *)(players+SIMULATION_PLAYERS_MACHINE_MASK_OFFSET)&(1u<<index)) &&
					memcmp(players+SIMULATION_PLAYERS_MACHINES_OFFSET+index*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE,
						main_game_globals->options.local_machine_identifier, SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE)==0)
				{
					*(long *)(players+SIMULATION_PLAYERS_UNKNOWN20_OFFSET)= index;
				}
			}
		}
		if (*(const long *)(players+SIMULATION_PLAYERS_UNKNOWN20_OFFSET)!=NONE)
		{
			simulation_players_set_world_machine(players);
		}
	}
}

/* 0x00083370: what the simulation has of the players, after a state of the
game was loaded, as bits say. Without the third bit: the rest of the game
is told which console this one is (unknown_0014c880): the last of those of
the players of the game that has the six bytes that the thing that is going
has, or none; and it has changed. With it: there is one console, this one,
with the six bytes that the players of the game have for it, if they have
any; the thing that is going gets it, the rest of the game is told
(unknown_0014c630, unknown_0014c880), and it has not changed. Then what it
has is brought up to date (simulation_players_refresh) */
void __attribute__((stdcall)) simulation_players_after_load(
	byte *players,
	unsigned long flags)
{
	const byte *globals= (const byte *)players_globals;
	byte machine[SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE];

	if (!(flags&4))
	{
		unsigned long machine_mask= *(const unsigned long *)(globals+SIMULATION_PLAYERS_GLOBALS_MACHINE_MASK_OFFSET);
		byte machines[MAXIMUM_SIMULATION_PLAYERS_MACHINES*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE];
		long found= NONE;
		long index;

		memcpy(machine, *(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET)+SIMULATION_PLAYERS_WORLD_MACHINE_OFFSET,
			sizeof(machine));
		memcpy(machines, globals+SIMULATION_PLAYERS_GLOBALS_MACHINES_OFFSET, sizeof(machines));
		for (index= 0; index<MAXIMUM_SIMULATION_PLAYERS_MACHINES; index++)
		{
			if ((machine_mask&(1u<<index)) &&
				memcmp(machines+index*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE, machine, sizeof(machine))==0)
			{
				found= index;
			}
		}
		unknown_0014c880(found==NONE ? NULL : machines+found*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE);
		players[SIMULATION_PLAYERS_CHANGED_OFFSET]= TRUE;
		simulation_players_refresh(players);
	}
	else
	{
		boolean exists= globals[SIMULATION_PLAYERS_GLOBALS_LOCAL_MACHINE_EXISTS_OFFSET];

		if (exists)
		{
			memcpy(machine, globals+SIMULATION_PLAYERS_GLOBALS_LOCAL_MACHINE_OFFSET, sizeof(machine));
		}
		*(unsigned long *)(players+SIMULATION_PLAYERS_MACHINE_MASK_OFFSET)= 1;
		*(long *)(players+SIMULATION_PLAYERS_UNKNOWN20_OFFSET)= 0;
		memset(players+SIMULATION_PLAYERS_MACHINES_OFFSET, 0,
			MAXIMUM_SIMULATION_PLAYERS_MACHINES*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE);
		if (exists)
		{
			memcpy(players+SIMULATION_PLAYERS_MACHINES_OFFSET+
					*(const long *)(players+SIMULATION_PLAYERS_UNKNOWN20_OFFSET)*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE,
				machine, sizeof(machine));
		}
		simulation_players_set_world_machine(players);
		unknown_0014c630(*(const unsigned long *)(players+SIMULATION_PLAYERS_MACHINE_MASK_OFFSET),
			players+SIMULATION_PLAYERS_MACHINES_OFFSET);
		unknown_0014c880(players+SIMULATION_PLAYERS_MACHINES_OFFSET+
			*(const long *)(players+SIMULATION_PLAYERS_UNKNOWN20_OFFSET)*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE);
		players[SIMULATION_PLAYERS_CHANGED_OFFSET]= FALSE;
		simulation_players_refresh(players);
	}
}

/* 0x00085650: a view gets its channel of the observer, for a console of a
session */
void __attribute__((stdcall)) unknown_00085650(const byte *peer, byte *observer, byte *view, long channel_index);

/* 0x00084630: a view of another console is made for the simulation, if the
thing that is going lets it, the console has a channel of the observer that
the session knows (network_session_has_channel_peer), that channel is in its
seventh state, and the channel of the network under it is in its fifth. The
kind of the view is by the state of the thing (2, 1, 4 and 3 for the states
2 to 5: the game looks at no other); it is made with the six bytes of the
console and its place in the session (simulation_view_create). Then the view
that gets the channel (unknown_00085650) is the one with those six bytes, if
the thing is not in its third state or its fifth
(simulation_world_get_view_by_unknown14), or the first of the first kind or
the third (simulation_world_unknown_0006acb0): TRUE if there is one */
boolean __attribute__((stdcall)) simulation_players_create_view(
	long channel_index,
	byte *players,
	long peer_index,
	const byte *peer)
{
	byte *world= *(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET);
	const byte *observer_channel;
	const byte *channel;
	word machine[SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE/sizeof(word)];
	byte *view;
	long kind;

	if (!world[SIMULATION_PLAYERS_WORLD_UNKNOWN2F_OFFSET] || channel_index==NONE ||
		!(boolean)network_session_has_channel_peer(*(byte *const *)(players+SIMULATION_PLAYERS_SESSION_OFFSET), channel_index))
	{
		return FALSE;
	}
	observer_channel= *(byte *const *)(players+SIMULATION_PLAYERS_UNKNOWN8_OFFSET)+
		channel_index*SIMULATION_PLAYERS_OBSERVER_CHANNEL_SIZE+SIMULATION_PLAYERS_OBSERVER_CHANNELS_OFFSET;
	if (*(const long *)observer_channel!=_simulation_players_observer_channel_state7)
	{
		return FALSE;
	}
	channel= unknown_004d87d4+
		*(const long *)(observer_channel+SIMULATION_PLAYERS_OBSERVER_CHANNEL_CHANNEL_OFFSET)*SIMULATION_PLAYERS_CHANNEL_SIZE;
	if (*(const long *)(channel+SIMULATION_PLAYERS_CHANNEL_STATE_OFFSET)!=_simulation_players_channel_state5)
	{
		return FALSE;
	}
	switch (*(const long *)(world+SIMULATION_PLAYERS_WORLD_STATE_OFFSET))
	{
	case 2:
		kind= 2;
		break;
	case 3:
		kind= 1;
		break;
	case 4:
		kind= 4;
		break;
	case 5:
		kind= 3;
		break;
	default:
		__builtin_unreachable();
	}
	memcpy(machine, peer+SIMULATION_PLAYERS_PEER_MACHINE_OFFSET, sizeof(machine));
	simulation_view_create(kind, machine, peer_index, 0);
	world= *(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET);
	if (simulation_unknown_00067e10(world))
	{
		view= simulation_world_get_view_by_unknown14(world, machine);
	}
	else
	{
		view= simulation_world_unknown_0006acb0(world);
	}
	if (!view)
	{
		return FALSE;
	}
	unknown_00085650(peer, *(byte *const *)(players+SIMULATION_PLAYERS_UNKNOWN8_OFFSET), view, channel_index);

	return TRUE;
}

/* 0x00084560: what a console that does not rule the game does for its view
of the one that does. If the thing that is going has no view of the first
kind or the third, or that view has failed, the thing gives up
(simulation_world_unknown_0006b040). FALSE if the thing has failed for good
(1 at 0x18). If it has no such view, one is made
(simulation_players_create_view) for the console that rules the session, by
the channel of the observer that the session has for it, or NONE */
boolean __attribute__((stdcall)) simulation_players_update_views_client(
	byte *players)
{
	byte *world= *(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET);
	struct simulation_view_iterator iterator= {SIMULATION_WORLD_UNKNOWN_0006ACB0_KINDS, 0};
	byte *view= NULL;

	simulation_world_next_view(&iterator, world, &view);
	if (!view || *(const long *)(view+SIMULATION_PLAYERS_VIEW_UNKNOWN28_OFFSET)!=0)
	{
		simulation_world_unknown_0006b040(world);
	}
	world= *(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET);
	if (*(const long *)(world+SIMULATION_PLAYERS_WORLD_UNKNOWN18_OFFSET)==1)
	{
		return FALSE;
	}
	view= NULL;
	iterator.kind_mask= SIMULATION_WORLD_UNKNOWN_0006ACB0_KINDS;
	iterator.index= 0;
	simulation_world_next_view(&iterator, world, &view);
	if (!view)
	{
		const byte *session= *(byte *const *)(players+SIMULATION_PLAYERS_SESSION_OFFSET);
		long host= *(const long *)(session+SIMULATION_PLAYERS_SESSION_HOST_PEER_OFFSET);
		long channel_index= NONE;

		if (host>=0 && host<*(const long *)(session+SIMULATION_PLAYERS_SESSION_PEER_COUNT_OFFSET))
		{
			const byte *peer_channel= session+SIMULATION_PLAYERS_SESSION_PEER_CHANNELS_OFFSET+
				host*SIMULATION_PLAYERS_SESSION_PEER_CHANNEL_SIZE;

			if (peer_channel[SIMULATION_PLAYERS_SESSION_PEER_CHANNEL_VALID_OFFSET])
			{
				channel_index= *(const long *)(peer_channel+SIMULATION_PLAYERS_SESSION_PEER_CHANNEL_INDEX_OFFSET);
			}
		}
		simulation_players_create_view(channel_index, players, host,
			session+SIMULATION_PLAYERS_SESSION_PEERS_OFFSET+host*SIMULATION_PLAYERS_MEMBERSHIP_PEER_SIZE);
	}

	return TRUE;
}

/* 0x0006ae20, 0x0006aee0: something that the thing that is going has to do
before its views are looked at, for a console that does not rule the session
and for the one that does. 0x00083f00: the consoles of the session are taken */
boolean __attribute__((stdcall)) unknown_00083f00(long peer_count, byte *players, long unknown7618, long membership_number,
	const byte *peers);

/* 0x000843f0: what the console that rules the game does for its views of
the others. Each view that has failed is done with
(simulation_world_view_delete, written here); the consoles of the others
are noted. FALSE if the thing that is going has failed for good. Then a
view is made (simulation_players_create_view) for each console of the
session that is not this one and has none, by the channel of the observer
that the session has for it, or NONE. The second thing that it is given is
not looked at */
boolean __attribute__((stdcall)) simulation_players_update_views_host(
	byte *players,
	long unused)
{
	struct simulation_view_iterator iterator= {0xFFFFFFFF, 0};
	unsigned long peer_mask= 0;
	byte *view;
	const byte *session;
	long local_peer;
	long peer;

	(void)unused;
	while (simulation_world_next_view(&iterator, *(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET), &view))
	{
		if (*(const long *)(view+SIMULATION_PLAYERS_VIEW_UNKNOWN28_OFFSET)!=0)
		{
			simulation_world_view_delete(view);
		}
		else
		{
			peer_mask|= 1u<<*(const long *)(view+SIMULATION_PLAYERS_VIEW_PEER_INDEX_OFFSET);
		}
	}
	if (*(const long *)(*(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET)+SIMULATION_PLAYERS_WORLD_UNKNOWN18_OFFSET)==1)
	{
		return FALSE;
	}
	session= *(byte *const *)(players+SIMULATION_PLAYERS_SESSION_OFFSET);
	local_peer= *(const long *)(session+SIMULATION_PLAYERS_SESSION_LOCAL_PEER_OFFSET);
	for (peer= 0; peer<*(const long *)(session+SIMULATION_PLAYERS_SESSION_PEER_COUNT_OFFSET); peer++)
	{
		if (peer!=local_peer && !(peer_mask&(1u<<peer)))
		{
			const byte *other_session= *(byte *const *)(players+SIMULATION_PLAYERS_SESSION_OFFSET);
			const byte *peer_channel= other_session+SIMULATION_PLAYERS_SESSION_PEER_CHANNELS_OFFSET+
				peer*SIMULATION_PLAYERS_SESSION_PEER_CHANNEL_SIZE;
			long channel_index= NONE;

			if (peer<*(const long *)(other_session+SIMULATION_PLAYERS_SESSION_PEER_COUNT_OFFSET) &&
				peer_channel[SIMULATION_PLAYERS_SESSION_PEER_CHANNEL_VALID_OFFSET])
			{
				channel_index= *(const long *)(peer_channel+SIMULATION_PLAYERS_SESSION_PEER_CHANNEL_INDEX_OFFSET);
			}
			simulation_players_create_view(channel_index, players, peer,
				session+SIMULATION_PLAYERS_SESSION_PEERS_OFFSET+peer*SIMULATION_PLAYERS_MEMBERSHIP_PEER_SIZE);
		}
	}

	return TRUE;
}

/* 0x00083220: the views of the simulation are kept up, outside the ticks;
FALSE makes the simulation fail. With no thing that is going, FALSE; with
one in its first state, there is nothing to do. It needs a game in the
network, in the third state of its life cycle or the eighth, and a session
in some state. If the session is not well (in its fifth state for the
console that rules it, established for the others), the thing gives up, and
that is all. Then: something that the thing has to do first, when the
console rules the session and the thing does not rule the game or the other
way round (simulation_world_become_authority, simulation_world_become_client); something more of the thing
if the session says so; the consoles of the session are taken
(unknown_00083f00); and the views, as the one that rules the game or as
another (simulation_players_update_views_host, simulation_players_update_views_client) */
boolean simulation_players_update_views(
	byte *players)
{
	byte *world= *(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET);
	byte *session;
	const byte *membership;
	boolean host;
	boolean authority;
	long local_peer;
	long host_peer;

	if (!world)
	{
		return FALSE;
	}
	if (*(const long *)(world+SIMULATION_PLAYERS_WORLD_STATE_OFFSET)==1)
	{
		return TRUE;
	}
	if (!unknown_00527330 ||
		(unknown_00527334!=_simulation_players_life_cycle_state3 && unknown_00527334!=_simulation_players_life_cycle_state8))
	{
		return FALSE;
	}
	session= *(byte *const *)(players+SIMULATION_PLAYERS_SESSION_OFFSET);
	if (!session || *(const long *)(session+SIMULATION_PLAYERS_SESSION_STATE_OFFSET)==0)
	{
		return FALSE;
	}
	if (!(network_session_unknown_00058d20(session) ?
			*(const long *)(session+SIMULATION_PLAYERS_SESSION_STATE_OFFSET)==_simulation_players_session_state5 :
			network_session_established(session)))
	{
		simulation_world_unknown_0006b040(world);
		return TRUE;
	}
	host= network_session_unknown_00058d20(session);
	authority= simulation_unknown_00067e10(world);
	if (!host)
	{
		if (authority && !simulation_world_become_client(world))
		{
			return FALSE;
		}
	}
	else if (!authority && !simulation_world_become_authority(world))
	{
		return FALSE;
	}
	if (network_session_unknown_00058d90(*(byte *const *)(players+SIMULATION_PLAYERS_SESSION_OFFSET)))
	{
		if (host)
		{
			simulation_world_unknown_0006b310(*(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET));
		}
		else
		{
			simulation_world_unknown_0006b350(*(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET));
		}
	}
	session= *(byte *const *)(players+SIMULATION_PLAYERS_SESSION_OFFSET);
	membership= network_session_get_membership(&local_peer, session, &host_peer);
	if (!unknown_00083f00(*(const long *)(membership+SIMULATION_PLAYERS_MEMBERSHIP_PEER_COUNT_OFFSET), players,
			*(const long *)(session+SIMULATION_PLAYERS_SESSION_UNKNOWN7618_OFFSET), *(const long *)membership,
			membership+0xC))
	{
		return FALSE;
	}
	if (simulation_unknown_00067e10(*(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET)))
	{
		return simulation_players_update_views_host(players, local_peer);
	}

	return simulation_players_update_views_client(players);
}

/* 0x004e9bb8: the name of a map that is being brought to the disk, or
nothing */
const char *unknown_004e9bb8;

/* 0x00083db0: what the simulation is at, as a number from 0 to 13. 0 with
no thing that is going, 1 with one in its first state, 2 while a map is
being brought to the disk (cached_map_file_status says neither that it is
there nor that it failed). 0 with no game in the network, or one that is
not in the third state of its life cycle or the eighth. Then by the number
that the thing has at 0x18: 4, 3, 5 and 6 for 3 to 6 when it rules the
game; 10, 3 and 13 for 3, 4 and 6 when it does not. And with any other, by
the state of the session, from 1 to 9: 11, 12, 9, 13, 4, 6, 5, 7 and 8 */
long simulation_players_get_status(
	const byte *players)
{
	const byte *world= *(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET);
	const byte *session;
	long state;

	if (!world)
	{
		return 0;
	}
	if (*(const long *)(world+SIMULATION_PLAYERS_WORLD_STATE_OFFSET)==1)
	{
		return 1;
	}
	if (unknown_004e9bb8)
	{
		long status= cached_map_file_status(unknown_004e9bb8);

		if (status!=_cached_map_file_status_cached && status!=_cached_map_file_status_failed)
		{
			return 2;
		}
	}
	if (!unknown_00527330 ||
		(unknown_00527334!=_simulation_players_life_cycle_state3 && unknown_00527334!=_simulation_players_life_cycle_state8))
	{
		return 0;
	}
	world= *(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET);
	state= *(const long *)(world+SIMULATION_PLAYERS_WORLD_STATE_OFFSET);
	if (state!=3 && state!=5)
	{
		switch (*(const long *)(world+SIMULATION_PLAYERS_WORLD_UNKNOWN18_OFFSET))
		{
		case 3:
			return 4;
		case 4:
			return 3;
		case 5:
			return 5;
		case 6:
			return 6;
		}
	}
	else
	{
		switch (*(const long *)(world+SIMULATION_PLAYERS_WORLD_UNKNOWN18_OFFSET))
		{
		case 3:
			return 10;
		case 4:
			return 3;
		case 6:
			return 13;
		}
	}
	session= *(byte *const *)(players+SIMULATION_PLAYERS_SESSION_OFFSET);
	if (!session)
	{
		return 0;
	}
	switch (*(const long *)(session+SIMULATION_PLAYERS_SESSION_STATE_OFFSET))
	{
	case 1:
		return 11;
	case 2:
		return 12;
	case 3:
		return 9;
	case 4:
		return 13;
	case 5:
		return 4;
	case 6:
		return 6;
	case 7:
		return 5;
	case 8:
		return 7;
	case 9:
		return 8;
	}

	return 0;
}

/* 0x00084270: a change of the consoles was taken by the game. Each player
that there is and has not left, whose console is none of those that there
are now, leaves: as an update of the first kind
(simulation_players_collection_apply_update), and when the thing that is
going rules the game and has that player, its byte at 0x25 is clear and its
byte at 0x24 set. Then the copy of the consoles that the game has is the
one of now, and the number at 0x18 is NONE */
void __attribute__((stdcall)) simulation_players_machines_changed(
	byte *players)
{
	long index;

	for (index= 0; index<MAXIMUM_SIMULATION_PLAYERS; index++)
	{
		const byte *player= players+SIMULATION_PLAYERS_PLAYERS_OFFSET+index*SIMULATION_PLAYERS_PLAYER_SIZE;
		unsigned long machine_mask;
		long machine;
		boolean found= FALSE;
		byte update[SIMULATION_PLAYERS_UPDATE_SIZE];
		byte *world;
		long state;

		if (!(*(const unsigned long *)(players+SIMULATION_PLAYERS_COLLECTION_OFFSET)&(1u<<index)) ||
			player[SIMULATION_PLAYERS_PLAYER_LEFT_OFFSET])
		{
			continue;
		}
		machine_mask= *(const unsigned long *)(players+SIMULATION_PLAYERS_MACHINE_MASK_OFFSET);
		for (machine= 0; machine<MAXIMUM_SIMULATION_PLAYERS_MACHINES; machine++)
		{
			if ((machine_mask&(1u<<machine)) &&
				memcmp(players+SIMULATION_PLAYERS_MACHINES_OFFSET+machine*SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE,
					player+SIMULATION_PLAYERS_PLAYER_MACHINE_OFFSET, SIMULATION_PLAYERS_MACHINE_IDENTIFIER_SIZE)==0)
			{
				found= TRUE;
				break;
			}
		}
		if (found)
		{
			continue;
		}
		memcpy(update+SIMULATION_PLAYERS_UPDATE_IDENTIFIER_OFFSET, player, SIMULATION_PLAYERS_PLAYER_IDENTIFIER_SIZE);
		*(long *)(update+SIMULATION_PLAYERS_UPDATE_TYPE_OFFSET)= _simulation_players_update_left;
		*(long *)update= index;
		simulation_players_collection_apply_update(players+SIMULATION_PLAYERS_COLLECTION_OFFSET, update);
		world= *(byte *const *)(players+SIMULATION_PLAYERS_WORLD_OFFSET);
		state= *(const long *)(world+SIMULATION_PLAYERS_WORLD_STATE_OFFSET);
		if (state!=3 && state!=5)
		{
			byte *world_player= world+SIMULATION_PLAYERS_WORLD_PLAYERS_OFFSET+index*SIMULATION_PLAYERS_WORLD_PLAYER_SIZE;

			if (*(const long *)world_player!=NONE)
			{
				if (world_player[SIMULATION_PLAYERS_WORLD_PLAYER_UNKNOWN25_OFFSET])
				{
					world_player[SIMULATION_PLAYERS_WORLD_PLAYER_UNKNOWN25_OFFSET]= FALSE;
				}
				world_player[SIMULATION_PLAYERS_WORLD_PLAYER_UNKNOWN24_OFFSET]= TRUE;
			}
		}
	}
	*(unsigned long *)(players+SIMULATION_PLAYERS_UNKNOWNBCC_OFFSET)=
		*(const unsigned long *)(players+SIMULATION_PLAYERS_MACHINE_MASK_OFFSET);
	memcpy(players+SIMULATION_PLAYERS_UNKNOWNBD0_OFFSET, players+SIMULATION_PLAYERS_MACHINES_OFFSET,
		SIMULATION_PLAYERS_UNKNOWNBD0_SIZE);
	*(long *)(players+SIMULATION_PLAYERS_UNKNOWN18_OFFSET)= NONE;
}
