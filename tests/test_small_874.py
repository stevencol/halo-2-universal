# two functions of the way to the main menu (guided by halo2-decompiled):
# whether a unit can do something, and the rounds of a magazine of a weapon
# and of the others of its unit

import random
import struct

from difftest import Buffer

from test_small_859 import BIG, big_objects
from test_units import COUNT

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(874)

    for n in range(400):
        units, pointers = big_objects(random_)
        index = random_.randrange(COUNT)
        at = 0x4C + COUNT * 12 + index * BIG
        struct.pack_into("<I", units, at + 0x14, random_.choice((NONE, NONE, 2)))
        struct.pack_into("<h", units, at + 0x346, 0x300)
        struct.pack_into("<I", units, at + 0x304, random_.getrandbits(32) & ~0x0C000000 | random_.choice((0, 0, 0, 0x04000000, 0x08000000)))
        units[at + 0x212] = random_.choice((0xFF, 0, 1, 3))
        units[at + 0x213] = random_.choice((0xFF, 0, 2))
        for slot in range(4):
            struct.pack_into("<I", units, at + 0x218 + slot * 4, random_.choice((NONE, random_.getrandbits(32))))
        active = {bit: random_.random() < 0.08 for bit in (0x16, 8, 0x12, 0, 1, 0x0A, 0x0B)}
        t.check(
            "unit_unknown_000c85c0",
            random_.getrandbits(16) << 16 | index,
            globals={"object_header_data": Buffer(bytes(units), pointers=tuple(pointers))},
            externals={
                "unit_unknown_000e6fe0": lambda harness, arguments, active=active: (int(active.get(arguments[1], False)), tuple(arguments)),
                "weapon_unknown_00100880": lambda harness, arguments, has=random_.choice((0, 0, 1)): (has, tuple(arguments)),
            },
        )

        # weapons: each of a tag (0 or 1), some of a unit with four weapons
        weapons, pointers = big_objects(random_)
        for object_ in range(COUNT):
            at = 0x4C + COUNT * 12 + object_ * BIG
            struct.pack_into("<I", weapons, at, random_.randrange(2))
            struct.pack_into("<I", weapons, at + 0x154, random_.choice((NONE, random_.randrange(COUNT))))
            for slot in range(4):
                struct.pack_into("<I", weapons, at + 0x218 + slot * 4, random_.choice((NONE, random_.randrange(COUNT))))
        tags = bytearray(random_.randbytes(0x20 + 2 * 0x2C8 + 2 * 3 * 0x5C))
        entries = 0x20 + 2 * 0x2C8
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 16 + 8, 0x20 + tag * 0x2C8)
            struct.pack_into("<iI", tags, 0x20 + tag * 0x2C8 + 0x2C0, random_.choice((0, 1, 3)), entries + tag * 3 * 0x5C)
        t.check(
            "weapon_unknown_001008f0",
            random_.randrange(3),
            random_.randrange(COUNT),
            random_.choice((0, 0, 1, 0x100)),
            globals={
                "object_header_data": Buffer(bytes(weapons), pointers=tuple(pointers)),
                "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18, 0x20 + 0x2C4, 0x20 + 0x2C8 + 0x2C4)),
            },
            externals={"weapon_unknown_00106030": lambda harness, arguments, infinite=random_.choice((0, 1)): (infinite, tuple(arguments))},
        )
