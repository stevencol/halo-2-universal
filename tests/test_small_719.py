# six small functions that read the scenario

import random
import struct

from difftest import Buffer

from test_small_717 import TAGS, tags

NONE = 0xFFFFFFFF
POINTS, UNKNOWN10C, UNKNOWN150, SQUADS, SCRIPTS = 0x400, 0x500, 0x620, 0x640, 0x7C0


def a_scenario(random_) -> Buffer:
    """A scenario with its blocks after it."""
    data = bytearray(random_.randbytes(0x840))
    struct.pack_into("<I", data, 0x11C, POINTS)
    struct.pack_into("<I", data, 0x10C, UNKNOWN10C)
    for index in range(4):
        struct.pack_into("<h", data, UNKNOWN10C + index * 0x44 + 0x40, random_.choice((-1, random_.randrange(256))))
    struct.pack_into("<2I", data, 0x150, 3, UNKNOWN150)
    for index in range(3):
        named = random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(TAGS)))
        struct.pack_into("<I", data, UNKNOWN150 + index * 8 + 4, named)
    struct.pack_into("<I", data, 0x244, SQUADS)
    for index in range(3):
        struct.pack_into("<h", data, SQUADS + index * 0x7C + 0x4C, random_.choice((-1, 0, 1, 2)))
    struct.pack_into("<I", data, 0x1BC, SCRIPTS)
    struct.pack_into("<I", data, 0x3C0, random_.choice((0, 1, 5, 15)))
    return Buffer(bytes(data), pointers=(0x11C, 0x10C, 0x154, 0x244, 0x1BC))


def run(t) -> None:
    random_ = random.Random(719)

    for n in range(300):
        t.check(
            "scenario_get_unknown11C",
            random_.randrange(8),
            random_.choice((0, Buffer(random_.randbytes(12)), Buffer(random_.randbytes(12)))),
            random_.choice((0, Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4)))),
            globals={"global_scenario": a_scenario(random_)},
        )
        count = random_.choice((0, 1, 3, 24, 0xFFFFFFFF))
        named = b"".join(struct.pack("<I", random_.randrange(8)) for _ in range(24)) + struct.pack("<I", count)
        t.check(
            "unknown_get_scenario_unknown11C_points",
            Buffer(named),
            Buffer(random_.randbytes(24 * 12)),
            globals={"global_scenario": a_scenario(random_)},
        )
        thing = bytearray(random_.randbytes(0x24))
        struct.pack_into("<h", thing, 0x20, random_.choice((-1, -0x8000, 0, 1, 2, 3, 5)))
        t.check(
            "unknown_get_scenario_unknown150_tag",
            Buffer(bytes(thing)),
            globals={"global_scenario": a_scenario(random_), "global_tag_instances": tags(random_)},
        )
        t.check(
            "game_unknown_001ec420_clear_bit",
            random_.choice((NONE, 0, 1, 2, 3)),
            globals={
                "global_scenario": a_scenario(random_),
                "game_unknown_001ec420_globals": Buffer(random_.choice((b"\xff" * 0x24, random_.randbytes(0x24)))),
            },
        )
        t.check(
            "game_unknown_00134d90_clear_bits",
            globals={
                "global_scenario": 0 if n % 7 == 0 else a_scenario(random_),
                "unknown_004e6740": 0 if n % 5 == 0 else Buffer(random_.choice((b"\xff" * 0x204, random_.randbytes(0x204)))),
            },
        )
        answer = random_.getrandbits(32)
        t.check(
            "squad_definition_run_script",
            random_.getrandbits(16) << 16 | random_.randrange(3),
            random_.getrandbits(32),
            globals={"global_scenario": a_scenario(random_), "unknown_0050242c": random_.getrandbits(32)},
            externals={"hs_unknown_002093d0": lambda harness, arguments, answer=answer: (answer, tuple(arguments))},
        )
