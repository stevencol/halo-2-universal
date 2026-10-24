# ten small functions: two of the rasterizer that clear and gather bits, the
# clearing of what the text keeps, a table of entries with three lists of bits,
# the sums of an entry list, how the screen is split, a number by the bits of
# the units and a short of the structure bsp

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(782)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    for n in range(400):
        t.check("rasterizer_unknown_0001c6b0", Buffer(random_.randbytes(0x2D8)))

        # a tag whose block at 0x08 has four elements, each with its entries
        tags = bytearray(random_.randbytes(0x100))
        tag = random_.randrange(4)
        struct.pack_into("<I", tags, tag * 0x10 + 8, 0x40)
        struct.pack_into("<I", tags, 0x40 + 8, 0x50)
        for element in range(4):
            entries = bytearray(random_.randbytes(16))
            if random_.random() < 0.5:
                entries[::2] = bytes(random_.choice((0, 31, 32, 7)) for n in range(8))
            tags[0xC0 + element * 16 : 0xC0 + element * 16 + 16] = entries
            struct.pack_into("<II", tags, 0x50 + element * 0x1C + 4, random_.choice((0, 1, 2, 8, random_.randrange(9))), 0xC0 + element * 16)
        pointers = (tag * 0x10 + 8, 0x48) + tuple(0x50 + element * 0x1C + 8 for element in range(4))
        t.check(
            "rasterizer_unknown_0001cb70",
            random_.randrange(4),
            random_.getrandbits(16) << 16 | tag,
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=pointers)},
        )

        t.check(
            "unknown_005093e0_clear",
            globals={
                "unknown_005093e0": Buffer(random_.randbytes(0x400)) if n % 5 else 0,
                "unknown_004b5690": random_.choice((0, 0, 1, random_.getrandbits(32))),
                "unknown_004b569d": bytes((random_.choice((0, 0, 1, 0x80)),)),
            },
        )

        # the table of entries: the index and the other at and around the ends
        count = random_.choice((0, 1, 5, 511, random_.randrange(512)))

        def entry_index() -> int:
            return word(random_.choice((-2, -1, 0, count - 1, count, count + 1, random_.randrange(max(count, 1)))))

        t.check(
            "unknown_0051f40c_set_other",
            entry_index(),
            entry_index(),
            globals={
                "unknown_0051f408": bytes((random_.choice((0, 1, 1, 1, 0x40)),)),
                "unknown_005234c4": random_.choice((NONE, 0, 0, 3)),
                "unknown_005233ec": count,
                "unknown_0051f40c": random_.randbytes(0x3FE0),
            },
        )
        t.check(
            "unknown_0051f40c_clear",
            random_.choice((0, 31, 32, 63, 510, random_.randrange(511))),
            globals={"unknown_0051f40c": random_.randbytes(0x3FE0), "unknown_005233f0": random_.randbytes(0xD4)},
        )

        entries = bytearray(random_.randbytes(4 * 16 + 8))
        for index in range(16):
            struct.pack_into("<H", entries, 6 + 4 * index, random_.choice((0, 1, 2, 3, 0xFFFF, random_.getrandbits(16))))
        t.check(
            "rasterizer_unknown_00023540",
            Buffer(bytes(entries)),
            random_.getrandbits(16) << 16 | random_.choice((0, 1, 2, 15, 16, 0xFFFF, random_.randrange(17))),
            Buffer(random_.randbytes(0x5C)),
            word(random_.choice((0, 1, 12, -4, random_.getrandbits(32)))),
        )

        players = word(random_.choice((-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 16, random_.randrange(30))))
        mode = random_.choice((0, 1, 1, 2, NONE))
        t.check("screen_split_get_grid", players, mode, Buffer(random_.randbytes(4)))

        grid = struct.pack("<hh", random_.choice((1, 2, 3, 4, -2)), random_.choice((1, 2, 3, 4, -1)))
        t.check(
            "screen_split_get_position",
            word(random_.choice((0, 0, 1, 2, 3, 5, 7, -3))),
            Buffer(grid),
            Buffer(random_.randbytes(4)),
            word(random_.choice((0, 1, 2, 3, 4, 8, 16, -1))),
            mode,
            Buffer(random_.randbytes(4)),
        )

        t.check(
            "unknown_004ba014_unknown_00030da0",
            random_.choice((0, 1, 0xFF, 0x100)),
            random_.choice((0, 1, 0xFF, 0x100)),
            globals={
                "unknown_004ba014": random_.getrandbits(3) | random_.getrandbits(29) << 3,
                "unknown_004858c4": Buffer(bytes((random_.choice((0, 0x20, 0xDF, 0xFF)),))),
            },
        )

        bsp = bytearray(random_.randbytes(0xD0))
        struct.pack_into("<iI", bsp, 0xAC, random_.choice((-1, 0, 1, 8, 8)), 0xC0)
        shorts = [random_.choice((-1, 0, 5, random_.getrandbits(15))) for n in range(8)]
        struct.pack_into("<8h", bsp, 0xC0, *shorts)
        which = random_.choice((0, 1, 7, random_.randrange(8), -1))
        value = random_.choice((shorts[which], shorts[which], -1, random_.getrandbits(15), shorts[which] + 0x10000))
        t.check(
            "unknown_004b9eec_matches",
            word(value),
            globals={
                "global_structure_bsp": Buffer(bytes(bsp), pointers=(0xB0,)),
                "global_structure_bsp_index": struct.pack("<h", random_.choice((-1, 0, 0, 2))),
                "unknown_004b9ee9": bytes((random_.choice((0, 1, 1, 0x80)),)),
                "unknown_004b9eec": word(which),
            },
        )
