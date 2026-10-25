# four medium functions of the way to the main menu: a walk over two lists,
# two havok components by the kind of their objects, something of 0x84
# bytes made nothing, and something of the network made with two random
# numbers

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(814)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        # the lists: two arrays of pointers to elements whose count is at 0x40
        lists = bytearray(random_.randbytes(0x20 + 8 * 4 + 8 * 0x44))
        first = random_.choice((0, 1, 3, 4))
        second = random_.choice((0, 1, 2, 4))
        struct.pack_into("<IiIIi", lists, 0x08, 0x14 + 0x20, first, 0, 0x14 + 0x30, second)
        pointers = [0x08, 0x14]
        for slot in range(8):
            struct.pack_into("<I", lists, 0x20 + slot * 4, 0x14 + 0x40 + slot * 0x44)
            struct.pack_into("<i", lists, 0x40 + slot * 0x44 + 0x40, random_.choice((0, 1, 2, 5)))
            pointers.append(0x20 + slot * 4)
        iterator = bytearray(random_.randbytes(0x14))
        iterator[1] = random_.choice((0, 0, 1))
        struct.pack_into("<iiIi", iterator, 4, random_.choice((-1, 0, 1, 2, 3)), random_.choice((0, 1, 4)), 0x14, random_.choice((0, 1, 2, 3)))
        t.check(
            "unknown_002797a0_next",
            Buffer(bytes(iterator) + bytes(lists), pointers=(0x0C,) + tuple(0x14 + p for p in pointers)),
        )

        table = some_objects(random_)
        headers = bytearray(table.data)
        for index in range(COUNT):
            headers[0x4C + index * 12 + 3] = random_.choice((0, 0, 1, 2))
        components = bytearray(random_.randbytes(0x50 + 4 * 0xA0))
        struct.pack_into("<I", components, 0x44, 0x50)
        for component in range(4):
            struct.pack_into("<II", components, 0x50 + component * 0xA0 + 4, random_.choice((0, 1 << 11)), salted(random_.randrange(COUNT)))
        t.check(
            "havok_unknown_002276d0",
            random_.choice((NONE, salted(random_.randrange(4)))),
            random_.choice((NONE, salted(random_.randrange(4)))),
            globals={
                "object_header_data": Buffer(bytes(headers), pointers=table.pointers),
                "havok_component_data": Buffer(bytes(components), pointers=(0x44,)),
            },
        )

        t.check("havok_unknown_001caed0", Buffer(random_.randbytes(0x84)))

        t.check(
            "network_unknown_00070190",
            Buffer(random_.randbytes(0xA98), places=(0x08,)),
            Buffer(random_.randbytes(0x20), places=(0x1C,)),
            globals={
                "global_random_seed_reference": Buffer(random_.randbytes(8)),
                "unknown_004ce1d4": random_.choice((0, 100, 0x7FFF, 0x8000, random_.getrandbits(32))),
                "unknown_004ce1dc": random_.choice((0, 30, 0xFFFF, random_.getrandbits(32))),
            },
        )
