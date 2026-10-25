# two functions of the way to the main menu (guided by halo2-decompiled): a
# tick of the vibrations of an object, and how much the motors of a gamepad
# rumble

import random
import struct

from difftest import Buffer, f32

from test_small_859 import BIG, big_objects
from test_units import COUNT

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(876)

    for n in range(400):
        objects, pointers = big_objects(random_)
        index = random_.randrange(COUNT)
        at = 0x4C + COUNT * 12 + index * BIG
        struct.pack_into("<I", objects, at, random_.randrange(2))
        struct.pack_into("<h", objects, at + 0x33E, 0x200)
        data = at + 0x200
        objects[data + 0x02] = random_.choice((0, 0, 1, 5))
        struct.pack_into("<h", objects, data + 0x36, random_.choice((-1, 0, 2)))
        struct.pack_into("<I", objects, data + 0x38, random_.choice((NONE, NONE, 3)))
        struct.pack_into("<f", objects, data + 0x40, random_.choice((0.0, 0.00005, 0.0001, 0.05, 0.075, 0.5, 0.9, 1.0, -0.1)))
        for slot in range(12):
            objects[data + 0x50 + slot] = random_.choice((0, 0, 1, 0x7F, 0x80, 0xFF, 0x40))
        tags = bytearray(random_.randbytes(0x20 + 2 * 0x1F0))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 16 + 8, 0x20 + tag * 0x1F0)
            struct.pack_into("<f", tags, 0x20 + tag * 0x1F0 + 0x1E8, random_.choice((0.0, 0.5, 0.9, 1.0, -1.0)))
        t.check(
            "unit_unknown_0010dc70",
            random_.getrandbits(16) << 16 | index,
            globals={
                "object_header_data": Buffer(bytes(objects), pointers=tuple(pointers)),
                "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18)),
                "unknown_00547648": struct.pack("<f", random_.choice((1 / 128, 0.01, 0.0078125, 0.009))),
                "global_random_seed_reference": Buffer(random_.randbytes(8)),
            },
        )

        # the eight effects of an entry, of two tags with three effects each
        entry = bytearray(random_.randbytes(0x88))
        for item in range(8):
            struct.pack_into(
                "<IIf",
                entry,
                item * 0x0C,
                random_.choice((NONE, 0, 1)),
                random_.choice((NONE, 0, 1, 2, 3, 0xFFFFFFFE)),
                random_.choice((0.0, 0.5, 1.0, 2.0)),
            )
            struct.pack_into("<f", entry, 0x60 + item * 4, random_.choice((0.0, 0.5, 1.0, 2.0, 5.0)))
        struct.pack_into("<2f", entry, 0x80, random_.choice((0.0, 0.2, 1.5, -0.5)), random_.choice((0.0, 0.3, 1.2, -1.0)))
        effects = 0x20 + 2 * 0x74
        tags = bytearray(random_.randbytes(effects + 2 * 3 * 0x4C))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 16 + 8, 0x20 + tag * 0x74)
            struct.pack_into("<iI", tags, 0x20 + tag * 0x74 + 0x6C, random_.choice((0, 1, 3)), effects + tag * 3 * 0x4C)
        for effect in range(6):
            for motor in range(2):
                struct.pack_into("<f", tags, effects + effect * 0x4C + 0x24 + motor * 0x0C, random_.choice((0.0, 0.5, 1.0, 3.0, 10.0)))
        rumble = bytearray(random_.randbytes(0x22C))
        struct.pack_into("<3f", rumble, 0x220, random_.uniform(0, 1), random_.uniform(0, 1), random_.choice((0.0, 0.0, 0.5, 1.0)))
        values = [f32(random_.choice((0.0, 0.25, 0.5, 1.0, random_.uniform(0, 1)))) for _ in range(32)]

        def function(harness, arguments, values=values):
            told = sum(1 for name, _ in harness._calls if name == "unknown_0013b390")
            return values[told % len(values)], tuple(arguments)

        t.check(
            "rumble_unknown_00222e90",
            Buffer(bytes(entry)),
            globals={
                "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18, 0x20 + 0x70, 0x20 + 0x74 + 0x70)),
                "unknown_00502120": Buffer(bytes(rumble)),
            },
            externals={"unknown_0013b390": function},
        )
