# five functions of the way to the main menu: the five counters of a unit,
# the count of an object, an object and those under it woken, one more of
# unknown_004ed28c, and whether a camera is good

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
ENTRY = 0x18


def run(t) -> None:
    random_ = random.Random(835)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def unit_vector() -> list[float]:
        vector = [random_.uniform(-1, 1) for _ in range(3)]
        length = sum(value * value for value in vector) ** 0.5 or 1
        return [value / length for value in vector]

    for n in range(400):
        # units are 0x250 bytes here; the counters are past the short at 0x33E (in the next ones)
        table = some_objects(random_)
        objects = bytearray(table.data) + bytearray(random_.randbytes(0x400))
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<h", objects, at + 0x33E, random_.choice((0, 0x40, 0x100)))
            offset = struct.unpack_from("<h", objects, at + 0x33E)[0]
            for counter in range(5):
                place = at + offset + 0x80 + counter * 4
                objects[place:place + 4] = bytes((random_.randrange(4), random_.randrange(4), 0, random_.getrandbits(8)))
        t.check(
            "unit_unknown_00113910",
            salted(random_.randrange(COUNT)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
        )

        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<I", objects, at + 0xD4, random_.choice((NONE, salted(1))))
        game = bytearray(random_.randbytes(0x20))
        game[0x0C] = random_.choice((0, 2, 4))
        t.check(
            "object_unknown_000cea00",
            salted(random_.randrange(COUNT)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers), "main_game_globals": Buffer(bytes(game))},
            externals={"unknown_000b58c0": lambda harness, arguments: (0, tuple(arguments))},
        )

        # objects in a tree: each with those under it at 0x10 and the next at 0x0C
        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<II", objects, at + 0x0C, NONE, NONE)
            objects[0x4C + index * 12 + 3] = random_.choice((0, 1, 2, 12, 5))
        for index in range(1, COUNT):
            parent = FIRST + random_.randrange(index) * OBJECT
            struct.pack_into("<I", objects, FIRST + index * OBJECT + 0x0C, struct.unpack_from("<I", objects, parent + 0x10)[0])
            struct.pack_into("<I", objects, parent + 0x10, salted(index))
        t.check(
            "object_unknown_000bc7b0",
            salted(0),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
            externals={"object_wake": lambda harness, arguments: (0, tuple(arguments))},
        )

        entries = bytearray(random_.randbytes(0x50 + 4 * ENTRY))
        struct.pack_into("<I", entries, 0x44, 0x50)
        made = random_.choice((NONE, salted(random_.randrange(4))))
        t.check(
            "sound_unknown_00188f90",
            random_.choice((NONE, 5, 5)),
            random_.choice((NONE, 7)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={"unknown_004ed28c": Buffer(bytes(entries), pointers=(0x44,))},
            externals={"datum_new": lambda harness, arguments, made=made: (made, tuple(arguments))},
        )

        camera = bytearray(random_.randbytes(0x8C))
        camera[0] = random_.choice((0, 1, 1, 1))
        forward = unit_vector()
        up = random_.choice(([forward[1], -forward[0], 0.0], unit_vector()))
        struct.pack_into("<3f", camera, 0x2C, *forward)
        struct.pack_into("<3f", camera, 0x38, *up)
        for at in (0x04, 0x10, 0x44):
            struct.pack_into("<3f", camera, at, *(random_.choice((1.0, 1.0, 1.0, -20.0, 60000.0)) for _ in range(3)))
        if random_.random() < 0.3:
            struct.pack_into("<I", camera, 0x10, 0x7FC00000)
        struct.pack_into("<ff", camera, 0x24, random_.choice((0.5, 0.5, 2.0, -1.0)), random_.choice((0.5, 0.5, 2.0, -1.0)))
        struct.pack_into("<f", camera, 0x88, random_.choice((10.0, 10.0, 4000.0)))
        # a camera that is good but for one value of its second point
        good = bytearray(random_.randbytes(0x8C))
        good[0] = 1
        struct.pack_into("<3f", good, 0x2C, 1.0, 0.0, 0.0)
        struct.pack_into("<3f", good, 0x38, 0.0, 1.0, 0.0)
        for at in (0x04, 0x10, 0x44):
            struct.pack_into("<3f", good, at, 1.0, 1.0, 1.0)
        struct.pack_into("<ff", good, 0x24, 0.5, 0.5)
        struct.pack_into("<f", good, 0x88, 10.0)
        struct.pack_into("<I", good, 0x10 + 4 * random_.randrange(3), random_.choice((0x7FC00000, 0x7F800000, 0x47C35000)))
        t.check(
            "game_unknown_001724a0",
            Buffer(bytes(good)),
            globals={
                "unknown_0054e858": f32(0.0).to_bytes(4, "little"),
                "unknown_0054e85c": f32(3.0).to_bytes(4, "little"),
                "unknown_0054e860": f32(3.0).to_bytes(4, "little"),
            },
        )
        t.check(
            "game_unknown_001724a0",
            random_.choice((Buffer(bytes(camera)), Buffer(bytes(camera)), 0)),
            globals={
                "unknown_0054e858": f32(0.0).to_bytes(4, "little"),
                "unknown_0054e85c": f32(3.0).to_bytes(4, "little"),
                "unknown_0054e860": f32(3.0).to_bytes(4, "little"),
            },
        )
