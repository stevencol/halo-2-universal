/*
FONT_CACHE.H

The characters of the fonts that the game has ready to draw: what it knows of
each, and the cache of their pixels. The names are guesses.
*/

#ifndef __FONT_CACHE_H
#define __FONT_CACHE_H

#include "cseries/cseries.h"
#include "memory/data.h"
#include "memory/lruv_cache.h"
#include "text/unicode.h"

/* ---------- constants */

enum
{
	/* the characters that are of no writing: those of each program */
	FIRST_PRIVATE_USE_CHARACTER= 0xE000,
	LAST_PRIVATE_USE_CHARACTER= 0xF8FF
};

/* ---------- structures */

/* what finds a character of a font among those that the game knows: only
what the code here looks at */
struct font_character_table
{
	byte unknown0[0x34];
	struct allocation *allocation;
};

/* 0x0054d570: the characters that the game knows, and the cache of their
pixels */
struct font_cache_globals
{
	struct font_character_table *character_table;
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
typedef char verify_font_character_table_size[
	sizeof(struct font_character_table) == 0x38 ? 1 : -1];

/* ---------- prototypes/FONT_CACHE.C */

boolean character_is_private_use(unsigned long character);
void font_characters_dispose(void);
void font_cache_dispose(void);
byte *font_cache_block_get_address(long block_index);

/* ---------- globals */

extern struct font_cache_globals font_cache_globals;

#endif
