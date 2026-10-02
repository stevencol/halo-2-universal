/*
TEXTURE_CACHE.H

The cache of the pixels of the bitmaps: they are read from the map into its
memory when a bitmap is about to be drawn. Worked out from halo2ship.exe; the
names are those of Halo 1's texture_cache.h where it has them, and guesses
where it does not.
*/

#ifndef __TEXTURE_CACHE_H
#define __TEXTURE_CACHE_H

#include "cseries/cseries.h"
#include "bitmaps/bitmap_definitions.h"
#include "math/real_math.h"
#include "memory/data.h"
#include "memory/lruv_cache.h"

/* ---------- constants */

enum
{
	MAXIMUM_CACHED_TEXTURES= 0x200,
	MAXIMUM_PREDICTED_TEXTURES= 200,

	/* the cache's memory is given out in pages of 4 KB */
	TEXTURE_CACHE_PAGE_SIZE_BITS= 12,
	TEXTURE_CACHE_PAGE_SIZE= 1<<TEXTURE_CACHE_PAGE_SIZE_BITS,

	/* a bitmap has its pixels at up to three levels of detail in the cache.
	The fields of a bitmap for the fourth to the sixth are the cache's: the
	blocks of the three, and their addresses */
	NUMBER_OF_TEXTURE_CACHE_LEVELS= 3,

	/* the frame count starts here: a bitmap's is 0 when it was never drawn */
	TEXTURE_CACHE_FIRST_FRAME_INDEX= 4,

	/* what a block of memory that the cache lends is marked with */
	TEXTURE_CACHE_STOLEN_MEMORY_SIGNATURE= 0x02281972
};

/* the flags of a bitmap that are the cache's */
enum
{
	/* the pixels are in the map, for the cache to read */
	_bitmap_cached_bit= 9,
	/* in the list of those that are about to be drawn */
	_bitmap_predicted_bit= 10
};

/* the flags of a block */
enum
{
	/* memory that the cache lent: not pixels */
	_texture_cache_block_stolen_bit,
	/* stays until the cache is emptied */
	_texture_cache_block_required_bit
};

/* what texture_cache_block_type tells of a block */
enum
{
	_texture_cache_block_type_first_level= 0,
	_texture_cache_block_type_stolen= 0x10,
	_texture_cache_block_type_other_level= 0x20
};

/* ---------- structures */

/* Direct3D's texture: what the console is given to draw with */
struct d3d_texture
{
	unsigned long common;
	unsigned long data;
	unsigned long lock;
	unsigned long format;
	unsigned long size;
};

/* a block of the cache: its index is the index of the lruv_cache's block */
struct texture_cache_block
{
	short identifier;
	byte flags;
	/* set by the task that reads the pixels */
	volatile boolean read_done;
	long level_index;
	long read_priority;
	/* NONE: nothing was read into it */
	long read_task_id;
	struct bitmap_data *bitmap;
	struct d3d_texture texture;
};

/* a bitmap that is about to be drawn, and whose pixels are not asked for yet */
struct predicted_texture
{
	short identifier;
	short pad;
	struct bitmap_data *bitmap;
};

#define TEXTURE_CACHE_CALLBACK __attribute__((stdcall))

/* what the cache calls of who has memory of it: once a frame, and when it
wants the memory back; this one has to give it back */
typedef void (TEXTURE_CACHE_CALLBACK *texture_cache_stolen_memory_proc)(void *address, long user_data);

/* before each block of memory that the cache lends */
struct texture_cache_stolen_memory
{
	long block_index;
	/* the address, changed with the signature; 0 when it was given back */
	unsigned long signature;
	void *address;
	long size;
	long user_data;
	texture_cache_stolen_memory_proc idle_proc;
	texture_cache_stolen_memory_proc reclaim_proc;
	struct texture_cache_stolen_memory *previous;
	struct texture_cache_stolen_memory *next;
};

/* 0x004e6454 */
struct texture_cache_globals
{
	struct data_array *blocks;
	struct data_array *predicted_textures;
	struct texture_cache_stolen_memory *stolen_memory;
	byte *base_address;
	struct lruv_cache *cache;
	/* the map has pixels that are read when it is loaded, at the address */
	boolean unknown14;
	unsigned long unknown18;
	long unknown1C;
	long unknown20;
	byte unknown24;
	/* the cache is being emptied: no block has to stay */
	boolean flushing;
	/* a bitmap was drawn without its pixels since the bias was last changed */
	boolean missed;
	byte unknown27;
	/* how many levels of detail the bitmaps lose, for the cache not to fill */
	real mip_bias;
	/* GetTickCount: when the bias is next looked at */
	unsigned long mip_bias_time;
	long reads_this_frame;
	long frame_index;
};

#if defined(__i386__)
typedef char verify_texture_cache_block_size[
	sizeof(struct texture_cache_block) == 0x28 ? 1 : -1];
typedef char verify_predicted_texture_size[
	sizeof(struct predicted_texture) == 8 ? 1 : -1];
typedef char verify_texture_cache_stolen_memory_size[
	sizeof(struct texture_cache_stolen_memory) == 0x24 ? 1 : -1];
typedef char verify_texture_cache_globals_size[
	sizeof(struct texture_cache_globals) == 0x38 ? 1 : -1];
#endif

/* ---------- prototypes/XBOX_TEXTURE_CACHE.C */

void texture_cache_new(void);
void texture_cache_delete(void);
void texture_cache_open(void);
void texture_cache_close(void);
void texture_cache_flush(void);
void texture_cache_idle(void);

void texture_cache_update_mip_bias(void);
void texture_cache_stolen_memory_idle(void);
void texture_cache_reset_frame_index(void);
void texture_cache_tick(void);

void texture_cache_bitmap_delete(struct bitmap_data *bitmap);
boolean texture_cache_bitmap_predict(struct bitmap_data *bitmap);

byte TEXTURE_CACHE_CALLBACK texture_cache_block_type(long block_index);
boolean TEXTURE_CACHE_CALLBACK texture_cache_block_locked(long block_index);
void TEXTURE_CACHE_CALLBACK texture_cache_block_deleted(long block_index);

void *texture_cache_steal_memory_now(long size, long user_data, texture_cache_stolen_memory_proc idle_proc,
	texture_cache_stolen_memory_proc reclaim_proc);
void texture_cache_return_memory(void *address);

/* ---------- prototypes of functions not decompiled yet */

/* 0x0012d400: takes memory of the cache for something else, throwing out
what it has to; its address, or NULL. How long it tries: not at all, a
little or for long (0 to 2) */
void *texture_cache_steal_memory(long wait, long size, long user_data, texture_cache_stolen_memory_proc idle_proc,
	texture_cache_stolen_memory_proc reclaim_proc);

/* 0x0012c450: starts to read the bitmaps that are about to be drawn */
void unknown_0012c450(void);

/* ---------- globals */

extern struct texture_cache_globals texture_cache_globals;
extern boolean texture_cache_mip_bias_enabled;
extern unsigned long texture_cache_warning_time;

#endif
