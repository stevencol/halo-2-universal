/*
XBOX_ANIMATION_CACHE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Halo 1 has no cache of animations; the names are guesses, after those of its
xbox_sound_cache.c.
*/

#include "animation_cache.h"

#include "cache/cache_files.h"
#include "cseries/async.h"
#include "memory/physical_memory.h"

/* ---------- prototypes of functions of the console's libraries */

/* 0x003314b0 */
long GetTickCount(void);

/* ---------- constants */

enum
{
	/* the priority that the task of a read is given when the game waits for it */
	ANIMATION_CACHE_WAIT_TASK_PRIORITY= 8,
	/* how many times a block that there has to be is looked for, with a tick
	of the cache going by each time */
	ANIMATION_CACHE_FORCED_BLOCK_TRIES= 3,
	/* the time kept when there is no room to tell of one more animation; in
	the debug game, most likely, between two warnings */
	ANIMATION_CACHE_WARNING_MILLISECONDS= 30000,

	ANIMATION_CACHE_LRUV_SIZE= sizeof(struct lruv_cache)+sizeof(struct data_array)+
		MAXIMUM_CACHED_ANIMATIONS*sizeof(struct lruv_cache_block)+
		BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CACHED_ANIMATIONS)*sizeof(unsigned long)
};

/* ---------- globals */

/* 0x004e3b48 */
struct animation_cache_globals animation_cache_globals;

/* 0x00510c20, 0x00510c21: the cache was made; a map is loaded */
boolean animation_cache_initialized;
boolean animation_cache_opened;

/* 0x0055e71c */
unsigned long animation_cache_warning_time;

/* 0x0055e720: the times that animation_cache_idle did its work */
long animation_cache_idle_count;

/* 0x00468810 */
const struct animation_cache_priority animation_cache_priorities[NUMBER_OF_ANIMATION_CACHE_PRIORITIES]=
{
	{ 8, 0, 0 },
	{ 5, 10, 30 },
	{ 2, 20, 50 }
};

/* ---------- code */

static inline __attribute__((always_inline)) struct animation_cache_block *animation_cache_block_get(
	long block_index)
{
	return &((struct animation_cache_block *)animation_cache_globals.blocks->data)[block_index&0xFFFF];
}

/* 0x00123180: makes the cache, with no memory yet */
void animation_cache_new(
	void)
{
	if (!animation_cache_initialized)
	{
		struct allocation *allocation;
		struct lruv_cache *cache;

		animation_cache_initialized= TRUE;
		animation_cache_globals.blocks= data_new("xbox animation", MAXIMUM_CACHED_ANIMATIONS,
			sizeof(struct animation_cache_block), 0, heap_allocation);
		animation_cache_globals.predicted_animations= data_new("xbox predicted animations",
			MAXIMUM_PREDICTED_ANIMATIONS, sizeof(struct predicted_animation), 0, heap_allocation);

		allocation= heap_allocation;
		cache= allocation->functions->allocate(allocation, ANIMATION_CACHE_LRUV_SIZE);
		lruv_initialize(cache, "xbox animation cache", 0, ANIMATION_CACHE_PAGE_SIZE_BITS, MAXIMUM_CACHED_ANIMATIONS,
			animation_cache_block_deleted, animation_cache_block_locked, NULL, allocation);
		animation_cache_globals.cache= cache;
		animation_cache_globals.page_count= 0;
		animation_cache_globals.size= 0;
		animation_cache_globals.base_address= NULL;
	}
}

/* 0x00123230: takes the cache's memory, when a map is loaded: less in all
but two of the ways that the memory is shared out (the geometry cache has six
of them). With 0x001232e0, in a table of those that take memory then
(0x00453c00) */
void ANIMATION_CACHE_CALLBACK animation_cache_allocate_memory(
	long configuration)
{
	if (animation_cache_globals.cache)
	{
		if (configuration>1)
		{
			animation_cache_globals.page_count= ANIMATION_CACHE_SIZE_SPLIT_SCREEN>>ANIMATION_CACHE_PAGE_SIZE_BITS;
			animation_cache_globals.size= ANIMATION_CACHE_SIZE_SPLIT_SCREEN;
		}
		else
		{
			animation_cache_globals.page_count= ANIMATION_CACHE_SIZE>>ANIMATION_CACHE_PAGE_SIZE_BITS;
			animation_cache_globals.size= ANIMATION_CACHE_SIZE;
		}
		animation_cache_globals.base_address= physical_memory_allocate_high((uint32_t)animation_cache_globals.size);
		lruv_resize(animation_cache_globals.cache, animation_cache_globals.page_count);
	}
}

/* 0x001232e0: the cache is left with no memory; who gave it takes it back */
void animation_cache_free_memory(
	void)
{
	animation_cache_globals.page_count= 0;
	animation_cache_globals.size= 0;
	if (animation_cache_globals.cache)
	{
		lruv_resize(animation_cache_globals.cache, 0);
		animation_cache_globals.base_address= NULL;
	}
}

/* 0x00123310 */
void animation_cache_delete(
	void)
{
	if (animation_cache_initialized)
	{
		if (animation_cache_globals.blocks)
		{
			data_dispose(animation_cache_globals.blocks);
			animation_cache_globals.blocks= NULL;
		}
		if (animation_cache_globals.predicted_animations)
		{
			data_dispose(animation_cache_globals.predicted_animations);
			animation_cache_globals.predicted_animations= NULL;
		}
		if (animation_cache_globals.cache)
		{
			struct lruv_cache *cache= animation_cache_globals.cache;

			cache->allocation->functions->deallocate(cache->allocation, cache);
			animation_cache_globals.cache= NULL;
		}
	}
	animation_cache_initialized= FALSE;
}

/* 0x001233a0: a map was loaded */
void animation_cache_open(
	void)
{
	if (animation_cache_initialized && !animation_cache_opened)
	{
		animation_cache_opened= TRUE;
		if (animation_cache_globals.blocks)
		{
			data_make_valid(animation_cache_globals.blocks);
		}
		if (animation_cache_globals.predicted_animations)
		{
			data_make_valid(animation_cache_globals.predicted_animations);
		}
	}
}

/* 0x001233f0: the map is going away */
void animation_cache_close(
	void)
{
	if (animation_cache_initialized && animation_cache_opened)
	{
		lruv_flush(animation_cache_globals.cache);
		if (animation_cache_globals.blocks)
		{
			animation_cache_globals.blocks->valid= FALSE;
		}
		if (animation_cache_globals.predicted_animations)
		{
			animation_cache_globals.predicted_animations->valid= FALSE;
		}
		animation_cache_opened= FALSE;
	}
}

/* 0x00123430: once a frame */
void animation_cache_idle(
	void)
{
	if (animation_cache_initialized && animation_cache_opened)
	{
		struct lruv_cache *cache= animation_cache_globals.cache;

		cache->hole_algorithm= _lruv_hole_algorithm_oldest;
		lruv_idle(cache);
		animation_cache_update_predicted();
		animation_cache_idle_count++;
	}
}

/* 0x001234a0: a block of the cache for the data of some animations; its
index, or NONE. If there has to be one, the blocks that are being read are
thrown out too, after waiting for them */
long animation_cache_block_new(
	struct cached_animation *animation,
	boolean force,
	long minimum_age)
{
	long block_index= lruv_block_new(animation_cache_globals.cache, animation->size, minimum_age);

	if (block_index==NONE && force)
	{
		struct lruv_cache *cache= animation_cache_globals.cache;
		long tries= ANIMATION_CACHE_FORCED_BLOCK_TRIES;

		cache->delete_block_proc= animation_cache_block_deleted;
		cache->locked_block_proc= NULL;
		do
		{
			lruv_idle(cache);
			block_index= lruv_block_new(cache, animation->size, minimum_age);
			tries--;
			cache= animation_cache_globals.cache;
		}
		while (block_index==NONE && tries>0);
		cache->delete_block_proc= animation_cache_block_deleted;
		cache->locked_block_proc= animation_cache_block_locked;
	}

	return block_index;
}

/* 0x00123550: a block for the data of some animations that are not in the
cache, if there is room; NULL if there is none */
struct animation_cache_block *animation_cache_block_request(
	long priority,
	struct cached_animation *animation)
{
	struct animation_cache_block *block= NULL;
	long block_index= lruv_block_new(animation_cache_globals.cache, animation->size,
		animation_cache_priorities[priority].minimum_age);

	if (block_index!=NONE)
	{
		block= animation_cache_block_get(datum_new_at_index(animation_cache_globals.blocks, block_index));
		block->read_done= FALSE;
		block->read_task_id= NONE;
		block->animation= animation;
		animation->cache_block_index= block_index;
	}

	return block;
}

/* 0x001235b0: starts to read the data of a block from the map, and waits for
it if it is wanted now. What the first argument is for is not known: the
function does not use it */
void animation_cache_block_read(
	struct animation_cache_block *block,
	long unknown,
	long priority)
{
	struct lruv_cache *cache= animation_cache_globals.cache;
	struct cached_animation *animation= block->animation;
	unsigned long size= (unsigned long)animation->size;
	void *address;

	(void)unknown;

	block->read_done= FALSE;
	address= animation_cache_globals.base_address+lruv_block_get_address(cache, animation->cache_block_index);
	if (size&(CACHE_FILE_SECTOR_SIZE-1))
	{
		size= (size|(CACHE_FILE_SECTOR_SIZE-1))+1;
	}
	block->read_done= FALSE;
	block->read_task_id= cache_file_read_async(animation->offset, (long)size, address, NULL,
		(uint8_t *)&block->read_done, ANIMATION_CACHE_READ_CATEGORY, animation_cache_priorities[priority].task_priority);
	if (priority==_animation_cache_priority_blocking)
	{
		animation_cache_block_wait(block);
	}

	cache= animation_cache_globals.cache;
	lruv_block_get(cache, block->animation->cache_block_index)->last_used_tick=
		cache->tick-animation_cache_priorities[priority].age;
}

/* 0x00123680: the index of an animation among those that are about to be
used; NONE if it is not there, and if it is in the cache already */
long animation_cache_find_predicted(
	const struct cached_animation *animation)
{
	if (animation->cache_block_index==NONE)
	{
		const struct data_array *data= animation_cache_globals.predicted_animations;
		long absolute_index;

		for (absolute_index= data_next_absolute_index(animation_cache_globals.predicted_animations, 0);
			absolute_index!=NONE;
			absolute_index= data_next_absolute_index(animation_cache_globals.predicted_animations, absolute_index+1))
		{
			const struct predicted_animation *predicted=
				(const struct predicted_animation *)((const byte *)data->data+data->size*absolute_index);

			if (predicted->animation==animation)
			{
				return (long)((unsigned long)(long)predicted->identifier<<16)|absolute_index;
			}
		}
	}

	return NONE;
}

/* 0x001236f0: tells the cache that an animation is about to be used. One
that is in the cache is kept there; one that is not is read when there is
time */
void animation_cache_predict(
	struct cached_animation *animation,
	boolean soon)
{
	if (animation_cache_opened)
	{
		long priority= soon ? _animation_cache_priority_predicted_soon : _animation_cache_priority_predicted;

		if (animation->cache_block_index==NONE)
		{
			if (animation_cache_find_predicted(animation)==NONE)
			{
				long index= datum_new(animation_cache_globals.predicted_animations);

				if (index!=NONE)
				{
					struct predicted_animation *predicted= &((struct predicted_animation *)
						animation_cache_globals.predicted_animations->data)[index&0xFFFF];

					predicted->priority= priority;
					predicted->animation= animation;
					animation_cache_update_predicted();
				}
				else if ((unsigned long)GetTickCount()>animation_cache_warning_time)
				{
					animation_cache_warning_time= (unsigned long)GetTickCount()+ANIMATION_CACHE_WARNING_MILLISECONDS;
				}
			}
		}
		else
		{
			struct lruv_cache *cache= animation_cache_globals.cache;
			struct lruv_cache_block *cache_block= lruv_block_get(cache, animation->cache_block_index);

			if (cache->tick-cache_block->last_used_tick>animation_cache_priorities[priority].age)
			{
				cache_block->last_used_tick= cache->tick-animation_cache_priorities[priority].age;
			}
		}
	}
}

/* 0x001237a0: waits for the read of a block to end; whether there was one to
wait for */
boolean animation_cache_block_wait(
	struct animation_cache_block *block)
{
	boolean waited= FALSE;

	if (block->read_task_id!=NONE)
	{
		if (!block->read_done)
		{
			async_task_set_priority(block->read_task_id, ANIMATION_CACHE_WAIT_TASK_PRIORITY);
			while (!block->read_done)
			{
				SwitchToThread();
			}
			waited= TRUE;
		}
		block->read_task_id= NONE;
	}

	return waited;
}

/* 0x001237e0: the data of some animations, read from the map if it is not in
the cache: the game waits for it. The second argument is given to
animation_cache_block_read */
void *animation_cache_get_data(
	struct cached_animation *animation,
	long unknown)
{
	long predicted_index= animation_cache_find_predicted(animation);
	struct lruv_cache *cache;

	if (predicted_index!=NONE)
	{
		datum_delete(animation_cache_globals.predicted_animations, predicted_index);
	}

	if (animation->cache_block_index==NONE)
	{
		long block_index= animation_cache_block_new(animation, TRUE, 0);
		struct animation_cache_block *block=
			animation_cache_block_get(datum_new_at_index(animation_cache_globals.blocks, block_index));

		block->read_task_id= NONE;
		block->read_done= FALSE;
		block->animation= animation;
		animation->cache_block_index= block_index;
		animation_cache_block_read(block, unknown, _animation_cache_priority_blocking);
	}
	else
	{
		animation_cache_block_wait(animation_cache_block_get(animation->cache_block_index));
	}

	cache= animation_cache_globals.cache;
	lruv_block_touch(cache, animation->cache_block_index);

	return animation_cache_globals.base_address+lruv_block_get_address(cache, animation->cache_block_index);
}

/* 0x001238b0: what the lruv_cache asks before it throws a block out: one
that is being read stays */
boolean ANIMATION_CACHE_CALLBACK animation_cache_block_locked(
	long block_index)
{
	const struct animation_cache_block *block= animation_cache_block_get(block_index);

	return block->read_task_id!=NONE && !block->read_done;
}

/* 0x001238f0: what the lruv_cache calls when it throws a block out */
void ANIMATION_CACHE_CALLBACK animation_cache_block_deleted(
	long block_index)
{
	struct animation_cache_block *block= animation_cache_block_get(block_index);

	/* as animation_cache_block_wait */
	if (block->read_task_id!=NONE)
	{
		if (!block->read_done)
		{
			async_task_set_priority(block->read_task_id, ANIMATION_CACHE_WAIT_TASK_PRIORITY);
			while (!block->read_done)
			{
				SwitchToThread();
			}
		}
		block->read_task_id= NONE;
	}
	block->animation->cache_block_index= NONE;
	datum_delete(animation_cache_globals.blocks, block_index);
}

/* 0x00123970: how many blocks are being read */
long animation_cache_reading_count(
	void)
{
	const struct data_array *data= animation_cache_globals.blocks;
	long count= 0;
	long absolute_index;

	for (absolute_index= data_next_absolute_index(animation_cache_globals.blocks, 0);
		absolute_index!=NONE;
		absolute_index= data_next_absolute_index(animation_cache_globals.blocks, absolute_index+1))
	{
		const struct animation_cache_block *block=
			(const struct animation_cache_block *)((const byte *)data->data+data->size*absolute_index);

		if (!block->read_done)
		{
			count++;
		}
	}

	return count;
}

/* 0x001239d0: starts to read the animations that are about to be used, a
few at a time: first those that are wanted soon. One that there is no room
for is left for another time */
void animation_cache_update_predicted(
	void)
{
	struct data_array *data= animation_cache_globals.predicted_animations;
	long reading_count;
	long absolute_index;
	long pass;

	if (!data->valid || async_globals.tasks_added>MAXIMUM_ANIMATION_CACHE_ASYNC_TASKS)
	{
		return;
	}

	reading_count= animation_cache_reading_count();
	for (pass= 0; pass<2; pass++)
	{
		for (absolute_index= data_next_absolute_index(data, 0);
			absolute_index!=NONE;
			absolute_index= data_next_absolute_index(data, absolute_index+1))
		{
			struct predicted_animation *predicted=
				(struct predicted_animation *)((byte *)data->data+data->size*absolute_index);
			long index= (long)((unsigned long)(long)predicted->identifier<<16)|absolute_index;
			long priority= predicted->priority;
			struct cached_animation *animation= predicted->animation;

			if (reading_count>=MAXIMUM_ANIMATION_CACHE_READS)
			{
				break;
			}
			if (pass==0 && priority!=_animation_cache_priority_predicted_soon)
			{
				continue;
			}
			if (!animation_cache_opened)
			{
				continue;
			}
			if (animation->cache_block_index==NONE)
			{
				struct animation_cache_block *block= animation_cache_block_request(priority, animation);

				if (!block)
				{
					continue;
				}
				animation_cache_block_read(block, 0x7000180, priority);
			}
			datum_delete(animation_cache_globals.predicted_animations, index);
			reading_count++;
		}
		data= animation_cache_globals.predicted_animations;
	}
}
