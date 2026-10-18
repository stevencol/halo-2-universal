# twelve small functions of the rasterizer, math, units and others

import random
import struct

from difftest import Buffer, GlobalData

NONE = 0xFFFFFFFF
DISPLAY_ENTRIES = 39  # rasterizer_display_entries
ENTRY = 0x98


def f32(value: float) -> int:
    return struct.unpack("<I", struct.pack("<f", value))[0]


def reals(random_, count: int) -> bytes:
    return struct.pack(f"<{count}f", *(random_.uniform(-1000.0, 1000.0) for _ in range(count)))


def high(random_, low: int) -> int:
    """A value whose low byte is what counts: random bits above it."""
    return random_.getrandbits(24) << 8 | low


def run(t) -> None:
    random_ = random.Random(739)

    for n in range(300):
        t.check(
            "unknown_004b6288_new",
            globals={
                "unknown_004b6280": random_.choice((0, 1, 0x3FF, 0x400, 0x401, 0x7FFFFFFF, random_.randrange(0x400))),
                "unknown_004b6288": Buffer(random_.randbytes(0x400 * 0x70)),
                "unknown_004b6291": bytes((random_.choice((0, 1, 0xFF)),)),
            },
        )

        first = random_.randbytes(4)
        second = random_.choice((
            first,
            first,
            bytes((first[0], first[1], first[2] ^ 1, first[3])),
            bytes((first[0], first[1], first[2] ^ 2, first[3])),
            bytes((first[0], first[1], first[2], first[3] ^ 0x80)),
            bytes((first[0] ^ 1, first[1], first[2], first[3])),
            bytes((first[0], first[1] ^ 0x80, first[2], first[3])),
            random_.randbytes(4),
        ))
        t.check("short_pairs_equal", Buffer(first), Buffer(second))

        # the lightmap points at 0x84 to a thing that is in the same buffer, at 0x88
        lightmap = bytearray(random_.randbytes(0x88 + 0x20))
        struct.pack_into("<i", lightmap, 0x80, random_.choice((-1, 0, 1, 5, random_.getrandbits(31))))
        struct.pack_into("<I", lightmap, 0x84, 0x88)
        struct.pack_into("<I", lightmap, 0x88 + 0x1C, random_.choice((NONE, 0, 1, random_.getrandbits(32))))
        value = random_.getrandbits(32)
        struct.pack_into("<I", lightmap, 0x88 + 4, value)
        structure = bytearray(random_.randbytes(12))
        struct.pack_into("<I", structure, 8, random_.choice((value, value, value ^ 1, random_.getrandbits(32))))
        t.check(
            "rasterizer_lightmap_matches_structure",
            globals={
                "global_structure_lightmap": random_.choice((0, Buffer(bytes(lightmap), pointers=(0x84,)), Buffer(bytes(lightmap), pointers=(0x84,)))),
                "global_structure_bsp": random_.choice((0, Buffer(bytes(structure)), Buffer(bytes(structure)))),
            },
        )

        rectangle = [random_.choice((0, 0, 0, 1, -1)), random_.choice((0, 0, 0, 1, 0x7FFF)), random_.choice((480, 480, 479, 640)), random_.choice((640, 640, 639, 480))]
        t.check(
            "rasterizer_unknown_00022710",
            globals={
                "unknown_00485648": struct.pack("<4h", *rectangle),
                "unknown_004ba04c": random_.choice((-1, 0, 1, 1, 2, random_.getrandbits(32))),
            },
        )

        entries = bytearray(random_.randbytes(DISPLAY_ENTRIES * ENTRY))
        for index in (18, 20):
            struct.pack_into("<I", entries, index * ENTRY + 0x88, random_.choice((0, 0, 1, random_.getrandbits(32))))
            entries[index * ENTRY + 0x95] = random_.choice((0, 0, 1, 0xFF))
        t.check("rasterizer_unknown_00025960", globals={"rasterizer_display_entries": GlobalData(bytes(entries))})

        t.check(
            "unknown_004b9970_reset",
            globals={
                "unknown_004b9970": random_.randbytes(0x30),
                "unknown_005093fc": bytes((random_.choice((0, 1, 0xFF)),)),
            },
        )

        a = reals(random_, 3)
        b = random_.choice((reals(random_, 3), a))
        t.check("distance_squared3d", Buffer(a), Buffer(b))

        t.check("matrix4x3_from_rows", Buffer(random_.randbytes(0x34)), Buffer(reals(random_, 12)))

        t.check(
            "unknown_004ba014_set_bit",
            high(random_, random_.choice((0, 1, 0xFF))),
            high(random_, random_.randrange(32)),
            globals={"unknown_004ba014": random_.choice((0, NONE, random_.getrandbits(32)))},
        )

        t.check(
            "unknown_004ba014_unknown_00030cd0",
            high(random_, random_.choice((0, 0, 1, 0xFF))),
            high(random_, random_.choice((0, 1, 0xFF))),
            globals={"unknown_004ba014": random_.choice((0, 0x10, 0x20, 0x30, 0xFFFFFFEF, 0xFFFFFFDF, NONE, random_.getrandbits(32)))},
        )

        t.check(
            "unknown_00040de0",
            random_.choice((0, 0, 1, random_.getrandbits(32))),
            high(random_, random_.choice((0, 0, 1, 0xFF))),
            high(random_, random_.choice((0, 1, 0xFF))),
        )

        t.check("unknown_00480118_get", globals={"unknown_00480118": random_.getrandbits(32)})
