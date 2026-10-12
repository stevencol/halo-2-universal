/*
GAME_ENGINE.C

the engine of the game: what each kind of game of several keeps. Worked out
from halo2ship.exe; the name of the file is a guess: what it keeps has at
0xC14 which of the kinds of game is being played (unknown_0055e4d0, of
game_voice.c, has something for each).
*/

#include "cseries/cseries.h"

#include <stdint.h>

#include "game/game.h"
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

/* what runs the kind of game that is played, or NULL */
#define GAME_ENGINE_GET() ((void *)unknown_0055e4d0[*(const long *)(unknown_004e9ae8+0xC14)])

void __attribute__((stdcall)) unknown_001e97b0(byte *unknown);

/* 0x0015dea0: whether something runs the kind of game that is played */
boolean game_engine_running(
	void)
{
	return GAME_ENGINE_GET()!=NULL;
}

/* 0x0015dd10: what the game engine keeps takes the bit 5 of its first
number; and if something runs the game, unknown_001e97b0 is done with what it
keeps at 0x304 */
void game_engine_unknown_0015dd10(
	void)
{
	byte *globals= unknown_004e9ae8;

	if (GAME_ENGINE_GET())
	{
		byte *unknown= (byte *)((uintptr_t)globals+0x304);

		if (unknown)
		{
			unknown_001e97b0(unknown);
		}
	}
	*(unsigned long *)globals|= 0x20;
}

/* 0x0015ea50: what runs the game is told of something that is not NONE (its
15th function; a player, perhaps) */
void game_engine_unknown_0015ea50(
	long index)
{
	void *engine= GAME_ENGINE_GET();

	if (engine && index!=NONE)
	{
		((void (__attribute__((thiscall)) *)(void *, long))(*(void ***)engine)[14])(engine, index);
	}
}

/* 0x0015eaf0: the bit 0 of the byte that the game has at 0x184 (of the
options of the game), if something runs the game; if not, false */
boolean game_engine_unknown_0015eaf0(
	void)
{
	if (GAME_ENGINE_GET())
	{
		return ((const byte *)main_game_globals)[0x184]&1;
	}

	return FALSE;
}

/* 0x0015fe50: the number that the game engine keeps at 0xC04 is set, unless
this console only predicts what another rules */
void game_engine_unknown_0015fe50(
	long unknown)
{
	if (main_game_globals->options.simulation!=_game_simulation_predicted)
	{
		*(long *)(unknown_004e9ae8+0xC04)= unknown;
	}
}

/* 0x0015ff50: what runs the game is asked something of two things (its 36th
function); false if nothing runs it */
boolean __attribute__((stdcall)) game_engine_unknown_0015ff50(
	long unknown,
	long unknown2)
{
	void *engine= GAME_ENGINE_GET();

	if (engine)
	{
		return ((boolean (__attribute__((thiscall)) *)(void *, long, long))(*(void ***)engine)[35])(engine, unknown,
			unknown2);
	}

	return FALSE;
}
