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

	/* a level of detail other than the first has to be of more than this */
	TEXTURE_CACHE_MINIMUM_LEVEL_SIZE= 0x400,

	/* of the reads from the map */
	TEXTURE_CACHE_READ_CATEGORY= 3,
	/* the priorities of the tasks that read: the game waits, the pixels are
	wanted soon, they are wanted, they can wait */
	TEXTURE_CACHE_BLOCKING_TASK_PRIORITY= 7,
	TEXTURE_CACHE_SOON_TASK_PRIORITY= 5,
	TEXTURE_CACHE_PREDICTED_TASK_PRIORITY= 2,
	TEXTURE_CACHE_LATER_TASK_PRIORITY= 1,
	/* the blocks used in the last so many ticks are not thrown out for the
	pixels of a bitmap: fewer are kept when the game waits */
	TEXTURE_CACHE_BLOCKING_AGE= 2,
	TEXTURE_CACHE_PREDICTED_AGE= 5,

	/* nothing more is read for what can wait when there are this many tasks,
	or this many reads were started in the frame */
	MAXIMUM_TEXTURE_CACHE_ASYNC_TASKS= 25,
	MAXIMUM_TEXTURE_CACHE_READS_PER_FRAME= 10,
	MAXIMUM_TEXTURE_CACHE_PREDICTED_PER_FRAME= 5,

	/* a bitmap keeps the texture of its first level for this many frames */
	TEXTURE_CACHE_HARDWARE_FORMAT_FRAMES= 2,

	NUMBER_OF_BITMAP_FORMATS= 24,
	NUMBER_OF_TEXTURE_CACHE_DEFAULT_TEXTURES= 3,

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

/* the flags that a texture is asked for with */
enum
{
	/* wait for the pixels */
	_texture_cache_block_bit,
	/* read them if they are not there */
	_texture_cache_load_bit,
	/* the first level of detail and no other; nothing is read */
	_texture_cache_exact_bit,
	_texture_cache_soon_bit,
	_texture_cache_later_bit
};

/* what the fields of Direct3D's texture are made of */
enum
{
	D3D_TEXTURE_COMMON= 0x40001,
	D3D_TEXTURE_FORMAT= 9,
	D3D_TEXTURE_FORMAT_CUBE_MAP= 0xD,
	D3D_TEXTURE_FORMAT_LINEAR= 0x10029,
	D3D_TEXTURE_DATA_MASK= 0x0FFFFFFF,
	D3D_TEXTURE_PITCH_ALIGNMENT= 64,
	/* the formats of the bitmaps of 32 bits that have the flag */
	D3D_FORMAT_UNKNOWN5= 0x33,
	D3D_FORMAT_LINEAR_UNKNOWN5= 0x36
};

/* how long texture_cache_steal_memory tries */
enum
{
	_texture_cache_steal_wait_none,
	_texture_cache_steal_wait_short,
	_texture_cache_steal_wait_long
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
	boolean has_preloaded_pixels;
	unsigned long preloaded_pixels;
	/* while it is not 0, the game waits for the pixels of the bitmaps (if it
	is 1), or reads them at once without waiting: of all, or of those of
	the usage 3 */
	long blocking_count;
	long blocking_usage;
	byte unknown24;
	/* the cache is being emptied: no block has to stay */
	boolean flushing;
	/* a bitmap was drawn without its pixels since the bias was last changed */
	boolean missed;
	/* the bitmaps of some usages are drawn with the pixels read with the map */
	boolean use_preloaded_pixels;
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

void *texture_cache_steal_memory(long wait, long size, long user_data, texture_cache_stolen_memory_proc idle_proc,
	texture_cache_stolen_memory_proc reclaim_proc);

void texture_cache_load_preloaded_pixels(void);
void texture_cache_bitmap_initialize_preloaded_format(struct bitmap_data *bitmap);
struct d3d_texture *texture_cache_bitmap_get_preloaded_format(struct bitmap_data *bitmap);
struct d3d_texture *texture_cache_bitmap_get_level(long level_index, boolean load, long priority,
	struct bitmap_data *bitmap, boolean block, boolean unknown, boolean *waited, boolean *failed);
long texture_cache_bitmap_choose_level(const struct bitmap_data *bitmap, real mip_bias);
boolean texture_cache_bitmap_load(struct bitmap_data *bitmap);
boolean texture_cache_usage_is_preloaded(long usage);
struct d3d_texture *texture_cache_bitmap_get_loaded(unsigned long flags, struct bitmap_data *bitmap, real mip_bias);
struct d3d_texture *texture_cache_bitmap_get_hardware_format(struct bitmap_data *bitmap, unsigned long flags,
	real mip_bias);
void texture_cache_update_predicted(void);
boolean texture_cache_start_loading_bitmap(boolean block, struct bitmap_data *bitmap, long level_index,
	boolean unknown, long priority);
void texture_cache_initialize_hardware_format(const struct bitmap_data *bitmap, long level_index,
	struct d3d_texture *texture);

/* ---------- prototypes of functions not decompiled yet */

/* 0x000359e0: how many mipmaps a texture of a size has after the first
level, and no more than a maximum; 0 if a size is not a power of two, or if
told that it has none. The name is a guess */
short bitmap_mipmap_count(short width, short depth, short height, short format, boolean none, short maximum);

/* 0x001365a0: the bytes of a row of the pixels of a bitmap, at a mipmap.
The name is a guess */
long bitmap_row_size(byte mipmap_index, const struct bitmap_data *bitmap);

/* ---------- globals */

extern struct texture_cache_globals texture_cache_globals;
extern boolean texture_cache_mip_bias_enabled;
extern unsigned long texture_cache_warning_time;
extern boolean texture_cache_low_detail;
extern boolean texture_cache_full_detail;
extern struct d3d_texture *texture_cache_default_textures[NUMBER_OF_TEXTURE_CACHE_DEFAULT_TEXTURES];
extern const long texture_cache_d3d_formats[NUMBER_OF_BITMAP_FORMATS];
extern const long texture_cache_d3d_linear_formats[NUMBER_OF_BITMAP_FORMATS];

#endif
