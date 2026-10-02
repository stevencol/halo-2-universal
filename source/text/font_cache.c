/*
FONT_CACHE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The characters of the fonts that the game has ready to draw. The names of the
file and of the functions are guesses.
*/

#include "font_cache.h"

#include <string.h>

/* ---------- globals */

/* 0x0054d570 */
struct font_cache_globals font_cache_globals;

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
