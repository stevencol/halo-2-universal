/*
FONT_CACHE.H

The characters of the fonts that the game has ready to draw: what it knows of
each, and the cache of their pixels. The names are guesses.
*/

#ifndef __FONT_CACHE_H
#define __FONT_CACHE_H

#include "cseries/cseries.h"
#include "memory/data.h"
#include "memory/hash_table.h"
#include "memory/lruv_cache.h"
#include "text/unicode.h"

/* ---------- structures */

/* what a character is found by: the key of the table of the characters */
struct font_character_key
{
	long font_slot_index;
	long character;
};

/* 0x0054d570: the characters that the game knows, and the cache of their
pixels */
struct font_cache_globals
{
	/* finds the index of a character by its key */
	struct hash_table *character_table;
	struct data_array *characters;
	struct data_array *unknown8;
	/* the blocks of the cache */
	struct data_array *blocks;
	struct lruv_cache *cache;
	/* where the pages of the cache are */
	byte *base_address;
};

typedef char verify_font_cache_globals_size[
	sizeof(struct font_cache_globals) == 0x18 ? 1 : -1];

/* ---------- prototypes/FONT_CACHE.C */

unsigned long HASH_TABLE_CALLBACK font_character_key_hash(long key);
boolean HASH_TABLE_CALLBACK font_character_key_compare(long key, long other);
long font_character_find(long font_slot_index, long character);
void font_characters_dispose(void);
void font_cache_dispose(void);
byte *font_cache_block_get_address(long block_index);

/* ---------- globals */

extern struct font_cache_globals font_cache_globals;

#endif
