# source/rasterizer/rasterizer_text.c

import random
import struct

from difftest import Buffer

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
