/*
BREAKABLE_SURFACES.C

the surfaces of the level that break. Worked out from halo2ship.exe; the name
of the file is a guess: what it keeps is a bit for each of 256 surfaces of
each structure, and it looks at the surfaces of the collision that have the
fourth bit.
*/

#include "cseries/cseries.h"
#include "game/game.h"
#include "saved games/game_state.h"
#include "scenario/scenario.h"

#include <string.h>

/* ---------- constants */

enum
{
	/* what unknown_004ed280 points to: 0x4204 bytes, of which a new map sets
	every bit of the first 0x4201 */
	BREAKABLE_SURFACES_SIZE= 0x4204,
	BREAKABLE_SURFACES_USED_SIZE= 0x4201,
	BREAKABLE_SURFACES_UNKNOWN_004ECA60_COUNT= 8,
	BREAKABLE_SURFACES_UNKNOWN_004ECA80_COUNT= 256
};

/* ---------- globals */

/* 0x004ed280, of sound_manager.c, which looks at it: what is kept, in the
state of the game */
extern byte *unknown_004ed280;

/* 0x004ea95c: a number. 0x004eca60: eight numbers that are NONE when there
is nothing. 0x004ea960, 0x004eaa60, 0x004eca80: three tables that are all
NONE when the game stops being active */
long unknown_004ea95c;
long unknown_004eca60[BREAKABLE_SURFACES_UNKNOWN_004ECA60_COUNT];
byte unknown_004ea960[0x100];
byte unknown_004eaa60[0x2000];
long unknown_004eca80[BREAKABLE_SURFACES_UNKNOWN_004ECA80_COUNT][2];

/* ---------- prototypes */

/* 0x000b5920: something that one of the eight numbers is of is done with.
0x00185630: what is done when a game in the network of kind 4 or 5 becomes
active */
void unknown_000b5920(long unknown);
void unknown_00185630(void);

void breakable_surfaces_unknown_00183f10(void);
void breakable_surfaces_unknown_00185a30(void);
void breakable_surfaces_unknown_00185a70(void);

/* ---------- code */

/* 0x00183e40: what is kept takes its memory of the state of the game, and
breakable_surfaces_unknown_00183f10 is done, which the game has written
here, as it has game_state_malloc. It is the first of the six functions of
a part of the game (game.c) */
void breakable_surfaces_initialize(
	void)
{
	unknown_004ed280= game_state_malloc(BREAKABLE_SURFACES_SIZE);
	breakable_surfaces_unknown_00183f10();
}

/* 0x00183ec0: for a new map, every bit of what is kept is set, but for its
last three bytes */
void breakable_surfaces_initialize_for_new_map(
	void)
{
	memset(unknown_004ed280, 0xFF, BREAKABLE_SURFACES_USED_SIZE);
}

/* 0x00183ee0: the fifth of the six, for when the game becomes active: in a
game in the network of kind 4 or 5, breakable_surfaces_unknown_00185a30 and
unknown_00185630 */
void breakable_surfaces_unknown_00183ee0(
	void)
{
	long simulation= main_game_globals->options.simulation;

	if (simulation>=4 && simulation<=5)
	{
		breakable_surfaces_unknown_00185a30();
		unknown_00185630();
	}
}

/* 0x00183f00: the sixth, for when it stops being so:
breakable_surfaces_unknown_00185a30 and breakable_surfaces_unknown_00185a70 */
void breakable_surfaces_unknown_00183f00(
	void)
{
	breakable_surfaces_unknown_00185a30();
	breakable_surfaces_unknown_00185a70();
}

/* 0x00183f10: the number is nothing and the eight are NONE */
void breakable_surfaces_unknown_00183f10(
	void)
{
	long index;

	unknown_004ea95c= 0;
	for (index= 0; index<BREAKABLE_SURFACES_UNKNOWN_004ECA60_COUNT; index++)
	{
		unknown_004eca60[index]= NONE;
	}
}

/* 0x00185a30: each of the eight numbers that is not NONE is done with
(unknown_000b5920), but not in a game in the network of kind 4, and is NONE;
and the number is nothing */
void breakable_surfaces_unknown_00185a30(
	void)
{
	long index;

	for (index= 0; index<BREAKABLE_SURFACES_UNKNOWN_004ECA60_COUNT; index++)
	{
		if (unknown_004eca60[index]!=NONE && main_game_globals->options.simulation!=4)
		{
			unknown_000b5920(unknown_004eca60[index]);
		}
		unknown_004eca60[index]= NONE;
	}
	unknown_004ea95c= 0;
}

/* 0x00185a70: the three tables are all NONE */
void breakable_surfaces_unknown_00185a70(
	void)
{
	long index;

	for (index= 0; index<BREAKABLE_SURFACES_UNKNOWN_004ECA80_COUNT; index++)
	{
		unknown_004eca80[index][1]= NONE;
		unknown_004eca80[index][0]= NONE;
	}
	memset(unknown_004ea960, 0xFF, sizeof(unknown_004ea960));
	memset(unknown_004eaa60, 0xFF, sizeof(unknown_004eaa60));
}

/* four small functions that are just before the ones of this file in the
game; in the game their first argument is in ecx (a class, most likely).
Whether they are of this file is not known */

/* 0x00183d90: what a thing has at 0xC: the one that it is of */
byte *unknown_node_get_parent(
	const byte *node)
{
	return *(byte *const *)(node+0xC);
}

/* 0x00183da0: the one that has none at 0xC, going up from a thing */
byte *unknown_node_get_root(
	byte *node)
{
	byte *parent;

	while ((parent= *(byte *const *)(node+0xC))!=NULL)
	{
		node= parent;
	}

	return node;
}

/* 0x00183e20: whether a byte is set */
long unknown_flag_get(
	const byte *flag)
{
	return *flag!=0;
}

/* 0x00183e30: the byte takes a value */
void __attribute__((stdcall)) unknown_flag_set(
	byte *flag,
	byte value)
{
	*flag= value;
}

/* 0x00183f50: whether a surface (8 bytes, of the list at 0x2C, as many as at
0x28) of a collision has the bit 0x08 at 0x04 and a byte at 0x05; the
collision is that of the level for NONE, or that (0xC8 bytes, after 0x70,
of the list at 0x13C) of an element (0x58 bytes, at 0x140) of the structure
bsp, whose short at 0x34 names it */
boolean __attribute__((stdcall)) breakable_surfaces_unknown_00183f50(
	long element,
	long surface,
	unsigned long value)
{
	const byte *collision;

	if (element==NONE)
	{
		collision= (const byte *)global_collision_bsp;
	}
	else
	{
		const byte *bsp= (const byte *)global_structure_bsp;

		if (element<0 || element>=*(const long *)(bsp+0x140))
		{
			return FALSE;
		}
		collision= *(const byte *const *)(bsp+0x13C)
			+*(const short *)(*(const byte *const *)(bsp+0x144)+element*0x58+0x34)*0xC8+0x70;
	}
	if (!collision || surface<0 || surface>=*(const long *)(collision+0x28))
	{
		return FALSE;
	}

	return ((*(const byte *const *)(collision+0x2C))[surface*8+4]&8)
		&& (*(const byte *const *)(collision+0x2C))[surface*8+5]==value;
}

/* the bits of the surfaces of the level (32 bytes for each structure bsp,
from 0x01) or of an element (0x400 bytes for each structure bsp, from 0x201) */
#define BREAKABLE_SURFACES_BITS(element) \
	((element)==NONE ? unknown_004ed280+global_structure_bsp_index*0x20+1 \
		: unknown_004ed280+global_structure_bsp_index*0x400+0x201+(element))

/* 0x00183fc0: those bits */
byte *breakable_surfaces_get_bits(
	long element)
{
	return BREAKABLE_SURFACES_BITS(element);
}

/* 0x00184000: whether a surface has its bit; TRUE for NONE */
long breakable_surfaces_test(
	long surface,
	long element)
{
	if (surface==NONE)
	{
		return TRUE;
	}

	return (((const unsigned long *)BREAKABLE_SURFACES_BITS(element))[surface>>5]&(1UL<<(surface&0x1F)))!=0;
}

/* 0x00184400: the number of unknown_004eca60 that the byte of a surface (of
the level, or of an element) names by its high three bits; that of 0xFF for
NONE */
long breakable_surfaces_unknown_00184400(
	long surface,
	long element)
{
	byte index= 0xFF;

	if (surface!=NONE)
	{
		index= element==NONE ? unknown_004ea960[surface] : unknown_004eaa60[element*8+surface];
	}

	return unknown_004eca60[index>>5];
}
