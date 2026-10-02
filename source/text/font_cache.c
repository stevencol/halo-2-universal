/*
FONT_CACHE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The characters of the fonts that the game has ready to draw. The names of the
file and of the functions are guesses.
*/

#include "font_cache.h"

/* ---------- globals */

/* 0x0054d570 */
struct font_cache_globals font_cache_globals;

/* ---------- code */

/* 0x00140420: whether a character is of no writing: one of those that each
program gives a meaning of its own. Only its 16 low bits are looked at */
boolean character_is_private_use(
	unsigned long character)
{
	return (wchar)character>=FIRST_PRIVATE_USE_CHARACTER && (wchar)character<=LAST_PRIVATE_USE_CHARACTER;
}

/* 0x00140aa0: the end of what the game knows of the characters */
void font_characters_dispose(
	void)
{
	if (font_cache_globals.character_table)
	{
		struct font_character_table *table= font_cache_globals.character_table;

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
