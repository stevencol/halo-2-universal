# the lights of the scenario are placed

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
SCENARIO = 0x200


def run(t) -> None:
    random_ = random.Random(689)

    for n in range(300):
        count = random_.choice((0, -1, 1, 4, 9))
        kinds = random_.choice((0, 1, 3))
        placements_at = SCENARIO
        kinds_at = placements_at + max(count, 1) * 0x6C
        data = bytearray(random_.randbytes(kinds_at + max(kinds, 1) * 0x28))
        struct.pack_into("<iIiI", data, 0xE8, count, placements_at, kinds, kinds_at)
        for index in range(max(count, 0)):
            struct.pack_into("<h", data, placements_at + index * 0x6C, random_.choice((-1, 0, 1, 2, 3, -2)))
        for index in range(kinds):
            struct.pack_into("<I", data, kinds_at + index * 0x28 + 4, random_.choice((NONE, random_.getrandbits(32), random_.getrandbits(32))))
        t.check(
            "lights_place_scenario_lights",
            random_.choice((0, 1, 2, 3, 4, 5, 6)),
            globals={"global_scenario": Buffer(bytes(data), pointers=(0xEC, 0xF4))},
            externals={
                # the third argument is a byte: the rest of it is whatever was on the stack
                "unknown_000c0520": lambda harness, arguments: (0, (arguments[0], arguments[1], arguments[2] & 0xFF, arguments[3]))
            },
        )
