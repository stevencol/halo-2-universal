# two functions of the way to the main menu: who drives a unit in a seat,
# and four names of something with their defaults

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
DEFINITION = 0x1D0
NAMES = (0x07000101, 0x07000001, NONE, 0x0400054B, 0x12345678)


def run(t) -> None:
    random_ = random.Random(840)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        # units: a parent with seats in its definition, and children chained from it
        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<I", objects, at, salted(random_.randrange(2)))
            struct.pack_into("<I", objects, at + 4, random_.getrandbits(32))
            struct.pack_into("<h", objects, at + 0x1FC, random_.choice((-1, 0, 1, 2)))
            struct.pack_into("<II", objects, at + 0x0C, random_.choice((NONE, salted(random_.randrange(COUNT)))), random_.choice((NONE, salted(random_.randrange(COUNT)))))
        # avoid loops in the chains at 0x0C: each points only to a later one
        for index in range(COUNT):
            later = [salted(other) for other in range(index + 1, COUNT)]
            struct.pack_into("<I", objects, FIRST + index * OBJECT + 0x0C, random_.choice(later + [NONE]))
        tags = bytearray(random_.randbytes(0x20 + 2 * DEFINITION + 2 * 3 * 0xB0))
        pointers = []
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x20 + tag * DEFINITION)
            struct.pack_into("<I", tags, 0x20 + tag * DEFINITION + 0x1CC, 0x20 + 2 * DEFINITION + tag * 3 * 0xB0)
            pointers += [tag * 0x10 + 8, 0x20 + tag * DEFINITION + 0x1CC]
        headers = Buffer(bytes(objects), pointers=table.pointers)
        found = random_.random() < 0.8

        def verify(harness, arguments, headers=headers, found=found):
            index = arguments[0] & 0xFFFF
            return (headers.address + FIRST + index * OBJECT if found and index < COUNT else 0, tuple(arguments))

        t.check(
            "unit_unknown_000cb6d0",
            random_.choice((NONE, salted(random_.randrange(COUNT)))),
            globals={"object_header_data": headers, "global_tag_instances": Buffer(bytes(tags), pointers=tuple(pointers))},
            externals={"object_try_and_get_and_verify_type": verify},
        )

        # an owner with names at 0x70 and its tag at 0x68; the definition has a block at 0x54 of pairs (8 bytes)
        owner = bytearray(random_.randbytes(0x80))
        struct.pack_into("<I", owner, 0x68, 0)
        struct.pack_into("<4I", owner, 0x70, *(random_.choice((NONE, 0x07000001, 0x0A0B0C0D, 0x030000D9)) for _ in range(4)))
        keys = sorted(random_.sample((0x030000D9, 0x0A0B0C0D, 0x05000000), 2))
        block = struct.pack("<iI", 2, 0x10 + 0x60) + b"".join(struct.pack("<iI", key, random_.choice((NONE, 0x030000D9, 0x11112222))) for key in keys)
        tags = bytearray(random_.randbytes(0x10)) + bytearray(0x54) + block[:8] + bytearray(4) + block[8:]
        struct.pack_into("<I", tags, 8, 0x10)
        t.check(
            "havok_unknown_001cc2f0",
            random_.choice(NAMES),
            random_.choice(NAMES),
            random_.choice(NAMES + (0x0A0B0C0D,)),
            Buffer(random_.randbytes(16)),
            Buffer(bytes(owner)),
            random_.choice(NAMES),
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x10 + 0x54 + 4))},
        )
