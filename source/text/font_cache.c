/*
FONT_CACHE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The characters of the fonts that the game has ready to draw. The names of the
file and of the functions are guesses.
*/

#include "font_cache.h"

#include "cseries/async.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x003312cc */
void *VirtualAlloc(void *address, unsigned long size, unsigned long type, unsigned long protection);

/* 0x002d1d3e */
uint32_t GetLastError(void);

/* 0x002d63fa, 0x002d62c4 */
uint32_t WaitForSingleObject(uint32_t handle, uint32_t milliseconds);
int ReleaseMutex(uint32_t mutex);

#define MEM_COMMIT 0x1000
#define MEM_TOP_DOWN 0x100000
#define PAGE_READWRITE 4
#define INFINITE 0xFFFFFFFF

/* ---------- globals */

/* 0x0054d570 */
struct font_cache_globals font_cache_globals;

/* 0x004b62b0: the places for the pictures of the characters that are drawn:
the character of each. They are of what draws the texts, which is not
decompiled */
struct font_glyph_slot font_glyph_slots[NUMBER_OF_FONT_GLYPH_SLOTS];

/* ---------- code */

/* 0x00141090: the number of a character of a font, for the table that finds
it */
unsigned long HASH_TABLE_CALLBACK font_character_key_hash(
	long key)
{
	const struct font_character_key *character= (const struct font_character_key *)key;

	return (unsigned long)(character->font_slot_index*3 + character->character);
}

/* 0x001410b0: whether two are the same character of the same font */
boolean HASH_TABLE_CALLBACK font_character_key_compare(
	long key,
	long other)
{
	const struct font_character_key *a= (const struct font_character_key *)key;
	const struct font_character_key *b= (const struct font_character_key *)other;

	return a->font_slot_index==b->font_slot_index && a->character==b->character;
}

/* 0x00140b20: the index of what the game knows of a character of a font;
NONE if it knows nothing of it */
long font_character_find(
	long font_slot_index,
	long character)
{
	struct hash_table *table= font_cache_globals.character_table;
	struct font_character_key key;
	const struct hash_table_entry *entry;
	long index= NONE;

	key.font_slot_index= font_slot_index;
	key.character= character;
	entry= hash_table_find(table, (long)&key);
	if (!entry)
	{
		return NONE;
	}
	memcpy(&index, entry+1, (unsigned long)table->size);

	return index;
}

static inline __attribute__((always_inline)) struct font_character *font_character_get(
	long character_index)
{
	return (struct font_character *)font_cache_globals.characters->data + (character_index&0xFFFF);
}

/* 0x00140cd0: the game forgets a character: its place among the pictures
that are drawn is no one's, its pixels leave the cache, and it is taken out
of the table and of the characters */
void font_character_delete(
	long character_index)
{
	struct font_character *character= font_character_get(character_index);

	if (character->glyph_slot_index!=NONE)
	{
		struct font_glyph_slot *slot= &font_glyph_slots[character->glyph_slot_index];

		if (slot->character_index!=NONE)
		{
			font_character_get(slot->character_index)->glyph_slot_index= NONE;
		}
		slot->character_index= NONE;
	}
	if (character->block_index!=NONE)
	{
		lruv_block_delete(font_cache_globals.cache, character->block_index);
	}
	hash_table_remove(font_cache_globals.character_table, (long)&character->key);
	datum_delete(font_cache_globals.characters, character_index);
}

/* 0x00140d50: whether a character is not to be forgotten now: it was used
in this frame or in the last one, or it is being loaded, or its pixels are in
the cache and cannot leave it */
boolean font_character_is_busy(
	long character_index)
{
	const struct font_character *character= font_character_get(character_index);
	boolean busy= (long)((unsigned long)font_cache_globals.frame_index-(unsigned long)character->frame_index)<=1 ||
		character->state==_font_character_state_unknown1 || character->state==_font_character_state_unknown3;

	if (character->block_index!=NONE)
	{
		const struct font_cache_block *block=
			(const struct font_cache_block *)font_cache_globals.blocks->data + (character->block_index&0xFFFF);

		return busy || !block->loaded;
	}

	return busy;
}

/* 0x00140b80: what the game knows of a character of a font: what there is,
or a new one, of which nothing is loaded yet. If there is no room for a new
one, the one that has gone the longest with no use takes its place, of those
that are not busy. NONE if there is none */
long font_character_new(
	long font_slot_index,
	long character)
{
	struct font_character *new_character;
	long index= font_character_find(font_slot_index, character);

	if (index!=NONE)
	{
		return index;
	}
	index= datum_new(font_cache_globals.characters);
	if (index==NONE)
	{
		struct data_array *data= font_cache_globals.characters;
		long oldest_age= 0;
		long absolute_index= NONE;

		while ((absolute_index= data_next_absolute_index(data, absolute_index+1))!=NONE)
		{
			const struct font_character *candidate=
				(const struct font_character *)((const byte *)data->data+data->size*absolute_index);
			long candidate_index= ((long)candidate->identifier<<16)|absolute_index;

			if (!font_character_is_busy(candidate_index))
			{
				long age= (long)((unsigned long)font_cache_globals.frame_index-(unsigned long)candidate->frame_index);

				if (age>oldest_age)
				{
					oldest_age= age;
					index= candidate_index;
				}
			}
		}
		if (index==NONE)
		{
			return NONE;
		}
		font_character_delete(index);
		index= datum_new_at_index(font_cache_globals.characters, index);
		if (index==NONE)
		{
			return NONE;
		}
	}

	new_character= font_character_get(index);
	memset((byte *)new_character+sizeof(new_character->identifier), 0,
		sizeof(*new_character)-sizeof(new_character->identifier));
	new_character->key.character= character;
	new_character->unknownC= FALSE;
	new_character->unknownD= FALSE;
	new_character->state= _font_character_state_unknown0;
	new_character->unknown14= NONE;
	new_character->unknown18= NONE;
	new_character->block_index= NONE;
	new_character->key.font_slot_index= font_slot_index;
	new_character->frame_index= font_cache_globals.frame_index;
	new_character->glyph_slot_index= NONE;
	hash_table_add(font_cache_globals.character_table, (long)&new_character->key, &index);

	return index;
}

/* an array for so many things, of the memory of the game: the array, the
things and a bit for each. NULL if there is no memory for it */
static inline __attribute__((always_inline)) struct data_array *font_cache_data_new(
	const char *name,
	long maximum_count,
	long size)
{
	struct allocation *allocation= heap_allocation;
	struct data_array *data= allocation->functions->allocate(allocation,
		sizeof(struct data_array)+(unsigned long)(maximum_count*size)+(unsigned long)((maximum_count+31)>>5)*sizeof(unsigned long));

	if (data)
	{
		data_initialize(data, name, maximum_count, size, 0, allocation);
		data->flags|= FLAG(_data_array_allocated_bit);
	}

	return data;
}

/* an array is ready to be used, with nothing in it */
static inline __attribute__((always_inline)) void font_cache_data_make_valid(
	struct data_array *data)
{
	data->valid= TRUE;
	data_delete_all(data);
}

/* 0x001409d0: what the game knows of the characters: room for 512, for 256
that are about to be drawn, and the table that finds them */
void font_characters_new(
	void)
{
	font_cache_globals.characters= font_cache_data_new("character data", MAXIMUM_FONT_CHARACTERS,
		sizeof(struct font_character));
	font_cache_globals.unknown8= font_cache_data_new("predicted data", MAXIMUM_PREDICTED_FONT_CHARACTERS,
		FONT_PREDICTED_CHARACTER_SIZE);
	font_cache_globals.character_table= hash_table_new(heap_allocation, MAXIMUM_FONT_CHARACTERS,
		"font character hash table", sizeof(long), FONT_CHARACTER_TABLE_LIST_COUNT, font_character_key_hash,
		font_character_key_compare);
	font_cache_data_make_valid(font_cache_globals.characters);
	font_cache_data_make_valid(font_cache_globals.unknown8);
}

/* 0x00140aa0: the end of what the game knows of the characters */
void font_characters_dispose(
	void)
{
	if (font_cache_globals.character_table)
	{
		struct hash_table *table= font_cache_globals.character_table;

		table->allocation->functions->deallocate(table->allocation, table);
		font_cache_globals.character_table= NULL;
	}
	if (font_cache_globals.unknown8)
	{
		data_dispose(font_cache_globals.unknown8);
		font_cache_globals.unknown8= NULL;
	}
	if (font_cache_globals.characters)
	{
		data_dispose(font_cache_globals.characters);
		font_cache_globals.characters= NULL;
	}
}

/* 0x001410d0: the cache of the pixels of the characters: 30,720 bytes of the
console, in pages of 16 bytes, for 512 blocks at most */
void font_cache_new(
	void)
{
	struct allocation *allocation;
	struct lruv_cache *cache;
	byte *base_address= VirtualAlloc(NULL, FONT_CACHE_SIZE, MEM_COMMIT|MEM_TOP_DOWN, PAGE_READWRITE);

	if (!base_address)
	{
		GetLastError();
	}
	font_cache_globals.base_address= base_address;
	font_cache_globals.blocks= font_cache_data_new("font pixel data", MAXIMUM_FONT_CACHE_BLOCKS,
		sizeof(struct font_cache_block));

	allocation= heap_allocation;
	cache= allocation->functions->allocate(allocation, FONT_CACHE_LRUV_SIZE);
	lruv_initialize(cache, "font pixel data", FONT_CACHE_SIZE>>FONT_CACHE_PAGE_SIZE_BITS, FONT_CACHE_PAGE_SIZE_BITS,
		MAXIMUM_FONT_CACHE_BLOCKS, font_cache_block_deleted, font_cache_block_locked, NULL, allocation);
	font_cache_globals.cache= cache;
	cache->hole_algorithm= _lruv_hole_algorithm_age_per_page;
	font_cache_data_make_valid(font_cache_globals.blocks);
}

/* 0x001411e0: the cache lets go of a block: the game waits until its pixels
have been read, counting the tasks each time, and then the character has no
pixels and the block is no more */
void LRUV_CALLBACK font_cache_block_deleted(
	long block_index)
{
	struct font_character *character= font_character_get(
		((const struct font_cache_block *)font_cache_globals.blocks->data)[block_index&0xFFFF].character_index);

	while (!((volatile struct font_cache_block *)font_cache_globals.blocks->data)[block_index&0xFFFF].loaded)
	{
		const struct async_task *task;
		long count= 0;

		WaitForSingleObject(async_globals.task_list_mutex, INFINITE);
		for (task= async_globals.task_list; task; task= task->next)
		{
			count++;
		}
		ReleaseMutex(async_globals.task_list_mutex);
		async_globals.tasks_added= count;
	}
	character->block_index= NONE;
	datum_delete(font_cache_globals.blocks, block_index);
}

/* 0x00141290: whether a block cannot leave the cache: its pixels are being
read */
boolean LRUV_CALLBACK font_cache_block_locked(
	long block_index)
{
	return !((const struct font_cache_block *)font_cache_globals.blocks->data)[block_index&0xFFFF].loaded;
}

/* 0x00141190: the end of the cache of the pixels of the characters */
void font_cache_dispose(
	void)
{
	if (font_cache_globals.cache)
	{
		struct lruv_cache *cache= font_cache_globals.cache;

		cache->allocation->functions->deallocate(cache->allocation, cache);
		font_cache_globals.cache= NULL;
	}
	if (font_cache_globals.blocks)
	{
		data_dispose(font_cache_globals.blocks);
		font_cache_globals.blocks= NULL;
	}
}

/* 0x00141450: where the pixels of a block of the cache are */
byte *font_cache_block_get_address(
	long block_index)
{
	const struct lruv_cache *cache= font_cache_globals.cache;
	const struct lruv_cache_block *blocks= cache->blocks->data;

	return font_cache_globals.base_address +
		(blocks[block_index&0xFFFF].first_page_index<<(cache->page_size_bits&31));
}
