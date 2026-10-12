/*
AI.C

the part of the game that the minds of the characters are: it starts and stops
the others of this folder. Worked out from halo2ship.exe; the name of the file
is a guess, by what its functions start.
*/

#include "cseries/cseries.h"

#include <string.h>

#include "game/game.h"
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

/* 0x00295970: what unknown_005047f4 points to has nothing: four shorts of
its start are zero and the number at 0xC is NONE, and of its three at the end,
each has a short that is zero (at 0x12) and a number that is NONE (at 0x18).
In the game it is far from the others of this file, so it may be of another */
void ai_unknown_00295970(
	void)
{
	byte *unknown= unknown_005047f4;
	long index;

	*(short *)unknown= 0;
	*(short *)(unknown+2)= 0;
	*(short *)(unknown+8)= 0;
	*(short *)(unknown+6)= 0;
	*(long *)(unknown+0xC)= NONE;
	for (index= 0; index<3; index++)
	{
		*(short *)(unknown+0x12+2*index)= 0;
		*(long *)(unknown+0x18+4*index)= NONE;
	}
}

/* the tables of the others of the folder */
extern struct data_array *actor_data;
extern struct data_array *swarm_data;
extern struct data_array *prop_data;
extern struct data_array *prop_ref_data;
extern struct data_array *tracking_data;
extern struct data_array *command_script_data;
extern struct data_array *joint_command_script_data;

/* 0x00502428, 0x0050242c: two numbers that are NONE in a new map */
long unknown_00502428;
long unknown_0050242c;

void unknown_001c8010(void);
void unknown_00200b30(void);
void vocalizations_initialize_for_new_map(void);

/* 0x001c79e0: the third of the functions of the part (game.c). In
multiplayer there are no minds: only the second byte of what the part keeps
says so. Otherwise all the tables are good to use, with nothing in them, the
blocks of the state of the game are zeros, and that byte is set */
void ai_initialize_for_new_map(
	void)
{
	if (main_game_globals->options.game_mode==_game_mode_multiplayer)
	{
		ai_globals[1]= FALSE;
	}
	else
	{
		unknown_001c8010();
		data_make_valid(actor_data);
		hash_table_clear(actor_firing_position_owners);
		data_make_valid(swarm_data);
		data_make_valid(prop_data);
		data_make_valid(prop_ref_data);
		data_make_valid(tracking_data);
		unknown_00200b30();
		memset(unknown_005044cc, 0, 0x20);
		data_make_valid(clump_data);
		data_make_valid(joint_state_data);
		data_make_valid(dynamic_firing_point_data);
		data_make_valid(command_script_data);
		data_make_valid(joint_command_script_data);
		unknown_00502428= NONE;
		unknown_0050242c= NONE;
		vocalizations_initialize_for_new_map();
		memset(unknown_005044d0, 0, 0x10);
		data_make_valid(flock_data);
		ai_unknown_00295970();
		*(short *)(ai_globals+0x3E)= 0;
		*(short *)(ai_globals+0x3C)= 0;
		memset(ai_globals+0x40, 0, 0x300);
		ai_globals[1]= TRUE;
	}
}

/* more tables of the others of the folder */
extern struct data_array *squad_data;
extern struct data_array *squad_group_data;
extern struct data_array *vocalization_record_data;

void swarms_dispose_from_old_map(void);
void unknown_00292e00(void);
void unknown_00292f60(void);

/* 0x001c7b20: the fourth of the functions of the part. If there were minds
in the map that goes, their tables cannot be used any more, and there are
none */
void ai_dispose_from_old_map(
	void)
{
	if (ai_globals[1])
	{
		squad_data->valid= FALSE;
		squad_group_data->valid= FALSE;
		clump_data->valid= FALSE;
		joint_state_data->valid= FALSE;
		command_script_data->valid= FALSE;
		joint_command_script_data->valid= FALSE;
		dynamic_firing_point_data->valid= FALSE;
		prop_data->valid= FALSE;
		prop_ref_data->valid= FALSE;
		tracking_data->valid= FALSE;
		actor_data->valid= FALSE;
		swarms_dispose_from_old_map();
		vocalization_record_data->valid= FALSE;
		unknown_00292e00();
		ai_globals[1]= FALSE;
	}
}

/* 0x001c7f60: the sixth: when the structure bsp goes, and there are minds,
unknown_00292f60 is done */
void ai_dispose_from_old_structure_bsp(
	void)
{
	if (ai_globals[1])
	{
		unknown_00292f60();
	}
}

/* 0x0025dd20 and 0x0025dd30, of actors.c */
unsigned long HASH_TABLE_CALLBACK actor_firing_position_owner_hash(long key);
boolean HASH_TABLE_CALLBACK actor_firing_position_owner_compare(long key, long other);

/* 0x001c7f80: after the state of the game is read (game_state.c), the table
of who has each place to fire from, which is in that state, gets its two
functions again: the ones it had may be of another run of the game */
void __attribute__((stdcall)) ai_after_load(
	unsigned long flags)
{
	(void)flags;
	actor_firing_position_owners->hash= actor_firing_position_owner_hash;
	actor_firing_position_owners->compare= actor_firing_position_owner_compare;
}

void unknown_001c81c0(void);
void unknown_00201c80(void);
void unknown_00268510(void);
void unknown_001e2f50(void);
void unknown_0020fb30(void);
void unknown_00293830(void);
void unknown_001e3240(void);

/* 0x001c83c0: each tick of the game (game.c), if there are minds in the map
and this console is not one that only predicts what another rules. With the
first byte of what the part keeps (the minds are on, it seems) six things are
done, the short at 0x22 goes down to zero, and the third byte is set; without
it, and with the third byte, unknown_001e3240 is done once and the byte is
cleared */
void ai_update(
	void)
{
	if (ai_globals[1] && main_game_globals->options.simulation!=_game_simulation_predicted)
	{
		if (ai_globals[0])
		{
			unknown_001c81c0();
			if (*(short *)(ai_globals+0x22)>0)
			{
				(*(short *)(ai_globals+0x22))--;
			}
			unknown_00201c80();
			unknown_00268510();
			unknown_001e2f50();
			unknown_0020fb30();
			unknown_00293830();
			ai_globals[2]= TRUE;
		}
		else if (ai_globals[2])
		{
			unknown_001e3240();
			ai_globals[2]= FALSE;
		}
	}
}
