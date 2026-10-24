# four medium functions of the way to the main menu: something of the AI made
# from its values, what a network session keeps, whether a unit is in some
# state, and a real of something by a name

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
BIG = 0x200
NAMES = (0x180005FC, 0x0D0005FF, 0x170005FA, 0x170005FB, 0x1D0005FD, 0x1D0005FE, 0x180005FD, 0, 0x1D0005FF)


def run(t) -> None:
    random_ = random.Random(806)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        t.check(
            "ai_unknown_002055f0",
            Buffer(random_.randbytes(0x40)),
            Buffer(random_.randbytes(12)),
            *(random_.getrandbits(32) for _ in range(7)),
            Buffer(random_.randbytes(12)),
            random_.getrandbits(32),
            random_.choice((0, Buffer(random_.randbytes(4)))),
        )

        session = bytearray(random_.randbytes(0x7420))
        struct.pack_into("<i", session, 0x741C, random_.choice((0, 2, 3, 5, 8, 9)))
        out = lambda: random_.choice((0, Buffer(random_.randbytes(4))))
        place = lambda: random_.choice((0, Buffer(random_.randbytes(4), places=(0,))))
        t.check(
            "network_session_unknown_000656e0",
            Buffer(bytes(session)),
            out(), out(), out(), out(), out(), place(), out(), out(), place(),
        )

        table = some_objects(random_)
        headers = bytearray(table.data[:FIRST])
        objects = bytearray()
        for index in range(COUNT):
            struct.pack_into("<I", headers, 0x4C + index * 12 + 8, FIRST + index * BIG)
            one = bytearray(random_.randbytes(BIG))
            struct.pack_into("<I", one, 0, salted(random_.randrange(4)))
            struct.pack_into("<h", one, 0x12A, 0x60)
            struct.pack_into("<I", one, 0x60, random_.choice((NONE, 0)))
            struct.pack_into("<h", one, 0x66, random_.choice((-1, 0)))
            struct.pack_into("<I", one, 0x60 + 0x68, random_.choice((NONE, 1, 1)))
            one[0x137] = random_.choice((0, 1))
            one[0x149] = random_.choice((0, 0x80))
            one[0x1F6] = random_.choice((0xFF, 0))
            one[0x1F7] = random_.choice((0xFF, 0))
            objects += one
        tags = bytearray(random_.randbytes(0x40 + 4 * 0xC0))
        for tag in range(4):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x40 + tag * 0xC0)
            struct.pack_into("<I", tags, 0x40 + tag * 0xC0 + 0xBC, random_.choice((0, 1 << 11)))
        t.check(
            "unit_unknown_00113260",
            salted(random_.randrange(COUNT)),
            globals={
                "object_header_data": Buffer(bytes(headers) + bytes(objects), pointers=table.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=tuple(tag * 0x10 + 8 for tag in range(4))),
                "unknown_004686a4": bytes((random_.choice((0, 1, 1)),)),
            },
        )

        t.check("unit_unknown_0014ccf0", random_.choice(NAMES), Buffer(random_.randbytes(8)), Buffer(random_.randbytes(4)))
