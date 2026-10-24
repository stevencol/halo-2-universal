# eleven small functions: the bits of a list of the first thing of
# game_unknown_00131ff0.c, a sphere in front of six planes, a point in a
# sphere, the ceiling of a real, two of tags near the effects, four numbers of
# the flags of the game engine and a colour of the game globals

import random
import struct

from difftest import Buffer, GlobalData, f32

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(783)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def real() -> float:
        return random_.choice((0.0, 1.0, -1.0, 0.5, random_.uniform(-10, 10), random_.uniform(-10, 10)))

    def reals(count: int) -> bytes:
        return struct.pack(f"<{count}f", *(real() for n in range(count)))

    def flags() -> int:
        # with 0x21, 0x41, 0x50 and 0x200001 the numbers reach the limits of 0x0009a600
        return random_.choice(
            (0, 0x20, 0x40, 0x60, 0x21, 0x41, 0x50, 0x200001, 0x200010, 1 << random_.randrange(32), random_.getrandbits(32) & random_.getrandbits(32), random_.getrandbits(32))
        )

    for n in range(400):
        # the first thing: its first long points to a list of entries
        thing = bytearray(random_.randbytes(0x2AE0))
        struct.pack_into("<I", thing, 0, 0x100)
        count = random_.choice((-1, 0, 1, 2, 40, random_.randrange(41)))
        struct.pack_into("<h", thing, 0x100 + 0xA6C, count)
        for index in range(max(count, 0)):
            struct.pack_into("<h", thing, 0x100 + 0xA6E + index * 0x1A, random_.choice((0, 31, 32, 511, random_.randrange(512))))
        t.check(
            "unknown_0003f3f0",
            globals={"unknown_00547f88": GlobalData(bytes(thing), pointers=(0,), places=(0,)), "unknown_004c56c0": random_.randbytes(64)},
        )

        t.check("unknown_0003fd20", Buffer(reals(24)), Buffer(reals(3)), f32(random_.choice((0.0, 1.0, 2.5, -1.0, random_.uniform(0, 5)))))

        point = reals(3)
        center = point if n % 9 == 0 else reals(3)
        t.check("point_in_sphere", Buffer(point), Buffer(center), f32(random_.choice((0.0, 1.0, 4.0, -2.0, random_.uniform(0, 12)))))

        value = random_.choice(
            (
                f32(random_.choice((0.0, -0.0, 0.5, -0.5, 1.0, -1.0, 1.5, -1.5, 2.0, 1e-30, -1e-30, 8388607.5, 16777217.0, 1e10, -1e10))),
                f32(random_.uniform(-1000, 1000)),
                f32(float(random_.randrange(-5000, 5000))),
                random_.getrandbits(32),
            )
        )
        t.check("real_ceiling_to_long", value)

        # a tag that starts with a count and its elements of 0x1C bytes
        tag = random_.randrange(4)
        tags = bytearray(random_.randbytes(0xC0))
        struct.pack_into("<I", tags, tag * 0x10 + 8, 0x40)
        struct.pack_into("<II", tags, 0x44, random_.choice((0, 1, 4, 3)), 0x50)
        for element in range(4):
            struct.pack_into("<i", tags, 0x50 + element * 0x1C + 0x0C, random_.choice((0, 0xF, 0x10, -1, -16, random_.getrandbits(32) - (1 << 31))))
        t.check(
            "tag_unknown_0004cbb0",
            random_.choice((NONE, random_.getrandbits(16) << 16 | tag, tag)),
            word(random_.choice((0, 1, 2, 3, 4, -1))),
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=(tag * 0x10 + 8, 0x48))},
        )

        # two lists of entries, the elements they name and their texts
        data = bytearray(random_.randbytes(0x400))
        pointers = [0x44, 0x54, 0x64]
        struct.pack_into("<I", data, 0x44, 0x100)
        struct.pack_into("<I", data, 0x54, 0x1E0)
        struct.pack_into("<I", data, 0x64, 0x210)
        for entry in range(8):
            at = 0x1E0 + entry * 12
            struct.pack_into("<H", data, at + 2, random_.randrange(4))
            struct.pack_into("<I", data, at + 8, 0x240 + entry * 8)
            pointers.append(at + 8)
        for element in range(4):
            at = 0x100 + element * 0x38
            data[at] = random_.choice((0, 2, 0xFD, 0xFF, random_.getrandbits(8)))
            struct.pack_into("<I", data, at + 0x34, 0x300 + element * 8)
            struct.pack_into("<I", data, 0x300 + element * 8 + 4, 0x320 + element * 0x10)
            data[0x340 + element * 0x10] = random_.choice((ord("."), ord("."), ord("a"), 0))
            pointers += [at + 0x34, 0x300 + element * 8 + 4]
        t.check(
            "unknown_0004dc60",
            random_.randrange(4),
            Buffer(bytes(data), pointers=tuple(pointers)),
            random_.choice((0, 1, 0x100, 0xFF)),
            random_.randrange(4),
        )

        for name in ("game_engine_unknown_0009a150", "game_engine_unknown_0009a2c0", "game_engine_unknown_0009a600", "game_engine_unknown_0009aa00"):
            t.check(name, random_.getrandbits(32), Buffer(struct.pack("<I", flags())), Buffer(random_.randbytes(4)))

        # the game globals name a tag whose first element has colours at 0x10
        tag = random_.randrange(4)
        tags = bytearray(random_.randbytes(0xB0))
        struct.pack_into("<I", tags, tag * 0x10 + 8, 0x40)
        struct.pack_into("<I", tags, 0x44, 0x50)
        struct.pack_into("<iI", tags, 0x60, random_.choice((0, 1, 3, 4, -1)), 0x80)
        game_globals = bytearray(random_.randbytes(0x170))
        struct.pack_into("<I", game_globals, 0x16C, random_.choice((NONE, tag, random_.getrandbits(16) << 16 | tag)))
        t.check(
            "game_engine_unknown_0007f720",
            Buffer(random_.randbytes(12)),
            random_.getrandbits(16) << 16 | word(random_.choice((-1, 0, 1, 2, 3, 4, -0x8000))) & 0xFFFF,
            globals={
                "global_game_globals": Buffer(bytes(game_globals)),
                "global_tag_instances": Buffer(bytes(tags), pointers=(tag * 0x10 + 8, 0x44, 0x64)),
                "unknown_00468734": Buffer(random_.randbytes(12)),
            },
        )
