# four medium functions of the way to the main menu: a point in a polygon, a
# long found by eight values, two points of an object and the cluster the
# game globals keep

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(808)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def told(harness, arguments):
        return 0, tuple(arguments)

    for n in range(400):
        # a square around the origin, points in and out of it
        count = random_.choice((0, 3, 4))
        square = [(-1.0, -1.0), (1.0, -1.0), (1.0, 1.0), (-1.0, 1.0)][:count]
        if random_.random() < 0.5:
            square.reverse()
        points = b"".join(struct.pack("<2I", f32(x), f32(y)) for x, y in square) + bytes(8)
        point = struct.pack("<2I", f32(random_.choice((0.0, 0.5, 1.0, 1.01, -2.0, random_.uniform(-2, 2)))), f32(random_.choice((0.0, -1.0, 0.9, random_.uniform(-2, 2)))))
        t.check(
            "ai_point_in_polygon2d",
            Buffer(point),
            Buffer(points),
            random_.getrandbits(16) << 16 | count,
            f32(random_.choice((0.0, 0.01, 1.0))),
        )

        # elements (0x14) at 0x1C, each with entries (0x14) at 0x0C
        values = [(random_.choice((1, 2)), random_.choice((0, 1)), random_.choice((0, 1))) for _ in range(3)]
        entries = [(random_.choice((5, 6)), random_.choice((7, 8)), random_.choice((0, 1)), random_.choice((0, 1)), random_.choice((0, 1))) for _ in range(9)]
        data = bytearray(random_.randbytes(0x30 + 3 * 0x14 + 9 * 0x14))
        struct.pack_into("<iI", data, 0x1C, random_.choice((0, 1, 3)), 0x30)
        pointers = [0x20]
        for element in range(3):
            at = 0x30 + element * 0x14
            struct.pack_into("<I", data, at + 4, values[element][0])
            data[at + 0x0A], data[at + 0x0B] = values[element][1], values[element][2]
            struct.pack_into("<iI", data, at + 0x0C, random_.choice((0, 1, 3)), 0x30 + 3 * 0x14 + element * 3 * 0x14)
            pointers.append(at + 0x10)
        for entry in range(9):
            at = 0x30 + 3 * 0x14 + entry * 0x14
            e = entries[entry]
            struct.pack_into("<II", data, at + 4, e[0], e[1])
            data[at + 0x0E], data[at + 0x0F], data[at + 0x0C] = e[2], e[3], e[4]
        v = random_.choice(values)
        e = random_.choice(entries)
        t.check(
            "ai_unknown_001db070",
            Buffer(bytes(data), pointers=tuple(pointers)),
            Buffer(random_.randbytes(4)),
            v[0], v[1], v[2], e[0], e[1], e[2], e[3], e[4],
        )

        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            struct.pack_into("<I", objects, FIRST + index * OBJECT + 0xD4, random_.choice((NONE, 3)))
        t.check(
            "object_unknown_000b7740",
            salted(random_.randrange(COUNT)),
            random_.choice((0, Buffer(random_.randbytes(12)))),
            random_.choice((0, Buffer(random_.randbytes(12)))),
            random_.choice((0, 1, 0x100)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
            externals={"unknown_000b58c0": told},
        )

        objects = bytearray(table.data)
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<I", objects, at + 4, random_.choice((0, 0x100)))
            struct.pack_into("<h", objects, at + 0x2C, random_.choice((-1, 0, 4)))
        game = bytearray(random_.randbytes(0x1200))
        struct.pack_into("<hI", game, 0x11FA, random_.choice((0, 1, 1, 2, 3)), salted(random_.randrange(COUNT)))
        found = random_.choice((0, 1, 1))
        parent = salted(random_.randrange(COUNT))
        t.check(
            "main_game_unknown_00138ab0",
            globals={"main_game_globals": Buffer(bytes(game)), "object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
            externals={
                "object_try_and_get_and_verify_type": lambda harness, arguments, found=found: (found, tuple(arguments)),
                "object_get_ultimate_parent": lambda harness, arguments, parent=parent: (parent, tuple(arguments)),
            },
        )
