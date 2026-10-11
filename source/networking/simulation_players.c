/*
SIMULATION_PLAYERS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

What the simulation of a game keeps of its players and of the consoles that
they are on. The names are guesses.
*/

#include "simulation_players.h"

#include "main/main.h"
#include "memory/data.h"
#include "networking/network_session.h"
#include "networking/simulation_world.h"

#include <string.h>

/* ---------- prototypes */

/* 0x00152df0: a player of the game has left */
void __attribute__((stdcall)) unknown_00152df0(long player_index);

/* ---------- code */

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
