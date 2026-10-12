/*
GAME_UNKNOWN_00131FF0.C

a part of the game: two things of 0x2AE0 bytes that share 0x452E8 bytes of
the console's memory, half each.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"

#include <stdint.h>

/* ---------- constants */

enum
{
	GAME_UNKNOWN_00131FF0_THING_SIZE= 0x2AE0,
	GAME_UNKNOWN_00131FF0_MEMORY_SIZE= 0x22974
};

/* ---------- structures */

/* what a thing is made with: six shorts, of which nothing is known */
struct game_unknown_00131ff0_parameters
{
	short unknown[6];
};

/* ---------- globals */

/* 0x00510c44: the memory. 0x00547f88 and 0x0054aa68: the two things */
byte *unknown_00510c44;
byte unknown_00547f88[GAME_UNKNOWN_00131FF0_THING_SIZE];
byte unknown_0054aa68[GAME_UNKNOWN_00131FF0_THING_SIZE];

/* ---------- prototypes */

void *VirtualAlloc(void *address, unsigned long size, unsigned long type, unsigned long protection);
int VirtualFree(void *address, unsigned long size, unsigned long type);
uint32_t GetLastError(void);

/* 0x00134980: a thing is made, with its six shorts and its memory.
0x00134b20: it is closed */
void unknown_00134980(const struct game_unknown_00131ff0_parameters *parameters, byte *memory, byte *thing);
void unknown_00134b20(byte *thing);

/* ---------- code */

/* 0x00131ff0: the first of the functions of the part (game.c): the memory
is taken from the console (VirtualAlloc; if there is none the error is asked
for, and it goes on all the same), and the two things are made */
void game_unknown_00131ff0_initialize(
	void)
{
	struct game_unknown_00131ff0_parameters first= {{1, 0x80, 0x40, 0x100, 0x180, 0}};
	struct game_unknown_00131ff0_parameters second= {{6, 0x40, 1, 0x80, 0x60, 0x100}};
	byte *memory= VirtualAlloc(NULL, 2*GAME_UNKNOWN_00131FF0_MEMORY_SIZE, 0x101000, 4);

	if (!memory)
	{
		GetLastError();
	}
	unknown_00510c44= memory;
	unknown_00134980(&first, memory, unknown_00547f88);
	unknown_00134980(&second, unknown_00510c44+GAME_UNKNOWN_00131FF0_MEMORY_SIZE, unknown_0054aa68);
}

/* 0x001320b0: the second: the two things are closed, and the memory is given
back, if there is any (VirtualFree; if that fails, the error is asked for) */
void game_unknown_00131ff0_dispose(
	void)
{
	unknown_00134b20(unknown_00547f88);
	unknown_00134b20(unknown_0054aa68);
	if (unknown_00510c44)
	{
		if (!VirtualFree(unknown_00510c44, 0, 0x8000))
		{
			GetLastError();
		}
		unknown_00510c44= NULL;
	}
}
