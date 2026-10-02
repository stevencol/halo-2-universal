/*
XBOX_GEOMETRY_CACHE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Halo 1 has no cache of geometry; the names are guesses, after those of its
xbox_texture_cache.c. Not here: unknown_0012e480, which is called each frame
after the cache's work.
*/

#include "geometry_cache.h"

#include "cache/cache_files.h"
#include "cseries/async.h"
#include "memory/physical_memory.h"
#include "saved games/game_state.h"
#include "shell/shell.h"
#include "structures/structure_bsp_definitions.h"

/* ---------- prototypes of functions of the console's libraries */

/* 0x003314b0 */
long GetTickCount(void);

/* 0x003fac30: sends the console what Direct3D has been told to draw */
void D3DDevice_KickOff(struct d3d_device *device);

/* ---------- constants */

enum
{
	GEOMETRY_CACHE_BLOCKS_SIZE= sizeof(struct data_array)+
		MAXIMUM_CACHED_GEOMETRY_BLOCKS*sizeof(struct geometry_cache_block)+
		BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CACHED_GEOMETRY_BLOCKS)*sizeof(unsigned long),
	GEOMETRY_CACHE_PREDICTED_SIZE= sizeof(struct data_array)+
		MAXIMUM_PREDICTED_GEOMETRY_BLOCKS*sizeof(struct predicted_geometry)+
		BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_PREDICTED_GEOMETRY_BLOCKS)*sizeof(unsigned long),
	GEOMETRY_CACHE_LRUV_SIZE= sizeof(struct lruv_cache)+sizeof(struct data_array)+
		MAXIMUM_CACHED_GEOMETRY_BLOCKS*sizeof(struct lruv_cache_block)+
		BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CACHED_GEOMETRY_BLOCKS)*sizeof(unsigned long),

	GEOMETRY_CACHE_WARNING_MILLISECONDS= 30000,

	/* the console's registers that are looked at after it is sent work */
	NV_PFIFO_CACHE1_DMA_PUT= 0x3240,
	NV_PFIFO_CACHE1_DMA_GET= 0x3244,
	NV_PGRAPH_STATUS= 0x400700
};

/* ---------- globals */

/* 0x004e648c */
struct geometry_cache_globals geometry_cache_globals;

/* 0x00468c4c: the game never waits for geometry; set in this build */
boolean geometry_cache_never_block= TRUE;

/* 0x0055e728: the time kept when there is no room to tell of one more block
of geometry; in the debug game, most likely, between two warnings */
unsigned long geometry_cache_warning_time;

/* ---------- code */

static inline __attribute__((always_inline)) struct geometry_cache_block *geometry_cache_block_get(
	long block_index)
{
	return &((struct geometry_cache_block *)geometry_cache_globals.blocks->data)[block_index&0xFFFF];
}

/* where the geometry of a block of the cache is */
static inline __attribute__((always_inline)) void *geometry_cache_block_get_data(
	long block_index)
{
	return geometry_cache_globals.base_address+
		lruv_block_get_address(geometry_cache_globals.cache, block_index);
}

/* as in the texture cache: the console is sent what there is to draw, and
its registers are looked at, with nothing done with what they say */
static inline __attribute__((always_inline)) void geometry_cache_kick_off(
	void)
{
	struct d3d_device *device= d3d_device;
	volatile byte *registers;

	D3DDevice_KickOff(device);
	registers= (volatile byte *)device->registers;
	if (*(volatile unsigned long *)(registers+NV_PFIFO_CACHE1_DMA_PUT)==
		*(volatile unsigned long *)(registers+NV_PFIFO_CACHE1_DMA_GET))
	{
		(void)*(volatile unsigned long *)(registers+NV_PGRAPH_STATUS);
	}
}

/* 0x0012d8b0: makes the cache, with no memory yet */
void geometry_cache_new(
	void)
{
	struct allocation *allocation;
	struct data_array *data;
	struct lruv_cache *cache;

	allocation= heap_allocation;
	data= allocation->functions->allocate(allocation, GEOMETRY_CACHE_BLOCKS_SIZE);
	if (data)
	{
		data_initialize(data, "xbox geometry", MAXIMUM_CACHED_GEOMETRY_BLOCKS, sizeof(struct geometry_cache_block), 0,
			allocation);
		data->flags|= FLAG(_data_array_allocated_bit);
	}
	geometry_cache_globals.blocks= data;

	allocation= heap_allocation;
	data= allocation->functions->allocate(allocation, GEOMETRY_CACHE_PREDICTED_SIZE);
	if (data)
	{
		data_initialize(data, "xbox predicted geometry", MAXIMUM_PREDICTED_GEOMETRY_BLOCKS,
			sizeof(struct predicted_geometry), 0, allocation);
		data->flags|= FLAG(_data_array_allocated_bit);
	}
	geometry_cache_globals.predicted_geometry= data;

	allocation= heap_allocation;
	cache= allocation->functions->allocate(allocation, GEOMETRY_CACHE_LRUV_SIZE);
	lruv_initialize(cache, "xbox geometry cache", 0, GEOMETRY_CACHE_PAGE_SIZE_BITS, MAXIMUM_CACHED_GEOMETRY_BLOCKS,
		geometry_cache_block_deleted, geometry_cache_block_locked, (void *)unknown_002bcc80, allocation);
	geometry_cache_globals.cache= cache;
}

/* 0x0012d970 */
void geometry_cache_delete(
	void)
{
	if (geometry_cache_globals.blocks)
	{
		data_dispose(geometry_cache_globals.blocks);
		geometry_cache_globals.blocks= NULL;
	}
	if (geometry_cache_globals.predicted_geometry)
	{
		data_dispose(geometry_cache_globals.predicted_geometry);
		geometry_cache_globals.predicted_geometry= NULL;
	}
	if (geometry_cache_globals.cache)
	{
		struct lruv_cache *cache= geometry_cache_globals.cache;

		cache->allocation->functions->deallocate(cache->allocation, cache);
		geometry_cache_globals.cache= NULL;
	}
}

/* 0x0012d9f0: takes the cache's memory, when a map is loaded: 7 MB, or half
a megabyte less in some of the six ways that the memory is shared out. With
0x0012dad0, in the table of those that take memory then (0x00453c00); the
game does not look whether the way is one of the six */
void GEOMETRY_CACHE_CALLBACK geometry_cache_allocate_memory(
	long configuration)
{
	if (geometry_cache_globals.cache)
	{
		long stage= physical_memory_globals.stage;
		long size;
		long page_count;
		uint32_t memory_size;
		uint32_t address;
		byte *base_address= NULL;

		switch (configuration)
		{
		case 3:
		case 5:
			size= GEOMETRY_CACHE_SIZE;
			break;
		default:
			size= configuration<=1 ? GEOMETRY_CACHE_SIZE : GEOMETRY_CACHE_SMALL_SIZE;
			break;
		}

		page_count= size/GEOMETRY_CACHE_PAGE_SIZE;
		memory_size= ((uint32_t)page_count*GEOMETRY_CACHE_PAGE_SIZE+PHYSICAL_MEMORY_PAGE_SIZE-1)&
			~(uint32_t)(PHYSICAL_MEMORY_PAGE_SIZE-1);
		address= physical_memory_globals.high_address[stage]-memory_size;
		if ((long)address>=(long)physical_memory_globals.low_address[stage])
		{
			physical_memory_globals.high_address[stage]= address;
			if (address)
			{
				base_address= (byte *)(uintptr_t)(address|(uint32_t)PHYSICAL_MEMORY_ADDRESS_FLAG);
				XPhysicalProtect(base_address, memory_size, PHYSICAL_MEMORY_PROTECTION);
			}
		}
		geometry_cache_globals.base_address= base_address;
		geometry_cache_globals.size= page_count*GEOMETRY_CACHE_PAGE_SIZE;
		lruv_resize(geometry_cache_globals.cache, page_count);
		geometry_cache_globals.cache->hole_algorithm= _lruv_hole_algorithm_age_per_page;
	}
}

/* 0x0012dad0: the cache is left with no memory; who gave it takes it back */
void geometry_cache_free_memory(
	void)
{
	if (geometry_cache_globals.cache)
	{
		lruv_resize(geometry_cache_globals.cache, 0);
		geometry_cache_globals.base_address= NULL;
	}
}

/* 0x0012daf0: a map was loaded */
void geometry_cache_open(
	void)
{
	geometry_cache_globals.blocking_count= 0;
	geometry_cache_globals.missed= FALSE;
	geometry_cache_globals.unknown20= 0;
	if (geometry_cache_globals.blocks)
	{
		data_make_valid(geometry_cache_globals.blocks);
	}
	if (geometry_cache_globals.predicted_geometry)
	{
		data_make_valid(geometry_cache_globals.predicted_geometry);
	}
}

/* 0x0012db40: the map is going away */
void geometry_cache_close(
	void)
{
	lruv_idle(geometry_cache_globals.cache);
	geometry_cache_flush();
	if (geometry_cache_globals.blocks)
	{
		geometry_cache_globals.blocks->valid= FALSE;
	}
	if (geometry_cache_globals.predicted_geometry)
	{
		geometry_cache_globals.predicted_geometry->valid= FALSE;
	}
}

/* 0x0012dba0: how many blocks are being read */
long geometry_cache_reading_count(
	void)
{
	long count= 0;

	if (geometry_cache_globals.predicted_geometry->valid)
	{
		const struct data_array *data= geometry_cache_globals.blocks;
		long absolute_index;

		for (absolute_index= data_next_absolute_index(geometry_cache_globals.blocks, 0);
			absolute_index!=NONE;
			absolute_index= data_next_absolute_index(geometry_cache_globals.blocks, absolute_index+1))
		{
			const struct geometry_cache_block *block=
				(const struct geometry_cache_block *)((const byte *)data->data+data->size*absolute_index);

			if (!block->read_done)
			{
				count++;
			}
		}
	}

	return count;
}

/* 0x0012dc10: starts to read the geometry that is about to be drawn, a few
blocks at a time */
void geometry_cache_update_predicted(
	void)
{
	struct data_array *data= geometry_cache_globals.predicted_geometry;
	long reading_count;
	long absolute_index;

	if (!data->valid || async_globals.tasks_added>MAXIMUM_GEOMETRY_CACHE_ASYNC_TASKS)
	{
		return;
	}

	reading_count= geometry_cache_reading_count();
	for (absolute_index= data_next_absolute_index(data, 0);
		absolute_index!=NONE && reading_count<MAXIMUM_GEOMETRY_CACHE_READS;
		absolute_index= data_next_absolute_index(data, absolute_index+1))
	{
		struct predicted_geometry *predicted=
			(struct predicted_geometry *)((byte *)data->data+data->size*absolute_index);
		long index= (long)((unsigned long)(long)predicted->identifier<<16)|absolute_index;

		geometry_cache_block_load(predicted->info, FLAG(_geometry_cache_load_bit)|FLAG(_geometry_cache_predicted_bit));
		predicted->info->flags&= (uint8_t)~FLAG(_geometry_block_predicted_bit);
		datum_delete(geometry_cache_globals.predicted_geometry, index);
		reading_count++;
	}
}

/* 0x0012dcb0: tells the cache that geometry is about to be drawn; whether it
is there. Geometry that is in the cache is kept there; other is read when
there is time, or at once while the game reads everything at once */
boolean geometry_cache_block_predict(
	struct geometry_block_info *info)
{
	if (info && (long)info->section_data_size>0 && (long)info->resource_data_size>=0)
	{
		if (info->loaded)
		{
			if (info->cache_block_index!=(uint32_t)NONE)
			{
				lruv_block_touch(geometry_cache_globals.cache, (long)info->cache_block_index);

				return TRUE;
			}
			info->loaded= FALSE;
		}
		else if (geometry_cache_globals.blocking_count>0 && game_state_globals.unknown2538)
		{
			geometry_cache_block_load(info, FLAG(_geometry_cache_block_bit)|FLAG(_geometry_cache_load_bit));
		}
		else if (!TEST_FLAG(info->flags, _geometry_block_predicted_bit))
		{
			struct data_array *data= geometry_cache_globals.predicted_geometry;
			long index= datum_new(data);

			if (index!=NONE)
			{
				((struct predicted_geometry *)data->data)[index&0xFFFF].info= info;
				info->flags|= FLAG(_geometry_block_predicted_bit);
			}
			else if ((unsigned long)GetTickCount()>geometry_cache_warning_time)
			{
				geometry_cache_warning_time= (unsigned long)GetTickCount()+GEOMETRY_CACHE_WARNING_MILLISECONDS;
			}
		}
	}

	return FALSE;
}

/* 0x0012dd80: once a frame */
void geometry_cache_idle(
	void)
{
	if (geometry_cache_globals.cache)
	{
		lruv_idle(geometry_cache_globals.cache);
		geometry_cache_update_predicted();
		unknown_0012e480();
	}
}

/* 0x0012ddd0: geometry is going away: it leaves the cache */
void geometry_cache_block_delete(
	struct geometry_block_info *info)
{
	if (TEST_FLAG(info->flags, _geometry_block_predicted_bit))
	{
		struct data_array *data= geometry_cache_globals.predicted_geometry;
		long absolute_index;

		for (absolute_index= data_next_absolute_index(data, 0);
			absolute_index!=NONE;
			absolute_index= data_next_absolute_index(data, absolute_index+1))
		{
			struct predicted_geometry *predicted=
				(struct predicted_geometry *)((byte *)data->data+data->size*absolute_index);

			if (predicted->info==info)
			{
				datum_delete(data, (long)((unsigned long)(long)predicted->identifier<<16)|absolute_index);
				break;
			}
		}
	}
	info->flags&= (uint8_t)~FLAG(_geometry_block_predicted_bit);
	if (info->cache_block_index!=(uint32_t)NONE)
	{
		lruv_block_delete(geometry_cache_globals.cache, (long)info->cache_block_index);
		info->cache_block_index= (uint32_t)NONE;
		info->loaded= FALSE;
	}
}

/* 0x0012de70: has the geometry of a block ready to draw; whether it is. The
flags say whether to wait for it, whether to read it, and how soon it is
wanted */
boolean geometry_cache_block_load(
	struct geometry_block_info *info,
	unsigned long flags)
{
	boolean block= (boolean)((flags>>_geometry_cache_block_bit)&1);
	boolean load= (boolean)((flags>>_geometry_cache_load_bit)&1);
	boolean predicted= (boolean)((flags>>_geometry_cache_predicted_bit)&1);
	boolean soon= (boolean)((flags>>_geometry_cache_soon_bit)&1);
	boolean unknown= FALSE;
	boolean structure;
	boolean priority_known= FALSE;
	unsigned long group_tag;
	long priority= 0;

	if (info->loaded)
	{
		lruv_block_touch(geometry_cache_globals.cache, (long)info->cache_block_index);

		return TRUE;
	}
	if ((long)info->section_data_size<=0 || (long)info->resource_data_size<0)
	{
		return FALSE;
	}

	group_tag= global_tag_instances[(short)info->owner_tag_index].group_tag;
	structure= group_tag==STRUCTURE_LIGHTMAP_TAG || group_tag==STRUCTURE_BSP_TAG;
	if (geometry_cache_never_block)
	{
		block= FALSE;
	}

	if (geometry_cache_globals.blocking_count>0 && load && !predicted && !block)
	{
		/* everything is read at once: all, or the geometry of the levels */
		if (geometry_cache_globals.blocking_usage==0 || (geometry_cache_globals.blocking_usage==1 && structure))
		{
			unknown= TRUE;
		}
		if (geometry_cache_globals.blocking_count==1)
		{
			if (unknown)
			{
				block= TRUE;
			}
			else
			{
				priority= soon ? GEOMETRY_CACHE_SOON_TASK_PRIORITY : GEOMETRY_CACHE_PREDICTED_TASK_PRIORITY;
				priority_known= TRUE;
			}
		}
		if (!priority_known && unknown)
		{
			priority= GEOMETRY_CACHE_PREDICTED_TASK_PRIORITY;
			priority_known= TRUE;
		}
	}
	if (!priority_known)
	{
		if (block)
		{
			priority= GEOMETRY_CACHE_BLOCKING_TASK_PRIORITY;
		}
		else if (soon)
		{
			priority= GEOMETRY_CACHE_SOON_TASK_PRIORITY;
		}
		else
		{
			priority= predicted ? GEOMETRY_CACHE_LATER_TASK_PRIORITY : GEOMETRY_CACHE_PREDICTED_TASK_PRIORITY;
		}
	}

	if (info->cache_block_index==(uint32_t)NONE && load)
	{
		geometry_cache_start_loading(block, info, unknown, priority);
	}

	if (info->cache_block_index==(uint32_t)NONE)
	{
		if (!predicted && load)
		{
			geometry_cache_globals.missed= TRUE;
		}
	}
	else
	{
		struct geometry_cache_block *cache_block= geometry_cache_block_get((long)info->cache_block_index);

		lruv_block_touch(geometry_cache_globals.cache, (long)info->cache_block_index);
		if (!cache_block->read_done)
		{
			if (priority>(long)cache_block->read_priority)
			{
				async_task_set_priority(cache_block->read_task_id, GEOMETRY_CACHE_BLOCKING_TASK_PRIORITY);
				cache_block->read_priority= (byte)priority;
			}
			if (block)
			{
				while (!cache_block->read_done)
				{
					SwitchToThread();
					main_loading_idle();
				}
			}
		}
		if (cache_block->read_done)
		{
			if (!info->loaded)
			{
				geometry_block_attach_data(geometry_cache_block_get_data((long)info->cache_block_index), info);
				info->loaded= TRUE;
			}

			return TRUE;
		}
	}

	if (structure)
	{
		geometry_cache_globals.structure_miss_count++;
	}

	return FALSE;
}

/* 0x0012e0c0: empties the cache */
void geometry_cache_flush(
	void)
{
	if (geometry_cache_globals.cache)
	{
		struct data_array *data;

		geometry_cache_kick_off();
		lruv_flush(geometry_cache_globals.cache);

		data= geometry_cache_globals.predicted_geometry;
		if (data->valid)
		{
			long absolute_index;

			for (absolute_index= data_next_absolute_index(data, 0);
				absolute_index!=NONE;
				absolute_index= data_next_absolute_index(data, absolute_index+1))
			{
				struct predicted_geometry *predicted=
					(struct predicted_geometry *)((byte *)data->data+data->size*absolute_index);

				predicted->info->flags&= (uint8_t)~FLAG(_geometry_block_predicted_bit);
				datum_delete(data, (long)((unsigned long)(long)predicted->identifier<<16)|absolute_index);
			}
		}
	}
}

/* 0x0012e150: the geometry of a tag leaves the cache */
void geometry_cache_delete_tag_blocks(
	long tag_index)
{
	struct data_array *data;
	long absolute_index;

	geometry_cache_kick_off();

	data= geometry_cache_globals.blocks;
	if (data && data->valid)
	{
		for (absolute_index= data_next_absolute_index(data, 0);
			absolute_index!=NONE;
			absolute_index= data_next_absolute_index(data, absolute_index+1))
		{
			struct geometry_cache_block *block=
				(struct geometry_cache_block *)((byte *)data->data+data->size*absolute_index);

			if ((long)block->info->owner_tag_index==tag_index)
			{
				geometry_cache_block_delete(block->info);
			}
		}

		data= geometry_cache_globals.predicted_geometry;
		for (absolute_index= data_next_absolute_index(data, 0);
			absolute_index!=NONE;
			absolute_index= data_next_absolute_index(data, absolute_index+1))
		{
			struct predicted_geometry *predicted=
				(struct predicted_geometry *)((byte *)data->data+data->size*absolute_index);

			if ((long)predicted->info->owner_tag_index==tag_index)
			{
				datum_delete(data, (long)((unsigned long)(long)predicted->identifier<<16)|absolute_index);
			}
		}
	}
}

/* 0x0012e210: what the lruv_cache asks before it throws a block out. One
that was used this tick stays, and one that is being read; one that was used
a few ticks ago, if the console still draws with it */
boolean GEOMETRY_CACHE_CALLBACK geometry_cache_block_locked(
	long block_index)
{
	const struct geometry_cache_block *block= geometry_cache_block_get(block_index);
	const struct lruv_cache *cache= geometry_cache_globals.cache;
	long age= cache->tick-lruv_block_get(cache, block_index)->last_used_tick;

	if (age==0)
	{
		return TRUE;
	}
	if (!block->read_done)
	{
		return TRUE;
	}
	if (age<GEOMETRY_CACHE_BUSY_AGE && block->info->loaded)
	{
		return geometry_block_busy(geometry_cache_block_get_data(block_index), block->info);
	}

	return FALSE;
}

/* 0x0012e290: what the lruv_cache calls when it throws a block out. It is
waited for first: for its read, and for the console. The fields of the tag
that had the geometry are left with none */
void GEOMETRY_CACHE_CALLBACK geometry_cache_block_deleted(
	long block_index)
{
	struct geometry_cache_block *block= geometry_cache_block_get(block_index);

	if (block->info)
	{
		struct lruv_cache *cache= geometry_cache_globals.cache;
		struct lruv_cache_block *cache_block= lruv_block_get(cache, block_index);

		/* used this tick: it is not kept for that */
		if (cache_block->last_used_tick==cache->tick)
		{
			cache_block->last_used_tick= cache->tick-1;
		}
		while (geometry_cache_block_locked(block_index))
		{
			async_globals.tasks_added= async_task_count();
			if (block->info->loaded)
			{
				geometry_block_wait(geometry_cache_block_get_data(block_index), block->info);
			}
		}

		if (block->info->loaded)
		{
			uint32_t *section= (uint32_t *)((byte *)block->info+block->info->owner_tag_section_offset);

			section[0]= 0;
			section[1]= 0;
			block->info->loaded= FALSE;
		}
		block->info->cache_block_index= (uint32_t)NONE;
	}
	datum_delete(geometry_cache_globals.blocks, block_index);
}

/* 0x0012e3a0: takes a block of the cache for geometry and starts to read
it; whether there was room */
boolean geometry_cache_start_loading(
	boolean block,
	struct geometry_block_info *info,
	boolean unknown,
	long priority)
{
	long block_index= lruv_block_new(geometry_cache_globals.cache, (long)info->block_size,
		(block || unknown) ? GEOMETRY_CACHE_BLOCKING_AGE : GEOMETRY_CACHE_PREDICTED_AGE);
	struct geometry_cache_block *cache_block;
	void *data;
	uint32_t size;

	if (block_index==NONE)
	{
		return FALSE;
	}

	data= geometry_cache_block_get_data(block_index);
	cache_block= geometry_cache_block_get(datum_new_at_index(geometry_cache_globals.blocks, block_index));
	cache_block->info= info;
	info->cache_block_index= (uint32_t)block_index;
	cache_block->info->loaded= FALSE;

	size= info->block_size;
	if (size&(CACHE_FILE_SECTOR_SIZE-1))
	{
		size= (size|(CACHE_FILE_SECTOR_SIZE-1))+1;
	}
	cache_block->read_task_id= cache_file_read_async(info->block_offset, (long)size, data, NULL,
		(uint8_t *)&cache_block->read_done, GEOMETRY_CACHE_READ_CATEGORY, priority);
	cache_block->read_priority= (byte)priority;

	return TRUE;
}
