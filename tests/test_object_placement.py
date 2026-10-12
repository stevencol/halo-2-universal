# the part of the game that places the objects of a structure bsp

import random

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def starts(harness, arguments) -> tuple:
    """What to compare of a call to unknown_001e95d0: the name as its text."""
    return 0, (bytes(harness.mu.mem_read(arguments[0], 0x20)).split(b"\0")[0],)


def run(t) -> None:
    random_ = random.Random(642)

    for n in range(60):
        allocated = random_.choice((0, 0x40, 0x1288))
        t.check(
            "object_placement_initialize",
            globals={
                "object_placement_globals": random_.getrandbits(32),
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
            },
            memory={BASE + allocated: Memory(random_.randbytes(4 + 0x10))},
        )
        t.check(
            "object_placement_initialize_for_new_map",
            globals={"object_placement_globals": Buffer(random_.randbytes(4 + 0x10))},
        )

    for n in range(200):
        unknown = bytearray(random_.randbytes(0x34))
        unknown[5] = random_.choice((0, 1, 1, 0x80))
        unknown[7] = random_.choice((0, 1, 1, 0x80))
        t.check(
            "object_placement_initialize_for_new_structure_bsp",
            globals={"unknown_00510c50": 0 if n % 8 == 0 else Buffer(bytes(unknown))},
            externals={
                "unknown_001e95d0": starts,
                "unknown_000d5640": lambda harness, arguments: 0,
                "unknown_001e9650": lambda harness, arguments: 0,
            },
        )
