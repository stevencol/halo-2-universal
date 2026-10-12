/*
DECALS.C

the marks that are left on the level. Worked out from halo2ship.exe; the name of the file is a guess, by
the name that the game gives to its table of data ("decals").
*/

#include "cseries/cseries.h"
#include "memory/data.h"
#include "memory/lruv_cache.h"
#include "saved games/game_state.h"

#include <string.h>

/* ---------- constants */

enum
{
	/* of what decal_globals points to, 0x3C2C bytes of the state of the
	game: 0xE00 numbers that are NONE for a new map, and three more: NONE
	and nothing twice */
	DECAL_GLOBALS_SIZE= 0x3C2C,
	DECAL_GLOBALS_UNKNOWN0_COUNT= 0xE00,
	DECAL_GLOBALS_UNKNOWN3800_OFFSET= 0x3800,
	DECAL_GLOBALS_UNKNOWN3804_OFFSET= 0x3804,
	DECAL_GLOBALS_UNKNOWN3808_OFFSET= 0x3808
};

/* ---------- globals */

/* 0x004ea950: the marks ("decals": 0x400 of 0x40 bytes). 0x004ea94c: what
else is kept of them */
struct data_array *decal_data;
byte *decal_globals;

/* ---------- code */

/* 0x0017d2e0: for a new map, what is kept of the marks is nothing, with
NONE for its 0xE00 numbers and for the one after them; and the table can be
used, and is empty (data_delete_all) */
void decals_initialize_for_new_map(
	void)
{
	byte *globals= decal_globals;
	struct data_array *data= decal_data;

	memset(globals, 0, DECAL_GLOBALS_SIZE);
	memset(globals, 0xFF, DECAL_GLOBALS_UNKNOWN0_COUNT*sizeof(long));
	*(long *)(globals+DECAL_GLOBALS_UNKNOWN3800_OFFSET)= NONE;
	*(long *)(globals+DECAL_GLOBALS_UNKNOWN3804_OFFSET)= 0;
	*(long *)(globals+DECAL_GLOBALS_UNKNOWN3808_OFFSET)= 0;
	data->valid= TRUE;
	data_delete_all(data);
}

/* 0x00509444: something of the console that the marks are drawn with.
0x00509448: the cache of what they are drawn from */
void *unknown_00509444;
struct lruv_cache *unknown_00509448;

/* 0x003fb640: of the libraries of the console. 0x0017d5f0: the marks are
done away with */
unsigned long D3DResource_Release(unsigned long resource);
void __attribute__((stdcall)) unknown_0017d5f0(long unknown);

/* 0x0017d2a0: the marks have no table; what the console had for them is
given back (D3DResource_Release), and the memory of the cache goes back to
where it came from */
void decals_dispose(
	void)
{
	void *resource= unknown_00509444;
	struct lruv_cache *cache;

	decal_data= NULL;
	if (resource)
	{
		D3DResource_Release((unsigned long)resource);
		unknown_00509444= NULL;
	}
	cache= unknown_00509448;
	cache->allocation->functions->deallocate(cache->allocation, cache);
}

/* 0x0017d330: when a map goes, the marks are done away with
(unknown_0017d5f0), the cache is emptied (lruv_flush), and the table cannot
be used */
void decals_dispose_from_old_map(
	void)
{
	unknown_0017d5f0(1);
	lruv_flush(unknown_00509448);
	decal_data->valid= FALSE;
}

/* 0x00043890: what more the marks need to start */
void unknown_00043890(void);

/* 0x0017d220: the marks make their table, in the memory of the state of the
game (data_new, which the game has written here): 0x400 of 0x40 bytes. What
else is kept of them takes its memory there too (game_state_malloc, written
here), and unknown_00043890 is done */
void decals_initialize(
	void)
{
	decal_data= data_new("decals", 0x400, 0x40, 0, game_state_allocation);
	decal_globals= game_state_malloc(DECAL_GLOBALS_SIZE);
	unknown_00043890();
}

/* 0x004c8798: for each block of the cache, the mark that it is of, or NONE
(the name is a guess) */
long decal_cache_block_decal_indices[0x400];

/* 0x0017d810: a mark, and those that follow it in its list, lose what they
have in the cache */
void unknown_0017d810(long decal_index);

/* 0x00043820: the first of the two functions of the cache: a block goes, and
its mark, if it has one, is told */
void LRUV_CALLBACK decal_cache_delete_block(
	long block_index)
{
	long *decal_index= &decal_cache_block_decal_indices[block_index&0xFFFF];

	if (*decal_index!=NONE)
	{
		unknown_0017d810(*decal_index);
		*decal_index= NONE;
	}
}

/* 0x00043850: the second: whether a block cannot go: its mark has one of the
two lowest bits of its byte at 2 */
boolean LRUV_CALLBACK decal_cache_block_locked(
	long block_index)
{
	const byte *decal= (const byte *)decal_data->data+
		(decal_cache_block_decal_indices[block_index&0xFFFF]&0xFFFF)*0x40;

	return (decal[2]&3)!=0;
}

/* 0x00043970: after the state of the game is read (game_state.c), the
cache, which is in that state, gets its two functions again */
void __attribute__((stdcall)) decals_after_load(
	unsigned long flags)
{
	(void)flags;
	unknown_00509448->delete_block_proc= decal_cache_delete_block;
	unknown_00509448->locked_block_proc= decal_cache_block_locked;
}

/* 0x00043990: a tick of the cache goes by (lruv_idle, which the game has
written here) */
void decals_cache_idle(
	void)
{
	lruv_idle(unknown_00509448);
}
