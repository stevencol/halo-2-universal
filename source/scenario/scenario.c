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

/* 0x001318a0: one of the things of 0x10 bytes of the block that the scenario
has at 0x348; NULL with no scenario or for a number that is not of the block */
const byte *scenario_get_unknown348(
	long index)
{
	const byte *scenario= (const byte *)global_scenario;

	if (scenario && index>=0 && index<*(const long *)(scenario+0x348))
	{
		return *(const byte *const *)(scenario+0x34C)+index*0x10;
	}

	return NULL;
}

/* 0x0011c800: what one of the things of 0x20 bytes that the scenario has at
0x11C says: a point (three numbers) and the number after it. Each goes where
the caller says, if it says where */
void scenario_get_unknown11C(
	long index,
	long *point,
	long *unknown)
{
	const long *element=
		(const long *)(*(const byte *const *)((const byte *)global_scenario+0x11C)+index*0x20);

	if (point)
	{
		point[0]= element[0];
		point[1]= element[1];
		point[2]= element[2];
	}
	if (unknown)
	{
		*unknown= element[3];
	}
}

/* 0x002bdd20: the points of those things that something names: it has their
numbers first and how many they are at 0x60. The game has the function
before this written in the loop */
void unknown_get_scenario_unknown11C_points(
	const long *unknown,
	long *points)
{
	long index;

	for (index= 0; index<unknown[0x18]; index++)
	{
		scenario_get_unknown11C(unknown[index], points+index*3, NULL);
	}
}

/* 0x002729b0: the tag that one of the things of 8 bytes that the scenario
has at 0x150 names at 4; the thing is the one that something names with its
short at 0x20. NULL if the short is not of the block or the thing names no
tag */
void *unknown_get_scenario_unknown150_tag(
	const byte *unknown)
{
	const byte *scenario= (const byte *)global_scenario;
	short index= *(const short *)(unknown+0x20);

	if (index>=0 && index<*(const long *)(scenario+0x150))
	{
		long tag_index= *(const long *)(*(const byte *const *)(scenario+0x154)+index*8+4);

		if (tag_index!=NONE)
		{
			return (void *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
		}
	}

	return NULL;
}

/* 0x0011be90: a place in a leaf of the structure bsp: its cluster is the
first short of the leaf (8 bytes, of the list at 0x30) */
void scenario_location_from_leaf(
	struct scenario_location *location,
	long leaf_index)
{
	location->leaf_index= leaf_index;
	if (leaf_index!=NONE)
	{
		location->cluster_index= *(const short *)(*(const byte *const *)((const byte *)global_structure_bsp+0x30)+leaf_index*8);
		location->structure_bsp_index= global_structure_bsp_index;
		return;
	}
	location->cluster_index= NONE;
	location->structure_bsp_index= global_structure_bsp_index;
}

/* 0x0011bf30: the bit 0x01 of the first byte of the tag of the element (0x64
bytes, of the block at 0xD4 of the structure bsp) that the cluster (0xB0
bytes, at 0xA0) of a place names at 0x72; FALSE if there is none */
boolean scenario_location_unknown_0011bf30(
	const struct scenario_location *location)
{
	const byte *bsp= (const byte *)global_structure_bsp;
	short index= *(const short *)(*(const byte *const *)(bsp+0xA0)+location->cluster_index*0xB0+0x72);
	long tag_index;

	if (index==NONE || index>=*(const long *)(bsp+0xD4))
	{
		return FALSE;
	}
	tag_index= *(const long *)(*(const byte *const *)(bsp+0xD8)+index*0x64+0x24);
	if (tag_index==NONE)
	{
		return FALSE;
	}

	return *(const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address&1;
}

/* 0x0011c010: a bit of the rows of bits at 0x58 of the structure bsp, one
row for each of its clusters (0x9C) */
long __attribute__((stdcall)) structure_bsp_unknown_0011c010(
	short column,
	short row)
{
	const byte *bsp= (const byte *)global_structure_bsp;
	const unsigned long *bits= *(const unsigned long *const *)(bsp+0x58);

	return (bits[((*(const long *)(bsp+0x9C)+0x1F)>>5)*row+(column>>5)]&(1UL<<(column&0x1F)))!=0;
}

/* 0x0011c0b0: which of the names (0x24 bytes, of the block at 0x48 of
something) is a text; NONE if none */
short __attribute__((stdcall)) scenario_unknown_0011c0b0(
	const byte *data,
	const char *name)
{
	short index;

	for (index= 0; index<*(const long *)(data+0x48); index++)
	{
		const char *entry= (const char *)(*(const byte *const *)(data+0x4C)+index*0x24);
		const char *other= name;

		while (*entry==*other && *entry)
		{
			entry++;
			other++;
		}
		if (*entry==*other)
		{
			return index;
		}
	}

	return NONE;
}
