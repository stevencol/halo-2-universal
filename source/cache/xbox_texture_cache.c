/*
XBOX_TEXTURE_CACHE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The names are those of Halo 1's xbox_texture_cache.c where it has them. Not
here yet: what reads the pixels of a bitmap (0x0012c640 to 0x0012ce00,
0x0012d5a0 on), unknown_0012c450 and texture_cache_steal_memory.
*/

#include "texture_cache.h"

#include "cseries/async.h"
#include "memory/physical_memory.h"
#include "shell/shell.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x003314b0 */
long GetTickCount(void);

/* 0x003fac30: sends the console what Direct3D has been told to draw */
void D3DDevice_KickOff(struct d3d_device *device);

/* 0x003fb710: whether the console still has to draw with a texture */
long D3DResource_IsBusy(struct d3d_texture *texture);

/* 0x003fad50: waits until it does not */
void D3DResource_BlockUntilNotBusy(struct d3d_texture *texture);

/* ---------- constants */

enum
{
	/* PAGE_READWRITE | PAGE_WRITECOMBINE: what the console draws from. The
	memory that the cache lends is PAGE_READWRITE alone */
	TEXTURE_CACHE_MEMORY_PROTECTION= 0x404,

	TEXTURE_CACHE_BLOCKS_SIZE= sizeof(struct data_array)+
		MAXIMUM_CACHED_TEXTURES*sizeof(struct texture_cache_block)+
		BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CACHED_TEXTURES)*sizeof(unsigned long),
	TEXTURE_CACHE_PREDICTED_SIZE= sizeof(struct data_array)+
		MAXIMUM_PREDICTED_TEXTURES*sizeof(struct predicted_texture)+
		BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_PREDICTED_TEXTURES)*sizeof(unsigned long),
	TEXTURE_CACHE_LRUV_SIZE= sizeof(struct lruv_cache)+sizeof(struct data_array)+
		MAXIMUM_CACHED_TEXTURES*sizeof(struct lruv_cache_block)+
		BIT_VECTOR_SIZE_IN_LONGS(MAXIMUM_CACHED_TEXTURES)*sizeof(unsigned long),

	/* the blocks used in the last so many ticks are what the cache needs */
	TEXTURE_CACHE_IN_USE_AGE= 5,
	/* how often the bias is looked at */
	TEXTURE_CACHE_MIP_BIAS_MILLISECONDS= 200,
	TEXTURE_CACHE_WARNING_MILLISECONDS= 30000,

	/* what a block of lent memory takes besides what was asked for: the
	structure before it, and what it takes to start at a page */
	TEXTURE_CACHE_STOLEN_MEMORY_EXTRA_SIZE= 0x2023,
	/* blocks used in the last tick are not thrown out for lent memory */
	TEXTURE_CACHE_STOLEN_MEMORY_AGE= 1,

	/* the console's registers that are looked at after it is sent work:
	where it is in the work, and what it is doing */
	NV_PFIFO_CACHE1_DMA_PUT= 0x3240,
	NV_PFIFO_CACHE1_DMA_GET= 0x3244,
	NV_PGRAPH_STATUS= 0x400700
};

#define TEXTURE_CACHE_MAXIMUM_MIP_BIAS 2.0f

/* ---------- globals */

/* 0x004e6454 */
struct texture_cache_globals texture_cache_globals;

/* 0x00468841 */
boolean texture_cache_mip_bias_enabled= TRUE;

/* 0x0055e724: the time kept when there is no room to tell of one more
bitmap; in the debug game, most likely, between two warnings */
unsigned long texture_cache_warning_time;

/* ---------- code */

static inline __attribute__((always_inline)) struct texture_cache_block *texture_cache_block_get(
	long block_index)
{
	return &((struct texture_cache_block *)texture_cache_globals.blocks->data)[block_index&0xFFFF];
}

/* the console is sent what there is to draw, and its registers are looked
at: nothing is done with what they say. The game has this where it waits
for the console */
static inline __attribute__((always_inline)) void texture_cache_kick_off(
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

/* 0x0012c0d0: makes the cache, with no memory yet */
void texture_cache_new(
	void)
{
	struct allocation *allocation;
	struct data_array *data;
	struct lruv_cache *cache;

	allocation= heap_allocation;
	data= allocation->functions->allocate(allocation, TEXTURE_CACHE_BLOCKS_SIZE);
	if (data)
	{
		data_initialize(data, "xbox texture", MAXIMUM_CACHED_TEXTURES, sizeof(struct texture_cache_block), 0,
			allocation);
		data->flags|= FLAG(_data_array_allocated_bit);
	}
	texture_cache_globals.blocks= data;

	allocation= heap_allocation;
	data= allocation->functions->allocate(allocation, TEXTURE_CACHE_PREDICTED_SIZE);
	if (data)
	{
		data_initialize(data, "xbox predicted texture", MAXIMUM_PREDICTED_TEXTURES,
			sizeof(struct predicted_texture), 0, allocation);
		data->flags|= FLAG(_data_array_allocated_bit);
	}
	texture_cache_globals.predicted_textures= data;

	allocation= heap_allocation;
	cache= allocation->functions->allocate(allocation, TEXTURE_CACHE_LRUV_SIZE);
	lruv_initialize(cache, "xbox texture cache", 0, TEXTURE_CACHE_PAGE_SIZE_BITS, MAXIMUM_CACHED_TEXTURES,
		texture_cache_block_deleted, texture_cache_block_locked, (void *)texture_cache_block_type, allocation);
	texture_cache_globals.cache= cache;
	cache->hole_algorithm= _lruv_hole_algorithm_age_per_page;
}

/* 0x0012c190: the list of the bitmaps that are about to be drawn is left
as it is */
void texture_cache_delete(
	void)
{
	if (texture_cache_globals.blocks)
	{
		data_dispose(texture_cache_globals.blocks);
		texture_cache_globals.blocks= NULL;
	}
	if (texture_cache_globals.cache)
	{
		struct lruv_cache *cache= texture_cache_globals.cache;

		cache->allocation->functions->deallocate(cache->allocation, cache);
		texture_cache_globals.cache= NULL;
	}
}

/* 0x0012c1e0: a map was loaded: the cache takes all the memory that is left,
in whole pages */
void texture_cache_open(
	void)
{
	long stage= physical_memory_globals.stage;
	long page_count= ((long)physical_memory_globals.high_address[stage]-
		(long)physical_memory_globals.low_address[stage])/TEXTURE_CACHE_PAGE_SIZE;
	uint32_t size= ((uint32_t)page_count*TEXTURE_CACHE_PAGE_SIZE+PHYSICAL_MEMORY_PAGE_SIZE-1)&
		~(uint32_t)(PHYSICAL_MEMORY_PAGE_SIZE-1);
	uint32_t address= physical_memory_globals.high_address[stage]-size;
	byte *base_address= NULL;

	if ((long)address>=(long)physical_memory_globals.low_address[stage])
	{
		physical_memory_globals.high_address[stage]= address;
		if (address)
		{
			base_address= (byte *)(uintptr_t)(address|(uint32_t)PHYSICAL_MEMORY_ADDRESS_FLAG);
			XPhysicalProtect(base_address, size, TEXTURE_CACHE_MEMORY_PROTECTION);
		}
	}
	texture_cache_globals.base_address= base_address;
	lruv_resize(texture_cache_globals.cache, page_count);

	data_make_valid(texture_cache_globals.blocks);
	data_make_valid(texture_cache_globals.predicted_textures);
}

/* 0x0012c290: the map is going away */
void texture_cache_close(
	void)
{
	texture_cache_globals.flushing= TRUE;
	texture_cache_flush();
	texture_cache_globals.blocks->valid= FALSE;
	texture_cache_globals.predicted_textures->valid= FALSE;
	if (texture_cache_globals.unknown18)
	{
		texture_cache_globals.unknown18= 0;
	}
	lruv_resize(texture_cache_globals.cache, 0);
	texture_cache_globals.base_address= NULL;
}

/* 0x0012c2e0: looks at how much of the cache is needed, and has the bitmaps
lose detail when it is nearly all of it, or when a bitmap was drawn without
its pixels; and get it back when there is room again */
void texture_cache_update_mip_bias(
	void)
{
	struct lruv_cache *cache= texture_cache_globals.cache;
	long page_count= cache->page_count;
	real in_use= 0.0f;
	real mip_bias= 0.0f;
	boolean biased= FALSE;

	if (page_count>0)
	{
		in_use= (real)lruv_compute_pages_in_use(cache, TEXTURE_CACHE_IN_USE_AGE)/(real)page_count;
		cache= texture_cache_globals.cache;
	}
	cache->hole_algorithm= _lruv_hole_algorithm_age_per_page;

	if (texture_cache_mip_bias_enabled && page_count>0)
	{
		mip_bias= texture_cache_globals.mip_bias;
		if (in_use>=0.95f || texture_cache_globals.missed)
		{
			mip_bias+= 0.05f;
		}
		else if (in_use>=0.85f)
		{
			mip_bias+= 0.02f;
		}
		else if (in_use<=0.5f)
		{
			mip_bias-= 0.05f;
		}
		else if (in_use<=0.6f)
		{
			mip_bias-= 0.02f;
		}
		else if (in_use<=0.7f)
		{
			mip_bias-= 0.01f;
		}

		if (mip_bias<0.0f)
		{
			mip_bias= 0.0f;
		}
		else if (mip_bias>TEXTURE_CACHE_MAXIMUM_MIP_BIAS)
		{
			mip_bias= TEXTURE_CACHE_MAXIMUM_MIP_BIAS;
		}
		biased= TRUE;
	}
	texture_cache_globals.mip_bias= biased ? mip_bias : 0.0f;
	texture_cache_globals.missed= FALSE;
}

/* 0x0012c400: once a frame, each of those that have memory of the cache is
told. One of them can give its memory back then: if it is not in the list
any more, the list is gone through again from the start */
void texture_cache_stolen_memory_idle(
	void)
{
	struct texture_cache_stolen_memory *memory= texture_cache_globals.stolen_memory;

	while (memory)
	{
		struct texture_cache_stolen_memory *other;

		if (memory->idle_proc)
		{
			memory->idle_proc(memory->address, memory->user_data);
		}
		for (other= texture_cache_globals.stolen_memory; other && other!=memory; other= other->next)
		{
		}
		memory= other ? memory->next : texture_cache_globals.stolen_memory;
	}
}

/* 0x0012c530: the count of frames starts again, and no bitmap is taken as
drawn in a frame of before */
void texture_cache_reset_frame_index(
	void)
{
	struct data_array *data= texture_cache_globals.blocks;

	if (data->valid)
	{
		long absolute_index;

		for (absolute_index= data_next_absolute_index(data, 0);
			absolute_index!=NONE;
			absolute_index= data_next_absolute_index(data, absolute_index+1))
		{
			struct texture_cache_block *block=
				(struct texture_cache_block *)((byte *)data->data+data->size*absolute_index);

			if (block->bitmap)
			{
				block->bitmap->base_address= 0;
				block->bitmap->hardware_format= 0;
				block->bitmap->frame_index= 0;
			}
		}
	}
	texture_cache_globals.frame_index= TEXTURE_CACHE_FIRST_FRAME_INDEX;
}

/* 0x0012c5b0: a frame goes by */
void texture_cache_tick(
	void)
{
	if (++texture_cache_globals.frame_index==0)
	{
		texture_cache_reset_frame_index();
	}
	lruv_idle(texture_cache_globals.cache);
}

/* 0x0012c600: once a frame */
void texture_cache_idle(
	void)
{
	texture_cache_stolen_memory_idle();
	unknown_0012c450();
	texture_cache_tick();
	if ((unsigned long)GetTickCount()>texture_cache_globals.mip_bias_time)
	{
		texture_cache_globals.mip_bias_time= (unsigned long)GetTickCount()+TEXTURE_CACHE_MIP_BIAS_MILLISECONDS;
		texture_cache_update_mip_bias();
	}
	texture_cache_globals.reads_this_frame= 0;
}

/* 0x0012c770: a bitmap is going away: its pixels leave the cache */
void texture_cache_bitmap_delete(
	struct bitmap_data *bitmap)
{
	if (TEST_FLAG(bitmap->flags, _bitmap_cached_bit))
	{
		long level_index;

		if (TEST_FLAG(bitmap->flags, _bitmap_predicted_bit))
		{
			struct data_array *data= texture_cache_globals.predicted_textures;
			long absolute_index;

			for (absolute_index= data_next_absolute_index(data, 0);
				absolute_index!=NONE;
				absolute_index= data_next_absolute_index(data, absolute_index+1))
			{
				struct predicted_texture *predicted=
					(struct predicted_texture *)((byte *)data->data+data->size*absolute_index);

				if (predicted->bitmap==bitmap)
				{
					datum_delete(data, (long)((unsigned long)(long)predicted->identifier<<16)|absolute_index);
					break;
				}
			}
		}

		for (level_index= 0; level_index<NUMBER_OF_TEXTURE_CACHE_LEVELS; level_index++)
		{
			if (bitmap->cache_block_indices[level_index]!=(uint32_t)NONE)
			{
				lruv_block_delete(texture_cache_globals.cache, (long)bitmap->cache_block_indices[level_index]);
				bitmap->cache_block_indices[level_index]= (uint32_t)NONE;
			}
			bitmap->cache_addresses[level_index]= 0;
		}
		bitmap->flags&= (uint16_t)~(FLAG(_bitmap_cached_bit)|FLAG(_bitmap_predicted_bit));
		bitmap->base_address= 0;
	}
}

/* 0x0012cc10: tells the cache that a bitmap is about to be drawn; whether
its pixels are there. One that is in the cache is kept there; one that is
not is read when there is time */
boolean texture_cache_bitmap_predict(
	struct bitmap_data *bitmap)
{
	struct data_array *data= texture_cache_globals.predicted_textures;

	if (data->valid)
	{
		long block_index= (long)bitmap->cache_block_indices[0];

		if (block_index!=NONE)
		{
			const struct texture_cache_block *block= texture_cache_block_get(block_index);

			lruv_block_touch(texture_cache_globals.cache, block_index);
			if (block->read_done)
			{
				return TRUE;
			}
		}
		else if (!TEST_FLAG(bitmap->flags, _bitmap_predicted_bit))
		{
			long index= datum_new(data);

			if (index!=NONE)
			{
				((struct predicted_texture *)data->data)[index&0xFFFF].bitmap= bitmap;
				bitmap->flags|= FLAG(_bitmap_predicted_bit);
			}
			else if ((unsigned long)GetTickCount()>texture_cache_warning_time)
			{
				texture_cache_warning_time= (unsigned long)GetTickCount()+TEXTURE_CACHE_WARNING_MILLISECONDS;
			}
		}
	}

	return FALSE;
}

/* 0x0012d0a0: empties the cache. Those that have memory of it are asked to
give it back first */
void texture_cache_flush(
	void)
{
	struct data_array *data;

	while (texture_cache_globals.stolen_memory)
	{
		struct texture_cache_stolen_memory *memory= texture_cache_globals.stolen_memory;

		memory->reclaim_proc(memory->address, memory->user_data);
	}
	texture_cache_kick_off();
	lruv_flush(texture_cache_globals.cache);

	data= texture_cache_globals.predicted_textures;
	if (data->valid)
	{
		long absolute_index;

		for (absolute_index= data_next_absolute_index(data, 0);
			absolute_index!=NONE;
			absolute_index= data_next_absolute_index(data, absolute_index+1))
		{
			struct predicted_texture *predicted=
				(struct predicted_texture *)((byte *)data->data+data->size*absolute_index);

			predicted->bitmap->flags&= (uint16_t)~FLAG(_bitmap_predicted_bit);
			datum_delete(data, (long)((unsigned long)(long)predicted->identifier<<16)|absolute_index);
		}
	}
}

/* 0x0012d160: what a block is, for the lruv_cache; nothing of what is
decompiled asks */
byte TEXTURE_CACHE_CALLBACK texture_cache_block_type(
	long block_index)
{
	const struct texture_cache_block *block= texture_cache_block_get(block_index);

	if (TEST_FLAG(block->flags, _texture_cache_block_stolen_bit))
	{
		return _texture_cache_block_type_stolen;
	}

	return block->level_index ? _texture_cache_block_type_other_level : _texture_cache_block_type_first_level;
}

/* a block cannot be thrown out while it is lent, while it has to stay,
while it is being read, and while the console draws with it. The game has
this in the two functions that follow */
static inline __attribute__((always_inline)) boolean texture_cache_block_busy(
	struct texture_cache_block *block)
{
	if (TEST_FLAG(block->flags, _texture_cache_block_stolen_bit))
	{
		return TRUE;
	}
	if (TEST_FLAG(block->flags, _texture_cache_block_required_bit) && !texture_cache_globals.flushing)
	{
		return TRUE;
	}
	if (block->read_task_id==NONE)
	{
		return FALSE;
	}
	if (!block->read_done)
	{
		return TRUE;
	}

	return D3DResource_IsBusy(&block->texture)!=0;
}

/* 0x0012d1a0: what the lruv_cache asks before it throws a block out */
boolean TEXTURE_CACHE_CALLBACK texture_cache_block_locked(
	long block_index)
{
	return texture_cache_block_busy(texture_cache_block_get(block_index));
}

/* 0x0012d200: what the lruv_cache calls when it throws a block out. If it is
of a bitmap, it is waited for first: for its read, and for the console */
void TEXTURE_CACHE_CALLBACK texture_cache_block_deleted(
	long block_index)
{
	struct texture_cache_block *block= texture_cache_block_get(block_index);

	if (block->bitmap)
	{
		struct bitmap_data *bitmap;

		while (texture_cache_block_busy(texture_cache_block_get(block_index)))
		{
			async_globals.tasks_added= async_task_count();
			if (block->read_done)
			{
				D3DResource_BlockUntilNotBusy(&block->texture);
			}
		}

		bitmap= block->bitmap;
		bitmap->cache_block_indices[block->level_index]= (uint32_t)NONE;
		block->bitmap->cache_addresses[block->level_index]= 0;
		if (block->level_index==0)
		{
			block->bitmap->base_address= 0;
			block->bitmap->hardware_format= 0;
			block->bitmap->frame_index= 0;
		}
	}
	datum_delete(texture_cache_globals.blocks, block_index);
}

/* 0x0012d2f0: takes memory of the cache for something else, if there is
room without waiting; its address, which is the start of a page, or NULL.
What tells whose it is goes right before it */
void *texture_cache_steal_memory_now(
	long size,
	long user_data,
	texture_cache_stolen_memory_proc idle_proc,
	texture_cache_stolen_memory_proc reclaim_proc)
{
	struct lruv_cache *cache= texture_cache_globals.cache;
	long block_index= lruv_block_new(cache, size+TEXTURE_CACHE_STOLEN_MEMORY_EXTRA_SIZE,
		TEXTURE_CACHE_STOLEN_MEMORY_AGE);
	struct texture_cache_stolen_memory *memory;
	struct texture_cache_block *block;
	uintptr_t address;

	if (block_index==NONE)
	{
		return NULL;
	}

	cache= texture_cache_globals.cache;
	address= ((uintptr_t)texture_cache_globals.base_address+lruv_block_get_address(cache, block_index)+
		sizeof(struct texture_cache_stolen_memory)+TEXTURE_CACHE_PAGE_SIZE-1)&
		~(uintptr_t)(TEXTURE_CACHE_PAGE_SIZE-1);
	memory= (struct texture_cache_stolen_memory *)(address-sizeof(struct texture_cache_stolen_memory));
	block= texture_cache_block_get(datum_new_at_index(texture_cache_globals.blocks, block_index));

	memory->signature= (unsigned long)address^TEXTURE_CACHE_STOLEN_MEMORY_SIGNATURE;
	memory->user_data= user_data;
	memory->block_index= block_index;
	memory->address= (void *)address;
	memory->size= size;
	memory->idle_proc= idle_proc;
	memory->reclaim_proc= reclaim_proc;
	memory->previous= NULL;
	memory->next= texture_cache_globals.stolen_memory;
	if (texture_cache_globals.stolen_memory)
	{
		texture_cache_globals.stolen_memory->previous= memory;
	}
	texture_cache_globals.stolen_memory= memory;
	XPhysicalProtect(memory->address, (uint32_t)memory->size, PHYSICAL_MEMORY_PROTECTION);

	memset(&block->flags, 0, sizeof(*block)-offsetof(struct texture_cache_block, flags));
	block->flags|= FLAG(_texture_cache_block_stolen_bit);

	return (void *)address;
}

/* 0x0012d520: gives back memory that was taken from the cache */
void texture_cache_return_memory(
	void *address)
{
	struct texture_cache_stolen_memory *memory= (struct texture_cache_stolen_memory *)address-1;
	struct texture_cache_block *block= texture_cache_block_get(memory->block_index);
	struct texture_cache_stolen_memory *next;
	struct texture_cache_stolen_memory *previous;

	XPhysicalProtect(memory->address, (uint32_t)memory->size, TEXTURE_CACHE_MEMORY_PROTECTION);
	block->flags&= (byte)~FLAG(_texture_cache_block_stolen_bit);

	next= memory->next;
	previous= memory->previous;
	memory->signature= 0;
	if (next)
	{
		next->previous= previous;
	}
	if (previous)
	{
		previous->next= next;
	}
	else
	{
		texture_cache_globals.stolen_memory= next;
	}
	lruv_block_delete(texture_cache_globals.cache, memory->block_index);
}
