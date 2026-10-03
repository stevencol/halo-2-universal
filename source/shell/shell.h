/*
SHELL.H

Where the game starts and ends. As Halo 1's shell.c and its file for the
console, which in this game are one file with the first functions of the
executable.
*/

#ifndef __SHELL_H
#define __SHELL_H

#include "memory/data.h"
#include "cseries/cseries.h"
#include "math/real_math.h"

#include <stdint.h>

/* ---------- constants */

enum
{
	SHELL_MEMORY_SIZE= 0x23000,
	SHELL_LAUNCH_NAME_LENGTH= 8,

	/* XGetVideoStandard */
	_xbox_video_standard_pal= 3,

	/* XGetVideoFlags */
	_xbox_video_widescreen_flag= 0x01,
	_xbox_video_480p_flag= 0x08,
	_xbox_video_letterbox_flag= 0x10,
	_xbox_video_pal_60hz_flag= 0x40
};

/* ---------- structures */

/* 0x00485ac0: how the console is set to show the picture. The names are
guesses */
struct xbox_video_settings
{
	/* only set for PAL: 50 or 60 */
	short refresh_rate;
	boolean widescreen;
	boolean letterbox;
	boolean pal;
	/* both are set from the same flag */
	boolean progressive_scan5;
	boolean progressive_scan;
};

/* 0x005107fc: memory that the game takes when it starts, and that its
systems take their own from, one after another: how much is taken, and a
checksum of the sizes */
struct shell_memory_globals
{
	long unknown0;
	void *memory;
	long size;
	unsigned long size_checksum;
};

/* what Direct3D is asked to make the device with (D3DPRESENT_PARAMETERS) */
struct d3d_present_parameters
{
	uint32_t back_buffer_width;
	uint32_t back_buffer_height;
	uint32_t back_buffer_format;
	uint32_t back_buffer_count;
	uint32_t multisample_type;
	uint32_t swap_effect;
	uint32_t device_window;
	uint32_t windowed;
	uint32_t enable_auto_depth_stencil;
	uint32_t auto_depth_stencil_format;
	uint32_t flags;
	uint32_t refresh_rate;
	uint32_t presentation_interval;
	uint32_t buffer_surfaces[3];
	uint32_t depth_stencil_surface;
};

/* the device of Direct3D, which is in the game's executable. Only what the
code here uses */
struct d3d_device
{
	byte unknown0[0x934];
	/* where the registers of the console's graphics are */
	void *registers;
	long reference_count;
	byte unknown93C[0x147C];
	/* called by Direct3D; the game sets it in main_time_initialize */
	void (*frame_callback)(void);
	byte unknown1DBC[0x6E4];
};

typedef char verify_xbox_video_settings_size[
	sizeof(struct xbox_video_settings) == 8 ? 1 : -1];
typedef char verify_d3d_present_parameters_size[
	sizeof(struct d3d_present_parameters) == 0x44 ? 1 : -1];
typedef char verify_d3d_device_size[
	sizeof(struct d3d_device) == 0x24A0 ? 1 : -1];

/* ---------- prototypes/SHELL.C */

struct bitmap_data;

void *__attribute__((stdcall)) XMemAlloc(unsigned long size, unsigned long attributes);
void __attribute__((stdcall)) XMemFree(void *address, unsigned long attributes);
void *__attribute__((stdcall)) xmem_static_allocate(unsigned long size, unsigned long attributes);
struct d3d_texture *shell_bitmap_get_texture_blocking(struct bitmap_data *bitmap, real mip_bias);
struct d3d_texture *shell_bitmap_get_texture(struct bitmap_data *bitmap, real mip_bias);

boolean shell_initialize(void);
void shell_dispose(void);
void shell_main(void);
boolean shell_platform_initialize(void);
void shell_get_video_settings(void);
boolean shell_create_device(void);
boolean shell_test_device(void);

struct data_array *shell_memory_data_new(const char *name, long maximum_count, long size);

/* ---------- globals */

extern struct shell_memory_globals shell_memory_globals;
extern void *xmem_static_buffer;
extern boolean xmem_static_buffer_used;
extern char shell_launch_name[SHELL_LAUNCH_NAME_LENGTH];
extern struct xbox_video_settings xbox_video_settings;
extern struct d3d_device *global_d3d_device;
extern struct d3d_device *d3d_device;
extern boolean application_paused;

/* ---------- inline code */

void crc_checksum_buffer(const void *buffer, long size, unsigned long *checksum);

/* memory of that which the game takes when it starts, at an address that is
a multiple of 4: what is skipped to get to it is taken too. The name is a
guess */
static inline __attribute__((always_inline)) void *shell_memory_allocate(
	long size)
{
	byte *end= (byte *)shell_memory_globals.memory+shell_memory_globals.size;
	byte *pointer= (byte *)(((uintptr_t)end+3)&~(uintptr_t)3);

	size+= (long)(pointer-end);
	shell_memory_globals.size+= size;
	crc_checksum_buffer(&size, sizeof(size), &shell_memory_globals.size_checksum);

	return pointer;
}

#endif
