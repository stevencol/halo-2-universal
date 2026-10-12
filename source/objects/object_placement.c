/*
OBJECT_PLACEMENT.C

the part of the game that puts the objects of the scenario in the structure
bsp that is loaded. Worked out from halo2ship.exe; the name of the file is a
guess, by what the game says that it is doing ("placing objects on bsp").
*/

#include "cseries/cseries.h"
#include "saved games/game_state.h"

/* ---------- globals */

/* 0x004e0324: four bytes of the state of the game, that start with a short
that is NONE in a new map (the structure bsp whose objects are placed,
perhaps; the name is a guess) */
byte *object_placement_globals;

/* 0x00510c50, of main.c: what it keeps */
struct unknown_00510c50;
extern struct unknown_00510c50 *unknown_00510c50;

/* ---------- prototypes */

/* 0x001e95d0 and 0x001e9650: what is being done starts, with its name, and
ends (they tell whoever shows it, it seems). 0x000d5640 places the objects */
void __attribute__((stdcall)) progress_new(const char *name);
void progress_done(void);
void unknown_000d5640(void);

/* ---------- code */

/* 0x000d4de0: the first of the functions of the part (game.c): the four
bytes take their memory in the state of the game (game_state_malloc, which
the game has written here) */
void object_placement_initialize(
	void)
{
	object_placement_globals= game_state_malloc(4);
}

/* 0x000d4e20: the third: for a new map the short is NONE */
void object_placement_initialize_for_new_map(
	void)
{
	*(short *)object_placement_globals= NONE;
}

/* 0x000d4e30: the fifth: for a new structure bsp the objects are placed,
unless what unknown_00510c50 points to has its bytes 5 and 7 set */
void object_placement_initialize_for_new_structure_bsp(
	void)
{
	const byte *unknown;

	progress_new("placing objects on bsp");
	unknown= (const byte *)unknown_00510c50;
	if (!unknown || !unknown[5] || !unknown[7])
	{
		unknown_000d5640();
	}
	progress_done();
}
