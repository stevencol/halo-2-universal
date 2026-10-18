/*
RASTERIZER_XBOX.C

Decompiled from halo2ship.exe (Xbox, build 02.09.27.09809). Each function
gives the address it has there.

Initialisation of the Xbox rasterizer: the display list, the display
parameters and several state tables that back the draw calls. The name of
the file is Halo 1's (rasterizer/xbox/rasterizer_xbox.c); that these
functions belong to it is a guess, and so are their names.
*/

#include "rasterizer/rasterizer_xbox.h"

#include "cache/texture_cache.h"
#include "main/main.h"
#include "shell/shell.h"
#include "game/game.h"

/* ---------- globals */

/* 0x005093c0, 0x005093c4: two values loaded from a table entry */
long rasterizer_unknown_005093c0;
long rasterizer_unknown_005093c4;

/* 0x005093c8: index passed to 0x00013d50 via ecx */
long rasterizer_unknown_005093c8;

/* 0x005093cc: second argument passed to 0x00013d50 */
long rasterizer_unknown_005093cc;

/* 0x00485ab0, 0x00485ab4: 64-bit vblank counter — this is main_vblank_count
   from main/main.h; the rasterizer uses its low long directly */

/* 0x00485ab8, 0x00485abc: saved value of the vblank counter when the
entry last changed (lo and hi longs) */
long rasterizer_frame_counter_saved_lo;
long rasterizer_frame_counter_saved_hi;

/* 0x0055e6b8: the table entry address that was used on the last call */
long rasterizer_last_table_entry;

/* 0x005093ac: number of valid entries in the table at 0x484dd4 */
long rasterizer_table_count;

/* 0x00484dd4: base of a table of 3-long entries (ptr, ?, ?) */
long rasterizer_table_00484dd4[RASTERIZER_TABLE_ENTRIES * 3];

/* 0x00485aa0: set to 1 when the display is initialised */
long rasterizer_display_initialized;

/* 0x00485aa4: always set to 0 during initialisation */
long rasterizer_unknown_00485aa4;

/* 0x00485ac2: fallback value returned when no valid display entry exists */
byte rasterizer_display_unknown_flag;

/* 0x00467014, 0x00467018: constants loaded into 0x485ad4/d8 at startup */
long rasterizer_constant_00467014; /* bits of 0.015625 */
long rasterizer_constant_00467018; /* bits of 1024.0 */

/* 0x00485ad4, 0x00485ad8: copies of the two constants above */
long rasterizer_unknown_00485ad4;
long rasterizer_unknown_00485ad8;

/* 0x0045dd7c, 0x0045dd80: screen dimensions used to compute clip coords */
real rasterizer_screen_height; /* = 480.0 */
real rasterizer_screen_width;  /* = 640.0 */

/* 0x00485a8a .. 0x00485a98: clip-space coordinates (shorts) */
short rasterizer_clip_unknown_00485a8a;
short rasterizer_clip_unknown_00485a8c;
short rasterizer_clip_unknown_00485a8e;
short rasterizer_clip_unknown_00485a90;
short rasterizer_clip_unknown_00485a92;
short rasterizer_clip_unknown_00485a94;
short rasterizer_clip_unknown_00485a96;
short rasterizer_clip_unknown_00485a98;

/* 0x004e6948: rasterizer uses main_game_globals (game/game.h) as a pointer
   to inspect device-state bytes at +1, +2 and +8 */

/* 0x004b4b58: base of the display-entry array, stride 0x98 each */
byte rasterizer_display_entries[RASTERIZER_DISPLAY_ENTRIES][RASTERIZER_DISPLAY_ENTRY_SIZE];

/* 0x00487b10: set to 1 when the primary display table is initialised */
byte rasterizer_primary_table_initialized;

/* 0x00487b18: count of entries in the primary display table */
long rasterizer_primary_table_count;

/* 0x00487b1c: primary display table data (0x9c00 longs = 159 KB) */
byte rasterizer_primary_table[0x9c00 * 4];

/* 0x004aeb1c: count in the secondary display table */
long rasterizer_secondary_table_count;

/* 0x004aeb20: flag in the secondary table */
byte rasterizer_secondary_table_flag;

/* 0x004aeb24 .. 0x004aeb2c, 0x004aeb50: secondary table misc fields */
long rasterizer_secondary_table_unknown_004aeb24;
long rasterizer_secondary_table_unknown_004aeb28;
long rasterizer_secondary_table_unknown_004aeb2c;
long rasterizer_secondary_table_unknown_004aeb50;

/* 0x004aeb54: secondary display table data (0x1800 longs = 24 KB) */
byte rasterizer_secondary_table[0x1800 * 4];

/* 0x00485b78 .. 0x00485b88: accumulated OR of state flags (five longs) */
long rasterizer_state_flags_00485b78;
long rasterizer_state_flags_00485b7c;
long rasterizer_state_flags_00485b80;
long rasterizer_state_flags_00485b84;
long rasterizer_state_flags_00485b88;

/* groups of five longs ORed into the state flags by the two accumulate fns */
long rasterizer_flags_group0_00485074;
long rasterizer_flags_group0_00485078;
long rasterizer_flags_group0_004850a4;
long rasterizer_flags_group0_004850a8;
long rasterizer_flags_group0_00485058;

long rasterizer_flags_group1_00485084;
long rasterizer_flags_group1_00485088;
long rasterizer_flags_group1_004850b4;
long rasterizer_flags_group1_004850b8;
long rasterizer_flags_group1_00485064;

/* 0x004e3b44: pointer used by the display-entry accessors */
long rasterizer_display_table_ptr;

/* 0x004670ec: table of pointers used when display_index == NONE */
long rasterizer_ptr_table_004670ec;

/* 0x004670f4: companion table of pitch values */
long rasterizer_pitch_table_004670f4;

/* ---------- code */

/* 0x00013d50: stores a table entry's two words and two extra arguments into
four globals; the entry address is in EAX on entry, its first two longs go
to 005093c0/c4; ecx goes to 005093c8; the stack argument, which it pops,
goes to 005093cc */
void __attribute__((stdcall)) rasterizer_store_entry(
	long *entry,
	long arg_ecx,
	long arg_stack)
{
	rasterizer_unknown_005093c0 = entry[0];
	rasterizer_unknown_005093c4 = entry[1];
	rasterizer_unknown_005093c8 = arg_ecx;
	rasterizer_unknown_005093cc = arg_stack;
}

/* 0x00014280: increments the vblank counter (main_vblank_count) and returns
how many ticks have elapsed since the table entry last changed; the entry
pointer is in ecx, and its second long (at ecx+4) is compared with the
saved address */
long rasterizer_count_since_entry_change(
	long entry_ptr)
{
	long count = 0;

	main_vblank_count++;

	if (*(long *)(entry_ptr + 4) != rasterizer_last_table_entry)
	{
		count = (long)main_vblank_count - rasterizer_frame_counter_saved_lo;
		rasterizer_last_table_entry       = *(long *)(entry_ptr + 4);
		rasterizer_frame_counter_saved_lo = (long)main_vblank_count;
		rasterizer_frame_counter_saved_hi = (long)(main_vblank_count >> 32);
	}
	return count;
}

/* 0x00015cd0: looks up a value from the display-entry table; ecx is the
entry index (0-based), edx is the byte offset (0-255) within the entry data;
returns 0xff4060 if the entry or its data is not valid */
long rasterizer_display_entry_get_value(
	long entry_index,
	long value_offset)
{
	long *entry;
	long data_ptr;

	if (entry_index < 0 || entry_index >= rasterizer_table_count)
		return 0xFF4060;

	entry = &rasterizer_table_00484dd4[entry_index * 3];
	/* the game ORs the high bit (a physical address) and its je after the or
	can never be taken, so there is no check for a null entry */
	data_ptr = *entry | 0x80000000;

	return *(long *)(data_ptr + (value_offset & 0xFF) * 4);
}

/* 0x000169f0: initialises the display globals at startup: marks it as
initialised, copies the two constants, and sets the refresh interval to 60
if it has not been set */
void rasterizer_display_initialize(
	void)
{
	rasterizer_display_initialized = 1;
	rasterizer_unknown_00485aa4    = 0;
	rasterizer_unknown_00485ad4    = rasterizer_constant_00467014;
	rasterizer_unknown_00485ad8    = rasterizer_constant_00467018;
	if (xbox_video_settings.refresh_rate == 0)
		xbox_video_settings.refresh_rate = 0x3C;
}

/* 0x00016a30: converts a screen position into clip-space shorts and stores
them into the eight clip globals; xmm0 = x (float), edx = clip_x (short),
ecx = clip_y (short); uses 640.0 and 480.0 constants */
void rasterizer_set_clip_coords(
	real x,
	short clip_x,
	short clip_y)
{
	rasterizer_clip_unknown_00485a8a = 0;
	rasterizer_clip_unknown_00485a8c = 0;

	rasterizer_clip_unknown_00485a90 = (short)(long)(x * rasterizer_screen_width);
	rasterizer_clip_unknown_00485a8e = (short)(long)(x * rasterizer_screen_height);

	rasterizer_clip_unknown_00485a98 = (short)(rasterizer_clip_unknown_00485a90 - clip_x);
	rasterizer_clip_unknown_00485a94 = clip_x;
	rasterizer_clip_unknown_00485a92 = clip_y;
	rasterizer_clip_unknown_00485a96 = (short)(rasterizer_clip_unknown_00485a8e - clip_y);
}

/* 0x00016a90: returns the display unknown flag; if the display descriptor is
valid and live (byte at +1 non-zero, short at +2 not -1, long at +8 == 3)
returns 0, otherwise returns the fallback byte */
byte rasterizer_get_display_flag(
	void)
{
	byte *desc = (byte *)main_game_globals;

	if (desc
		&& *(byte  *)(desc + 1) != 0
		&& *(short *)(desc + 2) != (short)-1
		&& *(long  *)(desc + 8) == 3)
	{
		return 0;
	}
	return rasterizer_display_unknown_flag;
}

/* 0x0001bdf0: ORs group-0 flags into the five state-flag globals */
void rasterizer_accumulate_state_flags_group0(
	void)
{
	rasterizer_state_flags_00485b78 |= rasterizer_flags_group0_00485074;
	rasterizer_state_flags_00485b7c |= rasterizer_flags_group0_00485078;
	rasterizer_state_flags_00485b80 |= rasterizer_flags_group0_004850a4;
	rasterizer_state_flags_00485b84 |= rasterizer_flags_group0_004850a8;
	rasterizer_state_flags_00485b88 |= rasterizer_flags_group0_00485058;
}

/* 0x0001be50: ORs group-1 flags into the five state-flag globals */
void rasterizer_accumulate_state_flags_group1(
	void)
{
	rasterizer_state_flags_00485b78 |= rasterizer_flags_group1_00485084;
	rasterizer_state_flags_00485b7c |= rasterizer_flags_group1_00485088;
	rasterizer_state_flags_00485b80 |= rasterizer_flags_group1_004850b4;
	rasterizer_state_flags_00485b84 |= rasterizer_flags_group1_004850b8;
	rasterizer_state_flags_00485b88 |= rasterizer_flags_group1_00485064;
}

/* 0x0001d660: zeroes both display tables and marks the primary one as
initialised */
void rasterizer_display_tables_initialize(
	void)
{
	long i;

	rasterizer_primary_table_count = 0;
	rasterizer_secondary_table_count = 0;
	for (i = 0; i < 0x9C00; i++)
		((long *)rasterizer_primary_table)[i] = 0;
	rasterizer_secondary_table_flag = 0;
	rasterizer_secondary_table_unknown_004aeb24 = 0;
	rasterizer_secondary_table_unknown_004aeb28 = 0;
	rasterizer_secondary_table_unknown_004aeb2c = 0;
	rasterizer_secondary_table_unknown_004aeb50 = 0;
	for (i = 0; i < 0x1800; i++)
		((long *)rasterizer_secondary_table)[i] = 0;
	rasterizer_primary_table_initialized = 1;
}

/* 0x0001d6b0: given a raw index (low 28 bits used), returns a pointer into
rasterizer_primary_table, or NULL if the index is NONE (0xFFFFFFFF) or
negative */
long rasterizer_primary_table_get(
	long raw_index)
{
	if (raw_index == (long)0xFFFFFFFF || raw_index < 0)
		return 0;
	return (long)rasterizer_primary_table + (raw_index & 0x0FFFFFFF);
}

/* 0x0001d6f0: builds a D3D/NV2A texture-format word: the format in bits 16
and up, a byte that depends on the two flags, 0x29, and, when the texture is
not linear, log2 of the width and the height (at least 1) in bits 20-27 */
long __attribute__((stdcall)) rasterizer_make_texture_format(
	long format,
	byte flag,
	byte linear,
	long width,
	long height)
{
	long result;
	long width_log;
	long height_log;

	if (linear)
		result = (flag ? 0x1C : 0) + 0x12;
	else
		result = (flag ? 0x28 : 0) + 6;
	result = (((format << 8) | result) << 8) | 0x29;

	if (!linear)
	{
		width_log = 1;
		if (width > 2)
		{
			do
				width_log++;
			while ((1 << width_log) < width);
		}
		height_log = 1;
		if (height > 2)
		{
			do
				height_log++;
			while ((1 << height_log) < height);
		}
		result |= ((height_log << 4) | width_log) << 20;
	}
	return result;
}

/* ---------- display-entry accessors (base rasterizer_display_entries,
   stride 0x98 each).

   Fields at these byte offsets within each entry:
     +0x88  active flag long (non-zero = entry in use)
     +0x95  disposed byte (non-zero = entry released)
     +0x78  mip count (long)
     +0x80  width (long)
     +0x84  height (long)
     +0x94  pitch flag byte
     +0x14  D3D format word (long)
     +0x28  secondary format word (long)
   All functions receive the entry index in EAX. */

/* 0x0001dcc0: returns a pointer to offset +0x04 of the entry (the name/data
pointer field), or NULL if the entry is inactive or disposed */
byte *rasterizer_display_entry_get_name(
	long entry_index)
{
	byte *entry = rasterizer_display_entries[entry_index];

	if (*(long *)(entry + 0x88) == 0)
		return NULL;
	if (*(byte *)(entry + 0x95) != 0)
		return NULL;
	return entry + 0x04;
}

/* 0x0001dcf0: returns a pointer to offset +0x18 of the entry (the mip-list
pointer field), or NULL if inactive, disposed, or mip count <= 0 */
byte *rasterizer_display_entry_get_mip_list(
	long entry_index)
{
	byte *entry = rasterizer_display_entries[entry_index];

	if (*(long *)(entry + 0x88) == 0)
		return NULL;
	if (*(byte *)(entry + 0x95) != 0)
		return NULL;
	if (*(long *)(entry + 0x78) <= 0)
		return NULL;
	return entry + 0x18;
}

/* 0x0001dd20: returns a pointer to the n-th mip record of the entry (0x18
bytes each, from +0x18), or NULL if inactive, disposed, or mip_index out of
range */
byte *rasterizer_display_entry_get_mip(
	long entry_index,
	long mip_index)
{
	byte *entry = rasterizer_display_entries[entry_index];

	if (*(long *)(entry + 0x88) == 0)
		return NULL;
	if (*(byte *)(entry + 0x95) != 0)
		return NULL;
	if (mip_index < 0 || mip_index >= *(long *)(entry + 0x78))
		return NULL;
	return entry + (mip_index * 3 + 3) * 8;
}

/* 0x0001dd60: returns the width and height of the entry via two output
pointers; for inactive/disposed entries it uses default values by index:
indices 0, 3 and 0x18 get 640 x 480, others get 0 x 0; returns TRUE if
values came from the entry data */
boolean rasterizer_display_entry_get_size(
	long entry_index,
	long *out_width,
	long *out_height)
{
	byte *entry = rasterizer_display_entries[entry_index];

	if (*(long *)(entry + 0x88) != 0 && *(byte *)(entry + 0x95) == 0)
	{
		*out_width  = *(long *)(entry + 0x80);
		*out_height = *(long *)(entry + 0x84);
		return TRUE;
	}
	if (entry_index == 0 || entry_index == 3 || entry_index == 0x18)
	{
		*out_width  = 0x280;
		*out_height = 0x1E0;
		return TRUE;
	}
	*out_width  = 0;
	*out_height = 0;
	return FALSE;
}

/* 0x0001ddd0: stores width (ESI) and height (EDI) into the entry and
computes the D3D format word from them; if the pitch flag (+0x94) is clear
the format word is zeroed */
void rasterizer_display_entry_set_size(
	long entry_index,
	long width,
	long height)
{
	byte *entry = rasterizer_display_entries[entry_index];
	long fmt;

	*(long *)(entry + 0x80) = width;
	*(long *)(entry + 0x84) = height;

	fmt = (((unsigned long)(width * 4) >> 6) * 0x1000000 - 0x1000000)
		| ((long)height * 0x1000 - 0x1000)
		| (width - 1);

	if (*(byte *)(entry + 0x94) == 0)
		fmt = 0;

	*(long *)(entry + 0x14) = fmt;
	*(long *)(entry + 0x28) = fmt;
}

/* 0x0001de20: returns TRUE if the entry is active and not disposed,
FALSE otherwise */
boolean rasterizer_display_entry_is_active(
	long entry_index)
{
	byte *entry = rasterizer_display_entries[entry_index];

	if (*(long *)(entry + 0x88) != 0 && *(byte *)(entry + 0x95) == 0)
		return TRUE;
	return FALSE;
}
/* 0x00025960: 20 if the display entry 20 is active, if not 18 if the entry
18 is, and NONE if neither (rasterizer_display_entry_is_active, written out
by the compiler) */
long rasterizer_unknown_00025960(
	void)
{
	if (*(long *)(rasterizer_display_entries[20] + 0x88)!=0 && rasterizer_display_entries[20][0x95]==0)
	{
		return 20;
	}
	if (*(long *)(rasterizer_display_entries[18] + 0x88)!=0 && rasterizer_display_entries[18][0x95]==0)
	{
		return 18;
	}

	return NONE;
}
