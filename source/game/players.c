/*
PLAYERS.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The players of a game. The names are guesses.
*/

#include "players.h"

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
