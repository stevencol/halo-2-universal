# four small functions of the way to the main menu: the random seed moved
# on by sixteen things, the smallest number above another, some numbers made
# zeros, and a place of one of sixteen things

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(828)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        game = bytearray(random_.randbytes(0x1130))
        struct.pack_into("<i", game, 8, random_.choice((0, 1, 1, 2)))
        game[0x1120] = random_.choice((0, 1, 1))
        data = bytearray(random_.randbytes(0x10 + 16 * 0x5C))
        struct.pack_into("<I", data, 8, random_.getrandbits(16))
        for thing in range(16):
            struct.pack_into("<ff", data, 0x10 + thing * 0x5C, random_.choice((-1.0, 0.0, 2.0)), random_.choice((-1.0, 0.0, 2.0)))
        t.check(
            "unknown_00138db0",
            Buffer(bytes(data)),
            globals={
                "main_game_globals": random_.choice((0, Buffer(bytes(game)), Buffer(bytes(game)))),
                "global_random_seed_reference": Buffer(random_.randbytes(8)),
            },
        )

        count = random_.choice((0, 1, 4))
        globals_ = bytearray(random_.randbytes(0x180 + 8 + 4 * 0x108))
        struct.pack_into("<iI", globals_, 0x170, random_.choice((0, 1, 1)), 0x180)
        struct.pack_into("<iI", globals_, 0x180, count, 0x188)
        for thing in range(4):
            struct.pack_into("<ii", globals_, 0x188 + thing * 0x108, random_.choice((1, 2)), random_.choice((-1, 3, 5, 9)))
        t.check(
            "unknown_0019c440",
            random_.choice((1, 2, 3)),
            random_.choice((-2, 3, 4, 10)),
            globals={
                "global_scenario": random_.choice((0, 0x1000, 0x1000)),
                "global_game_globals": Buffer(bytes(globals_), pointers=(0x174, 0x184)),
            },
        )

        owner = bytearray(random_.randbytes(0x2A80))
        struct.pack_into("<i", owner, 0x2A78, random_.choice((0, 1)))
        t.check(
            "unknown_0002cb30",
            globals={
                "unknown_004c0b80": Buffer(bytes(owner)),
                "unknown_004c0b78": random_.getrandbits(32),
                "unknown_004c0b7c": random_.getrandbits(32),
                "unknown_004c1528": random_.getrandbits(32),
                "unknown_004c152c": random_.randbytes(0x480),
                "unknown_004c19ac": random_.getrandbits(32),
            },
        )

        # sixteen entries; a tag whose definition has a block at 0x28, and a structure bsp
        entries = bytearray(random_.randbytes(16 * 0x20))
        for entry in range(16):
            bits = random_.randrange(4) << 9 | random_.randrange(4) << 18 | random_.getrandbits(9)
            struct.pack_into("<IiI", entries, entry * 0x20 + 4, random_.choice((NONE, salted(0))), 0, bits)
        tags = bytearray(random_.randbytes(0x10 + 0x30))
        struct.pack_into("<II", tags, 8, 0x10, 0)
        struct.pack_into("<I", tags, 0x10 + 0x28, 0x100)
        bsp = bytearray(random_.randbytes(0x150 + 4 * 0x58))
        struct.pack_into("<I", bsp, 0xA0, 0x400)
        struct.pack_into("<I", bsp, 0x13C, 0x800)
        struct.pack_into("<I", bsp, 0x144, 0x150)
        for element in range(4):
            struct.pack_into("<h", bsp, 0x150 + element * 0x58 + 0x34, random_.randrange(5))
        t.check(
            "unknown_00044940",
            random_.randrange(16) | random_.getrandbits(16) << 16,
            random_.choice((0, 1)),
            globals={
                "unknown_004ba138": bytes(entries),
                "global_tag_instances": Buffer(bytes(tags), pointers=(8,)),
                "global_structure_bsp": Buffer(bytes(bsp), pointers=(0x144,)),
            },
        )
