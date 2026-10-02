/*
ANIMATION_CACHE.H

The cache of the data of the animations: the map has it apart from the tags,
and it is read from the disc when an animation is about to be used. Worked
out from halo2ship.exe; Halo 1 has no such cache, and the names are guesses
after those of its sound cache.
*/

#ifndef __ANIMATION_CACHE_H
#define __ANIMATION_CACHE_H

#include "cseries/cseries.h"
#include "memory/data.h"
#include "memory/lruv_cache.h"

/* ---------- constants */

enum
{
	MAXIMUM_CACHED_ANIMATIONS= 0x200,
	MAXIMUM_PREDICTED_ANIMATIONS= 0x80,

	/* the cache's memory is given out in pages of 2 KB */
	ANIMATION_CACHE_PAGE_SIZE_BITS= 11,
	ANIMATION_CACHE_SIZE= 0x400000,
	ANIMATION_CACHE_SIZE_SPLIT_SCREEN= 0x300000,

	/* of the reads from the map */
	ANIMATION_CACHE_READ_CATEGORY= 6,
	/* how many are read at a time, and how many tasks there can be for the
	cache to start another */
	MAXIMUM_ANIMATION_CACHE_READS= 8,
	MAXIMUM_ANIMATION_CACHE_ASYNC_TASKS= 25
};

/* how soon the data of an animation is wanted */
enum
{
	/* now: the game waits for it */
	_animation_cache_priority_blocking,
	_animation_cache_priority_predicted_soon,
	_animation_cache_priority_predicted,
	NUMBER_OF_ANIMATION_CACHE_PRIORITIES
};

/* ---------- structures */

/* in a tag: where in the map the data of some animations is, and its block
of the cache */
struct cached_animation
{
	long unknown0;
	long size;
	/* as cache_file_read_async takes it */
	unsigned long offset;
	/* NONE if it is not in the cache */
	long cache_block_index;
};

/* a block of the cache: its index is the index of the lruv_cache's block */
struct animation_cache_block
{
	short identifier;
	short pad;
	/* set by the task that reads it */
	volatile boolean read_done;
	/* NONE when nobody has to wait for the read */
	long read_task_id;
	struct cached_animation *animation;
};

/* an animation that some code said it was about to use, and is not in the
cache yet */
struct predicted_animation
{
	short identifier;
	short pad;
	long priority;
	struct cached_animation *animation;
};

struct animation_cache_priority
{
	/* of the task that reads */
	long task_priority;
	/* a block read with this priority counts as used this many ticks ago */
	long age;
	/* blocks used in the last so many ticks are not thrown out for it */
	long minimum_age;
};

/* 0x004e3b48 */
struct animation_cache_globals
{
	struct data_array *blocks;
	struct data_array *predicted_animations;
	byte *base_address;
	struct lruv_cache *cache;
	long size;
	long page_count;
};

typedef char verify_cached_animation_size[
	sizeof(struct cached_animation) == 0x10 ? 1 : -1];
typedef char verify_animation_cache_priority_size[
	sizeof(struct animation_cache_priority) == 0xC ? 1 : -1];
#if defined(__i386__)
typedef char verify_animation_cache_block_size[
	sizeof(struct animation_cache_block) == 0x10 ? 1 : -1];
typedef char verify_predicted_animation_size[
	sizeof(struct predicted_animation) == 0xC ? 1 : -1];
typedef char verify_animation_cache_globals_size[
	sizeof(struct animation_cache_globals) == 0x18 ? 1 : -1];
#endif

/* ---------- prototypes/XBOX_ANIMATION_CACHE.C */

#define ANIMATION_CACHE_CALLBACK __attribute__((stdcall))

void animation_cache_new(void);
void ANIMATION_CACHE_CALLBACK animation_cache_allocate_memory(long player_count);
void animation_cache_free_memory(void);
void animation_cache_delete(void);
void animation_cache_open(void);
void animation_cache_close(void);
void animation_cache_idle(void);

long animation_cache_block_new(struct cached_animation *animation, boolean force, long minimum_age);
struct animation_cache_block *animation_cache_block_request(long priority, struct cached_animation *animation);
void animation_cache_block_read(struct animation_cache_block *block, long unknown, long priority);
boolean animation_cache_block_wait(struct animation_cache_block *block);
boolean ANIMATION_CACHE_CALLBACK animation_cache_block_locked(long block_index);
void ANIMATION_CACHE_CALLBACK animation_cache_block_deleted(long block_index);

long animation_cache_find_predicted(const struct cached_animation *animation);
void animation_cache_predict(struct cached_animation *animation, boolean soon);
void *animation_cache_get_data(struct cached_animation *animation, long unknown);
long animation_cache_reading_count(void);
void animation_cache_update_predicted(void);

/* ---------- globals */

extern struct animation_cache_globals animation_cache_globals;
extern boolean animation_cache_initialized;
extern boolean animation_cache_opened;
extern unsigned long animation_cache_warning_time;
extern long animation_cache_idle_count;
extern const struct animation_cache_priority animation_cache_priorities[NUMBER_OF_ANIMATION_CACHE_PRIORITIES];

#endif
