/*
HAVOK.C

the part of the game that the physics are (Havok, the library): it starts and
stops the others of this folder. Worked out from halo2ship.exe; the name of
the file is a guess, by what its functions start.
*/

#include "cseries/cseries.h"

#include <stdint.h>
#include <string.h>

#include "memory/data.h"
#include "saved games/game_state.h"

/* ---------- constants */

enum
{
	/* of what unknown_0051e9cc points to: numbers that are NONE in a new map */
	HAVOK_UNKNOWN_0051E9CC_COUNT= 2000
};

/* ---------- globals */

/* 0x0051e9a0: four bytes of the state of the game: a number that is zero
when the part starts. 0x00479888: memory of the console that the part gives
back when it stops; the number before it says how far back it really starts.
0x0051e9cc: the 2000 numbers */
long *unknown_0051e9a0;
void *unknown_00479888;
long *unknown_0051e9cc;

/* 0x0051e9b8, of havok_component.c */
extern struct data_array *havok_component_data;

/* 0x00510c44, of game_unknown_00131ff0.c: 0x452E8 bytes of the console's
memory, which that part shares between its two things */
extern byte *unknown_00510c44;

/* 0x0047989c: something that gives memory, made at 0x105410 of the memory
of unknown_00479888: 0x24 bytes that start with where its functions are
(0x004576a4, a table of functions of the game). 0x00510c48: set when it is
made */
void *unknown_0047989c;
boolean unknown_00510c48;
void *unknown_004576a4[1];

/* ---------- prototypes */

void *VirtualAlloc(void *address, unsigned long size, unsigned long type, unsigned long protection);
int VirtualFree(void *address, unsigned long size, unsigned long type);
uint32_t GetLastError(void);

void havok_unknown_001edbc0(void);
void havok_unknown_00146b30(void);
void havok_unknown_00146b80(void);
void unknown_00146de0(void);
void impacts_initialize(void);
void impacts_dispose(void);
void havok_components_initialize(void);

/* ---------- code */

/* 0x001c25a0: the first of the functions of the part (game.c): the four
bytes take their memory in the state of the game (game_state_malloc, which
the game has written here), and the others start */
void havok_initialize(
	void)
{
	unknown_0051e9a0= game_state_malloc(sizeof(long));
	*unknown_0051e9a0= 0;
	havok_unknown_001edbc0();
	impacts_initialize();
	havok_components_initialize();
	havok_unknown_00146b30();
	havok_unknown_00146b80();
}

/* 0x001c2600: the second: unknown_00146de0, the memory of the console is
given back (VirtualFree; if that fails, the error is asked for), and so are
the table of the components (data_dispose, which the game has written here)
and those of the impacts */
void havok_dispose(
	void)
{
	unknown_00146de0();
	if (!VirtualFree((byte *)unknown_00479888-((long *)unknown_00479888)[-1], 0, 0x8000))
	{
		GetLastError();
	}
	unknown_00479888= NULL;
	data_dispose(havok_component_data);
	havok_component_data= NULL;
	impacts_dispose();
}

/* 0x001c2670: the third: for a new map the 2000 numbers are NONE */
void havok_initialize_for_new_map(
	void)
{
	long index;

	for (index= 0; index<HAVOK_UNKNOWN_0051E9CC_COUNT; index++)
	{
		unknown_0051e9cc[index]= NONE;
	}
}

/* 0x001edbc0: the 2000 numbers take their memory in the state of the game
(game_state_malloc, which the game has written here), if they have none. In
the game it is far from the others of this file, so it may be of another */
void havok_unknown_001edbc0(
	void)
{
	if (!unknown_0051e9cc)
	{
		unknown_0051e9cc= game_state_malloc(HAVOK_UNKNOWN_0051E9CC_COUNT*(long)sizeof(long));
	}
}

/* 0x00146b30: the memory of the console that havok_dispose gives back is
taken: 0x105470 bytes that start at an address that is a multiple of 0x1000,
with their size and how far back the memory really starts kept just before
them; none if there is no memory (and the error is asked for). Far from the
others too */
void havok_unknown_00146b30(
	void)
{
	byte *memory= VirtualAlloc(NULL, 0x105470+0x1000, 0x101000, 4);

	if (!memory)
	{
		GetLastError();
		unknown_00479888= NULL;
	}
	else
	{
		byte *aligned= (byte *)(((uintptr_t)memory+0x1000)&~(uintptr_t)0xFFF);

		((long *)aligned)[-2]= 0x105470;
		((long *)aligned)[-1]= (long)(aligned-memory);
		unknown_00479888= aligned;
	}
}

/* 0x00146b80: the thing that gives memory is made, in its place of the
memory of unknown_00479888, to give the 0x452E8 bytes of unknown_00510c44:
from the first address that is a multiple of 16, and a size that is a
multiple of 4. Far from the others too */
void havok_unknown_00146b80(
	void)
{
	byte *memory= unknown_00510c44;
	unsigned long size= 0x452E8;
	byte *thing;

	unknown_00510c48= TRUE;
	if ((uintptr_t)memory&0xF)
	{
		unsigned long padding= (unsigned long)(0-(uintptr_t)memory)&0xF;

		memory+= padding;
		size-= padding;
	}
	thing= (byte *)((uintptr_t)unknown_00479888+0x105410);
	if (thing)
	{
		*(long *)(thing+4)= 1;
		*(long *)(thing+8)= 0;
		*(long *)(thing+0xC)= 0;
		*(long *)(thing+0x10)= 0;
		*(long *)(thing+0x14)= NONE;
		*(long *)(thing+0x20)= 0;
		*(byte **)(thing+0x18)= memory;
		*(unsigned long *)(thing+0x1C)= size&0x0FFFFFFC;
		*(void **)thing= unknown_004576a4;
		unknown_0047989c= thing;
	}
	else
	{
		unknown_0047989c= NULL;
	}
}
