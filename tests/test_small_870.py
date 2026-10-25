# two functions of the way to the main menu (guided by halo2-decompiled):
# where the camera of a unit is, and what a weapon keeps of its triggers
# starting over

import random
import struct

from difftest import Buffer

from test_small_859 import BIG, big_objects
from test_units import COUNT

NONE = 0xFFFFFFFF
DEFINITION = 0x200


def run(t) -> None:
    random_ = random.Random(870)

    for n in range(400):
        units, pointers = big_objects(random_)
        index = random_.randrange(COUNT)
        for object_ in range(COUNT):
            struct.pack_into("<I", units, 0x4C + COUNT * 12 + object_ * BIG, random_.randrange(2) | random_.getrandbits(16) << 16)
        at = 0x4C + COUNT * 12 + index * BIG
        struct.pack_into("<I", units, at + 0x14, random_.choice((NONE, random_.randrange(COUNT))))
        struct.pack_into("<h", units, at + 0x1FC, random_.choice((-1, 0, 1, 2)))
        units = Buffer(bytes(units), pointers=tuple(pointers))
        tags = bytearray(random_.randbytes(0x20 + 2 * DEFINITION + 3 * 0xB0))
        seats = 0x20 + 2 * DEFINITION
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 16 + 8, 0x20 + tag * DEFINITION)
            struct.pack_into("<I", tags, 0x20 + tag * DEFINITION + 0x1CC, seats)
        tags = Buffer(bytes(tags), pointers=(8, 0x18, 0x20 + 0x1CC, 0x20 + DEFINITION + 0x1CC))

        def markers(harness, arguments, found=random_.choice((0, 1, 1, 0x10000)), marker=random_.randbytes(0x70)):
            harness.mu.mem_write(arguments[1], marker)
            return found, (arguments[0] & 0xFF, *arguments[2:])

        def origin(harness, arguments, point=random_.randbytes(12)):
            harness.mu.mem_write(arguments[1], point)
            return 0, (arguments[0],)

        t.check(
            "unit_unknown_000d1080",
            random_.getrandbits(16) << 16 | index,
            Buffer(random_.randbytes(0x34)),
            Buffer(random_.randbytes(0x14)),
            globals={"object_header_data": units, "global_tag_instances": tags},
            externals={"unknown_000b8d30": markers, "object_get_origin": origin},
        )

        weapons, pointers = big_objects(random_)
        index = random_.randrange(COUNT)
        struct.pack_into("<h", weapons, 0x4C + COUNT * 12 + index * BIG + 0x33E, 0x200)
        t.check(
            "weapon_unknown_0010f040",
            random_.getrandbits(16) << 16 | index,
            globals={"object_header_data": Buffer(bytes(weapons), pointers=tuple(pointers))},
        )
