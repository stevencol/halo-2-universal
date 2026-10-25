# two functions of the way to the main menu: the animations of what three
# searches find predicted, and a block at the end of an object

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(843)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def block(keys: list[int], size: int, at: int) -> bytes:
        data = struct.pack("<iI", len(keys), at + 8)
        for key in keys:
            data += struct.pack("<i", key) + bytes(size - 4)
        return data

    for n in range(400):
        # as in test_small_829, with two lists of indices (0x24/0x28 and 0x2C/0x30) in what is found, and animations at 0xB0
        data = bytearray(random_.randbytes(0x34))
        top = block([1, 2], 0x14, 0x34)
        data += top
        second_at = len(data)
        middle = block([3, 4], 0x14, second_at)
        third_at = second_at + len(middle)
        last = block([5, 6], 0x34, third_at)
        data += middle + last
        lists_at = len(data)
        data += struct.pack("<8i", *(random_.randrange(4) for _ in range(8)))
        animations_at = len(data)
        animations = bytearray(random_.randbytes(4 * 0x14))
        for animation in range(4):
            animations[animation * 0x14 + 0x10] = random_.choice((0, 1))
        data += animations
        data += bytes(0xB4 - 0x34) if len(data) < 0xB4 else b""
        struct.pack_into("<iI", data, 0x34 + 8 + 4, 2, second_at + 8)
        struct.pack_into("<iI", data, second_at + 8 + 4, 2, third_at + 8)
        for element in range(2):
            at = third_at + 8 + element * 0x34
            struct.pack_into("<iIiI", data, at + 0x24, random_.randrange(5), lists_at, random_.randrange(5), lists_at + 16)
        struct.pack_into("<I", data, 0xB0, animations_at)
        pointers = [0x34 + 4, 0x34 + 8 + 8, second_at + 4, second_at + 8 + 8, third_at + 4, 0xB0]
        for element in range(2):
            pointers += [third_at + 8 + element * 0x34 + 0x28, third_at + 8 + element * 0x34 + 0x30]
        t.check(
            "havok_unknown_001ddb90",
            random_.choice((0, 1)),
            Buffer(bytes(data), pointers=tuple(pointers)),
            random_.choice((1, 1, 9)),
            random_.choice((3, 3, 9)),
            random_.choice((5, 6, 9)),
            random_.choice((0, 1)),
            externals={"animation_cache_predict": lambda harness, arguments: (0, (arguments[0], arguments[1] & 0xFF))},
        )

        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            struct.pack_into("<h", objects, 0x4C + index * 12 + 6, random_.choice((0x100, 0x120)))
        ok = random_.random() < 0.8
        t.check(
            "object_header_block_allocate",
            salted(random_.randrange(COUNT)),
            random_.choice((0x10, 0x40, 0x80)),
            random_.choice((0, 4, 13, 0x20)),
            random_.choice((0, 2, 4)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers), "unknown_004de2ec": random_.getrandbits(32)},
            externals={"memory_pool_block_reallocate": lambda harness, arguments, ok=ok: (int(ok), (arguments[0], arguments[2], arguments[3], arguments[4]))},
        )
