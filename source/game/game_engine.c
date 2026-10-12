/*
GAME_ENGINE.C

the engine of the game: what each kind of game of several keeps. Worked out
from halo2ship.exe; the name of the file is a guess: what it keeps has at
0xC14 which of the kinds of game is being played (unknown_0055e4d0, of
game_voice.c, has something for each).
*/

#include "cseries/cseries.h"
#include "networking/game_voice.h"
#include "saved games/game_state.h"

#include <string.h>

/* ---------- constants */

enum
{
	/* what unknown_004e9ae8 points to */
	GAME_ENGINE_GLOBALS_SIZE= 0xC9C
};

/* ---------- globals */

/* 0x00502258: 0x9C bytes that are nothing when the engine starts */
byte unknown_00502258[0x9C];

/* ---------- code */

/* 0x00157450: what the engine keeps takes its memory of the state of the
game (game_state_malloc, which the game has written here) and is nothing,
as unknown_00502258 is; and no kind of game is being played */
void game_engine_initialize(
	void)
{
	byte *globals= game_state_malloc(GAME_ENGINE_GLOBALS_SIZE);

	memset(globals, 0, GAME_ENGINE_GLOBALS_SIZE);
	memset(unknown_00502258, 0, sizeof(unknown_00502258));
	unknown_004e9ae8= globals;
	*(long *)(globals+UNKNOWN_004E9AE8_UNKNOWNC14_OFFSET)= NONE;
}

/* 0x0055e4d0, of game_voice.c: for each kind of game, what runs it (a thing
with its functions), or nothing */
extern long unknown_0055e4d0[];

/* 0x0015e020: whether two teams are enemies, in multiplayer: what runs the
kind of game that is played says (its 28th function); they are not if nothing
runs it */
boolean __attribute__((stdcall)) game_engine_teams_are_enemies(
	short team,
	short other)
{
	void *engine= (void *)unknown_0055e4d0[*(const long *)(unknown_004e9ae8+0xC14)];

	if (engine)
	{
		return ((boolean (__attribute__((thiscall)) *)(void *, short, short))(*(void ***)engine)[27])(engine, team,
			other);
	}

	return FALSE;
}
