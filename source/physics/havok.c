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

/* ---------- prototypes */

int VirtualFree(void *address, unsigned long size, unsigned long type);
uint32_t GetLastError(void);

void unknown_001edbc0(void);
void unknown_00146b30(void);
void unknown_00146b80(void);
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
	unknown_001edbc0();
	impacts_initialize();
	havok_components_initialize();
	unknown_00146b30();
	unknown_00146b80();
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
