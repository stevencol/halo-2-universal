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

/* ---------- constants */

enum
{
	NUMBER_OF_FONT_GLYPH_SLOTS= 512,

	MAXIMUM_FONT_CHARACTERS= 512,
	MAXIMUM_PREDICTED_FONT_CHARACTERS= 256,
	FONT_PREDICTED_CHARACTER_SIZE= 0xC,
	FONT_CHARACTER_TABLE_LIST_COUNT= 1024,

	/* the cache of the pixels: its bytes, in pages of 16 */
	FONT_CACHE_SIZE= 0x7800,
	FONT_CACHE_PAGE_SIZE_BITS= 4,
	MAXIMUM_FONT_CACHE_BLOCKS= 512,
	/* the cache, the array of its blocks and the blocks */
	FONT_CACHE_LRUV_SIZE= 0x30FC
};

/* how far the game is with a character: the last is that it is ready to be
drawn */
enum
{
	_font_character_state_unknown0,
	_font_character_state_unknown1,
	_font_character_state_unknown2,
	_font_character_state_unknown3,
	_font_character_state_ready,
	NUMBER_OF_FONT_CHARACTER_STATES
};

/* ---------- structures */

/* what a character is found by: the key of the table of the characters */
struct font_character_key
{
	long font_slot_index;
	long character;
};

/* what the game knows of a character of a font */
struct font_character
{
	short identifier;
	word pad2;
	struct font_character_key key;
	boolean unknownC;
	boolean unknownD;
	byte padE[2];
	long state;
	long unknown14;
	long unknown18;
	byte unknown1C[0x10];
	/* of the cache: where its pixels are; NONE if they are not there */
	long block_index;
	/* the last frame that it was used in */
	long frame_index;
	/* its place among the pictures that are drawn; NONE if it has none */
	long glyph_slot_index;
};

/* what the game keeps of a block of the cache of the pixels */
struct font_cache_block
{
	short identifier;
	/* its pixels have been read: until then the block cannot leave the
	cache */
	boolean loaded;
	byte pad3;
	/* the character that the pixels are of */
	long character_index;
	byte unknown8[4];
};

struct font_glyph_slot
{
	/* NONE if there is none */
	long character_index;
	byte unknown4[4];
};

typedef char verify_font_character_size[
	sizeof(struct font_character) == 0x38 ? 1 : -1];
typedef char verify_font_cache_block_size[
	sizeof(struct font_cache_block) == 0xC ? 1 : -1];

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
	/* one more each frame */
	long frame_index;
};

typedef char verify_font_cache_globals_size[
	sizeof(struct font_cache_globals) == 0x1C ? 1 : -1];

/* ---------- prototypes/FONT_CACHE.C */

unsigned long HASH_TABLE_CALLBACK font_character_key_hash(long key);
boolean HASH_TABLE_CALLBACK font_character_key_compare(long key, long other);
long font_character_find(long font_slot_index, long character);
long font_character_new(long font_slot_index, long character);
void font_character_delete(long character_index);
boolean font_character_is_busy(long character_index);
void font_characters_new(void);
void font_characters_dispose(void);
void font_cache_new(void);
void LRUV_CALLBACK font_cache_block_deleted(long block_index);
boolean LRUV_CALLBACK font_cache_block_locked(long block_index);
void font_cache_dispose(void);
byte *font_cache_block_get_address(long block_index);

/* ---------- globals */

extern struct font_cache_globals font_cache_globals;
extern struct font_glyph_slot font_glyph_slots[NUMBER_OF_FONT_GLYPH_SLOTS];

#endif
