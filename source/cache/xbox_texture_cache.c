/*
XBOX_TEXTURE_CACHE.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The names are those of Halo 1's xbox_texture_cache.c where it has them.
*/

#include "texture_cache.h"

#include "cache/cache_files.h"
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

/* 0x0046883c, 0x0046883d: for all the bitmaps to be drawn with one level
of detail less, or with all their detail; the first wins. Nothing of what
is decompiled sets them */
boolean texture_cache_low_detail;
boolean texture_cache_full_detail;

/* 0x00485ae4: what is drawn with when the game was to wait for the pixels
of a bitmap and there are none: one for each type of bitmap */
struct d3d_texture *texture_cache_default_textures[NUMBER_OF_TEXTURE_CACHE_DEFAULT_TEXTURES];

/* 0x00450768, 0x004507c8: Direct3D's format for each format of a bitmap, for
the textures whose sizes are powers of two and for those that are not; NONE
where there is none */
const long texture_cache_d3d_formats[NUMBER_OF_BITMAP_FORMATS]=
{
	0x19, 0x00, 0x01, 0x1A, NONE, NONE, 0x05, NONE, 0x02, 0x04, 0x07, 0x06,
	NONE, NONE, 0x0C, 0x0E, 0x0F, 0x0B, 0x0B, NONE, NONE, NONE, 0x28, 0x28
};
const long texture_cache_d3d_linear_formats[NUMBER_OF_BITMAP_FORMATS]=
{
	0x1F, 0x13, 0x1B, 0x20, NONE, NONE, 0x11, NONE, 0x10, 0x1D, 0x1E, 0x12,
	NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, 0x17, 0x17
};

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
	if (texture_cache_globals.preloaded_pixels)
	{
		texture_cache_globals.preloaded_pixels= 0;
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
	texture_cache_update_predicted();
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

/* 0x0012d400: takes memory of the cache for something else; its address, or
NULL. If there is no room, a few frames of the cache go by for the blocks to
get old, and then the console is waited for: not at all, a little or for
long */
void *texture_cache_steal_memory(
	long wait,
	long size,
	long user_data,
	texture_cache_stolen_memory_proc idle_proc,
	texture_cache_stolen_memory_proc reclaim_proc)
{
	uint64_t start= __builtin_ia32_rdtsc();
	real timeout= 0.0f;
	long idle_count= 0;
	void *address= NULL;

	switch (wait)
	{
	case _texture_cache_steal_wait_short:
		idle_count= 30;
		timeout= 0.1f;
		break;
	case _texture_cache_steal_wait_long:
		idle_count= 90;
		timeout= 1.0f;
		break;
	}

	if (size>0 && texture_cache_globals.cache->page_count>0)
	{
		long count= 0;

		while ((address= texture_cache_steal_memory_now(size, user_data, idle_proc, reclaim_proc))==NULL)
		{
			if (count<idle_count)
			{
				count++;
				texture_cache_idle();
			}
			else
			{
				int64_t elapsed= (int64_t)(__builtin_ia32_rdtsc()-start);

				if (elapsed<0)
				{
					elapsed= 0;
				}
				if (!(timeout>system_ticks_to_seconds(elapsed)))
				{
					break;
				}
				texture_cache_kick_off();
				SwitchToThread();
			}
		}
	}

	return address;
}

/* ---------- the pixels of a bitmap */

/* how many times a number can be halved before it is 1 */
static inline __attribute__((always_inline)) short texture_cache_log2(
	unsigned long value)
{
	short count= 0;

	if (value)
	{
		while (value!=1)
		{
			value>>= 1;
			count++;
		}
	}

	return count;
}

/* whether the bitmap has pixels for a level of detail: all but the first
have to be of more than 1 KB */
static inline __attribute__((always_inline)) boolean texture_cache_bitmap_has_level(
	const struct bitmap_data *bitmap,
	long level_index)
{
	return bitmap->pixels_offsets[level_index]!=(uint32_t)NONE && bitmap->pixels_sizes[level_index]!=0 &&
		(level_index==0 || (long)bitmap->pixels_sizes[level_index]>TEXTURE_CACHE_MINIMUM_LEVEL_SIZE);
}

/* the format that Direct3D has for the pixels of a bitmap */
static inline __attribute__((always_inline)) long texture_cache_d3d_format(
	const struct bitmap_data *bitmap,
	short format,
	const long *formats,
	long format_for_x8r8g8b8)
{
	long d3d_format= formats[format];

	if (TEST_FLAG(bitmap->flags, _bitmap_unknown5_bit) &&
		(format==_bitmap_format_x8r8g8b8 || format==_bitmap_format_a8r8g8b8))
	{
		d3d_format= format_for_x8r8g8b8;
	}

	return d3d_format;
}

/* the detail that the bitmap loses: what is asked, and what the bitmap and
the cache add. The game has this in two functions */
static inline __attribute__((always_inline)) real texture_cache_bitmap_mip_bias(
	const struct bitmap_data *bitmap,
	real mip_bias)
{
	if (texture_cache_mip_bias_enabled)
	{
		mip_bias= (real)bitmap->mip_bias*0.01f+mip_bias+texture_cache_globals.mip_bias;
	}

	return mip_bias;
}

/* 0x0012c640: reads the pixels that the map has apart, all at once, into
memory taken for them: the game waits. What bitmaps they are of is not known
yet */
void texture_cache_load_preloaded_pixels(
	void)
{
	struct tag_iterator iterator;
	struct cache_file_header *header= &cache_file_globals.header;
	long tag_index;

	/* the game goes through the bitmaps of each bitmap group, and does
	nothing with them: what it did is not in this build */
	iterator.next_tag_instance_index= 0;
	iterator.group_tag= BITMAP_SIGNATURE;
	while ((tag_index= tag_iterator_next(&iterator))!=NONE)
	{
	}

	if (header->unknown158_offset && header->unknown15C_size)
	{
		uint32_t size= header->unknown15C_size;
		uint32_t read_size= size;
		long stage= physical_memory_globals.stage;
		uint32_t memory_size= (((size+PHYSICAL_MEMORY_PAGE_SIZE-1)&~(uint32_t)(PHYSICAL_MEMORY_PAGE_SIZE-1))+
			PHYSICAL_MEMORY_PAGE_SIZE-1)&~(uint32_t)(PHYSICAL_MEMORY_PAGE_SIZE-1);
		uint32_t address= physical_memory_globals.high_address[stage]-memory_size;
		void *memory= NULL;
		volatile uint8_t done;

		if (read_size&(CACHE_FILE_SECTOR_SIZE-1))
		{
			read_size= (read_size|(CACHE_FILE_SECTOR_SIZE-1))+1;
		}
		if ((long)address>=(long)physical_memory_globals.low_address[stage])
		{
			physical_memory_globals.high_address[stage]= address;
			if (address)
			{
				memory= (void *)(uintptr_t)(address|(uint32_t)PHYSICAL_MEMORY_ADDRESS_FLAG);
				XPhysicalProtect(memory, memory_size, TEXTURE_CACHE_MEMORY_PROTECTION);
			}
		}
		texture_cache_globals.preloaded_pixels= (unsigned long)(uintptr_t)memory;
		texture_cache_globals.has_preloaded_pixels= TRUE;
		cache_file_read_async(header->unknown158_offset, (long)read_size, memory, NULL, (uint8_t *)&done,
			TEXTURE_CACHE_READ_CATEGORY, TEXTURE_CACHE_BLOCKING_TASK_PRIORITY);
		while (!done)
		{
			SwitchToThread();
		}
	}
	else
	{
		texture_cache_globals.has_preloaded_pixels= FALSE;
	}
}

/* 0x0012c820: makes the texture of a bitmap whose pixels were read with the
map. Until then the texture has what the map says of them: where they are in
what was read, their format and their size */
void texture_cache_bitmap_initialize_preloaded_format(
	struct bitmap_data *bitmap)
{
	struct d3d_texture *texture= (struct d3d_texture *)&bitmap->texture;
	unsigned long address= texture->data+texture_cache_globals.preloaded_pixels;
	short format= (short)texture->format;
	unsigned short width= (unsigned short)(texture->format>>16);
	unsigned short height= (unsigned short)texture->size;
	short depth= (short)(texture->size>>16);
	long d3d_format= texture_cache_d3d_format(bitmap, format, texture_cache_d3d_formats, D3D_FORMAT_UNKNOWN5);
	short mipmap_count= bitmap_mipmap_count((short)width, depth, (short)height, format,
		(boolean)((bitmap->flags>>_bitmap_linear_bit)&1), 0);

	texture->data= 0;
	texture->lock= 0;
	texture->common= D3D_TEXTURE_COMMON;
	texture->format= bitmap->type==_bitmap_type_cube_map ? D3D_TEXTURE_FORMAT_CUBE_MAP : D3D_TEXTURE_FORMAT;
	texture->format|= (unsigned long)((((bitmap->type!=_bitmap_type_3d ? 2 : 3)|(d3d_format<<4))<<4)|
		(((long)mipmap_count<<16)+0x10000));
	texture->format|= (unsigned long)((long)texture_cache_log2((unsigned long)(long)(short)width)<<20);
	texture->format|= (unsigned long)((long)texture_cache_log2((unsigned long)(long)(short)height)<<24);
	texture->format|= (unsigned long)((long)texture_cache_log2((unsigned long)(long)depth)<<28);
	texture->size= 0;
	texture->data= address&D3D_TEXTURE_DATA_MASK;
}

/* 0x0012c960: the texture of a bitmap whose pixels were read with the map;
NULL if they were not */
struct d3d_texture *texture_cache_bitmap_get_preloaded_format(
	struct bitmap_data *bitmap)
{
	struct d3d_texture *texture= NULL;

	if (texture_cache_globals.has_preloaded_pixels)
	{
		struct d3d_texture *bitmap_texture= (struct d3d_texture *)&bitmap->texture;

		if (bitmap_texture->common!=0 || (long)bitmap_texture->lock>0)
		{
			texture= bitmap_texture;
			if ((long)bitmap_texture->common<=0)
			{
				texture_cache_bitmap_initialize_preloaded_format(bitmap);
			}
		}
	}

	return texture;
}

/* 0x0012c990: the texture of a level of detail of a bitmap, if its pixels
are in the cache; NULL if they are not. If asked to, starts to read them,
and can wait for them. The two flags are set when the game waited, and when
there was no room in the cache */
struct d3d_texture *texture_cache_bitmap_get_level(
	long level_index,
	boolean load,
	long priority,
	struct bitmap_data *bitmap,
	boolean block,
	boolean unknown,
	boolean *waited,
	boolean *failed)
{
	boolean busy= !(async_globals.tasks_added<=MAXIMUM_TEXTURE_CACHE_ASYNC_TASKS &&
		texture_cache_globals.reads_this_frame<MAXIMUM_TEXTURE_CACHE_READS_PER_FRAME);
	struct texture_cache_block *cache_block;
	long block_index;

	if (!load || (!block && priority<=TEXTURE_CACHE_PREDICTED_TASK_PRIORITY && busy))
	{
		/* only what is there */
		block_index= (long)bitmap->cache_block_indices[level_index];
		if (block_index!=NONE)
		{
			cache_block= texture_cache_block_get(block_index);
			lruv_block_touch(texture_cache_globals.cache, block_index);
			if (cache_block->read_done)
			{
				return &cache_block->texture;
			}
		}

		return NULL;
	}

	if (!texture_cache_bitmap_has_level(bitmap, level_index))
	{
		return NULL;
	}
	if (bitmap->cache_block_indices[level_index]==(uint32_t)NONE)
	{
		if (!texture_cache_start_loading_bitmap(block, bitmap, level_index, unknown, priority))
		{
			*failed= TRUE;
		}
		if (bitmap->cache_block_indices[level_index]==(uint32_t)NONE)
		{
			return NULL;
		}
	}

	block_index= (long)bitmap->cache_block_indices[level_index];
	cache_block= texture_cache_block_get(block_index);
	lruv_block_touch(texture_cache_globals.cache, block_index);
	if (!cache_block->read_done)
	{
		if (block)
		{
			*waited= TRUE;
			async_task_set_priority(cache_block->read_task_id, TEXTURE_CACHE_BLOCKING_TASK_PRIORITY);
			while (!cache_block->read_done)
			{
				SwitchToThread();
				main_loading_idle();
			}
		}
		else if (priority>cache_block->read_priority)
		{
			cache_block->read_priority= priority;
			async_task_set_priority(cache_block->read_task_id, priority);
		}
		if (!cache_block->read_done)
		{
			return NULL;
		}
	}

	return &cache_block->texture;
}

/* 0x0012cb10: the level of detail to draw a bitmap with, for the detail that
it loses: the one with the most detail that it has, of those that have no
more than is needed */
long texture_cache_bitmap_choose_level(
	const struct bitmap_data *bitmap,
	real mip_bias)
{
	long level_index;
	long result= 0;

	for (level_index= 0; level_index<NUMBER_OF_TEXTURE_CACHE_LEVELS; level_index++)
	{
		if (texture_cache_bitmap_has_level(bitmap, level_index))
		{
			real level_mip_biases[NUMBER_OF_TEXTURE_CACHE_LEVELS]= { 0.0f, 1.0f, 2.0f };

			if (level_mip_biases[level_index]>mip_bias)
			{
				break;
			}
			result= level_index;
		}
	}

	return result;
}

/* 0x0012cb80: reads all the levels of detail of a bitmap, waiting for each,
and has them stay in the cache until it is emptied; whether they are all
there */
boolean texture_cache_bitmap_load(
	struct bitmap_data *bitmap)
{
	boolean loaded= TRUE;
	long level_index;

	for (level_index= 0; level_index<NUMBER_OF_TEXTURE_CACHE_LEVELS; level_index++)
	{
		if (texture_cache_bitmap_has_level(bitmap, level_index))
		{
			boolean waited;
			boolean failed;

			texture_cache_bitmap_get_level(level_index, TRUE, TEXTURE_CACHE_BLOCKING_TASK_PRIORITY, bitmap, TRUE, FALSE,
				&waited, &failed);
			if (bitmap->cache_block_indices[level_index]==(uint32_t)NONE)
			{
				loaded= FALSE;
			}
			else
			{
				texture_cache_block_get((long)bitmap->cache_block_indices[level_index])->flags|=
					FLAG(_texture_cache_block_required_bit);
			}
		}
	}

	return loaded;
}

/* 0x0012ccb0: whether the bitmaps of a use are those whose pixels are read
with the map. What the uses are is not known */
boolean texture_cache_usage_is_preloaded(
	long usage)
{
	switch (usage)
	{
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 17:
	case 18:
		return TRUE;
	}

	return FALSE;
}

/* 0x0012ccf0: the texture to draw a bitmap with, if it is there: nothing is
read. The texture of the first level is kept in the bitmap for two frames */
struct d3d_texture *texture_cache_bitmap_get_loaded(
	unsigned long flags,
	struct bitmap_data *bitmap,
	real mip_bias)
{
	struct d3d_texture *texture= NULL;
	boolean exact= (boolean)((flags>>_texture_cache_exact_bit)&1);

	mip_bias= texture_cache_bitmap_mip_bias(bitmap, mip_bias);
	if (exact)
	{
		mip_bias= 0.0f;
	}

	if (!TEST_FLAG(bitmap->flags, _bitmap_cached_bit))
	{
		texture= (struct d3d_texture *)(uintptr_t)bitmap->hardware_format;
	}
	else if (texture_cache_globals.use_preloaded_pixels && texture_cache_usage_is_preloaded(bitmap->usage))
	{
		texture= exact ? NULL : texture_cache_bitmap_get_preloaded_format(bitmap);
	}
	else if (bitmap->maximum_mip_bias>mip_bias)
	{
		long level_index= mip_bias>=2.0f ? 2 : mip_bias>=1.0f ? 1 : 0;
		long block_index= (long)bitmap->cache_block_indices[level_index];

		if (block_index!=NONE)
		{
			struct texture_cache_block *cache_block= texture_cache_block_get(block_index);

			lruv_block_touch(texture_cache_globals.cache, block_index);
			if (cache_block->read_done)
			{
				texture= &cache_block->texture;
				if (level_index==0)
				{
					bitmap->frame_index= texture_cache_globals.frame_index+TEXTURE_CACHE_HARDWARE_FORMAT_FRAMES;
					bitmap->hardware_format= (uint32_t)(uintptr_t)texture;
				}
			}
		}
	}

	if (bitmap->frame_index>=texture_cache_globals.frame_index && bitmap->hardware_format)
	{
		return (struct d3d_texture *)(uintptr_t)bitmap->hardware_format;
	}

	return texture;
}

/* 0x0012ce00: the texture to draw a bitmap with. In Halo 1,
_texture_cache_bitmap_get_hardware_format. The flags say whether to wait for
the pixels, whether to read them, and how soon they are wanted; if the game
is to wait and there is nothing to draw with, the texture is one of three
that the game has for that, by the type of the bitmap */
struct d3d_texture *texture_cache_bitmap_get_hardware_format(
	struct bitmap_data *bitmap,
	unsigned long flags,
	real mip_bias)
{
	boolean block= (boolean)((flags>>_texture_cache_block_bit)&1);
	boolean load= (boolean)((flags>>_texture_cache_load_bit)&1);
	boolean exact= (boolean)((flags>>_texture_cache_exact_bit)&1);
	boolean unknown= FALSE;
	boolean waited;
	boolean failed= FALSE;
	struct d3d_texture *texture= NULL;
	struct d3d_texture *level_texture;
	real biased= texture_cache_bitmap_mip_bias(bitmap, mip_bias);
	long level_index;
	long priority;

	if (texture_cache_full_detail)
	{
		biased= 0.0f;
	}
	if (texture_cache_low_detail)
	{
		biased= 1.0f;
	}

	if (!TEST_FLAG(bitmap->flags, _bitmap_cached_bit))
	{
		return (struct d3d_texture *)(uintptr_t)bitmap->hardware_format;
	}

	if (!exact)
	{
		texture= texture_cache_bitmap_get_preloaded_format(bitmap);
		if (texture_cache_globals.blocking_count>0 && (texture_cache_globals.blocking_usage==0 ||
			(texture_cache_globals.blocking_usage==1 && bitmap->usage==3)))
		{
			unknown= TRUE;
			block= texture_cache_globals.blocking_count==1;
		}
		else if (texture && !TEST_FLAG(bitmap->flags, _bitmap_unknown7_bit))
		{
			block= FALSE;
		}
	}

	if ((texture_cache_globals.use_preloaded_pixels && texture_cache_usage_is_preloaded(bitmap->usage)) ||
		mip_bias>=bitmap->maximum_mip_bias)
	{
		if (texture)
		{
			return texture;
		}
	}

	level_index= exact ? 0 : texture_cache_bitmap_choose_level(bitmap, biased);
	priority= TEXTURE_CACHE_PREDICTED_TASK_PRIORITY;
	if (!unknown)
	{
		if (block)
		{
			priority= TEXTURE_CACHE_BLOCKING_TASK_PRIORITY;
		}
		else if (TEST_FLAG(flags, _texture_cache_soon_bit))
		{
			priority= TEXTURE_CACHE_SOON_TASK_PRIORITY;
		}
		else if (TEST_FLAG(flags, _texture_cache_later_bit))
		{
			priority= TEXTURE_CACHE_LATER_TASK_PRIORITY;
		}
	}

	level_texture= texture_cache_bitmap_get_level(level_index, load, priority, bitmap, block, unknown, &waited, &failed);
	if (level_texture)
	{
		texture= level_texture;
	}
	else if (!exact)
	{
		/* any level that is there will do */
		for (level_index= 0; level_index<NUMBER_OF_TEXTURE_CACHE_LEVELS; level_index++)
		{
			long block_index= (long)bitmap->cache_block_indices[level_index];

			if (block_index!=NONE)
			{
				struct texture_cache_block *cache_block= texture_cache_block_get(block_index);

				lruv_block_touch(texture_cache_globals.cache, block_index);
				if (cache_block->read_done)
				{
					texture= &cache_block->texture;
					block= FALSE;
				}
			}
		}

		if (block && !texture)
		{
			texture_cache_globals.missed= TRUE;
			switch (bitmap->type)
			{
			case _bitmap_type_2d:
				return texture_cache_default_textures[0];
			case _bitmap_type_3d:
				return texture_cache_default_textures[1];
			default:
				return texture_cache_default_textures[2];
			}
		}
	}

	if (failed)
	{
		texture_cache_globals.missed= TRUE;
	}

	return texture;
}

/* 0x0012c450: starts to read the bitmaps that are about to be drawn, a few
each frame */
void texture_cache_update_predicted(
	void)
{
	struct data_array *data= texture_cache_globals.predicted_textures;
	long count= 0;
	long absolute_index;

	if (!data->valid || async_globals.tasks_added>MAXIMUM_TEXTURE_CACHE_ASYNC_TASKS)
	{
		return;
	}

	for (absolute_index= data_next_absolute_index(data, 0);
		absolute_index!=NONE && count<MAXIMUM_TEXTURE_CACHE_PREDICTED_PER_FRAME;
		absolute_index= data_next_absolute_index(data, absolute_index+1))
	{
		struct predicted_texture *predicted=
			(struct predicted_texture *)((byte *)data->data+data->size*absolute_index);
		long index= (long)((unsigned long)(long)predicted->identifier<<16)|absolute_index;
		struct bitmap_data *bitmap= predicted->bitmap;

		if (bitmap->frame_index<=texture_cache_globals.frame_index || !bitmap->hardware_format)
		{
			if (!texture_cache_bitmap_get_loaded(FLAG(_texture_cache_load_bit), bitmap, 0.0f))
			{
				texture_cache_bitmap_get_hardware_format(bitmap, FLAG(_texture_cache_load_bit), 0.0f);
			}
		}
		predicted->bitmap->flags&= (uint16_t)~FLAG(_bitmap_predicted_bit);
		datum_delete(texture_cache_globals.predicted_textures, index);
		count++;
	}
}

/* 0x0012d5a0: takes a block of the cache for a level of detail of a bitmap
and starts to read its pixels; whether there was room. In Halo 1,
texture_cache_start_loading_bitmap */
boolean texture_cache_start_loading_bitmap(
	boolean block,
	struct bitmap_data *bitmap,
	long level_index,
	boolean unknown,
	long priority)
{
	struct lruv_cache *cache= texture_cache_globals.cache;
	long block_index= lruv_block_new(cache, (long)bitmap->pixels_sizes[level_index],
		(block || unknown) ? TEXTURE_CACHE_BLOCKING_AGE : TEXTURE_CACHE_PREDICTED_AGE);
	struct texture_cache_block *cache_block;
	unsigned long address;
	uint32_t size;

	if (block && !unknown)
	{
		priority= TEXTURE_CACHE_BLOCKING_TASK_PRIORITY;
	}
	if (block_index==NONE)
	{
		return FALSE;
	}

	cache= texture_cache_globals.cache;
	address= (unsigned long)(uintptr_t)texture_cache_globals.base_address+lruv_block_get_address(cache, block_index);
	datum_new_at_index(texture_cache_globals.blocks, block_index);
	cache_block= texture_cache_block_get(block_index);
	cache_block->read_done= FALSE;
	cache_block->flags&= (byte)~FLAG(_texture_cache_block_stolen_bit);
	cache_block->level_index= level_index;
	cache_block->bitmap= bitmap;
	memset(&cache_block->texture, 0, sizeof(cache_block->texture));
	bitmap->cache_block_indices[level_index]= (uint32_t)block_index;
	bitmap->cache_addresses[level_index]= address;
	if (level_index==0)
	{
		bitmap->base_address= address;
	}
	texture_cache_initialize_hardware_format(bitmap, level_index, &cache_block->texture);

	size= bitmap->pixels_sizes[level_index];
	if (size&(CACHE_FILE_SECTOR_SIZE-1))
	{
		size= (size|(CACHE_FILE_SECTOR_SIZE-1))+1;
	}
	cache_block->read_task_id= cache_file_read_async(bitmap->pixels_offsets[level_index], (long)size,
		(void *)(uintptr_t)address, NULL, (uint8_t *)&cache_block->read_done, TEXTURE_CACHE_READ_CATEGORY, priority);
	cache_block->read_priority= priority;
	texture_cache_globals.reads_this_frame++;

	return TRUE;
}

/* 0x0012d6d0: makes the texture that Direct3D draws a level of detail of a
bitmap with. In Halo 1, texture_cache_initialize_hardware_format */
void texture_cache_initialize_hardware_format(
	const struct bitmap_data *bitmap,
	long level_index,
	struct d3d_texture *texture)
{
	texture->data= 0;
	texture->lock= 0;
	texture->common= D3D_TEXTURE_COMMON;
	if (TEST_FLAG(bitmap->flags, _bitmap_linear_bit))
	{
		/* not of sizes that are powers of two: one level, and its size apart */
		long d3d_format= texture_cache_d3d_format(bitmap, bitmap->format, texture_cache_d3d_linear_formats,
			D3D_FORMAT_LINEAR_UNKNOWN5);
		long pitch;

		texture->format= (unsigned long)((d3d_format<<8)|D3D_TEXTURE_FORMAT_LINEAR);
		pitch= bitmap_row_size(0, bitmap);
		texture->size= (unsigned long)(((((pitch-1)/D3D_TEXTURE_PITCH_ALIGNMENT)<<12)|(bitmap->height-1))<<12)|
			(unsigned long)(bitmap->width-1);
	}
	else
	{
		real level_mip_biases[NUMBER_OF_TEXTURE_CACHE_LEVELS]= { 0.0f, 1.0f, 2.0f };
		long shift= (long)level_mip_biases[level_index];
		long height= bitmap->height>>((short)shift&31);
		long width= bitmap->width>>((short)shift&31);
		long depth= (signed char)bitmap->depth>>(shift&31);
		long mipmap_count= bitmap_mipmap_count((short)width, (short)depth, (short)height, bitmap->format, FALSE,
			(short)(bitmap->mipmap_count-(short)shift));
		long d3d_format= texture_cache_d3d_format(bitmap, bitmap->format, texture_cache_d3d_formats,
			D3D_FORMAT_UNKNOWN5);
		long width_bits= texture_cache_log2((unsigned long)width);
		long height_bits= texture_cache_log2((unsigned long)height);
		long depth_bits= texture_cache_log2((unsigned long)depth);

		texture->format= (unsigned long)((((((((depth_bits<<4)|height_bits)<<4)|width_bits)<<12)|d3d_format)<<4)|
			(bitmap->type!=_bitmap_type_3d ? 2 : 3))<<4;
		texture->format|= (unsigned long)(bitmap->type==_bitmap_type_cube_map ? 4 : 0);
		texture->format|= (unsigned long)((mipmap_count+1)<<16);
		texture->format|= D3D_TEXTURE_FORMAT;
		texture->size= 0;
	}
	texture->data= bitmap->cache_addresses[level_index]&D3D_TEXTURE_DATA_MASK;
}
