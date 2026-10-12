/*
GAME_UNKNOWN_0020AEE0.C

a part of the game: it takes 0x1880 bytes of the state of the game and as many of
the console, and uses those of the console.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"
#include "saved games/game_state.h"

#include <string.h>

/* ---------- constants */

enum
{
	GAME_UNKNOWN_0020AEE0_SIZE= 0x1880,
	/* how the memory of the console is asked for and given back, and how it
	is to be: read and written */
	GAME_UNKNOWN_0020AEE0_ALLOCATION_TYPE= 0x101000,
	GAME_UNKNOWN_0020AEE0_FREE_TYPE= 0x8000,
	GAME_UNKNOWN_0020AEE0_PROTECTION= 4
};

/* ---------- prototypes */

void *VirtualAlloc(void *address, unsigned long size, unsigned long type, unsigned long protection);
int VirtualFree(void *address, unsigned long size, unsigned long type);
uint32_t GetLastError(void);

/* ---------- globals */

/* 0x0051e9ec: what is used. 0x0051e9f0: the memory of the state of the game.
0x0051e9f4: that of the console */
byte *unknown_0051e9ec;
byte *unknown_0051e9f0;
byte *unknown_0051e9f4;

/* ---------- code */

/* 0x0020aee0: the two memories are taken: that of the state of the game
(game_state_malloc, written here) and that of the console (VirtualAlloc; if
there is none, the error is asked for and nothing is done with it). The one
of the console is the one that is used */
void game_unknown_0020aee0_initialize(
	void)
{
	byte *memory;

	unknown_0051e9f0= game_state_malloc(GAME_UNKNOWN_0020AEE0_SIZE);
	memory= VirtualAlloc(NULL, GAME_UNKNOWN_0020AEE0_SIZE, GAME_UNKNOWN_0020AEE0_ALLOCATION_TYPE,
		GAME_UNKNOWN_0020AEE0_PROTECTION);
	if (!memory)
	{
		GetLastError();
	}
	unknown_0051e9f4= memory;
	unknown_0051e9ec= memory;
}

/* 0x0020af50: the memory of the console is given back (VirtualFree; if that
fails, the error is asked for), and there is none */
void game_unknown_0020aee0_dispose(
	void)
{
	if (!VirtualFree(unknown_0051e9f4, 0, GAME_UNKNOWN_0020AEE0_FREE_TYPE))
	{
		GetLastError();
	}
	unknown_0051e9f4= NULL;
	unknown_0051e9f0= NULL;
	unknown_0051e9ec= NULL;
}

/* 0x0020af90: for a new map, what is used is nothing */
void game_unknown_0020aee0_initialize_for_new_map(
	void)
{
	memset(unknown_0051e9ec, 0, GAME_UNKNOWN_0020AEE0_SIZE);
}
