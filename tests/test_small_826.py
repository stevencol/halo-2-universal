# five small functions of the way to the main menu: a binary search, a short
# of a tag or of the structure bsp, a vector made one long, whether an
# address is in the cache, and a particle location last in a list

import random
import struct

from difftest import Buffer, Pointer

NONE = 0xFFFFFFFF
LOCATION = 0x34


def run(t) -> None:
    random_ = random.Random(826)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        count = random_.choice((0, 1, 2, 5, 8))
        size = random_.choice((4, 8, 12))
        keys = sorted(random_.sample(range(-20, 20), count))
        block = bytearray(struct.pack("<iI", count, 8))
        for key in keys:
            block += struct.pack("<i", key) + random_.randbytes(size - 4)
        t.check(
            "havok_unknown_001dd560",
            Buffer(bytes(block), pointers=(4,)),
            random_.choice(keys + [-21, 21, 0]) & 0xFFFFFFFF if keys else 3,
            size,
        )

        definition = bytearray(random_.randbytes(0x60 + 3 * 0x14))
        struct.pack_into("<I", definition, 0x5C, 0x70)
        tags = bytearray(random_.randbytes(0x20))
        struct.pack_into("<I", tags, 8, 0x20)
        struct.pack_into("<I", tags, 0x18, 0x20)
        bsp = bytearray(random_.randbytes(0x20 + 3 * 0x14))
        struct.pack_into("<I", bsp, 0x10, 0x20)
        data = bytearray(random_.randbytes(0x20))
        struct.pack_into("<hhI", data, 0x18, random_.choice((-1, 0, 1, 2)), 0, random_.choice((NONE, salted(0), salted(1))))
        t.check(
            "unknown_001ee410",
            Buffer(random_.randbytes(2)),
            Buffer(bytes(data)),
            globals={
                "global_tag_instances": Buffer(bytes(tags) + bytes(definition), pointers=(8, 0x18, 0x20 + 0x5C)),
                "global_structure_bsp": Buffer(bytes(bsp), pointers=(0x10,)),
                "unknown_0054e898": random_.randbytes(2),
            },
        )

        vector = [random_.choice((0.0, random_.uniform(-3, 3))) for _ in range(3)]
        t.check("real_math_unknown_0004efb0", Buffer(struct.pack("<3f", *vector)))

        cache = bytearray(random_.randbytes(0x28))
        cache[0] = random_.choice((0, 1))
        struct.pack_into("<i", cache, 0x24, random_.choice((0, 0x1000, 0x100000)))
        t.check(
            "unknown_00279740",
            random_.choice((0x80061000, 0x80060FFF, 0x80062000, 0x80161000, 0x80161001, 0x7FFFFFFF, 0, random_.getrandbits(32))),
            globals={"cache_file_globals": bytes(cache)},
        )

        locations = bytearray(random_.randbytes(0x50 + 4 * LOCATION))
        struct.pack_into("<I", locations, 0x44, 0x50)
        data_array = Buffer(bytes(locations), pointers=(0x44,))
        t.check(
            "particle_location_list_add",
            Pointer(data_array, 0x50 + random_.randrange(4) * LOCATION),
            Buffer(struct.pack("<I", random_.choice((NONE, salted(1))))),
            Buffer(struct.pack("<I", random_.choice((NONE, salted(2), 0)))),
            globals={"particle_location_data": data_array},
        )
