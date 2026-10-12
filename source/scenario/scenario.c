/*
SCENARIO.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The scenario that is loaded and its structure bsp. The names are those of
Halo 1's scenario.c.
*/

#include "scenario.h"

#include "cache/cache_files.h"
#include "game/game.h"
#include "main/main.h"
#include "physics/bsp3d.h"
#include "scenario/scenario_definitions.h"

/* ---------- prototypes of functions not decompiled yet */

/* 0x0016e2c0: does something with each element of a block of a structure
bsp that was just loaded */
void unknown_0016e2c0(long structure_bsp_tag_index);

/* ---------- constants */

enum
{
	/* in a structure bsp, the address of its collision bsp */
	STRUCTURE_BSP_COLLISION_BSP_OFFSET= 0x18,
	/* in a structure bsp, the address of its leaves: 8 bytes each, which
	start with the index of the cluster */
	STRUCTURE_BSP_LEAVES_OFFSET= 0x30
};

/* ---------- globals */

/* 0x004686c0: the tag index of the scenario; NONE if there is none */
long global_scenario_index= NONE;

/* 0x004686c4: which structure bsp of the scenario is loaded; NONE if none
is */
short global_structure_bsp_index= NONE;

/* 0x004e0350: the scenario tag */
struct scenario *global_scenario;

/* 0x004e034c: the globals tag of the cache file */
struct game_globals *global_game_globals;

/* 0x004e0348, 0x004e0344, 0x004e0340 and 0x004e033c: the structure bsp that
is loaded, its lightmap, and its collision bsp twice. In Halo 1 the last is
the bsp3d of the collision bsp; here it gets the same address, and its name
is a guess */
void *global_structure_bsp;
void *global_structure_lightmap;
void *global_collision_bsp;
void *global_bsp3d;

/* ---------- code */

static inline __attribute__((always_inline)) void scenario_structure_bsp_globals_clear(
	void)
{
	global_structure_bsp= NULL;
	global_structure_lightmap= NULL;
	global_collision_bsp= NULL;
	global_bsp3d= NULL;
}

/* 0x0011c1b0: makes a structure bsp of the scenario the one that is loaded,
or none of them with NONE; whether it is done. The parts of the game are told
before, take down what they have of the old one, and set up for the new
one */
boolean scenario_switch_structure_bsp(
	long structure_bsp_index,
	long unload_old)
{
	const struct tag_block *structure_bsps=
		(const struct tag_block *)((const byte *)global_scenario+SCENARIO_STRUCTURE_BSPS_OFFSET);
	short game_structure_bsp_index= main_game_globals->structure_bsp_index;
	short index= (short)structure_bsp_index;
	boolean result= TRUE;
	long system_index;

	if (index!=NONE && (index<0 || index>=structure_bsps->count))
	{
		return FALSE;
	}
	if (global_structure_bsp_index==index && game_structure_bsp_index==index)
	{
		return TRUE;
	}

	for (system_index= 0; system_index<NUMBER_OF_GAME_SYSTEMS; system_index++)
	{
		if (game_systems[system_index].prepare_for_new_structure_bsp)
		{
			game_systems[system_index].prepare_for_new_structure_bsp(structure_bsp_index);
		}
	}

	if (game_structure_bsp_index!=NONE && game_structure_bsp_index!=index)
	{
		game_dispose_from_old_structure_bsp();
		game_structure_bsp_index= main_game_globals->structure_bsp_index;
	}

	if (global_structure_bsp_index!=NONE && global_structure_bsp_index!=index)
	{
		if ((byte)unload_old)
		{
			cache_file_structure_bsp_unload(
				&((const struct scenario_structure_bsp_reference *)(uintptr_t)structure_bsps->address)
					[global_structure_bsp_index]);
		}
		global_structure_bsp_index= NONE;
		scenario_structure_bsp_globals_clear();
	}

	if (global_structure_bsp_index!=index)
	{
		const struct scenario_structure_bsp_reference *reference=
			&((const struct scenario_structure_bsp_reference *)(uintptr_t)structure_bsps->address)[index];

		scenario_structure_bsp_globals_clear();
		global_structure_bsp_index= index;
		if (cache_file_structure_bsp_load(reference))
		{
			unknown_0016e2c0((long)reference->structure_bsp.index);
			if (reference->structure_lightmap.index!=(uint32_t)NONE)
			{
				global_structure_lightmap=
					(void *)(uintptr_t)global_tag_instances[reference->structure_lightmap.index&0xFFFF].address;
			}
			else
			{
				global_structure_lightmap= NULL;
			}
			global_structure_bsp=
				(void *)(uintptr_t)global_tag_instances[reference->structure_bsp.index&0xFFFF].address;
			global_collision_bsp= *(void **)((byte *)global_structure_bsp+STRUCTURE_BSP_COLLISION_BSP_OFFSET);
			global_bsp3d= global_collision_bsp;
		}
		else
		{
			/* the parts of the game are not set up for a structure bsp
			that is not there */
			global_structure_bsp_index= NONE;
			main_time_skip();
			return FALSE;
		}
	}

	if (game_structure_bsp_index!=index)
	{
		game_initialize_for_new_structure_bsp();
	}
	/* the time that loading took is not a part of the next frame */
	main_time_skip();

	return result;
}

/* 0x0011be50: the scenario and its structure bsp are unloaded. As in Halo
1 */
void scenario_unload(
	void)
{
	scenario_tags_unload();
	global_scenario_index= NONE;
	global_scenario= NULL;
	global_structure_bsp_index= NONE;
	global_game_globals= NULL;
	scenario_structure_bsp_globals_clear();
}

/* 0x0011bed0: where a point is in the structure bsp that is loaded: its
leaf and the leaf's cluster, which are NONE out of the bsp, and the index of
the bsp, NONE if none is loaded. As in Halo 1 */
void scenario_location_from_point(
	struct scenario_location *location,
	const real_point3d *point)
{
	short structure_bsp_index= global_structure_bsp_index;

	if (structure_bsp_index==NONE)
	{
		location->cluster_index= NONE;
		location->structure_bsp_index= NONE;
		location->leaf_index= NONE;
		return;
	}
	location->leaf_index= bsp3d_test_point((const struct bsp3d *)global_bsp3d, 0, point);
	if (location->leaf_index!=NONE)
	{
		const byte *leaves= *(byte *const *)((const byte *)global_structure_bsp+STRUCTURE_BSP_LEAVES_OFFSET);

		location->structure_bsp_index= structure_bsp_index;
		location->cluster_index= *(const short *)(leaves+location->leaf_index*8);
	}
	else
	{
		location->structure_bsp_index= structure_bsp_index;
		location->cluster_index= NONE;
	}
}

/* 0x0011bff0: the cluster of a leaf of the structure bsp (the short that
each of its leaves, of 8 bytes, starts with; the leaves are at 0x30 of it),
or NONE for no leaf. The game has it far from the rest of this file */
long structure_bsp_leaf_get_cluster_index(
	long leaf_index)
{
	if (leaf_index!=NONE)
	{
		return *(const short *)(*(const byte *const *)((const byte *)global_structure_bsp+0x30)+leaf_index*8);
	}

	return NONE;
}

/* 0x001fa7f0: the first of what the structure bsp has at 0xC4, or NULL if it
has none. Far from the rest too */
void *structure_bsp_get_unknownC4(
	void)
{
	const byte *structure_bsp= global_structure_bsp;

	if (*(const long *)(structure_bsp+0xC4)>0)
	{
		return *(void *const *)(structure_bsp+0xC8);
	}

	return NULL;
}
