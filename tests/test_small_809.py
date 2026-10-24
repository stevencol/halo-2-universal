# four medium functions of the way to the main menu: an element of a block of
# a tag by its kind, flags moved from others, the bits of two parts of
# something by a real, and the highest number of some elements of the game
# globals

import random
import struct

from difftest import Buffer, f32

NONE = 0xFFFFFFFF
BLOCKS = (0x4C, 0x54, 0x5C, 0x64, 0x6C, 0x74, 0x94, 0xA4, 0x10C)


def run(t) -> None:
    random_ = random.Random(809)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    for n in range(400):
        tag = bytearray(random_.randbytes(0x110 + 0x400))
        for at in BLOCKS:
            struct.pack_into("<I", tag, at, 0x110 + random_.randrange(0x100))
        t.check(
            "tag_unknown_001eaf30",
            random_.getrandbits(16) << 16 | random_.choice((0, 1, 2, 3, 4, 5, 6, 7, 13, 14, 15, 16, 0xFFFF)),
            random_.getrandbits(16) << 16 | random_.randrange(3),
            Buffer(bytes(tag), pointers=BLOCKS),
        )

        thing = bytearray(random_.randbytes(0x2AE0))
        thing[0x2A60] = random_.choice((0, 0x80, 0x7F))
        tags = bytearray(random_.randbytes(0x40 + 4 * 0x10))
        for index in range(4):
            struct.pack_into("<I", tags, index * 0x10 + 8, 0x40 + index * 0x10)
            tags[0x40 + index * 0x10] = random_.choice((0, 2, 0xFD))
        t.check(
            "game_unknown_00131ff0_flags_from",
            random_.getrandbits(16) << 16 | random_.choice((0, 0x400, 0x200, 0x4000, 0x2C00 << 2, random_.getrandbits(16))),
            Buffer(bytes(thing)),
            globals={
                "unknown_004b9ef0": random_.choice((3, 3, 2)),
                "unknown_004b9f8c": random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(4))),
                "unknown_004b9ffc": f32(random_.choice((0.0, 0.0, -0.0, 1.0))),
                "unknown_004ba004": bytes((random_.choice((0, 1)),)),
                "global_tag_instances": Buffer(bytes(tags), pointers=tuple(index * 0x10 + 8 for index in range(4))),
            },
        )

        data = bytearray(random_.randbytes(0x84))
        for at in (0x00, 0x20):
            struct.pack_into("<I", data, at, random_.choice((NONE, 3)))
            struct.pack_into("<h", data, at + 6, random_.choice((-1, 2)))
        struct.pack_into("<I", data, 0x80, f32(random_.choice((0.0, 0.45, 0.5, 0.55, 0.56, 1.0))))
        t.check("havok_unknown_001cb680", Buffer(bytes(data)), globals={"unknown_0046fbf4": bytes((random_.choice((0, 1, 1)),))})

        count = random_.choice((0, 1, 3, 4))
        game = bytearray(random_.randbytes(0x180 + 4 * 0x108 + 8))
        struct.pack_into("<iI", game, 0x170, random_.choice((0, 1, 1)), 0x180)
        struct.pack_into("<iI", game, 0x180, count, 0x188)
        for element in range(4):
            struct.pack_into("<Ii", game, 0x188 + element * 0x108, random_.choice((1, 2)), random_.choice((-1, 0, 3, 7, -5)))
        t.check(
            "levels_unknown_0019c4e0",
            random_.choice((1, 2, 3)),
            globals={
                "global_game_globals": Buffer(bytes(game), pointers=(0x174, 0x184)) if n % 9 else 0,
                "global_scenario": Buffer(random_.randbytes(16)) if n % 7 else 0,
            },
        )
