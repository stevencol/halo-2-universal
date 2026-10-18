# mutations of milestone 738 (rasterizer_xbox.c): one line of each function

TESTS = "test_small_738"

R = "source/rasterizer/rasterizer_xbox.c"

MUTATIONS = [
    (R, "rasterizer_unknown_005093c4 = entry[1];", "rasterizer_unknown_005093c4 = entry[0];"),
    (R, "rasterizer_unknown_005093cc = arg_stack;", "rasterizer_unknown_005093cc = arg_ecx;"),
    (R, "\tmain_vblank_count++;", "\tmain_vblank_count += 2;"),
    (R, "rasterizer_frame_counter_saved_hi = (long)(main_vblank_count >> 32);", "rasterizer_frame_counter_saved_hi = (long)main_vblank_count;"),
    (R, "\tdata_ptr = *entry | 0x80000000;", "\tdata_ptr = entry[1] | 0x80000000;"),
    (R, "return *(long *)(data_ptr + (value_offset & 0xFF) * 4);", "return *(long *)(data_ptr + (value_offset & 0x7F) * 4);"),
    (R, "\trasterizer_unknown_00485ad8    = rasterizer_constant_00467018;", "\trasterizer_unknown_00485ad8    = rasterizer_constant_00467014;"),
    (R, "\t\txbox_video_settings.refresh_rate = 0x3C;", "\t\txbox_video_settings.refresh_rate = 0x32;"),
    (R, "rasterizer_clip_unknown_00485a96 = (short)(rasterizer_clip_unknown_00485a8e - clip_y);", "rasterizer_clip_unknown_00485a96 = (short)(rasterizer_clip_unknown_00485a8e - clip_x);"),
    (R, "\t\t&& *(long  *)(desc + 8) == 3)", "\t\t&& *(long  *)(desc + 8) == 2)"),
    (R, "rasterizer_state_flags_00485b84 |= rasterizer_flags_group0_004850a8;", "rasterizer_state_flags_00485b84 |= rasterizer_flags_group0_004850a4;"),
    (R, "rasterizer_state_flags_00485b88 |= rasterizer_flags_group1_00485064;", "rasterizer_state_flags_00485b88 = rasterizer_flags_group1_00485064;"),
    (R, "\tfor (i = 0; i < 0x1800; i++)", "\tfor (i = 0; i < 0x17FF; i++)"),
    (R, "\treturn (long)rasterizer_primary_table + (raw_index & 0x0FFFFFFF);", "\treturn (long)rasterizer_primary_table + (raw_index & 0x1FFFFFFF);"),
    (R, "\t\tresult = (flag ? 0x28 : 0) + 6;", "\t\tresult = (flag ? 0x1C : 0) + 6;"),
    (R, "\t\t\twhile ((1 << height_log) < height);", "\t\t\twhile ((1 << height_log) <= height);"),
    (R, "\treturn entry + 0x04;", "\treturn entry + 0x08;"),
    (R, "\tif (*(long *)(entry + 0x78) <= 0)", "\tif (*(long *)(entry + 0x78) < 0)"),
    (R, "\treturn entry + (mip_index * 3 + 3) * 8;", "\treturn entry + (mip_index * 3 + 2) * 8;"),
    (R, "\tif (entry_index == 0 || entry_index == 3 || entry_index == 0x18)", "\tif (entry_index == 0 || entry_index == 3)"),
    (R, "\tif (*(byte *)(entry + 0x94) == 0)", "\tif (*(byte *)(entry + 0x94) != 0)"),
    (R, "\t*(long *)(entry + 0x28) = fmt;", "\t*(long *)(entry + 0x24) = fmt;"),
]
