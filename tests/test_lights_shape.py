# what a light is like: from its definition, or from where the scenario places it

import random
import struct

from difftest import Buffer

from test_data import array

NONE = 0xFFFFFFFF
TAGS, DEFINITION_SIZE = 4, 0x40
SCENARIO, PLACEMENTS = 0x100, 5


def definition(random_) -> bytearray:
    data = bytearray(random_.randbytes(DEFINITION_SIZE))
    struct.pack_into("<h", data, 4, random_.choice((0, 0, 1, 2, 3, -1)))
    struct.pack_into("<2f", data, 0x18, random_.choice((1.0, 0.0, 30.0, random_.uniform(0.1, 9))), random_.choice((0.0, 0.0, 45.0, random_.uniform(0.1, 9))))
    return data


def placement(random_) -> bytearray:
    data = bytearray(random_.randbytes(0x6C))
    struct.pack_into("<h", data, 0x3C, random_.choice((0, 0, 1, 2, 3, -1)))
    data[0x3E] = random_.choice((0, 1, 1, 0xFE, 0xFF))
    struct.pack_into("<f", data, 0x68, random_.choice((1.0, 0.0, 12.5, random_.uniform(0.1, 9))))
    return data


def run(t) -> None:
    random_ = random.Random(692)

    for n in range(300):
        t.check("light_unknown_000c1930", Buffer(bytes(definition(random_))), Buffer(random_.randbytes(0x7C + 8)))
        t.check(
            "light_unknown_000c1980",
            Buffer(bytes(placement(random_))),
            Buffer(random_.randbytes(0x7C + 8)),
            Buffer(bytes(definition(random_))),
        )

        definition_index = random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 16)) + definition(random_)
        struct.pack_into("<I", tags, definition_index * 16 + 8, TAGS * 16)
        scenario = bytearray(random_.randbytes(SCENARIO))
        struct.pack_into("<I", scenario, 0xEC, SCENARIO)
        for _ in range(PLACEMENTS):
            scenario += placement(random_)
        lights = array(random_, 8, 0x110, {index: random_.randint(1, 0x7FFF) for index in range(8)})
        data = bytearray(lights.data)
        light_index = random_.getrandbits(16) << 16 | random_.randrange(8)
        at = 0x4C + (light_index & 0xFFFF) * 0x110
        struct.pack_into(
            "<2I",
            data,
            at + 4,
            random_.getrandbits(16) << 16 | definition_index,
            random_.choice((NONE, random_.randrange(PLACEMENTS), random_.randrange(PLACEMENTS))),
        )
        t.check(
            "light_unknown_000c19f0",
            light_index,
            Buffer(random_.randbytes(0x7C + 8)),
            globals={
                "light_data": Buffer(bytes(data), pointers=lights.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=(definition_index * 16 + 8,)),
                "global_scenario": Buffer(bytes(scenario), pointers=(0xEC,)),
            },
        )
