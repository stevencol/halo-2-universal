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
#include "cache/cache_files.h"
#include "main/main.h"
#include "networking/game_voice.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"

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

/* 0x004b9ed8: which of the players of this console something is of now, or
NONE (it is of another part of the game, not known) */
long unknown_004b9ed8;

/* 0x00157bb0: the fourth of the functions of the part (game.c): when a map
goes, what runs the game is told (its third function), and no kind of game
is played */
void game_engine_dispose_from_old_map(
	void)
{
	void *engine= GAME_ENGINE_GET();

	if (engine)
	{
		((void (__attribute__((thiscall)) *)(void *))(*(void ***)engine)[2])(engine);
		*(long *)(unknown_004e9ae8+0xC14)= NONE;
	}
}

/* 0x00157a40: the short that the game engine keeps at 0xA, if something runs
the game and the byte at 0x184 of the game has its bit 0; if not, zero */
short game_engine_unknown_00157a40(
	void)
{
	const byte *globals= unknown_004e9ae8;

	if (GAME_ENGINE_GET() && (((const byte *)main_game_globals)[0x184]&1))
	{
		return *(const short *)(globals+0xA);
	}

	return 0;
}

/* 0x001587b0: the short at 4 of one of the things of 0x1C bytes that the
game engine keeps from 0x304, if something runs the game; if not, zero */
long game_engine_unknown_001587b0(
	long index)
{
	const byte *globals= unknown_004e9ae8;

	if (GAME_ENGINE_GET() && (uintptr_t)globals+0x304!=0)
	{
		return *(const short *)(globals+0x304+(index&0xFFFF)*0x1C+4);
	}

	return 0;
}

/* 0x001595d0: what runs the game is told (its 14th function) of the player
of this console that unknown_004b9ed8 says, if there is one */
void game_engine_unknown_001595d0(
	void)
{
	if (unknown_004b9ed8!=NONE)
	{
		long player_index= *(const long *)((const byte *)players_globals+unknown_004b9ed8*4+0xC);

		if (player_index!=NONE)
		{
			void *engine= GAME_ENGINE_GET();

			if (engine)
			{
				((void (__attribute__((thiscall)) *)(void *, long))(*(void ***)engine)[13])(engine, player_index);
			}
		}
	}
}

/* 0x00162030: what the game engine keeps from 0x304, if something runs the
game; if not, NULL */
byte *game_engine_unknown_00162030(
	void)
{
	byte *globals= unknown_004e9ae8;

	if (GAME_ENGINE_GET())
	{
		return globals+0x304;
	}

	return NULL;
}

/* 0x00163040: whether a bit of the byte that the game engine keeps at 0xF4
is set, if something runs the game */
boolean __attribute__((stdcall)) game_engine_unknown_00163040(
	long bit)
{
	const byte *globals= unknown_004e9ae8;

	if (GAME_ENGINE_GET())
	{
		return (globals[0xF4]&(byte)(1<<bit))!=0;
	}

	return FALSE;
}

/* 0x00163080: the bit 0 of the byte that the game engine keeps at 0xC10, if
something runs the game */
boolean game_engine_unknown_00163080(
	void)
{
	const byte *globals= unknown_004e9ae8;

	if (GAME_ENGINE_GET())
	{
		return globals[0xC10]&1;
	}

	return FALSE;
}

/* ---------- the two entities of the simulation of the game engine

What the game engine keeps has two entities of the simulation, at 0x24 and
0x28 (NONE when there is none). The functions that follow are among those of
the entities of the simulation in the game (0x9ba90 to 0xa7840), and are
given to it as functions of kinds of entity, it seems: they take their
arguments off the stack, and most of them are not used. They may be of
another file */

void __attribute__((stdcall)) unknown_001e98e0(byte *unknown, long unknown2, long unknown3);
void __attribute__((stdcall)) unknown_000b58c0(long entity_index, long unknown);

/* 0x0009ba90: the entity at 0x28 is a given one. (If something runs the game
and there was one, it is NONE first, which changes nothing) */
boolean __attribute__((stdcall)) game_engine_entity28_set(
	const long *entity_index,
	long unknown2,
	long unknown3,
	long unknown4,
	long unknown5,
	long unknown6)
{
	byte *globals= unknown_004e9ae8;

	(void)unknown2;
	(void)unknown3;
	(void)unknown4;
	(void)unknown5;
	(void)unknown6;
	if (GAME_ENGINE_GET() && *(long *)(globals+0x28)!=NONE)
	{
		*(long *)(globals+0x28)= NONE;
	}
	*(long *)(globals+0x28)= *entity_index;

	return TRUE;
}

/* 0x0009bad0: the entity at 0x28 is no more, if it is a given one (or if
nothing runs the game and NONE is given); whether it was */
boolean __attribute__((stdcall)) game_engine_entity28_clear(
	const long *entity_index)
{
	byte *globals= unknown_004e9ae8;
	long current= NONE;

	if (GAME_ENGINE_GET())
	{
		current= *(long *)(globals+0x28);
	}
	if (*entity_index==current)
	{
		*(long *)(globals+0x28)= NONE;

		return TRUE;
	}

	return FALSE;
}

/* 0x0009bb10: the entity at 0x28 takes a new index, if the lowest ten bits
of the two are the same (the same place, another time, it seems) */
void __attribute__((stdcall)) game_engine_entity28_update(
	const long *entity_index)
{
	byte *globals= unknown_004e9ae8;

	if (GAME_ENGINE_GET() && *(long *)(globals+0x28)!=NONE && !((*entity_index^*(long *)(globals+0x28))&0x3FF))
	{
		*(long *)(globals+0x28)= *entity_index;
	}
}

/* 0x0009bb70: unknown_001e98e0 is done with what the game engine keeps from
0x304 (NULL if nothing runs the game) and two of the four arguments */
boolean __attribute__((stdcall)) game_engine_unknown_0009bb70(
	long unknown,
	long unknown2,
	long unknown3,
	long unknown4)
{
	(void)unknown;
	(void)unknown3;
	unknown_001e98e0(game_engine_unknown_00162030(), unknown2, unknown4);

	return TRUE;
}

/* 0x000a4910: as game_engine_entity28_clear, for the entity at 0x24 */
boolean __attribute__((stdcall)) game_engine_entity24_clear(
	const long *entity_index)
{
	byte *globals= unknown_004e9ae8;
	long current= NONE;

	if (GAME_ENGINE_GET())
	{
		current= *(long *)(globals+0x24);
	}
	if (*entity_index==current)
	{
		*(long *)(globals+0x24)= NONE;

		return TRUE;
	}

	return FALSE;
}

/* 0x000a7810: unknown_000b58c0 is done with the entity at 0x24, if there is
one and something runs the game */
void __attribute__((stdcall)) game_engine_unknown_000a7810(
	long unknown)
{
	const byte *globals= unknown_004e9ae8;

	if (GAME_ENGINE_GET() && *(const long *)(globals+0x24)!=NONE)
	{
		unknown_000b58c0(*(const long *)(globals+0x24), unknown);
	}
}

/* 0x0009bb50: 0x1B0 bytes of the last of four arguments are left with
nothing; always true. It is between game_engine_entity28_update and
game_engine_unknown_0009bb70 in the game */
boolean __attribute__((stdcall)) game_engine_unknown_0009bb50(
	long unknown,
	long unknown2,
	long unknown3,
	byte *result)
{
	(void)unknown;
	(void)unknown2;
	(void)unknown3;
	memset(result, 0, 0x1B0);

	return TRUE;
}

/* 0x000a4950: the least of the numbers of the flags that something has (0x4A,
7, 6, 10 and 21 for the first five); 0 if it has none of them */
long game_engine_unknown_000a4950(
	const unsigned long *flags)
{
	long least= 0x7FFFFFFF;

	if (*flags&1)
	{
		least= 0x4A;
	}
	if ((*flags&2) && least>7)
	{
		least= 7;
	}
	if ((*flags&4) && least>6)
	{
		least= 6;
	}
	if ((*flags&8) && least>10)
	{
		least= 10;
	}
	if ((*flags&0x10) && least>21)
	{
		least= 21;
	}

	return least==0x7FFFFFFF ? 0 : least;
}

/* 0x0009a150: the least of the numbers of the flags (game_engine_unknown_000a4950),
given back where the third says. It is called through a table */
void __attribute__((stdcall)) game_engine_unknown_0009a150(
	long unused,
	const unsigned long *flags,
	long *result)
{
	*result= game_engine_unknown_000a4950(flags);
}

/* 0x0009a2c0: as 0x0009a150, plus two; no more than 11 with the flag 0x20
and no more than 23 with 0x40 */
void __attribute__((stdcall)) game_engine_unknown_0009a2c0(
	long unused,
	const unsigned long *flags,
	long *result)
{
	unsigned long bits= *flags;
	long value= game_engine_unknown_000a4950(flags)+2;

	if ((bits&0x20) && value>=11)
	{
		value= 11;
	}
	if ((bits&0x40) && value>=23)
	{
		value= 23;
	}
	*result= value;
}

/* 0x0009a600: as 0x0009a150, plus 0x11; no more than 0x3E with the flag
0x20 and no more than 0x20 with any of the sixteen flags from 0x40 */
void __attribute__((stdcall)) game_engine_unknown_0009a600(
	long unused,
	const unsigned long *flags,
	long *result)
{
	unsigned long bits= *flags;
	long value= game_engine_unknown_000a4950(flags)+0x11;
	long index;

	if ((bits&0x20) && value>=0x3E)
	{
		value= 0x3E;
	}
	for (index= 0; index<16; index++)
	{
		if ((bits&(1UL<<(index+6))) && value>=0x20)
		{
			value= 0x20;
		}
	}
	*result= value;
}

/* 0x0009aa00: as 0x0009a150, plus one; no more than 0x16 with the flag 0x20 */
void __attribute__((stdcall)) game_engine_unknown_0009aa00(
	long unused,
	const unsigned long *flags,
	long *result)
{
	long value= game_engine_unknown_000a4950(flags)+1;

	if ((*(const byte *)flags&0x20) && value>=0x16)
	{
		value= 0x16;
	}
	*result= value;
}

/* 0x00468734: points to the three longs given when the game globals have
none */
const unsigned long *unknown_00468734;

/* 0x0007f720: three longs (a colour, by its size) of the element of the
block at 0x10 of the first element of the tag that the game globals name at
0x16C; those of unknown_00468734 if there is no tag or no such element */
void __attribute__((stdcall)) game_engine_unknown_0007f720(
	unsigned long *result,
	short index)
{
	const unsigned long *value= unknown_00468734;
	long tag_index= global_game_globals->unknown16C;

	if (tag_index!=NONE)
	{
		const byte *tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
		const byte *element= *(const byte *const *)(tag+0x04);

		if (index>=0 && index<*(const long *)(element+0x10))
		{
			value= *(const unsigned long *const *)(element+0x14)+3*index;
		}
	}
	result[0]= value[0];
	result[1]= value[1];
	result[2]= value[2];
}
