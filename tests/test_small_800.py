# seven small functions: a ring of entries of the simulation view, shorts
# that take the bits of others, something of 0x24 bytes, the pages of a range
# losing their bits, and three of the actors

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
PLAYER = 0x21C
BIG = 0x300


def run(t) -> None:
    random_ = random.Random(800)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        t.check(
            "simulation_view_unknown_00197120",
            Buffer(random_.randbytes(0x24)),
            globals={
                "unknown_00510ca0": bytes((random_.choice((0, 1, 1)),)),
                "unknown_00510ca1": bytes((random_.choice((0, 0, 1)),)),
                "unknown_00510ca4": random_.choice((0, 1, 998, 999, random_.randrange(1000))),
                "unknown_00515c34": bytes(1000 * 0x24),
            },
        )

        count = random_.choice((0, 1, 4, 8, -1))
        values = [random_.getrandbits(16) for _ in range(8)]
        others = [value ^ random_.choice((0, 0, 0x8000, 1, 0x4000)) for value in values]
        t.check(
            "simulation_view_unknown_001984e0",
            word(count),
            Buffer(random_.randbytes(16)),
            Buffer(struct.pack("<8H", *values)),
            Buffer(struct.pack("<8H", *others)),
        )

        t.check("simulation_view_unknown_0019ebb0", Buffer(random_.randbytes(0x24)), random_.getrandbits(32), random_.getrandbits(32))

        data = bytearray(random_.randbytes(0x1000))
        base = random_.choice((0, 0x10000, 0x7F000))
        struct.pack_into("<I", data, 4, base)
        struct.pack_into("<I", data, 0xFFC, 0x1000)
        address = base + random_.choice((0, 1, 0xFFF, 0x1000, random_.randrange(0x100000)))
        t.check(
            "memory_unknown_001a1a00",
            Buffer(bytes(data) + random_.randbytes(4), pointers=(0xFFC,)),
            address,
            random_.choice((0, 1, 0xFFF, 0x1000, 0x1001, random_.randrange(0x20000))),
            random_.getrandbits(32),
        )

        # objects of kinds 0, 1, 2 and 7
        table = some_objects(random_)
        headers = bytearray(table.data[:FIRST])
        for index in range(COUNT):
            headers[0x4C + index * 12 + 2] = random_.choice((0, 0x10, 0xEF))
            headers[0x4C + index * 12 + 3] = random_.choice((0, 1, 2, 7, 32))
            struct.pack_into("<I", headers, 0x4C + index * 12 + 8, FIRST + index * BIG)
        for index in range(COUNT):
            one = bytearray(random_.randbytes(BIG))
            one[4] = random_.choice((0, 1, 0xFE))
            struct.pack_into("<I", one, 0x14, random_.choice((NONE, 3)))
            struct.pack_into("<h", one, 0x1FC, random_.choice((-1, 0, 2)))
            struct.pack_into("<I", one, 0x2B0, f32(random_.choice((0.0, 0.5, 1.0, 2.0))))
            headers += one
        objects = lambda: {"object_header_data": Buffer(bytes(headers), pointers=table.pointers)}
        t.check("actor_unknown_001a58b0", salted(random_.randrange(COUNT)), globals=objects())

        players = bytearray(random_.randbytes(0x50 + 4 * PLAYER))
        struct.pack_into("<I", players, 0x44, 0x50)
        for player in range(4):
            struct.pack_into("<I", players, 0x50 + player * PLAYER + 0x2C, random_.choice((NONE, salted(random_.randrange(COUNT)))))
        t.check(
            "actor_unknown_001a6d30",
            salted(random_.randrange(4)),
            globals={**objects(), "player_data": Buffer(bytes(players), pointers=(0x44,))},
        )

        actors = bytearray(random_.randbytes(0x50 + 2 * 0x888))
        struct.pack_into("<I", actors, 0x44, 0x50)
        for actor in range(2):
            at = 0x50 + actor * 0x888
            actors[at + 0x86] = random_.choice((0, 5, 31, 32, 0xFF))
            actors[at + 0x266] = random_.choice((0, 1))
            actors[at + 0x267] = random_.choice((0, 0, 1))
        t.check("actor_unknown_001a7ad0", salted(random_.randrange(2)), globals={"actor_data": Buffer(bytes(actors), pointers=(0x44,))})
