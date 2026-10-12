/*
GAME_UNKNOWN_00210DB0.C

a part of the game: sixteen numbers of the state of the game, that are NONE when a
structure bsp comes.
Worked out from halo2ship.exe; what it is for is not known, and the name of
the file is only the address of its first function.
*/

#include "cseries/cseries.h"

#include <string.h>

#include "saved games/game_state.h"

/* ---------- constants */

enum
{
	GAME_UNKNOWN_00210DB0_COUNT= 16
};

/* ---------- globals */

/* 0x004f93a4: the sixteen numbers */
long *unknown_004f93a4;

/* ---------- prototypes */

void unknown_00210f00(void);
void unknown_00210e20(void);
void unknown_00210e80(void);

/* ---------- code */

/* 0x00210db0: the numbers take their memory in the state of the game
(game_state_malloc, which the game has written here) */
void game_unknown_00210db0_initialize(
	void)
{
	unknown_004f93a4= game_state_malloc(GAME_UNKNOWN_00210DB0_COUNT*sizeof(long));
}

/* 0x00210df0: the fifth of the functions of the part: for a new structure
bsp the numbers are NONE, and unknown_00210f00 and unknown_00210e20 are done */
void game_unknown_00210db0_initialize_for_new_structure_bsp(
	void)
{
	long index;

	for (index= 0; index<GAME_UNKNOWN_00210DB0_COUNT; index++)
	{
		unknown_004f93a4[index]= NONE;
	}
	unknown_00210f00();
	unknown_00210e20();
}

/* 0x00210e10: the sixth: when the structure bsp goes, unknown_00210e80 is
done */
void game_unknown_00210db0_dispose_from_old_structure_bsp(
	void)
{
	unknown_00210e80();
}
