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
	DECAL_GLOBALS_UNKNOWN3808_OFFSET= 0x3808,

	/* of a mark: which of seven lists it is in (a byte, at 3) and of which
	of 0x200 places (a short, at 0xA; NONE for the list of the marks of no
	place); the mark before it and the one after it in the list; and the
	next of the marks that were made with it. The names are guesses */
	DECAL_SIZE= 0x40,
	DECAL_LIST_OFFSET= 3,
	DECAL_PLACE_OFFSET= 0xA,
	DECAL_PREVIOUS_OFFSET= 0x30,
	DECAL_NEXT_OFFSET= 0x34,
	DECAL_NEXT_OF_CHAIN_OFFSET= 0x3C,
	DECAL_PLACES_PER_LIST_BITS= 9
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

/* 0x003fb640: of the libraries of the console. 0x0017d5f0 is below */
unsigned long D3DResource_Release(unsigned long resource);
void __attribute__((stdcall)) decals_clear_flags(boolean both);

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

/* 0x0017d330: when a map goes, the marks lose their two bits
(decals_clear_flags), the cache is emptied (lruv_flush), and the table cannot
be used */
void decals_dispose_from_old_map(
	void)
{
	decals_clear_flags(1);
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
void decal_delete_chain(long decal_index);

/* 0x00043820: the first of the two functions of the cache: a block goes, and
its mark, if it has one, is told */
void LRUV_CALLBACK decal_cache_delete_block(
	long block_index)
{
	long *decal_index= &decal_cache_block_decal_indices[block_index&0xFFFF];

	if (*decal_index!=NONE)
	{
		decal_delete_chain(*decal_index);
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

#define DECAL_GET(decal_index) ((byte *)decal_data->data+((decal_index)&0xFFFF)*DECAL_SIZE)

/* 0x0017d860: a mark is taken out of its list and is no more. The first of
a list is kept in what decal_globals points to: one of the 0xE00 numbers, by
the list and the place of the mark, or the number at 0x3800 for a mark of no
place */
void decal_delete(
	long decal_index)
{
	byte *decal= DECAL_GET(decal_index);

	if (*(long *)(decal+DECAL_NEXT_OFFSET)!=NONE)
	{
		*(long *)(DECAL_GET(*(long *)(decal+DECAL_NEXT_OFFSET))+DECAL_PREVIOUS_OFFSET)=
			*(long *)(decal+DECAL_PREVIOUS_OFFSET);
	}
	if (*(long *)(decal+DECAL_PREVIOUS_OFFSET)!=NONE)
	{
		*(long *)(DECAL_GET(*(long *)(decal+DECAL_PREVIOUS_OFFSET))+DECAL_NEXT_OFFSET)=
			*(long *)(decal+DECAL_NEXT_OFFSET);
	}
	else if (*(short *)(decal+DECAL_PLACE_OFFSET)==NONE)
	{
		*(long *)(decal_globals+DECAL_GLOBALS_UNKNOWN3800_OFFSET)= *(long *)(decal+DECAL_NEXT_OFFSET);
	}
	else
	{
		((long *)decal_globals)[((char)decal[DECAL_LIST_OFFSET]<<DECAL_PLACES_PER_LIST_BITS)+
			*(short *)(decal+DECAL_PLACE_OFFSET)]= *(long *)(decal+DECAL_NEXT_OFFSET);
	}
	datum_delete(decal_data, decal_index);
}

/* 0x0017d810: a mark is deleted, and those that were made with it after it */
void decal_delete_chain(
	long decal_index)
{
	long next_index= *(long *)(DECAL_GET(decal_index)+DECAL_NEXT_OF_CHAIN_OFFSET);

	decal_delete(decal_index);
	while (next_index!=NONE)
	{
		decal_index= next_index;
		next_index= *(long *)(DECAL_GET(decal_index)+DECAL_NEXT_OF_CHAIN_OFFSET);
		decal_delete(decal_index);
	}
}

/* 0x0017d190: after the state of the game is read (game_state.c), the table
of the mark of each block of the cache, which is not in that state, is made
again: each mark that is the first of its chain (it has its own index at
0x38) has its block of the cache at 4. The game has the walk over the table
written here, as in the one that follows */
void __attribute__((stdcall)) decals_after_load_cache_blocks(
	unsigned long flags)
{
	(void)flags;
	if (decal_data->valid)
	{
		long absolute_index= NONE;

		for (;;)
		{
			const byte *decal;
			long decal_index;

			absolute_index= data_next_absolute_index(decal_data, absolute_index+1);
			if (absolute_index==NONE)
			{
				break;
			}
			decal= (const byte *)decal_data->data+absolute_index*decal_data->size;
			decal_index= (*(const short *)decal<<16)|absolute_index;
			decal= DECAL_GET(decal_index);
			if (*(const long *)(decal+0x38)==decal_index)
			{
				decal_cache_block_decal_indices[*(const long *)(decal+4)&0xFFFF]= decal_index;
			}
		}
	}
}

/* 0x0017d5f0: every mark loses the bit 0 of its byte at 2, and the bit 1
too if both are asked for; what decal_globals points to counts the marks
that have each (at 0x3804 and 0x3808), and counts one less for each bit that
goes */
void __attribute__((stdcall)) decals_clear_flags(
	boolean both)
{
	if (decal_data->valid)
	{
		byte *globals= decal_globals;
		long absolute_index= NONE;

		for (;;)
		{
			byte *decal;

			absolute_index= data_next_absolute_index(decal_data, absolute_index+1);
			if (absolute_index==NONE)
			{
				break;
			}
			decal= (byte *)decal_data->data+absolute_index*decal_data->size;
			if (decal[2]&1)
			{
				decal[2]&= ~1;
				(*(long *)(globals+DECAL_GLOBALS_UNKNOWN3804_OFFSET))--;
			}
			if (both && (decal[2]&2))
			{
				decal[2]&= ~2;
				(*(long *)(globals+DECAL_GLOBALS_UNKNOWN3808_OFFSET))--;
			}
		}
	}
}

/* 0x0017cc60: what is done with a mark each frame */
void decal_update(long decal_index);

/* 0x0017ccd0: the first mark of a chain gets older; whether it is no more */
boolean unknown_0017ccd0(long decal_index);

/* 0x0017d690: each frame (game.c), if the table can be used: a tick of the
cache goes by, and each mark is updated */
void decals_update(
	void)
{
	if (decal_data->valid)
	{
		long absolute_index= NONE;

		decals_cache_idle();
		for (;;)
		{
			const byte *decal;

			absolute_index= data_next_absolute_index(decal_data, absolute_index+1);
			if (absolute_index==NONE)
			{
				break;
			}
			decal= (const byte *)decal_data->data+absolute_index*decal_data->size;
			decal_update((*(const short *)decal<<16)|absolute_index);
		}
	}
}

/* 0x0017cc60: a mark that is the first of its chain gets older
(unknown_0017ccd0), and if it is still there, the others of its chain take
the highest byte of its colour (at 0x24; how much it shows, it seems) as it
was before */
void decal_update(
	long decal_index)
{
	byte *decal= DECAL_GET(decal_index);

	if (*(long *)(decal+0x38)==decal_index)
	{
		unsigned long alpha= decal[0x27];
		long next_index= *(long *)(decal+DECAL_NEXT_OF_CHAIN_OFFSET);

		if (!unknown_0017ccd0(decal_index))
		{
			while (next_index!=NONE)
			{
				decal= DECAL_GET(next_index);
				next_index= *(long *)(decal+DECAL_NEXT_OF_CHAIN_OFFSET);
				*(unsigned long *)(decal+0x24)= (*(unsigned long *)(decal+0x24)&0xFFFFFF)|(alpha<<24);
			}
		}
	}
}

void unknown_0023aa70(void);

/* 0x0017d440: the sixth of the functions of the part (game.c): when the
structure bsp goes, after unknown_0023aa70, the marks of every place are of
no place: each list of each place is put, whole, before the list of the
marks of no place, and is empty */
void decals_dispose_from_old_structure_bsp(
	void)
{
	unknown_0023aa70();
	if (decal_data->valid)
	{
		byte *globals= decal_globals;
		long place;

		for (place= 0; place<(1<<DECAL_PLACES_PER_LIST_BITS); place++)
		{
			long list;

			for (list= 0; list<7; list++)
			{
				long *first= &((long *)globals)[(list<<DECAL_PLACES_PER_LIST_BITS)+place];
				long first_index= *first;
				long decal_index= first_index;

				while (decal_index!=NONE)
				{
					byte *decal= DECAL_GET(decal_index);
					long next_index= *(long *)(decal+DECAL_NEXT_OFFSET);

					*(short *)(decal+DECAL_PLACE_OFFSET)= NONE;
					if (next_index==NONE)
					{
						long *others= (long *)(globals+DECAL_GLOBALS_UNKNOWN3800_OFFSET);

						*(long *)(decal+DECAL_NEXT_OFFSET)= *others;
						if (*others!=NONE)
						{
							*(long *)(DECAL_GET(*others)+DECAL_PREVIOUS_OFFSET)= decal_index;
						}
						*others= first_index;
						*first= NONE;
					}
					decal_index= next_index;
				}
			}
		}
	}
}
