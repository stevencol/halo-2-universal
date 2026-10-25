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

#include <string.h>

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

/* 0x0016e150: a row of bits (one for each cluster, at 0x9C) of the bits at
0x58 of a structure bsp; after the first square of them when the bits (0x54
bytes) have room for more */
unsigned long *structure_bsp_unknown_0016e150(
	short row,
	const byte *bsp)
{
	long clusters= *(const long *)(bsp+0x9C);
	long longs= (clusters+0x1F)>>5;
	long square= clusters*longs;

	if ((unsigned long)(square*4)<*(const unsigned long *)(bsp+0x54))
	{
		return *(unsigned long *const *)(bsp+0x58)+row*longs+square;
	}

	return *(unsigned long *const *)(bsp+0x58)+((clusters+0x1F)>>5)*row;
}

/* 0x0016e1b0: which element (0x48 bytes) of a block has an index in its
range (from its short at 0x06, as many as at 0x08), and the index in it;
NONE if none. The long first given is cleared */
void __attribute__((stdcall)) structure_bsp_unknown_0016e1b0(
	long index,
	const long *block,
	long *cleared,
	long *element,
	long *offset)
{
	long which;

	*cleared= 0;
	for (which= 0; which<block[0]; which++)
	{
		const byte *entry= (const byte *)(uintptr_t)block[1]+which*0x48;
		long first= *(const unsigned short *)(entry+0x06);

		if (first<=index && first+*(const unsigned short *)(entry+0x08)>index)
		{
			*offset= index-first;
			break;
		}
	}
	*element= which!=block[0] ? which : NONE;
}

/* 0x00130f60: the second long of an entry (8 bytes, of the block at 0x08 of
the scenario) that something names (its byte at 0x6C, or at 0x6E when that
is NONE, for a tag without the bit 0x10 of its byte at 0x10), or the one
the element (0x44 bytes, at 0x214) of the structure bsp names at 0x40 when
it has the bit 0x01 at 0x3C; and which it is. NONE if none */
long scenario_unknown_00130f60(
	const byte *data,
	long *index)
{
	const byte *scenario= (const byte *)global_scenario;
	long count= *(const long *)(scenario+0x08);
	const long *entries= *(const long *const *)(scenario+0x0C);
	signed char which= (signed char)data[0x6C];
	const byte *element;
	short other;

	*index= NONE;
	if (which!=NONE)
	{
		if (which>=0 && which<count)
		{
			*index= which;
			return entries[which*2+1];
		}
	}
	else
	{
		which= (signed char)data[0x6E];
		if (which!=NONE && which>=0 && which<count && entries[which*2+1]!=NONE
			&& !(((const byte *)(uintptr_t)global_tag_instances[entries[which*2+1]&0xFFFF].address)[0x10]&0x10))
		{
			*index= which;
			return entries[which*2+1];
		}
	}
	element= *(const byte *const *)(scenario+0x214)+global_structure_bsp_index*0x44;
	if (!(element[0x3C]&1))
	{
		return NONE;
	}
	other= *(const short *)(element+0x40);
	if (other==NONE || other<0 || other>=count || entries[other*2+1]==NONE)
	{
		return NONE;
	}
	*index= other;
	return entries[other*2+1];
}

/* 0x0014b360: the entry of the scenario (its block at 0x08) that a cluster
of the structure bsp names (its byte at 0x6E, or 0 with the bit 0x02 at 0x12
of the scenario), and the point at 0xA0 of its tag when that has something
at 0x04 and the bit 0x20 of its byte at 0x10 */
void scenario_unknown_0014b360(
	long cluster_index,
	long *index,
	boolean *found,
	unsigned long *point)
{
	const byte *scenario= (const byte *)global_scenario;
	long count= *(const long *)(scenario+0x08);
	short which;
	long tag_index= NONE;
	const byte *tag;

	*found= FALSE;
	*index= NONE;
	if (cluster_index==NONE)
	{
		return;
	}
	*index= (signed char)(*(const byte *const *)((const byte *)global_structure_bsp+0xA0))[cluster_index*0xB0+0x6E];
	if (*index==NONE && (scenario[0x12]&2) && count>0)
	{
		*index= 0;
	}
	which= (short)*index;
	if (which>=0 && which<count)
	{
		tag_index= (*(const long *const *)(scenario+0x0C))[which*2+1];
	}
	if (tag_index==NONE)
	{
		return;
	}
	tag= (const byte *)(uintptr_t)global_tag_instances[tag_index&0xFFFF].address;
	if (!tag || *(const long *)(tag+0x04)==NONE || !(tag[0x10]&0x20))
	{
		return;
	}
	point[0]= ((const unsigned long *)(tag+0xA0))[0];
	point[1]= ((const unsigned long *)(tag+0xA0))[1];
	point[2]= ((const unsigned long *)(tag+0xA0))[2];
	*found= TRUE;
}

/* 0x0019c440: of the things (0x108 bytes, as many as at 0x00 of the block
at 0x174 of the game globals, when they have 0x170) with a number, the
smallest of their longs at 0x04 above another; NONE if none is */
long __attribute__((stdcall)) unknown_0019c440(
	long campaign,
	long map)
{
	const byte *globals= (const byte *)global_game_globals;
	const byte *block= NULL;
	long result= NONE;
	long index;

	if (global_scenario && globals && *(const long *)(globals+0x170))
	{
		block= *(const byte *const *)(globals+0x174);
	}
	if (!block)
	{
		return NONE;
	}
	for (index= 0; index<*(const long *)block; index++)
	{
		const byte *thing= *(const byte *const *)(block+0x04)+index*0x108;
		long value= *(const long *)(thing+0x04);

		if (*(const long *)thing==campaign && value>map && (result==NONE || value<result))
		{
			result= value;
		}
	}

	return result;
}

/* 0x0014b240, of the structure bsp */
real unknown_0014b240(const byte *structure_bsp, const byte *portal, const real_point3d *position);

/* 0x0012e960: the clusters of the structure bsp near a point, from its
cluster through the portals (0x24 bytes each, at 0x60: the clusters at
both sides; those of a cluster, of 0xB0 bytes at 0xA0, in the shorts at 0x90,
as many as at 0x8C): each (0x18 bytes: the cluster, the distance of its
nearest portal, by unknown_0014b240, and NONE at 0x14) less than 10 away,
in the order they are reached; how many */
long __attribute__((stdcall)) scenario_unknown_0012e960(
	long cluster_index,
	const real_point3d *point,
	byte *clusters)
{
	const byte *structure_bsp= global_structure_bsp;
	unsigned long visited[512/32];
	short indices[512];
	long current= 0;
	long count= 1;

	memset(visited, 0, sizeof(visited));
	indices[cluster_index]= 0;
	visited[cluster_index>>5]|= 1UL<<(cluster_index&31);
	*(long *)clusters= cluster_index;
	*(real *)(clusters+0x04)= 0.0f;
	*(long *)(clusters+0x14)= NONE;
	do
	{
		const byte *entry= clusters+current*0x18;
		const byte *cluster= *(const byte *const *)(structure_bsp+0xA0)+*(const long *)entry*0xB0;
		long portal_index;

		current++;
		for (portal_index= 0; portal_index<*(const long *)(cluster+0x8C); portal_index++)
		{
			const byte *portal= *(const byte *const *)(structure_bsp+0x60)
				+*(const short *)(*(const byte *const *)(cluster+0x90)+portal_index*2)*0x24;
			long other= *(const short *)portal;
			real distance;

			if (other==*(const long *)entry)
			{
				other= *(const short *)(portal+0x02);
			}
			if (other==NONE)
			{
				continue;
			}
			distance= unknown_0014b240(structure_bsp, portal, point);
			if (visited[other>>5]&(1UL<<(other&31)))
			{
				byte *existing= clusters+indices[other]*0x18;

				if (distance>*(const real *)(existing+0x04))
				{
					distance= *(const real *)(existing+0x04);
				}
				*(real *)(existing+0x04)= distance;
			}
			else if (10.0f>distance)
			{
				byte *added= clusters+count*0x18;

				indices[other]= (short)count;
				*(long *)added= other;
				*(real *)(added+0x04)= distance;
				*(long *)(added+0x14)= NONE;
				count++;
				visited[other>>5]|= 1UL<<(other&31);
			}
		}
	}
	while (current<count);

	return count;
}
