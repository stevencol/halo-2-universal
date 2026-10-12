# two more walks over the lights

import random
import struct

from difftest import Buffer

from test_lights_walks import lights
from test_units import COUNT, OBJECT, some_objects

FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(693)

    for n in range(200):
        t.check(
            "lights_unknown_000c15a0",
            globals={"light_data": lights(random_)},
            externals={"unknown_000c12d0": lambda harness, arguments: 0},
        )

        object_index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        table = some_objects(random_)
        data = bytearray(table.data)
        at = FIRST + (object_index & 0xFFFF) * OBJECT + 4
        flags = struct.unpack_from("<I", data, at)[0]
        struct.pack_into("<I", data, at, flags | 0x40 if random_.random() < 0.7 else flags & ~0x40)
        # some of the lights are on the object
        some = lights(random_)
        light_bytes = bytearray(some.data)
        for index in range((len(light_bytes) - 0x4C) // 0x110):
            if random_.random() < 0.4:
                struct.pack_into("<I", light_bytes, 0x4C + index * 0x110 + 0x58, random_.choice((object_index, object_index, object_index ^ 0x10000)))
        t.check(
            "lights_unknown_000c1670",
            object_index,
            globals={
                "object_header_data": Buffer(bytes(data), pointers=table.pointers),
                "light_data": Buffer(bytes(light_bytes[: len(some.data)]), pointers=some.pointers),
            },
            externals={"unknown_000c3260": lambda harness, arguments: (0, (arguments[0], arguments[1] & 0xFF))},
        )
