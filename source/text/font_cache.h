/*
FONT_CACHE.H

The characters of the fonts that the game has ready to draw: what it knows of
each, and the cache of their pixels. The names are guesses.
*/

#ifndef __FONT_CACHE_H
#define __FONT_CACHE_H

#include "cseries/cseries.h"

#include <stdint.h>

#include "memory/data.h"
#include "memory/hash_table.h"
#include "memory/lruv_cache.h"
#include "text/unicode.h"

/* ---------- constants */

enum
{
	NUMBER_OF_FONT_GLYPH_SLOTS= 512,

	/* in the file of a font: a table with where each character is, and
	what the file says of each, 16 bytes */
	FONT_FILE_LOCATIONS_OFFSET= 0x400,
	FONT_FILE_HEADERS_OFFSET= 0x4040,
	MAXIMUM_FONT_CHARACTER_WIDTH= 128,
	MAXIMUM_FONT_CHARACTER_PIXELS_SIZE= 0x1000,
	FONT_CACHE_TASK_CATEGORY= 7,
	FONT_CACHE_READ_PRIORITY= 2,
	FONT_CACHE_READ_PRIORITY_WAIT= 6,
	/* what the task that reads pixels gets when the game waits for it */
	FONT_CACHE_PIXELS_PRIORITY= 7,

	/* how a character is asked for: the game waits for it, and it is made
	known if it is not */
	_font_character_request_wait_bit= 0,
	_font_character_request_new_bit= 1,

	MAXIMUM_FONT_CHARACTERS= 512,
	MAXIMUM_PREDICTED_FONT_CHARACTERS= 256,
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
	/* nothing is known of it */
	_font_character_state_new,
	/* a task reads where it is in the file of its font */
	_font_character_state_reading_location,
	_font_character_state_location_read,
	/* a task reads what the file says of it */
	_font_character_state_reading_header,
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

/* what the file of a font says of a character */
struct font_character_header
{
	/* how far the character after it is */
	short advance;
	/* how many bytes its pixels take in the file */
	word pixels_size;
	short width;
	short height;
	/* where its pixels are from where it is drawn: to the right, and up */
	short origin_x;
	short origin_y;
	/* where its pixels are in the file */
	uint32_t pixels_offset;
};

/* what the game knows of a character of a font */
struct font_character
{
	short identifier;
	word pad2;
	struct font_character_key key;
	/* the tasks that read these two are done */
	boolean location_loaded;
	boolean header_loaded;
	byte padE[2];
	long state;
	/* the task that reads */
	long task;
	/* where the character is in the file of its font */
	long location;
	struct font_character_header header;
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
	/* the task that reads them */
	long task;
};

/* a character that was asked for and was not there: it is asked for again
each frame */
struct font_predicted_character
{
	short identifier;
	word pad2;
	long font_index;
	long character;
};

struct font_glyph_slot
{
	/* NONE if there is none */
	long character_index;
	/* where its pixels are in the bitmap of the pictures */
	short x0;
	short y0;
};

typedef char verify_font_character_size[
	sizeof(struct font_character) == 0x38 ? 1 : -1];
typedef char verify_font_predicted_character_size[
	sizeof(struct font_predicted_character) == 0xC ? 1 : -1];
typedef char verify_font_cache_block_size[
	sizeof(struct font_cache_block) == 0xC ? 1 : -1];

/* 0x0054d570: the characters that the game knows, and the cache of their
pixels */
struct font_cache_globals
{
	/* finds the index of a character by its key */
	struct hash_table *character_table;
	struct data_array *characters;
	/* the characters that were asked for and were not there */
	struct data_array *predicted_characters;
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
void font_character_read_location(struct font_character *character, boolean wait);
void font_character_header_validate(struct font_character_header *header);
void font_character_location_wait(struct font_character *character, boolean wait);
void font_character_read_header(struct font_character *character, boolean wait);
void font_character_header_wait(struct font_character *character, boolean wait);
boolean font_character_update(long character_index, boolean wait);
long font_character_new(long font_slot_index, long character);
long font_character_request(long font_index, long character, unsigned long flags);
boolean font_character_pixels_request(long character_index, unsigned long flags);
struct font_character_header *font_character_get_header(long character, long font_index, unsigned long flags);
boolean font_character_get_pixels(long character_index, word *pixels);
byte *font_cache_block_wait(long block_index, boolean wait);
boolean font_character_pixels_new(long character_index, boolean wait);
long font_cache_read(long font_slot_index, boolean *done, void *buffer, long size, uint32_t offset, long priority,
	long *bytes_read);
void font_character_delete(long character_index);
boolean font_character_is_busy(long character_index);
void font_cache_frame(void);
boolean font_character_precache(long font_index, long character);
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

/* ---------- inline */

/* what the game knows of a character, by its index */
static inline __attribute__((always_inline)) struct font_character *font_character_get(
	long character_index)
{
	return (struct font_character *)font_cache_globals.characters->data + (character_index&0xFFFF);
}

#endif
