/*
RASTERIZER_XBOX.H

Initialisation of the Xbox rasterizer: display list, display parameters
and state tables. The name of the file is Halo 1's
(rasterizer/xbox/rasterizer_xbox.c); that these functions belong to it is a
guess, and so are their names.
*/

#ifndef __RASTERIZER_XBOX_H
#define __RASTERIZER_XBOX_H

#include "cseries/cseries.h"
#include "math/real_math.h"

/* 0x00484dd4 has room for 53 entries of 3 longs before the next global */
#define RASTERIZER_TABLE_ENTRIES 53
/* 0x004b4b58 has room for 39 entries of 0x98 bytes before 0x004b6294 */
#define RASTERIZER_DISPLAY_ENTRIES 39
#define RASTERIZER_DISPLAY_ENTRY_SIZE 0x98

/* ---------- globals */

extern long   rasterizer_unknown_005093c0;
extern long   rasterizer_unknown_005093c4;
extern long   rasterizer_unknown_005093c8;
extern long   rasterizer_unknown_005093cc;
extern long   rasterizer_frame_counter_saved_lo;
extern long   rasterizer_frame_counter_saved_hi;
extern long   rasterizer_last_table_entry;
extern long   rasterizer_table_count;
extern long   rasterizer_table_00484dd4[];
extern long   rasterizer_display_initialized;
extern long   rasterizer_constant_00467014;
extern long   rasterizer_constant_00467018;
extern long   rasterizer_unknown_00485ad4;
extern long   rasterizer_unknown_00485ad8;
extern real   rasterizer_screen_height;
extern real   rasterizer_screen_width;
extern short  rasterizer_clip_unknown_00485a8a;
extern short  rasterizer_clip_unknown_00485a8c;
extern short  rasterizer_clip_unknown_00485a8e;
extern short  rasterizer_clip_unknown_00485a90;
extern short  rasterizer_clip_unknown_00485a92;
extern short  rasterizer_clip_unknown_00485a94;
extern short  rasterizer_clip_unknown_00485a96;
extern short  rasterizer_clip_unknown_00485a98;
extern byte   rasterizer_display_entries[RASTERIZER_DISPLAY_ENTRIES][RASTERIZER_DISPLAY_ENTRY_SIZE];
extern long   rasterizer_primary_table_count;
extern byte   rasterizer_primary_table[];
extern long   rasterizer_secondary_table_count;
extern byte   rasterizer_secondary_table_flag;
extern long   rasterizer_secondary_table_unknown_004aeb24;
extern long   rasterizer_secondary_table_unknown_004aeb28;
extern long   rasterizer_secondary_table_unknown_004aeb2c;
extern long   rasterizer_secondary_table_unknown_004aeb50;
extern byte   rasterizer_secondary_table[];
extern long   rasterizer_state_flags_00485b78;
extern long   rasterizer_state_flags_00485b7c;
extern long   rasterizer_state_flags_00485b80;
extern long   rasterizer_state_flags_00485b84;
extern long   rasterizer_state_flags_00485b88;
extern long   rasterizer_flags_group0_00485074;
extern long   rasterizer_flags_group0_00485078;
extern long   rasterizer_flags_group0_004850a4;
extern long   rasterizer_flags_group0_004850a8;
extern long   rasterizer_flags_group0_00485058;
extern long   rasterizer_flags_group1_00485084;
extern long   rasterizer_flags_group1_00485088;
extern long   rasterizer_flags_group1_004850b4;
extern long   rasterizer_flags_group1_004850b8;
extern long   rasterizer_flags_group1_00485064;
extern long   rasterizer_display_table_ptr;
extern long   rasterizer_ptr_table_004670ec;
extern long   rasterizer_pitch_table_004670f4;

/* ---------- prototypes/RASTERIZER_XBOX.C */

void __attribute__((stdcall)) rasterizer_store_entry(long *entry, long arg_ecx, long arg_stack);
long    rasterizer_count_since_entry_change(long entry_ptr);
long    rasterizer_display_entry_get_value(long entry_index, long value_offset);
void    rasterizer_display_initialize(void);
void    rasterizer_set_clip_coords(real x, short clip_x, short clip_y);
byte    rasterizer_get_display_flag(void);
void    rasterizer_accumulate_state_flags_group0(void);
void    rasterizer_accumulate_state_flags_group1(void);
void    rasterizer_display_tables_initialize(void);
long    rasterizer_primary_table_get(long raw_index);
long __attribute__((stdcall)) rasterizer_make_texture_format(long format, byte flag, byte linear, long width, long height);
byte   *rasterizer_display_entry_get_name(long entry_index);
byte   *rasterizer_display_entry_get_mip_list(long entry_index);
byte   *rasterizer_display_entry_get_mip(long entry_index, long mip_index);
boolean rasterizer_display_entry_get_size(long entry_index, long *out_width, long *out_height);
void    rasterizer_display_entry_set_size(long entry_index, long width, long height);
boolean rasterizer_display_entry_is_active(long entry_index);

#endif
