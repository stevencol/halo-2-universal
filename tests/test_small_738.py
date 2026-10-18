# seventeen small rasterizer functions (rasterizer_xbox.c)

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, GlobalData, Memory

NONE = 0xFFFFFFFF
ENTRY_STRIDE = 0x98  # bytes per display entry
DISPLAY_ENTRIES = 39  # rasterizer_display_entries
TABLE_ENTRIES = 53  # rasterizer_table_00484dd4


def entry(random_, *, active=None, disposed=None, width=None, height=None,
          mip_count=None, pitch_flag=None):
    """Build a single display-entry blob of ENTRY_STRIDE bytes."""
    data = bytearray(random_.randbytes(ENTRY_STRIDE))
    # +0x88: active flag (long)
    a = random_.choice((0, 0, 1, random_.getrandbits(32))) if active is None else (1 if active else 0)
    struct.pack_into("<I", data, 0x88, a)
    # +0x95: disposed flag (byte)
    d = random_.choice((0, 0, 1)) if disposed is None else (1 if disposed else 0)
    data[0x95] = d
    # +0x78: mip count
    mc = random_.choice((0, 1, 4, 16, random_.randrange(1, 64))) if mip_count is None else mip_count
    struct.pack_into("<i", data, 0x78, mc)
    # +0x80/0x84: width/height
    w = random_.choice((1, 2, 4, 64, 128, 256, 512, 640, random_.randrange(1, 1024))) if width is None else width
    h = random_.choice((1, 2, 4, 64, 128, 256, 480, 512, random_.randrange(1, 1024))) if height is None else height
    struct.pack_into("<I", data, 0x80, w)
    struct.pack_into("<I", data, 0x84, h)
    # +0x94: pitch flag
    pf = random_.choice((0, 0, 1)) if pitch_flag is None else pitch_flag
    data[0x94] = pf
    return bytes(data)


def make_entries(random_, count=8):
    """Build a buffer with `count` display entries."""
    return Buffer(b"".join(entry(random_) for _ in range(count)))


def run(t) -> None:
    random_ = random.Random(738)

    COUNT = 6  # number of display entries in the table for most tests

    for n in range(300):
        # ----------------------------------------------------------------
        # rasterizer_store_entry  0x00013d50
        # eax = pointer to entry (two longs), ecx = arg_ecx, stack = arg_stack
        # ----------------------------------------------------------------
        two_longs = Buffer(struct.pack("<II", random_.getrandbits(32), random_.getrandbits(32)))
        t.check(
            "rasterizer_store_entry",
            two_longs,                 # eax → entry[0], entry[1]
            random_.getrandbits(32),   # ecx
            random_.getrandbits(32),   # stack
            globals={
                "rasterizer_unknown_005093c0": random_.getrandbits(32),
                "rasterizer_unknown_005093c4": random_.getrandbits(32),
                "rasterizer_unknown_005093c8": random_.getrandbits(32),
                "rasterizer_unknown_005093cc": random_.getrandbits(32),
            },
        )

        # ----------------------------------------------------------------
        # rasterizer_count_since_entry_change  0x00014280
        # ecx = pointer to struct where [ecx+4] is compared with the saved entry
        # ----------------------------------------------------------------
        saved = random_.choice((0, 1, random_.getrandbits(32)))
        entry_val = random_.choice((saved, saved, random_.getrandbits(32)))
        ctr = random_.getrandbits(63)  # signed 64-bit, keep positive
        ep = Buffer(struct.pack("<II", random_.getrandbits(32), entry_val))
        t.check(
            "rasterizer_count_since_entry_change",
            ep,
            globals={
                "main_vblank_count": struct.pack("<q", ctr),
                "rasterizer_frame_counter_saved_lo": ctr & 0xFFFFFFFF,
                "rasterizer_frame_counter_saved_hi": (ctr >> 32) & 0xFFFFFFFF,
                "rasterizer_last_table_entry": saved,
            },
        )

        # ----------------------------------------------------------------
        # rasterizer_display_entry_get_value  0x00015cd0
        # ecx = entry_index, edx = value_offset (byte index)
        # ----------------------------------------------------------------
        tcount = random_.choice((0, 1, COUNT, TABLE_ENTRIES))
        # each entry is 3 longs; the first one, ORed with 0x80000000, is a
        # physical address of 0x100 longs (some of them already have the bit)
        table_entries = []
        for i in range(TABLE_ENTRIES):
            data = PHYSICAL_ADDRESS + 0x70000 + i * 0x400
            table_entries.append(struct.pack("<III", data & random_.choice((0x7FFFFFFF, NONE)),
                                             random_.getrandbits(32), random_.getrandbits(32)))
        table = b"".join(table_entries)
        idx = random_.choice((-1, 0, tcount - 1, tcount, TABLE_ENTRIES, random_.randrange(0, TABLE_ENTRIES)))
        offset = random_.choice((0, 1, 0x7F, 0xFF, 0x100, NONE, random_.getrandbits(32)))
        t.check(
            "rasterizer_display_entry_get_value",
            idx,
            offset,
            globals={
                "rasterizer_table_count": tcount,
                "rasterizer_table_00484dd4": table,
            },
            memory={PHYSICAL_ADDRESS + 0x70000: Memory(random_.randbytes(TABLE_ENTRIES * 0x400))},
        )

        # ----------------------------------------------------------------
        # rasterizer_display_initialize  0x000169f0
        # ----------------------------------------------------------------
        t.check(
            "rasterizer_display_initialize",
            globals={
                "rasterizer_display_initialized": random_.getrandbits(32),
                "rasterizer_unknown_00485aa4":    random_.getrandbits(32),
                "rasterizer_unknown_00485ad4":    random_.getrandbits(32),
                "rasterizer_unknown_00485ad8":    random_.getrandbits(32),
                "xbox_video_settings": struct.pack("<h", random_.choice((0, 0, 0x3c, random_.randrange(1, 0x100)))) + random_.randbytes(8),
                "rasterizer_constant_00467014": random_.getrandbits(32),
                "rasterizer_constant_00467018": random_.getrandbits(32),
            },
        )

        # ----------------------------------------------------------------
        # rasterizer_set_clip_coords  0x00016a30
        # xmm0 = x (float), edx = clip_x (short), ecx = clip_y (short)
        # ----------------------------------------------------------------
        x_val = random_.choice((0.0, 0.5, 1.0, -1.0, random_.uniform(-2.0, 2.0)))
        cx = random_.randrange(-32768, 32768)
        cy = random_.randrange(-32768, 32768)
        t.check(
            "rasterizer_set_clip_coords",
            struct.unpack("<I", struct.pack("<f", x_val))[0],  # xmm0 as int
            cx & 0xFFFF,
            cy & 0xFFFF,
            globals={
                "rasterizer_screen_width":  struct.pack("<f", 640.0),
                "rasterizer_screen_height": struct.pack("<f", 480.0),
                "rasterizer_clip_unknown_00485a8a": struct.pack("<h", random_.randrange(-32768, 32768)),
                "rasterizer_clip_unknown_00485a8c": struct.pack("<h", random_.randrange(-32768, 32768)),
                "rasterizer_clip_unknown_00485a8e": struct.pack("<h", random_.randrange(-32768, 32768)),
                "rasterizer_clip_unknown_00485a90": struct.pack("<h", random_.randrange(-32768, 32768)),
                "rasterizer_clip_unknown_00485a92": struct.pack("<h", random_.randrange(-32768, 32768)),
                "rasterizer_clip_unknown_00485a94": struct.pack("<h", random_.randrange(-32768, 32768)),
                "rasterizer_clip_unknown_00485a96": struct.pack("<h", random_.randrange(-32768, 32768)),
                "rasterizer_clip_unknown_00485a98": struct.pack("<h", random_.randrange(-32768, 32768)),
            },
        )

        # ----------------------------------------------------------------
        # rasterizer_get_display_flag  0x00016a90
        # ----------------------------------------------------------------
        def make_desc():
            d = bytearray(12)
            d[1] = random_.choice((0, 0, 1, 0xFF))
            struct.pack_into("<h", d, 2, random_.choice((-1, 0, 1, 0x7FFF)))
            struct.pack_into("<I", d, 8, random_.choice((0, 1, 2, 3, 4)))
            return Buffer(bytes(d))

        t.check(
            "rasterizer_get_display_flag",
            globals={
                "main_game_globals": random_.choice((0, make_desc())),
                "rasterizer_display_unknown_flag": bytes((random_.choice((0, 1, 0xFF)),)),
            },
        )

        # ----------------------------------------------------------------
        # rasterizer_accumulate_state_flags_group0  0x0001bdf0
        # rasterizer_accumulate_state_flags_group1  0x0001be50
        # ----------------------------------------------------------------
        flags5 = lambda: {k: random_.getrandbits(32) for k in (
            "rasterizer_state_flags_00485b78",
            "rasterizer_state_flags_00485b7c",
            "rasterizer_state_flags_00485b80",
            "rasterizer_state_flags_00485b84",
            "rasterizer_state_flags_00485b88",
        )}
        g0 = lambda: {
            "rasterizer_flags_group0_00485074": random_.getrandbits(32),
            "rasterizer_flags_group0_00485078": random_.getrandbits(32),
            "rasterizer_flags_group0_004850a4": random_.getrandbits(32),
            "rasterizer_flags_group0_004850a8": random_.getrandbits(32),
            "rasterizer_flags_group0_00485058": random_.getrandbits(32),
        }
        g1 = lambda: {
            "rasterizer_flags_group1_00485084": random_.getrandbits(32),
            "rasterizer_flags_group1_00485088": random_.getrandbits(32),
            "rasterizer_flags_group1_004850b4": random_.getrandbits(32),
            "rasterizer_flags_group1_004850b8": random_.getrandbits(32),
            "rasterizer_flags_group1_00485064": random_.getrandbits(32),
        }
        t.check("rasterizer_accumulate_state_flags_group0", globals={**flags5(), **g0()})
        t.check("rasterizer_accumulate_state_flags_group1", globals={**flags5(), **g1()})

        # ----------------------------------------------------------------
        # rasterizer_display_tables_initialize  0x0001d660
        # ----------------------------------------------------------------
        t.check(
            "rasterizer_display_tables_initialize",
            globals={
                "rasterizer_primary_table_count":    random_.getrandbits(32),
                "rasterizer_secondary_table_count":  random_.getrandbits(32),
                "rasterizer_primary_table":          random_.randbytes(0x9C00 * 4),
                "rasterizer_secondary_table":        random_.randbytes(0x1800 * 4),
                "rasterizer_primary_table_initialized": bytes((random_.choice((0, 1)),)),
                "rasterizer_secondary_table_flag":      bytes((random_.choice((0, 1)),)),
                "rasterizer_secondary_table_unknown_004aeb24": random_.getrandbits(32),
                "rasterizer_secondary_table_unknown_004aeb28": random_.getrandbits(32),
                "rasterizer_secondary_table_unknown_004aeb2c": random_.getrandbits(32),
                "rasterizer_secondary_table_unknown_004aeb50": random_.getrandbits(32),
            },
        )

        # ----------------------------------------------------------------
        # rasterizer_primary_table_get  0x0001d6b0
        # ecx = raw_index
        # ----------------------------------------------------------------
        raw = random_.choice((
            NONE, 0, 1, 0x9C00 * 4 - 1, 0x10000000, 0x10000100,
            random_.randrange(0, 0x9C00 * 4),
            random_.getrandbits(32) & 0xF000FFFF,
        ))
        # inside the table, the pointer is compared as a place in it
        t.check(
            "rasterizer_primary_table_get",
            raw,
            globals={"rasterizer_primary_table": GlobalData(random_.randbytes(0x9C00 * 4))},
        )

        # ----------------------------------------------------------------
        # rasterizer_make_texture_format  0x0001d6f0
        # eax = format, cl = flag, dl = linear, stack = width, height
        # ----------------------------------------------------------------
        sizes = (-1, 0, 1, 2, 3, 4, 5, 64, 100, 480, 512, 640, 0x10000)
        t.check(
            "rasterizer_make_texture_format",
            random_.getrandbits(32),
            random_.choice((0, 1, 0x100, random_.getrandbits(32))),
            random_.choice((0, 1, 0x100, random_.getrandbits(32))),
            random_.choice(sizes + (random_.randrange(1, 0x10000),)),
            random_.choice(sizes + (random_.randrange(1, 0x10000),)),
        )

        # ----------------------------------------------------------------
        # display-entry accessors  0x0001dcc0 .. 0x0001de20
        # All take entry_index in EAX (via ecx for the few that need it);
        # the global is given whole, DISPLAY_ENTRIES entries.
        # ----------------------------------------------------------------
        entries_buf = GlobalData(b"".join(entry(random_) for _ in range(DISPLAY_ENTRIES)))
        idx_e = random_.choice((0, 1, 3, 0x18, DISPLAY_ENTRIES - 1, random_.randrange(0, DISPLAY_ENTRIES)))

        t.check("rasterizer_display_entry_get_name",     idx_e, globals={"rasterizer_display_entries": entries_buf})
        t.check("rasterizer_display_entry_get_mip_list", idx_e, globals={"rasterizer_display_entries": entries_buf})

        mip_idx = random_.choice((-1, 0, 1, 3, random_.randrange(0, 8)))
        # the last entry apart: a mip past its end would point past the global
        t.check("rasterizer_display_entry_get_mip",      min(idx_e, DISPLAY_ENTRIES - 2), mip_idx,
                globals={"rasterizer_display_entries": entries_buf})

        out_w = Buffer(random_.randbytes(4))
        out_h = Buffer(random_.randbytes(4))
        t.check("rasterizer_display_entry_get_size", idx_e, out_w, out_h,
                globals={"rasterizer_display_entries": entries_buf})

        # set_size takes ecx=entry_index, esi=width, edi=height
        w2 = random_.choice((1, 2, 64, 128, 256, 640, random_.randrange(1, 1024)))
        h2 = random_.choice((1, 2, 64, 128, 256, 480, random_.randrange(1, 1024)))
        t.check("rasterizer_display_entry_set_size",     idx_e, w2, h2,
                globals={"rasterizer_display_entries": entries_buf})

        t.check("rasterizer_display_entry_is_active",    idx_e,
                globals={"rasterizer_display_entries": entries_buf})
