# eight small functions of Havok: bits under a mask, whether there is room
# for a kind of tag, two headers of 0x10 bytes, three copies of the same
# clearing, and three entries cleared

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(801)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    for n in range(400):
        count = random_.choice((0, 1, 32, 33, 100, 256, -5))
        t.check(
            "havok_unknown_001c2330",
            Buffer(random_.randbytes(32)),
            word(count),
            Buffer(random_.randbytes(32)),
            Buffer(random_.randbytes(32)),
        )

        tags = bytearray(random_.randbytes(0x40 + 4 * 0x10))
        for tag in range(4):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x40 + tag * 0x10)
            tags[0x40 + tag * 0x10] = random_.choice((0, 1, 2, 7, 11, 12, 32, 33))
        components = bytearray(random_.randbytes(0x50))
        struct.pack_into("<i", components, 0x20, random_.choice((0, 5, 10)))
        struct.pack_into("<i", components, 0x3C, random_.choice((0, 5, 9, 10)))
        t.check(
            "havok_unknown_001c3930",
            random_.getrandbits(16) << 16 | random_.randrange(4),
            globals={
                "global_tag_instances": Buffer(bytes(tags), pointers=tuple(tag * 0x10 + 8 for tag in range(4))),
                "unknown_0047f058": bytes((random_.choice((0, 1)),)),
                "unknown_0051e9a0": Buffer(struct.pack("<i", random_.choice((0, 0x1FF, 0x200, 0x201, -1)))),
                "havok_component_data": Buffer(bytes(components)),
            },
        )

        t.check("havok_unknown_001c5e80", Buffer(random_.randbytes(0x10)), random_.choice((0, Buffer(random_.randbytes(8)))))
        t.check("havok_unknown_001c61c0", Buffer(random_.randbytes(0x10)), random_.choice((0, Buffer(random_.randbytes(8)))))
        t.check("havok_unknown_001c62f0", Buffer(random_.randbytes(0x20)))
        t.check("havok_unknown_001c6340", Buffer(random_.randbytes(0x20)))
        t.check("havok_unknown_001c63f0", Buffer(random_.randbytes(0x20)))
        t.check("havok_unknown_001cb0a0", Buffer(random_.randbytes(0x60)))
