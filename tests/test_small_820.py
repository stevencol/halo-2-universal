# four small functions of the way to the main menu: three bytes of an object
# cleared before it wakes, the shorts of a chain of particle emitters added,
# sixteen differences by sixteen numbers, and two names set

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
EMITTER = 0x4C


def run(t) -> None:
    random_ = random.Random(820)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def name(length: int) -> Buffer:
        return Buffer("".join(chr(random_.randrange(0x41, 0x7B)) for _ in range(length)).encode("utf-16-le") + b"\0\0")

    for n in range(400):
        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            struct.pack_into("<h", objects, FIRST + index * OBJECT + 0x12A, random_.choice((0, 0x10, 0x100)))
        t.check(
            "object_unknown_000ba3d0",
            salted(random_.randrange(COUNT)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
            externals={"object_wake": lambda harness, arguments: (0, tuple(arguments))},
        )

        emitters = bytearray(random_.randbytes(0x50 + 6 * EMITTER))
        struct.pack_into("<I", emitters, 0x44, 0x50)
        order = random_.sample(range(6), random_.randrange(6))
        for place, emitter in enumerate(order):
            following = order[place + 1] if place + 1 < len(order) else None
            struct.pack_into("<I", emitters, 0x50 + emitter * EMITTER + 8, NONE if following is None else salted(following))
        data = bytearray(random_.randbytes(8))
        struct.pack_into("<I", data, 4, salted(order[0]) if order else NONE)
        t.check("particle_unknown_00248d50", Buffer(bytes(data)), globals={"particle_emitter_data": Buffer(bytes(emitters), pointers=(0x44,))})

        big = bytearray(0xF0400)
        big[0x60:0xB0] = random_.randbytes(0x50)
        big[0xF0340:0xF03D0] = bytes(random_.randrange(256) if random_.random() < 0.3 else 0 for _ in range(0x90))
        t.check("havok_unknown_0022cb90", Buffer(bytes(big)))

        t.check(
            "network_life_cycle_unknown_000645d0",
            name(random_.choice((0, 3, 15, 16, 20))),
            name(random_.choice((0, 5, 31, 32, 40))),
            globals={"unknown_004cd86a": random_.randbytes(32), "unknown_004cd88a": random_.randbytes(64)},
        )
