/*
SHELL.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

The first functions of the executable: where the game starts, what it sets up
before its main loop, and what it takes down after it. shell_initialize and
shell_dispose are those of Halo 1's shell.c; the other names are guesses.
*/

#include "memory/lruv_cache.h"
#include "sound/sound_definitions.h"
#include "shell.h"

#include "cache/animation_cache.h"
#include "cache/cache_files.h"
#include "cache/texture_cache.h"
#include "networking/game_voice.h"
#include "bitmaps/bitmap_definitions.h"
#include "input/input.h"
#include "cseries/async.h"
#include "main/global_preferences.h"
#include "memory/physical_memory.h"
#include "saved games/game_state.h"
#include "text/font_cache.h"
#include "text/fonts.h"

#include <string.h>

/* ---------- prototypes of functions of the console's libraries */

/* 0x003312cc */
void *VirtualAlloc(void *address, uint32_t size, uint32_t allocation_type, uint32_t protection);

/* 0x003312fa */
int VirtualFree(void *address, uint32_t size, uint32_t free_type);

/* 0x002d15ea */
void XPhysicalFree(void *address);

/* 0x002d1f3a */
int CloseHandle(uint32_t handle);

/* 0x002d0915 */
uint32_t XGetLaunchInfo(uint32_t *launch_data_type, void *launch_data);

/* 0x00330310 */
uint32_t XGetVideoStandard(void);

/* 0x00330339 */
uint32_t XGetVideoFlags(void);

/* 0x003212d6 */
unsigned int _control87(unsigned int value, unsigned int mask);

/* 0x002d5d0a: of the console's libraries; which one is not known */
long unknown_002d5d0a(void);

/* 0x003f5240: Direct3D's CreateDevice, built for this game: the behaviour
flags and where to leave the device come in registers */
long d3d_create_device(uint32_t behavior_flags, struct d3d_device **device,
	struct d3d_present_parameters *parameters);

/* 0x003f9d00 and 0x003fc530: of Direct3D; the second takes the device down */
void unknown_003f9d00(long unknown);
void d3d_device_destroy(struct d3d_device *device);

/* ---------- prototypes of functions not decompiled yet */

/* what shell_initialize sets up, in the order it calls them. Halo 1's does
the errors, the tag files, the real math, the state of the game, the
rasterizer, the input and the sound */
void unknown_00012560(void);

/* what shell_dispose takes down */
void unknown_000141c0(void);

/* 0x0012b690: the game, until it ends. The name is a guess */
void main_loop(void);

/* 0x002d09c0 and 0x002d0a60: of the console's libraries: what XMemAlloc and
XMemFree do if the game does not do it */
void *__attribute__((stdcall)) XMemAllocDefault(unsigned long size, unsigned long attributes);
void __attribute__((stdcall)) XMemFreeDefault(void *address, unsigned long attributes);

/* ---------- constants */

/* the attributes of the console's allocations: who asks, and what */
#define XMEM_ALLOCATOR_ID_SHIFT 16
#define XMEM_ALLOCATOR_ID_MASK 0xFF
#define XMEM_STATIC_ALLOCATOR_ID 0x82
#define XMEM_SOUND_FIRST_ALLOCATOR_ID 0x89
#define XMEM_SOUND_LAST_ALLOCATOR_ID 0x8A
#define XMEM_ALLOCATION_KIND_MASK 0x1FFF
#define XMEM_STATIC_ALLOCATION_KIND 1
#define XMEM_ZERO_INITIALIZE 0x40000000
#define XMEM_STATIC_BUFFER_SIZE 0xCC

#define MEM_COMMIT 0x1000
#define MEM_RELEASE 0x8000
#define PAGE_READWRITE 4

enum
{
	/* _control87: single precision, round to nearest, no exceptions */
	FLOATING_POINT_CONTROL= 0x9001F,
	FLOATING_POINT_CONTROL_MASK= 0xFFFFF,

	LAUNCH_DATA_SIZE= 0xC00,

	/* D3DCREATE_HARDWARE_VERTEXPROCESSING */
	D3D_CREATE_FLAGS= 0x40,
	/* D3DPRESENTFLAG_LOCKABLE_BACKBUFFER, _WIDESCREEN and _PROGRESSIVE */
	_d3d_present_flag_lockable_backbuffer= 0x01,
	_d3d_present_flag_widescreen= 0x10,
	_d3d_present_flag_progressive= 0x40,
	/* D3DPRESENT_INTERVAL_IMMEDIATE */
	D3D_PRESENT_INTERVAL_IMMEDIATE= (int)0x80000000,

	SHELL_DEVICE_WIDTH= 640,
	SHELL_DEVICE_HEIGHT= 480,
	/* D3DFMT_A8R8G8B8 and D3DFMT_D24S8, as the console numbers them */
	SHELL_DEVICE_FORMAT= 6,
	SHELL_DEVICE_DEPTH_FORMAT= 0x2A,

	UNKNOWN_TABLE_SIZE= 0xD4
};

/* ---------- globals */

/* 0x00510c3c: the buffer that the game keeps for the allocator 0x82, and
whether it was given */
void *xmem_static_buffer;
boolean xmem_static_buffer_used;

/* 0x005107fc */
struct shell_memory_globals shell_memory_globals;

/* 0x00509340: the game does not go on. The name is that of Halo 1's
variable */
boolean application_paused;

/* 0x00509344: a word that the game was started with: "xdemo " from the disc
of demos. What reads it is not decompiled */
char shell_launch_name[SHELL_LAUNCH_NAME_LENGTH];

/* 0x00485ac0 */
struct xbox_video_settings xbox_video_settings;

/* 0x005093b0: the device that the game made */
struct d3d_device *global_d3d_device;

/* 0x00407488: Direct3D's own pointer to its device */
struct d3d_device *d3d_device;

/* 0x00509350: set when the device is asked for */
long shell_device_requested;

/* 0x0047ffd8, and 0x00484ce8 where it is copied when the device is made.
What they are is not known */
byte unknown_0047ffd8[UNKNOWN_TABLE_SIZE];
byte unknown_00484ce8[UNKNOWN_TABLE_SIZE];

/* ---------- code */

/* the allocator that asks for memory: of the attributes of the console's
allocations */
static inline __attribute__((always_inline)) unsigned long xmem_allocator_id(
	unsigned long attributes)
{
	return (attributes>>XMEM_ALLOCATOR_ID_SHIFT)&XMEM_ALLOCATOR_ID_MASK;
}

/* 0x00012280: the game's own allocation of the memory that the console's
libraries ask for: one allocator (0x82) gets a buffer that the game keeps
for it, two (0x89 and 0x8A) the memory of the sound, and the others the
console's */
void *__attribute__((stdcall)) XMemAlloc(
	unsigned long size,
	unsigned long attributes)
{
	unsigned long allocator= xmem_allocator_id(attributes);

	if (allocator==XMEM_STATIC_ALLOCATOR_ID)
	{
		return xmem_static_allocate(size, attributes);
	}
	if (allocator>XMEM_SOUND_FIRST_ALLOCATOR_ID-1 && allocator<=XMEM_SOUND_LAST_ALLOCATOR_ID)
	{
		return game_voice_allocate(size, attributes);
	}

	return XMemAllocDefault(size, attributes);
}

/* 0x000122c0: gives back what XMemAlloc gave: the buffer that the game keeps
is free again */
void __attribute__((stdcall)) XMemFree(
	void *address,
	unsigned long attributes)
{
	unsigned long allocator= xmem_allocator_id(attributes);

	if (allocator==XMEM_STATIC_ALLOCATOR_ID)
	{
		if (xmem_static_buffer==address)
		{
			xmem_static_buffer_used= FALSE;

			return;
		}
	}
	else if (allocator>XMEM_SOUND_FIRST_ALLOCATOR_ID-1 && allocator<=XMEM_SOUND_LAST_ALLOCATOR_ID)
	{
		game_voice_free(address, attributes);

		return;
	}
	XMemFreeDefault(address, attributes);
}

/* 0x00012310 and 0x00012360: the texture that a bitmap is drawn with, read
if it is not there, and waiting for it in the first one; a bitmap that has
had its texture this frame keeps it. NULL if it cannot be had now. The
names are guesses */
static inline __attribute__((always_inline)) struct d3d_texture *shell_bitmap_texture(
	struct bitmap_data *bitmap,
	unsigned long flags,
	real mip_bias)
{
	struct d3d_texture *texture= NULL;

	if (bitmap->frame_index>texture_cache_globals.frame_index)
	{
		texture= (struct d3d_texture *)(uintptr_t)bitmap->hardware_format;
	}
	if (!texture)
	{
		texture= texture_cache_bitmap_get_loaded(flags, bitmap, mip_bias);
		if (!texture)
		{
			texture= texture_cache_bitmap_get_hardware_format(bitmap, flags, mip_bias);
		}
	}

	return texture;
}

struct d3d_texture *shell_bitmap_get_texture_blocking(
	struct bitmap_data *bitmap,
	real mip_bias)
{
	return shell_bitmap_texture(bitmap, FLAG(_texture_cache_block_bit)|FLAG(_texture_cache_load_bit), mip_bias);
}

struct d3d_texture *shell_bitmap_get_texture(
	struct bitmap_data *bitmap,
	real mip_bias)
{
	return shell_bitmap_texture(bitmap, FLAG(_texture_cache_load_bit), mip_bias);
}

/* 0x0012c090: the buffer that the game keeps for the allocator 0x82, for
the allocations of the kind 1: cleared if they ask for it. The others are
the console's. The buffer is not looked at to see if it is free. The name is
a guess */
void *__attribute__((stdcall)) xmem_static_allocate(
	unsigned long size,
	unsigned long attributes)
{
	void *buffer= xmem_static_buffer;

	if ((attributes&XMEM_ALLOCATION_KIND_MASK)!=XMEM_STATIC_ALLOCATION_KIND)
	{
		return XMemAllocDefault(size, attributes);
	}
	xmem_static_buffer_used= TRUE;
	if (attributes&XMEM_ZERO_INITIALIZE)
	{
		memset(buffer, 0, XMEM_STATIC_BUFFER_SIZE);
	}

	return buffer;
}

/* 0x00012000: sets up the game; whether it can run */
boolean shell_initialize(
	void)
{
	unknown_002d5d0a();
	shell_memory_globals.memory= VirtualAlloc(NULL, SHELL_MEMORY_SIZE, MEM_COMMIT, PAGE_READWRITE);
	shell_memory_globals.size= 0;

	if (!shell_platform_initialize())
	{
		return FALSE;
	}
	_control87(FLOATING_POINT_CONTROL, FLOATING_POINT_CONTROL_MASK);

	async_initialize();
	global_preferences_initialize();
	fonts_initialize();
	cache_files_initialize();
	animation_cache_new();
	game_state_initialize();
	unknown_00012560();
	if (!global_d3d_device)
	{
		return FALSE;
	}
	input_initialize();
	sound_initialize();

	return TRUE;
}

/* 0x00012080: takes the game down */
void shell_dispose(
	void)
{
	game_state_dispose();
	sound_dispose();
	input_dispose();
	unknown_000141c0();
	animation_cache_delete();
	font_cache_dispose();
	font_characters_dispose();

	/* the thread of the tasks is not ended: only its handles are closed */
	memset(font_slots, 0, sizeof(font_slots));
	CloseHandle(async_globals.thread);
	CloseHandle(async_globals.semaphore);
	CloseHandle(async_globals.task_list_mutex);
	CloseHandle(async_globals.free_list_mutex);

	if (shell_memory_globals.memory)
	{
		VirtualFree(shell_memory_globals.memory, 0, MEM_RELEASE);
		shell_memory_globals.unknown0= 0;
		shell_memory_globals.memory= NULL;
		shell_memory_globals.size= 0;
		shell_memory_globals.size_checksum= 0;
	}
}

/* 0x00012190: the game, from start to end: what the console's start-up code
calls. It does not return */
void shell_main(
	void)
{
	shell_test_device();
	physical_memory_initialize();
	physical_memory_stage_push();

	if (shell_initialize())
	{
		main_loop();
		shell_dispose();
	}

	memset(&physical_memory_globals, 0, sizeof(physical_memory_globals));
	XPhysicalFree((void *)(uintptr_t)(PHYSICAL_MEMORY_BASE_ADDRESS|(uint32_t)PHYSICAL_MEMORY_ADDRESS_FLAG));

	/* the game's function has no return: it ends in a breakpoint */
	__builtin_trap();
}

/* 0x00012200: looks at how the game was started: from the disc of demos, it
adds a word to shell_launch_name. Always true. The name is that of the
function that Halo 1's shell_initialize calls */
boolean shell_platform_initialize(
	void)
{
	byte launch_data[LAUNCH_DATA_SIZE];
	uint32_t launch_data_type;

	if (XGetLaunchInfo(&launch_data_type, launch_data)==0 &&
		launch_data_type==0 &&
		memcmp(launch_data, "XDEMOS", 7)==0)
	{
		unsigned long length;

		for (length= 0; length<SHELL_LAUNCH_NAME_LENGTH && shell_launch_name[length]; length++)
		{
		}
		strncpy(shell_launch_name+length, "xdemo ", SHELL_LAUNCH_NAME_LENGTH-length);
		shell_launch_name[SHELL_LAUNCH_NAME_LENGTH-1]= 0;
	}

	return TRUE;
}

/* 0x000123b0: asks the console how it is set to show the picture */
void shell_get_video_settings(
	void)
{
	uint32_t standard= XGetVideoStandard();
	uint32_t flags= XGetVideoFlags();

	if (standard==_xbox_video_standard_pal)
	{
		xbox_video_settings.refresh_rate= (flags&_xbox_video_pal_60hz_flag) ? 60 : 50;
	}
	xbox_video_settings.widescreen= (flags&_xbox_video_widescreen_flag)!=0;
	xbox_video_settings.letterbox= (flags&_xbox_video_letterbox_flag)!=0;
	xbox_video_settings.progressive_scan= (flags&_xbox_video_480p_flag)!=0;
	xbox_video_settings.progressive_scan5= (flags&_xbox_video_480p_flag)!=0;
	xbox_video_settings.pal= standard==_xbox_video_standard_pal;
}

/* 0x00012490: makes the device of Direct3D, of 640 by 480; whether it was
made */
boolean shell_create_device(
	void)
{
	struct d3d_present_parameters parameters;

	memset(&parameters, 0, sizeof(parameters));
	parameters.refresh_rate= (uint32_t)(long)xbox_video_settings.refresh_rate;
	shell_device_requested= 1;
	parameters.flags= _d3d_present_flag_lockable_backbuffer;
	parameters.windowed= 0;
	parameters.swap_effect= 1;
	parameters.enable_auto_depth_stencil= 1;
	parameters.auto_depth_stencil_format= SHELL_DEVICE_DEPTH_FORMAT;
	parameters.back_buffer_format= SHELL_DEVICE_FORMAT;
	parameters.back_buffer_width= SHELL_DEVICE_WIDTH;
	parameters.back_buffer_height= SHELL_DEVICE_HEIGHT;
	parameters.presentation_interval= (uint32_t)D3D_PRESENT_INTERVAL_IMMEDIATE;
	if (xbox_video_settings.widescreen)
	{
		parameters.flags= _d3d_present_flag_lockable_backbuffer|_d3d_present_flag_widescreen;
	}
	if (xbox_video_settings.progressive_scan)
	{
		parameters.flags|= _d3d_present_flag_progressive;
	}

	d3d_create_device(D3D_CREATE_FLAGS, &global_d3d_device, &parameters);
	if (!global_d3d_device)
	{
		global_d3d_device= NULL;
		return FALSE;
	}
	memcpy(unknown_00484ce8, unknown_0047ffd8, sizeof(unknown_00484ce8));

	return TRUE;
}

/* 0x00012420: makes the device and lets it go at once, before anything else
of the game: whether it could be made. The game makes it again later */
boolean shell_test_device(
	void)
{
	boolean created;

	shell_get_video_settings();
	created= shell_create_device();
	if (created)
	{
		unknown_003f9d00(0);
		if (global_d3d_device)
		{
			/* Direct3D's Release, which the compiler put here */
			struct d3d_device *device= d3d_device;

			if (device->reference_count==1)
			{
				d3d_device_destroy(device);
				memset(device, 0, sizeof(*device));
				global_d3d_device= NULL;
				d3d_device= NULL;
			}
			else
			{
				device->reference_count--;
				global_d3d_device= NULL;
			}
		}
	}

	return created;
}

/* 0x0011cc20: a data array in the memory that the game takes when it
starts. The array is not valid yet. The name is a guess */
struct data_array *shell_memory_data_new(
	const char *name,
	long maximum_count,
	long size)
{
	struct data_array *data= shell_memory_allocate(
		maximum_count*size + ((maximum_count+31)>>5)*4 + (long)sizeof(struct data_array));

	data_initialize(data, name, maximum_count, size, 0, lruv_blocks_allocation);

	return data;
}

/* 0x0012c040: the buffer that the console's libraries are given takes its
memory, of that which the game takes when it starts, at an address that is a
multiple of 16; and it is not in use. The game has it far from the rest of
this file */
void xmem_static_buffer_allocate(
	void)
{
	byte *end= (byte *)shell_memory_globals.memory+shell_memory_globals.size;
	byte *pointer= (byte *)(((uintptr_t)end+15)&~(uintptr_t)15);
	long size= (long)(pointer-end)+XMEM_STATIC_BUFFER_SIZE;

	shell_memory_globals.size+= size;
	crc_checksum_buffer(&size, sizeof(size), &shell_memory_globals.size_checksum);
	xmem_static_buffer= pointer;
	xmem_static_buffer_used= FALSE;
}
