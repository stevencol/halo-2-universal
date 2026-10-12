/*
PLAYERS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The players of a game. The names are guesses.
*/

#include "players.h"

#include "main/main.h"
#include "memory/data.h"
#include "saved games/game_state.h"
#include "networking/transport_security.h"

#include <string.h>

/* ---------- code */

/* 0x0014b410: who the player of a gamepad of this console is, when it is no
one of the service of games: the six bytes that this console is known by
(unknown_004cf7cc), then the number of the gamepad in sixteen bits, and a
number that says that it was made here */
void __attribute__((stdcall)) player_identifier_from_controller(
	byte *identifier,
	long controller_index)
{
	unsigned long long value= 0;
	unsigned long made= PLAYER_IDENTIFIER_LOCAL;
	long index;

	memset(identifier, 0, PLAYER_IDENTIFIER_SIZE);
	for (index= TRANSPORT_SECURITY_ADDRESS_UNKNOWNA_SIZE-1; index>=0; index--)
	{
		value= (value<<8)|unknown_004cf7cc[index];
	}
	value= (value<<16)|(unsigned long long)(long long)controller_index;
	memcpy(identifier, &value, sizeof(value));
	memcpy(identifier+sizeof(value), &made, sizeof(made));
}

/* 0x0014dde0: the first of the four places of this console that has a
player, or NONE */
long players_first_local_player(
	void)
{
	long local_player_index;

	for (local_player_index= 0; local_player_index<MAXIMUM_LOCAL_PLAYERS; local_player_index++)
	{
		if (players_globals->local_players[local_player_index]!=NONE)
		{
			return local_player_index;
		}
	}

	return NONE;
}

/* 0x0014de10: the next of the places of this console that has a player,
after one; the first after NONE. NONE if there are no more */
long players_next_local_player(
	long local_player_index)
{
	local_player_index= local_player_index==NONE ? 0 : local_player_index+1;
	for (; local_player_index<MAXIMUM_LOCAL_PLAYERS; local_player_index++)
	{
		if (players_globals->local_players[local_player_index]!=NONE)
		{
			return local_player_index;
		}
	}

	return NONE;
}

/* 0x004ed39c, of game.c: with it set, the players are left as they are when
a map goes. 0x001c7fe0: something else that stops with the players */
extern boolean unknown_004ed39c;
void game_unknown_001c7fe0(void);

/* 0x0014b560: the players have no table and nothing kept. It is the second
of the functions of the part of the game that the players are (game.c) */
void players_dispose(
	void)
{
	if (player_data)
	{
		player_data= NULL;
	}
	if (players_globals)
	{
		players_globals= NULL;
	}
}

/* 0x0014b600: when a map goes, unless unknown_004ed39c is set: the table of
the players cannot be used, game_unknown_001c7fe0 is done, and what is kept of
them has no players: nothing for its first number and its two counts, and
NONE for its two lists of four */
void players_dispose_from_old_map(
	void)
{
	byte *globals;
	long index;

	if (unknown_004ed39c)
	{
		return;
	}
	player_data->valid= FALSE;
	game_unknown_001c7fe0();
	globals= (byte *)players_globals;
	*(long *)globals= 0;
	*(short *)(globals+8)= 0;
	*(short *)(globals+0xA)= 0;
	for (index= 0; index<4; index++)
	{
		*(long *)(globals+0xC+index*4)= NONE;
	}
	for (index= 0; index<4; index++)
	{
		*(long *)(globals+0x1C+index*4)= NONE;
	}
}

/* 0x0014b4b0: the players make their table, in the memory of the state of
the game (data_new, which the game has written here): sixteen of 0x21C
bytes. What is kept of them takes 0x130 bytes there (game_state_malloc,
written here too), with no players: nothing for its two counts and NONE for
its two lists of four */
void players_initialize(
	void)
{
	byte *globals;
	long index;

	player_data= data_new("players", 0x10, 0x21C, 0, game_state_allocation);
	globals= game_state_malloc(0x130);
	*(short *)(globals+8)= 0;
	*(short *)(globals+0xA)= 0;
	players_globals= (struct players_globals *)globals;
	for (index= 0; index<4; index++)
	{
		*(long *)(globals+0xC+index*4)= NONE;
	}
	for (index= 0; index<4; index++)
	{
		*(long *)(globals+0x1C+index*4)= NONE;
	}
}
