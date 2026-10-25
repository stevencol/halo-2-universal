# three functions of the way to the main menu (guided by halo2-decompiled):
# how much the height of a biped changes, the havok components of an object
# and its children, and the clusters that the listeners hear

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(860)

    for n in range(400):
        # a biped and the definition of its tag
        table = some_objects(random_)
        objects = bytearray(table.data)
        index = random_.randrange(COUNT)
        struct.pack_into("<I", objects, 0x4C + COUNT * 12 + index * OBJECT, random_.randrange(2) | random_.getrandbits(16) << 16)
        tags = bytearray(random_.randbytes(2 * 16 + 2 * 0x270))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 16 + 8, 0x20 + tag * 0x270)
            struct.pack_into("<I", tags, 0x20 + tag * 0x270 + 0x264, random_.getrandbits(32) & ~2 | random_.choice((0, 0, 2)))
            struct.pack_into("<2f", tags, 0x20 + tag * 0x270 + 0x268, random_.uniform(-3, 3), random_.uniform(-3, 3))
        times = bytearray(random_.randbytes(0x24))
        struct.pack_into("<h", times, 2, random_.choice((30, 60, -1, 0)))
        change = f32(random_.choice((0.0, 0.0, 1.0, -0.25, random_.uniform(-2, 2), float("nan"))))
        t.check(
            "biped_unknown_000dd8e0",
            random_.getrandbits(16) << 16 | index,
            Buffer(random_.randbytes(4)),
            globals={
                "object_header_data": Buffer(bytes(objects), pointers=table.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18)),
                "game_time_globals": Buffer(bytes(times)),
            },
            externals={
                "biped_unknown_000dd7d0": lambda harness, arguments, change=change: (change, tuple(arguments)),
                "unit_unknown_000e4050": lambda harness, arguments, said=random_.choice((0, 1)): (said, tuple(arguments)),
                "unit_unknown_000e0dc0": lambda harness, arguments, said=random_.choice((0, 1)): (said, tuple(arguments)),
            },
        )

        # an object and its children: each has a child at 0x10 and a next at 0x0C, of the later ones
        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            at = 0x4C + COUNT * 12 + index * OBJECT
            later = list(range(index + 1, COUNT))
            struct.pack_into("<I", objects, at + 0x10, random_.choice(later + [NONE]) if later else NONE)
            struct.pack_into("<I", objects, at + 0x0C, random_.choice(later + [NONE, NONE]) if later else NONE)
            struct.pack_into("<I", objects, at + 0xB4, random_.choice((NONE, 0, 5)))
        t.check(
            "havok_unknown_001c36f0",
            random_.choice((NONE, 2, 3, 4)),
            random_.randrange(2),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
            externals={
                "unknown_00146bf0": lambda harness, arguments: (0, ()),
                "unknown_001c35f0": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_00278f00": lambda harness, arguments: (0, ()),
            },
        )

        # the listeners and the clusters of the structure bsp
        bsp = bytearray(random_.randbytes(0xA0))
        clusters = random_.choice((0, 1, 5, 31, 32, 33, 70))
        struct.pack_into("<i", bsp, 0x9C, clusters)
        sound = bytearray(random_.randbytes(0x8C + 4 * 0x48))
        for listener in range(4):
            at = 0x8C + listener * 0x48
            struct.pack_into("<h", sound, at, random_.choice((-1, 0, 3)))
            sound[at + 2] = random_.choice((0, 1, 1))
        distances = [f32(random_.choice((0.0, 100.0, 256.0, 300.0, random_.uniform(0, 600)))) for _ in range(4 * 70)]

        def distance(harness, arguments, distances=distances):
            told = sum(1 for name, _ in harness._calls if name == "unknown_00249d60")
            return distances[told % len(distances)], (arguments[0], arguments[1] & 0xFFFF)

        t.check(
            "sound_unknown_0018cb10",
            globals={
                "global_structure_bsp": Buffer(bytes(bsp)),
                "game_sound_globals": Buffer(bytes(sound)),
                "unknown_0054e8a0": random_.randbytes(0x40),
            },
            externals={"unknown_00249d60": distance},
        )
