/*
AI.C

the part of the game that the minds of the characters are: it starts and stops
the others of this folder. Worked out from halo2ship.exe; the name of the file
is a guess, by what its functions start.
*/

#include "cseries/cseries.h"

#include <string.h>

#include "memory/data.h"
#include "memory/hash_table.h"
#include "saved games/game_state.h"

/* ---------- constants */

enum
{
	/* of what ai_globals points to */
	AI_GLOBALS_SIZE= 0x374,

	/* of what unknown_004f55cc points to: two things of 0x1C bytes */
	AI_UNKNOWN_COUNT= 2,
	AI_UNKNOWN_SIZE= 0x1C
};

/* ---------- globals */

/* 0x004f55d0: what the part keeps, in the state of the game (0x374 bytes;
the name is a guess). 0x004f55cc: the two things. 0x005044cc, 0x005044d0,
0x005047f4: 0x20, 0x10 and 0x24 bytes of the state of the game, of which
nothing else is known */
byte *ai_globals;
byte *unknown_004f55cc;
byte *unknown_005044cc;
byte *unknown_005044d0;
byte *unknown_005047f4;

/* the tables that the game makes here, and that are of other files of its
own that are not known: 0x00502420 ("clump": 0x14 of 0x50 bytes), 0x00502424
("joint state": 0x14 of 0xBC bytes), 0x0051eca4 ("dynamic firing points": 0xF
of 0x484 bytes) and 0x0051ecb4 ("flocks": 0xA of 0x28 bytes). 0x004f5768: a
short that is NONE when the tables are made */
struct data_array *clump_data;
struct data_array *joint_state_data;
struct data_array *dynamic_firing_point_data;
struct data_array *flock_data;
short unknown_004f5768;

/* 0x00557c6c, of actors.c */
extern struct hash_table *actor_firing_position_owners;

/* ---------- prototypes */

void actors_initialize(void);
void swarms_initialize(void);
void props_initialize(void);
void squads_initialize(void);
void command_scripts_initialize(void);
void vocalizations_initialize(void);
void unknown_00292130(void);
void unknown_001a6d80(void);

/* ---------- code */

/* 0x001c7fe0: each of the two things has NONE for its first two numbers and
nothing for the short at 0xA */
void ai_unknown_001c7fe0(
	void)
{
	byte *unknown= unknown_004f55cc;
	long index;

	for (index= 0; index<AI_UNKNOWN_COUNT; index++)
	{
		*(long *)unknown= NONE;
		*(long *)(unknown+4)= NONE;
		*(short *)(unknown+0xA)= 0;
		unknown+= AI_UNKNOWN_SIZE;
	}
}

/* 0x001c7fa0: what the part keeps is all zeros, and the two things are as
ai_unknown_001c7fe0 leaves them (the game has written it here again) */
void ai_unknown_001c7fa0(
	void)
{
	memset(ai_globals, 0, AI_GLOBALS_SIZE);
	ai_unknown_001c7fe0();
}

/* 0x001c7790: the first of the functions of the part (game.c). All that the
minds of the characters keep takes its memory in the state of the game
(game_state_malloc and data_new, which the game has written here each time),
and the others of the folder make their tables */
void ai_initialize(
	void)
{
	ai_globals= game_state_malloc(AI_GLOBALS_SIZE);
	unknown_004f55cc= game_state_malloc(AI_UNKNOWN_COUNT*AI_UNKNOWN_SIZE);
	ai_unknown_001c7fa0();
	actors_initialize();
	swarms_initialize();
	props_initialize();
	squads_initialize();
	unknown_005044cc= game_state_malloc(0x20);
	clump_data= data_new("clump", 0x14, 0x50, 0, game_state_allocation);
	joint_state_data= data_new("joint state", 0x14, 0xBC, 0, game_state_allocation);
	unknown_004f5768= NONE;
	dynamic_firing_point_data= data_new("dynamic firing points", 0xF, 0x484, 0, game_state_allocation);
	command_scripts_initialize();
	vocalizations_initialize();
	unknown_005044d0= game_state_malloc(0x10);
	unknown_00292130();
	unknown_001a6d80();
	flock_data= data_new("flocks", 0xA, 0x28, 0, game_state_allocation);
	unknown_005047f4= game_state_malloc(0x24);
}

/* 0x001c79c0: the second: the table of who has each place to fire from is
given back to what gave its memory, and there is none. Nothing else is given
back: the rest is of the state of the game */
void ai_dispose(
	void)
{
	struct hash_table *table= actor_firing_position_owners;

	table->allocation->functions->deallocate(table->allocation, table);
	actor_firing_position_owners= NULL;
}
