# source/rasterizer/rasterizer_text.c

import random
import struct

from difftest import Buffer, GlobalData, Mock, mock_address
from test_async import calls_so_far

NONE = 0xFFFFFFFF
CACHE_SIZE, SLOTS, SLOT_SIZE = 0x10, 0x200, 8
BITMAP_SIZE = 0x74
KNOWN_DATA, KNOWN, KNOWN_COUNT, KNOWN_SLOT = 0x4C, 0x38, 6, 0x34


def run(t) -> None:
    random_ = random.Random(100)

    # ---------- the bitmap of the characters that are drawn

    for n in range(60):
        bitmap = Buffer(random_.randbytes(BITMAP_SIZE))
        made = n % 5 != 4
        t.check(
            "rasterizer_text_cache_initialize",
            globals={
                "hardware_character_cache": random_.randbytes(CACHE_SIZE),
                "font_glyph_slots": random_.randbytes(SLOTS * SLOT_SIZE),
            },
            extra=[bitmap],
            externals={
                "bitmap_2d_new": lambda harness, arguments, bitmap=bitmap, made=made: (
                    bitmap.address if made else 0,
                    tuple(value & 0xFFFF for value in arguments),
                ),
                # only its low byte tells whether the texture was made
                "unknown_0001d000": lambda harness, arguments, could=random_.choice((1, 1, 1, 0, 0x100, 0x80)): could,
            },
        )

    # ---------- the places of the characters are left free

    for n in range(120):
        known = bytearray(random_.randbytes(KNOWN_DATA + KNOWN_COUNT * KNOWN))
        struct.pack_into("<I", known, 0x44, KNOWN_DATA)
        slots = bytearray(random_.randbytes(SLOTS * SLOT_SIZE))
        used = random_.choice((0, 1, 3, KNOWN_COUNT))
        for slot in range(SLOTS):
            struct.pack_into("<I", slots, slot * SLOT_SIZE, NONE)
        # the first place and the last one, at times
        places = random_.sample([0, SLOTS - 1, 1, 2, 3, 4, 5, 255, 256, 257, SLOTS - 2, *random_.sample(range(SLOTS), 4)], used)
        for character, slot in enumerate(places):
            struct.pack_into("<I", slots, slot * SLOT_SIZE, character | random_.choice((0, 0x10000, 0xE1740000)))
            struct.pack_into("<I", known, KNOWN_DATA + character * KNOWN + KNOWN_SLOT, slot)
        cache = bytearray(random_.randbytes(CACHE_SIZE))
        cache[0] = random_.choice((1, 1, 1, 0, 0x80))
        t.check(
            "rasterizer_text_cache_flush",
            globals={
                "hardware_character_cache": bytes(cache),
                "font_glyph_slots": bytes(slots),
                "font_cache_globals+4": Buffer(bytes(known), pointers=(0x44,)),
            },
        )

    # ---------- painting a character

    VERTICES_SIZE = 0x50
    nan = float("nan")

    def real_bits(value: float) -> int:
        return struct.unpack("<I", struct.pack("<f", value))[0]

    def corners(address: int) -> bytes:
        return bytes(t.harness.mu.mem_read(address, VERTICES_SIZE))

    for n in range(400):
        # what the function that is given the corners does with them, and
        # whether it lets the character be painted: only the low byte tells
        lets = random_.choice((1, 1, 1, 0, 0x100, 0x80))
        moved = random_.randbytes(VERTICES_SIZE) if n % 3 == 0 else None

        def given(call, lets=lets, moved=moved):
            if moved:
                t.harness.mu.mem_write(call.stack[0], moved)
            return lets

        t.check(
            "rasterizer_draw_character",
            random_.getrandbits(32),
            mock_address(0) if n % 4 else 0,
            random_.choice((0, 1, 12, 16, 300, -3, 0x12345)) & 0xFFFFFFFF,
            random_.choice((0, 1, 9, 14, 256, -5, 0x54321)) & 0xFFFFFFFF,
            Buffer(random_.randbytes(4) + struct.pack("<2h", random_.choice((1, 17, 240, 0x7FFF, -2)), random_.choice((1, 33, 200, 0x7FFE, -9)))),
            real_bits(random_.choice((0.0, 12.5, 320.0, -8.25, 40000.0, nan))),
            real_bits(random_.choice((0.0, 7.75, 240.0, -3.5, 1e10, nan))),
            random_.choice((0, 0, 1, 5, 0x7FFF, 0xFFFF, 0x10003)),
            random_.choice((0, 0, 2, 7, 0x7FFF, 0xFFFE, 0xABCD0004)),
            real_bits(random_.choice((1.0, 1.0, 0.5, 2.0, 1.5, 0.0, -1.0, nan))),
            random_.getrandbits(32),
            mocks=[Mock(given, stack_arguments=2, record=lambda call: (corners(call.stack[0]), call.stack[1]))],
            externals={
                "unknown_000363a0": lambda harness, arguments: (0, (corners(arguments[0]),)),
            },
        )

    # ---------- painting a character of a text, and its shadow

    for n in range(400):
        known = bytearray(random_.randbytes(KNOWN_DATA + KNOWN_COUNT * KNOWN))
        struct.pack_into("<I", known, 0x44, KNOWN_DATA)
        index = random_.randrange(KNOWN_COUNT)
        struct.pack_into("<I", known, KNOWN_DATA + index * KNOWN + KNOWN_SLOT, NONE if n % 4 == 0 else random_.randrange(SLOTS))
        known = Buffer(bytes(known), pointers=(0x44,))
        # the character is known, or it is after it is asked for, or not at all
        found = random_.choice(([index | 0xE1740000], [index], [NONE, index | 0x10000], [NONE, NONE]))
        # its picture is put in the bitmap of the characters, at times
        put = random_.choice((None, None, random_.randrange(SLOTS), NONE))

        def cache(harness, arguments, known=known, index=index, put=put):
            if put is not None:
                harness.mu.mem_write(known.address + KNOWN_DATA + index * KNOWN + KNOWN_SLOT, struct.pack("<I", put))
            return 0

        def painted(harness, arguments):
            color, proc, height, width, slot, x, y, skipped_width, skipped_height, scale, data = arguments
            return 0, (color, proc, height, width, harness.normalize(slot), x, y, skipped_width & 0xFFFF, skipped_height & 0xFFFF, scale, data)

        scale = random_.choice((1.0, 1.0, 2.0, 0.5, 1.5, 0.0, -1.0)) if n % 31 != 30 else nan
        t.check(
            "rasterizer_draw_character_with_dropshadow",
            random_.getrandbits(32),
            random_.randrange(11),
            random_.choice((0x41, 0x3042, 0xE100, random_.getrandbits(32))),
            random_.getrandbits(32),
            random_.choice((0, 0, 0x00FFFFFF, 0x01000000, 0xFF000000, 0x80123456, random_.getrandbits(32))),
            real_bits(random_.choice((0.0, 12.5, 320.0, -8.25))),
            real_bits(random_.choice((0.0, 7.75, 240.0, -3.5))),
            real_bits(random_.choice((0.0, 0.0, 3.0, 2.75, -1.5, 40000.0))),
            real_bits(random_.choice((0.0, 0.0, 5.0, 4.25, -2.5, 70000.0))),
            real_bits(random_.choice((9.0, 14.0, 0.5, 13.99, -3.5))),
            real_bits(random_.choice((12.0, 16.0, 0.75, 15.99, -7.5))),
            real_bits(scale),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={
                "font_slot_indices": struct.pack("<11I", *(random_.getrandbits(32) for _ in range(11))),
                "font_cache_globals+4": known,
                "font_glyph_slots": GlobalData(random_.randbytes(SLOTS * SLOT_SIZE)),
            },
            externals={
                "font_character_find": lambda harness, arguments, found=found: (found[min(calls_so_far(harness, "font_character_find"), len(found) - 1)], tuple(arguments)),
                "font_character_get_header": lambda harness, arguments: 0,
                "cache_hardware_format_character": cache,
                "rasterizer_draw_character": painted,
            },
        )

    # ---------- the picture of a character goes into the bitmap of the characters

    KNOWN_STATE, KNOWN_HEADER = 0x10, 0x1C
    READ_INDEX, FRAME_INDEX, HARDWARE_FORMAT = 2, 0x70, 0x50

    for n in range(500):
        known = bytearray(random_.randbytes(KNOWN_DATA + KNOWN_COUNT * KNOWN))
        struct.pack_into("<I", known, 0x44, KNOWN_DATA)
        index = random_.randrange(KNOWN_COUNT)
        width, height = random_.choice((1, 6, 9, 14, 30, 0)), random_.choice((1, 8, 12, 16, 33, 0))
        place = KNOWN_DATA + index * KNOWN
        struct.pack_into("<I", known, place + KNOWN_STATE, 4)
        struct.pack_into("<2h", known, place + KNOWN_HEADER + 4, width, height)
        # it has a place, at times: nothing is done
        struct.pack_into("<I", known, place + KNOWN_SLOT, NONE if n % 12 else random_.randrange(SLOTS))

        # the places that are taken go from the oldest to before the next
        # one; of the others, what there was
        read = random_.choice((0, 0, 5, 200, SLOTS - 1, SLOTS - 3, random_.randrange(SLOTS)))
        taken = random_.choice((0, 1, 2, 5, 20, SLOTS - 1, SLOTS - 1, SLOTS - 2))
        write = (read + taken) % SLOTS
        slots = bytearray(random_.randbytes(SLOTS * SLOT_SIZE))
        rows = sorted(random_.choice((1, 1, 14, 30, 60, 200, 240, 0, -5)) for _ in range(4))
        for slot in range(SLOTS):
            character = random_.choice((NONE, NONE, random_.randrange(KNOWN_COUNT) | random_.choice((0, 0x10000, 0xE1740000))))
            struct.pack_into("<I2h", slots, slot * SLOT_SIZE, character, random_.randrange(1, 250), random_.choice(rows))
        # where the next picture goes: near the right and the bottom of the
        # bitmap at times, so that the rows and the bitmap start again
        x0 = random_.choice((1, 1, 0, -2, 40, 200, 242, 247, 250, 255, 256, 257))
        y0 = random_.choice((1, 1, 0, -2, 30, 200, 223, 240, 244, 248, 255, 256, 257))
        maximum = random_.choice((0, 0, 9, 13, 17, 34))
        cache = struct.pack("<2B5h", random_.getrandbits(8), random_.getrandbits(8), read, write, x0, y0, maximum)

        pitch = random_.choice((0x200, 0x200, 0x208))
        texture = Buffer(random_.randbytes(pitch * 304))
        bitmap = bytearray(random_.randbytes(BITMAP_SIZE))
        struct.pack_into("<2h", bitmap, 4, *random_.choice(((256, 256), (256, 256), (128, 300), (40, 20), (0, 0))))
        frame = random_.choice((5, 100, 0x7FFFFFFE, -3))
        struct.pack_into("<i", bitmap, FRAME_INDEX, random_.choice((frame, frame - 1, frame + 1, frame + 1)))
        struct.pack_into("<I", bitmap, HARDWARE_FORMAT, random_.choice((0, 0x80123400, 0x80123400)))
        bitmap = Buffer(bytes(bitmap))
        picture = random_.randbytes(2 * width * height)
        locked = n % 9 != 8

        def pixels_(harness, arguments, picture=picture):
            character_index, pixels = arguments
            harness.mu.mem_write(pixels, picture)
            return 1, (character_index,)

        def lock(harness, arguments, texture=texture, pitch=pitch, locked=locked):
            bitmap_, wait, pitch_ = arguments
            if locked:
                harness.mu.mem_write(pitch_, struct.pack("<I", pitch))
            return (texture.address + 2 * pitch + 8 if locked else 0), (bitmap_, wait & 0xFF)

        t.check(
            "cache_hardware_format_character",
            index | random_.choice((0, 0x10000, 0xE1740000)),
            globals={
                "hardware_character_cache": cache,
                "hardware_character_cache+12": bitmap,
                "font_glyph_slots": bytes(slots),
                "font_cache_globals+4": Buffer(bytes(known), pointers=(0x44,)),
                "texture_cache_globals+52": frame & 0xFFFFFFFF,
            },
            extra=[texture],
            externals={
                "font_character_get_pixels": pixels_,
                "unknown_0001d5e0": lock,
                "texture_cache_bitmap_get_loaded": lambda harness, arguments, loaded=random_.choice((0, 0, 0x80123400)): loaded,
                "texture_cache_bitmap_get_hardware_format": lambda harness, arguments: 0x80123400,
            },
        )
