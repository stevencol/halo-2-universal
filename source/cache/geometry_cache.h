/*
GEOMETRY_CACHE.H

The cache of the geometry that is drawn: the vertices and the triangles of
the models and of the levels, which the map has apart from the tags and are
read when they are about to be drawn. Worked out from halo2ship.exe; Halo 1
has no such cache, and the names are guesses after those of its texture
cache.
*/

#ifndef __GEOMETRY_CACHE_H
#define __GEOMETRY_CACHE_H

#include "cseries/cseries.h"
#include "geometry/geometry_definitions.h"
#include "memory/data.h"
#include "memory/lruv_cache.h"

/* ---------- constants */

enum
{
	MAXIMUM_CACHED_GEOMETRY_BLOCKS= 0x200,
	MAXIMUM_PREDICTED_GEOMETRY_BLOCKS= 350,

	/* the cache's memory is given out in pages of 4 KB */
	GEOMETRY_CACHE_PAGE_SIZE_BITS= 12,
	GEOMETRY_CACHE_PAGE_SIZE= 1<<GEOMETRY_CACHE_PAGE_SIZE_BITS,
	GEOMETRY_CACHE_SIZE= 0x700000,
	GEOMETRY_CACHE_SMALL_SIZE= 0x680000,

	/* of the reads from the map */
	GEOMETRY_CACHE_READ_CATEGORY= 4,
	/* the priorities of the tasks that read: the game waits, the geometry
	is wanted soon, it is wanted, it can wait */
	GEOMETRY_CACHE_BLOCKING_TASK_PRIORITY= 7,
	GEOMETRY_CACHE_SOON_TASK_PRIORITY= 5,
	GEOMETRY_CACHE_PREDICTED_TASK_PRIORITY= 2,
	GEOMETRY_CACHE_LATER_TASK_PRIORITY= 1,
	/* the blocks used in the last so many ticks are not thrown out for new
	geometry: fewer are kept when the game waits */
	GEOMETRY_CACHE_BLOCKING_AGE= 1,
	GEOMETRY_CACHE_PREDICTED_AGE= 5,
	/* a block used in the last so many ticks can still be drawn by the console */
	GEOMETRY_CACHE_BUSY_AGE= 5,

	/* how many are read at a time, and how many tasks there can be for the
	cache to start another */
	MAXIMUM_GEOMETRY_CACHE_READS= 5,
	MAXIMUM_GEOMETRY_CACHE_ASYNC_TASKS= 25,

	NUMBER_OF_GEOMETRY_CACHE_CONFIGURATIONS= 6
};

/* the flags that geometry is asked for with */
enum
{
	/* wait for it */
	_geometry_cache_block_bit,
	/* read it if it is not there */
	_geometry_cache_load_bit,
	/* about to be drawn: nothing depends on it yet */
	_geometry_cache_predicted_bit,
	_geometry_cache_soon_bit
};

/* the flags of a geometry_block_info that are the cache's */
enum
{
	/* in the list of those that are about to be drawn */
	_geometry_block_predicted_bit= 2
};

/* ---------- structures */

/* a block of the cache: its index is the index of the lruv_cache's block */
struct geometry_cache_block
{
	short identifier;
	/* set by the task that reads it */
	volatile boolean read_done;
	byte read_priority;
	long read_task_id;
	struct geometry_block_info *info;
};

/* geometry that is about to be drawn, and is not asked for yet */
struct predicted_geometry
{
	short identifier;
	short pad;
	struct geometry_block_info *info;
};

/* 0x004e648c */
struct geometry_cache_globals
{
	struct data_array *blocks;
	struct data_array *predicted_geometry;
	byte *base_address;
	long size;
	struct lruv_cache *cache;
	/* while it is not 0, the game waits for the geometry (if it is 1), or
	reads it at once without waiting: all of it, or that of the levels */
	long blocking_count;
	long blocking_usage;
	/* the times that geometry of a level was not there to draw */
	long structure_miss_count;
	/* GetTickCount: of what unknown_0012e480 last did */
	unsigned long unknown20;
	/* geometry was asked for that there was no room for */
	boolean missed;
};

#if defined(__i386__)
typedef char verify_geometry_cache_block_size[
	sizeof(struct geometry_cache_block) == 0xC ? 1 : -1];
typedef char verify_predicted_geometry_size[
	sizeof(struct predicted_geometry) == 8 ? 1 : -1];
typedef char verify_geometry_cache_globals_size[
	sizeof(struct geometry_cache_globals) == 0x28 ? 1 : -1];
#endif

/* ---------- prototypes/XBOX_GEOMETRY_CACHE.C */

#define GEOMETRY_CACHE_CALLBACK __attribute__((stdcall))

void geometry_cache_new(void);
void geometry_cache_delete(void);
void GEOMETRY_CACHE_CALLBACK geometry_cache_allocate_memory(long configuration);
void geometry_cache_free_memory(void);
void geometry_cache_open(void);
void geometry_cache_close(void);
void geometry_cache_flush(void);
void geometry_cache_idle(void);

long geometry_cache_reading_count(void);
void geometry_cache_update_predicted(void);
boolean geometry_cache_block_predict(struct geometry_block_info *info);
void geometry_cache_block_delete(struct geometry_block_info *info);
boolean geometry_cache_block_load(struct geometry_block_info *info, unsigned long flags);
void geometry_cache_delete_tag_blocks(long tag_index);
boolean geometry_cache_start_loading(boolean block, struct geometry_block_info *info, boolean unknown,
	long priority);

boolean GEOMETRY_CACHE_CALLBACK geometry_cache_block_locked(long block_index);
void GEOMETRY_CACHE_CALLBACK geometry_cache_block_deleted(long block_index);

/* ---------- prototypes of functions not decompiled yet */

/* 0x00223c70: the geometry of a block was read: the fields of its tag are
set to where it is. The name is a guess */
void geometry_block_attach_data(void *data, struct geometry_block_info *info);

/* 0x00223d80: whether the console still draws with the geometry of a block.
The name is a guess */
boolean geometry_block_busy(void *data, const struct geometry_block_info *info);

/* 0x00223e00: waits until it does not. The name is a guess */
void geometry_block_wait(void *data, const struct geometry_block_info *info);

/* 0x0012e480: after the cache's work of each frame; it tells the player
when geometry is missing, most likely */
void unknown_0012e480(void);

/* 0x002bcc80: what the cache gives the lruv_cache as its third function */
void unknown_002bcc80(void);

/* ---------- globals */

extern struct geometry_cache_globals geometry_cache_globals;
extern boolean geometry_cache_never_block;
extern unsigned long geometry_cache_warning_time;

#endif
