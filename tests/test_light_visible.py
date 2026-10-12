# whether a light is to be seen, and what it is like where it is

import random
import struct

from difftest import Buffer

from test_data import array
from test_lights_shape import PLACEMENTS, SCENARIO, TAGS, definition, placement

NONE = 0xFFFFFFFF
KINDS = 8


def run(t) -> None:
    random_ = random.Random(696)

    for n in range(500):
        definition_index = random_.randrange(TAGS)
        one = definition(random_)
        struct.pack_into("<I", one, 0, random_.choice((0, 0, 0x100, random_.getrandbits(32))))
        tags = bytearray(random_.randbytes(TAGS * 16)) + one
        struct.pack_into("<I", tags, definition_index * 16 + 8, TAGS * 16)
        scenario = bytearray(random_.randbytes(SCENARIO))
        struct.pack_into("<I", scenario, 0xEC, SCENARIO)
        for _ in range(PLACEMENTS):
            scenario += placement(random_)
        lights = array(random_, 8, 0x110, {index: random_.randint(1, 0x7FFF) for index in range(8)})
        data = bytearray(lights.data)
        light_index = random_.getrandbits(16) << 16 | random_.randrange(8)
        at = 0x4C + (light_index & 0xFFFF) * 0x110
        data[at + 2] = random_.choice((2, 2, 2, 0, 0xFF, 0xFD))
        struct.pack_into(
            "<2I",
            data,
            at + 4,
            random_.getrandbits(16) << 16 | definition_index,
            random_.choice((NONE, random_.randrange(PLACEMENTS))),
        )
        engine = bytearray(random_.randbytes(0xC9C))
        struct.pack_into("<I", engine, 0, random_.choice((0, 2, random_.getrandbits(32))))
        struct.pack_into("<I", engine, 0xC14, random_.randrange(KINDS))
        kinds = b"".join(struct.pack("<I", random_.choice((0, 0, random_.getrandbits(32)))) for _ in range(KINDS))
        globals_ = bytearray(random_.randbytes(0xC + 4))
        globals_[8] = random_.choice((1, 1, 1, 0, 0x80))

        # what the light is like where it is: the reals that are looked at
        reals = [random_.choice((0.0, 0.0001, 0.00011, 1.0, -1.0, 5e-5, 2e-4)) for _ in range(4)]

        def placed(harness, arguments, reals=reals):
            harness.mu.mem_write(arguments[1] + 0x44, struct.pack("<f", reals[0]))
            harness.mu.mem_write(arguments[1] + 0x64, struct.pack("<3f", *reals[1:]))
            return 0

        t.check(
            "light_unknown_000c17f0",
            Buffer(random_.randbytes(0x7C + 8)),
            light_index,
            random_.choice((0, 1, 1, 0x100)),
            globals={
                "light_data": Buffer(bytes(data), pointers=lights.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=(definition_index * 16 + 8,)),
                "global_scenario": Buffer(bytes(scenario), pointers=(0xEC,)),
                "light_globals": Buffer(bytes(globals_)),
                "unknown_004e9ae8": Buffer(bytes(engine)),
                "unknown_0055e4d0": kinds,
            },
            externals={"unknown_000c1a80": placed},
        )
