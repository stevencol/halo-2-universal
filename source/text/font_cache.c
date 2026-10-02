/*
FONT_CACHE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The characters of the fonts that the game has ready to draw. The names of the
file and of the functions are guesses.
*/

#include "font_cache.h"

#include "cseries/async.h"
#include "main/global_preferences.h"
#include "text/draw_string.h"
#include "text/fonts.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x003312cc */
void *VirtualAlloc(void *address, unsigned long size, unsigned long type, unsigned long protection);

/* 0x002d1d3e */
uint32_t GetLastError(void);

/* 0x002d63fa, 0x002d62c4 */
uint32_t WaitForSingleObject(uint32_t handle, uint32_t milliseconds);
int ReleaseMutex(uint32_t mutex);

/* 0x002d2084 */
int SwitchToThread(void);

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
		character->state==_font_character_state_reading_location || character->state==_font_character_state_reading_header;

	if (character->block_index!=NONE)
	{
		const struct font_cache_block *block=
			(const struct font_cache_block *)font_cache_globals.blocks->data + (character->block_index&0xFFFF);

		return busy || !block->loaded;
	}

	return busy;
}

/* the file of the font of a place; NONE if the font is not there */
static inline __attribute__((always_inline)) uint32_t font_cache_get_font_file(
	long font_slot_index)
{
	uint32_t file= (uint32_t)NONE;

	if (font_get(font_slot_index))
	{
		file= font_slots[font_slot_index].file;
	}

	return file;
}

/* the priority of a read of the file of a font: higher if the game is to
wait for it */
static inline __attribute__((always_inline)) long font_cache_read_priority(
	boolean wait)
{
	return wait ? FONT_CACHE_READ_PRIORITY_WAIT : FONT_CACHE_READ_PRIORITY;
}

/* waits for a read of the file of a font, if it is to: its task is hurried,
and the game stands by until it is done */
static inline __attribute__((always_inline)) void font_cache_read_wait(
	const struct font_character *character,
	const boolean *loaded,
	boolean wait)
{
	if (!*loaded && wait)
	{
		async_task_set_priority(character->task, FONT_LOAD_PRIORITY);
		while (!*(volatile const boolean *)loaded)
		{
			SwitchToThread();
		}
	}
}

/* 0x00140dc0: the first step of getting a character ready: a task reads
where the character is in the file of its font, from the table that the file
has, which has 4 bytes for each character */
void font_character_read_location(
	struct font_character *character,
	boolean wait)
{
	uint32_t offset= (uint32_t)character->key.character*sizeof(long)+FONT_FILE_LOCATIONS_OFFSET;
	long priority= font_cache_read_priority(wait);
	uint32_t file= font_cache_get_font_file(character->key.font_slot_index);

	character->task= async_read_position(file, sizeof(character->location), offset, FONT_CACHE_TASK_CATEGORY,
		priority, NULL, &character->location, &character->location_loaded);
	character->state= _font_character_state_reading_location;
}

/* 0x00140e40: what the file says of a character, made to be what can be: a
picture of 128 by 128 at the most, with pixels of 1 to 4096 bytes. If it was
not, the file is not to be trusted: the preferences forget which fonts are on
the hard disk, and are written at once */
void font_character_header_validate(
	struct font_character_header *header)
{
	if (header->width>=0 && header->width<=MAXIMUM_FONT_CHARACTER_WIDTH &&
		header->height>=0 && header->height<=MAXIMUM_FONT_CHARACTER_WIDTH &&
		header->pixels_size>0 && header->pixels_size<=MAXIMUM_FONT_CHARACTER_PIXELS_SIZE)
	{
		return;
	}
	header->width= header->width<0 ? 0 : header->width>MAXIMUM_FONT_CHARACTER_WIDTH ? MAXIMUM_FONT_CHARACTER_WIDTH :
		header->width;
	header->height= header->height<0 ? 0 : header->height>MAXIMUM_FONT_CHARACTER_WIDTH ? MAXIMUM_FONT_CHARACTER_WIDTH :
		header->height;
	header->pixels_size= header->pixels_size<1 ? 1 : header->pixels_size>MAXIMUM_FONT_CHARACTER_PIXELS_SIZE ?
		MAXIMUM_FONT_CHARACTER_PIXELS_SIZE : header->pixels_size;
	global_preferences_forget_fonts();
}

/* 0x00140f00: the second step: once it is known where the character is, the
third one is started */
void font_character_location_wait(
	struct font_character *character,
	boolean wait)
{
	font_cache_read_wait(character, &character->location_loaded, wait);
	if (!character->location_loaded)
	{
		return;
	}
	character->state= _font_character_state_location_read;
	font_character_read_header(character, wait);
}

/* 0x00140f50: the third step: a task reads what the file says of the
character, 16 bytes */
void font_character_read_header(
	struct font_character *character,
	boolean wait)
{
	uint32_t offset= (uint32_t)(character->location+FONT_FILE_HEADERS_OFFSET)<<4;
	long priority= font_cache_read_priority(wait);
	uint32_t file= font_cache_get_font_file(character->key.font_slot_index);

	character->task= async_read_position(file, sizeof(character->header), offset, FONT_CACHE_TASK_CATEGORY,
		priority, NULL, &character->header, &character->header_loaded);
	character->state= _font_character_state_reading_header;
}

/* 0x00140fc0: the fourth step: once what the file says of the character is
read, the character is ready */
void font_character_header_wait(
	struct font_character *character,
	boolean wait)
{
	font_cache_read_wait(character, &character->header_loaded, wait);
	if (!character->header_loaded)
	{
		return;
	}
	font_character_header_validate(&character->header);
	character->state= _font_character_state_ready;
}

/* 0x00141020: one step more of getting a character ready. Whether it is */
boolean font_character_update(
	long character_index,
	boolean wait)
{
	struct font_character *character= font_character_get(character_index);

	switch (character->state)
	{
	case _font_character_state_new:
		font_character_read_location(character, wait);
		break;
	case _font_character_state_reading_location:
		font_character_location_wait(character, wait);
		break;
	case _font_character_state_location_read:
		font_character_read_header(character, wait);
		break;
	case _font_character_state_reading_header:
		font_character_header_wait(character, wait);
		break;
	}

	return character->state==_font_character_state_ready;
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
	new_character->location_loaded= FALSE;
	new_character->header_loaded= FALSE;
	new_character->state= _font_character_state_new;
	new_character->task= NONE;
	new_character->location= NONE;
	new_character->block_index= NONE;
	new_character->key.font_slot_index= font_slot_index;
	new_character->frame_index= font_cache_globals.frame_index;
	new_character->glyph_slot_index= NONE;
	hash_table_add(font_cache_globals.character_table, (long)&new_character->key, &index);

	return index;
}

/* 0x001407d0: a character of a font that is ready: what the file of the
font says of it has been read. With the first flag the game waits for it;
with the second, a character that is not known is made known. Its index;
NONE if it is not ready. It is told that the character was used in this
frame */
long font_character_request(
	long font_index,
	long character,
	unsigned long flags)
{
	long font_slot_index= font_slot_indices[font_index];
	long character_index= font_character_find(font_slot_index, character);
	boolean wait= flags&FLAG(_font_character_request_wait_bit);
	boolean ready;

	if (character_index==NONE)
	{
		if (!((flags>>_font_character_request_new_bit)&1))
		{
			return NONE;
		}
		character_index= font_character_new(font_slot_index, character);
		if (character_index==NONE)
		{
			return NONE;
		}
	}
	ready= font_character_get(character_index)->state==_font_character_state_ready;
	if (!ready)
	{
		do
		{
			ready= font_character_update(character_index, wait);
		} while (wait && !ready);
	}
	font_character_get(character_index)->frame_index= font_cache_globals.frame_index;

	return ready ? character_index : NONE;
}

/* 0x00140870: the pixels of a character are in the cache, or are on their
way if the game is not to wait for them: with the first flag it does. Whether
they are there. The block is told that it was used now */
boolean font_character_pixels_request(
	long character_index,
	unsigned long flags)
{
	struct font_character *character;
	struct lruv_cache *cache;
	byte *pixels;

	/* of the flags only the first is left, where the flags are */
	flags= (flags&~0xFFUL) | (flags&FLAG(_font_character_request_wait_bit));
	if (character_index==NONE)
	{
		return FALSE;
	}
	character= font_character_get(character_index);
	if (character->block_index==NONE)
	{
		font_character_pixels_new(character_index, (boolean)flags);
	}
	if (character->block_index==NONE)
	{
		return FALSE;
	}
	pixels= font_cache_block_wait(character->block_index, (boolean)flags);
	cache= font_cache_globals.cache;
	((struct lruv_cache_block *)cache->blocks->data)[character->block_index&0xFFFF].last_used_tick= cache->tick;

	return pixels!=NULL;
}

/* 0x00140900: what the file of a font says of a character, once it and its
pixels are there; NULL if they are not */
struct font_character_header *font_character_get_header(
	long character,
	long font_index,
	unsigned long flags)
{
	long character_index= font_character_request(font_index, character, flags);
	struct font_character *known;

	if (character_index==NONE || !font_character_pixels_request(character_index, flags))
	{
		return NULL;
	}
	known= font_character_get(character_index);

	return known->state==_font_character_state_ready ? &known->header : NULL;
}

/* the next thing of an array that is in use, from a place on; NONE if there
is none */
static inline __attribute__((always_inline)) long font_cache_data_next(
	const struct data_array *data,
	long absolute_index)
{
	if (absolute_index<0)
	{
		return NONE;
	}
	for (; absolute_index<data->first_unallocated; absolute_index++)
	{
		if (data->in_use_bit_vector[absolute_index>>5]&(1UL<<(absolute_index&31)))
		{
			return absolute_index;
		}
	}

	return NONE;
}

/* 0x001406b0: a frame goes by: the cache gets older, and each of the
characters that were asked for and were not there is asked for again; one
that is there now, with its pixels, is no longer waited for */
void font_cache_frame(
	void)
{
	struct data_array *predicted;
	long absolute_index= NONE;

	lruv_idle(font_cache_globals.cache);
	font_cache_globals.frame_index++;
	predicted= font_cache_globals.predicted_characters;
	while ((absolute_index= font_cache_data_next(predicted, absolute_index+1))!=NONE)
	{
		const struct font_predicted_character *wanted=
			(const struct font_predicted_character *)((const byte *)predicted->data+predicted->size*absolute_index);
		long predicted_index= ((long)wanted->identifier<<16)|absolute_index;
		long character_index= font_character_request(wanted->font_index, wanted->character,
			FLAG(_font_character_request_new_bit));

		if (character_index!=NONE &&
			font_character_pixels_request(character_index, FLAG(_font_character_request_new_bit)) &&
			font_character_get(character_index)->state==_font_character_state_ready)
		{
			datum_delete(font_cache_globals.predicted_characters, predicted_index);
		}
	}
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
	font_cache_globals.predicted_characters= font_cache_data_new("predicted data", MAXIMUM_PREDICTED_FONT_CHARACTERS,
		sizeof(struct font_predicted_character));
	font_cache_globals.character_table= hash_table_new(heap_allocation, MAXIMUM_FONT_CHARACTERS,
		"font character hash table", sizeof(long), FONT_CHARACTER_TABLE_LIST_COUNT, font_character_key_hash,
		font_character_key_compare);
	font_cache_data_make_valid(font_cache_globals.characters);
	font_cache_data_make_valid(font_cache_globals.predicted_characters);
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
	if (font_cache_globals.predicted_characters)
	{
		data_dispose(font_cache_globals.predicted_characters);
		font_cache_globals.predicted_characters= NULL;
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

/* 0x001414d0: whether a character of a font and its pixels are there to be
drawn. If they are not, they are asked for, with no wait, and the character
is noted, if it was not, to be asked for again each frame until it is there */
boolean font_character_precache(
	long font_index,
	long character)
{
	struct data_array *predicted;
	long absolute_index= NONE;
	long predicted_index;

	if (font_character_get_header(character, font_index, FLAG(_font_character_request_new_bit)))
	{
		return TRUE;
	}
	predicted= font_cache_globals.predicted_characters;
	while ((absolute_index= font_cache_data_next(predicted, absolute_index+1))!=NONE)
	{
		const struct font_predicted_character *wanted=
			(const struct font_predicted_character *)((const byte *)predicted->data+predicted->size*absolute_index);

		if (!wanted)
		{
			break;
		}
		if (wanted->font_index==font_index && wanted->character==character)
		{
			return FALSE;
		}
	}
	predicted_index= datum_new(font_cache_globals.predicted_characters);
	if (predicted_index!=NONE)
	{
		struct font_predicted_character *wanted=
			(struct font_predicted_character *)font_cache_globals.predicted_characters->data + (predicted_index&0xFFFF);

		wanted->font_index= font_index;
		wanted->character= character;
	}

	return FALSE;
}

/* where the pixels of a block of the cache are */
static inline __attribute__((always_inline)) byte *font_cache_block_address(
	long block_index)
{
	const struct lruv_cache *cache= font_cache_globals.cache;
	const struct lruv_cache_block *blocks= cache->blocks->data;

	return font_cache_globals.base_address +
		(blocks[block_index&0xFFFF].first_page_index<<(cache->page_size_bits&31));
}

/* 0x00140960: the pixels of a character, for which the game waits. Whether
they are there */
boolean font_character_get_pixels(
	long character_index,
	word *pixels)
{
	boolean loaded= font_character_pixels_request(character_index,
		FLAG(_font_character_request_wait_bit)|FLAG(_font_character_request_new_bit));

	if (loaded)
	{
		const struct font_character *character= font_character_get(character_index);

		font_character_decode_pixels(font_cache_block_address(character->block_index), character->header.pixels_size,
			pixels);
	}

	return loaded;
}

/* 0x001412c0: where the pixels of a block of the cache are, once they have
been read: the game waits for them if it is to, hurrying the task that reads
them. NULL if they are not there */
byte *font_cache_block_wait(
	long block_index,
	boolean wait)
{
	struct font_cache_block *block;

	if (block_index==NONE)
	{
		return NULL;
	}
	block= (struct font_cache_block *)font_cache_globals.blocks->data + (block_index&0xFFFF);
	if (!block->loaded && wait)
	{
		async_task_set_priority(block->task, FONT_CACHE_PIXELS_PRIORITY);
		while (!*(volatile boolean *)&block->loaded)
		{
			SwitchToThread();
		}
	}
	if (!block->loaded)
	{
		return NULL;
	}

	return font_cache_block_address(block_index);
}

/* 0x00141350: room in the cache for the pixels of a character that is ready,
and a task that reads them from the file of its font: the game waits for it
if it is to. Whether the character has a block: FALSE if it is not ready, or
if there is no room */
boolean font_character_pixels_new(
	long character_index,
	boolean wait)
{
	struct font_character *character= font_character_get(character_index);
	struct font_cache_block *block;
	long block_index;
	long size;
	byte *pixels;

	if (character->block_index!=NONE)
	{
		return TRUE;
	}
	if (character->state!=_font_character_state_ready)
	{
		return FALSE;
	}
	size= character->header.pixels_size;
	block_index= lruv_block_new(font_cache_globals.cache, size, 1);
	if (block_index==NONE)
	{
		return FALSE;
	}
	block_index= datum_new_at_index(font_cache_globals.blocks, block_index);
	if (block_index==NONE)
	{
		return FALSE;
	}
	block= (struct font_cache_block *)font_cache_globals.blocks->data + (block_index&0xFFFF);
	pixels= font_cache_block_get_address(block_index);
	character->block_index= block_index;
	block->character_index= character_index;
	block->task= font_cache_read(character->key.font_slot_index, &block->loaded, pixels, size,
		character->header.pixels_offset, font_cache_read_priority(wait), NULL);
	if (wait)
	{
		while (!*(volatile boolean *)&block->loaded)
		{
			SwitchToThread();
		}
	}

	return TRUE;
}

/* 0x00141480: a task that reads from the file of the font of a place */
long font_cache_read(
	long font_slot_index,
	boolean *done,
	void *buffer,
	long size,
	uint32_t offset,
	long priority,
	long *bytes_read)
{
	return async_read_position(font_cache_get_font_file(font_slot_index), size, offset, FONT_CACHE_TASK_CATEGORY,
		priority, bytes_read, buffer, done);
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
