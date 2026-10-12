# a new light at a place, or on an object

import random
import struct

from difftest import Buffer

from test_data import array
from test_real_math import f32
from test_units import COUNT, some_objects

SIZE = 0x110
NONE = 0xFFFFFFFF
TAGS, DEFINITION_SIZE = 6, 0xB8


def vector(random_) -> Buffer:
    return Buffer(struct.pack("<3f", *(random_.uniform(-50, 50) for _ in range(3))) + random_.randbytes(4))


def run(t) -> None:
    random_ = random.Random(690)

    for n in range(400):
        definition_index = random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 16 + DEFINITION_SIZE))
        struct.pack_into("<I", tags, definition_index * 16 + 8, TAGS * 16)
        struct.pack_into("<f", tags, TAGS * 16 + 0xB0, random_.choice((0.0, -1.0, 0.5, 30.0, float("nan"))))
        maximum_count = random_.choice((0x15E, 40, 4, 1))
        count = random_.choice((0, maximum_count, random_.randint(0, maximum_count)))
        in_use = {index: random_.randint(-0x8000, 0x7FFF) or 1 for index in random_.sample(range(maximum_count), count)}
        clock = bytearray(random_.randbytes(0x24))
        t.check(
            "light_new_at",
            definition_index | random_.getrandbits(16) << 16,
            random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(COUNT))),
            random_.getrandbits(32),
            vector(random_),
            vector(random_),
            vector(random_),
            f32(random_.choice((1.0, 0.0, 2.5, random_.uniform(0, 8)))),
            globals={
                "global_tag_instances": Buffer(bytes(tags), pointers=(definition_index * 16 + 8,)),
                "game_time_globals": Buffer(bytes(clock)),
                "light_data": array(random_, maximum_count, SIZE, in_use),
                "object_header_data": some_objects(random_),
                "unknown_004e0308": random_.getrandbits(32),
            },
            externals={"unknown_000c0c80": lambda harness, arguments: 0},
        )
