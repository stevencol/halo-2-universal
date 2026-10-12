# the part with the cached object render states starts and takes a new map

import random

from difftest import PHYSICAL_ADDRESS, Memory

from test_data import array
from test_effects_initialize import tables
from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(644)

    for n in range(40):
        allocation, buffers, mocks = tables(random_, (0x1006C,))
        t.check(
            "cached_object_render_states_initialize",
            globals={
                "game_state_allocation": allocation,
                "cached_object_render_state_data": random_.getrandbits(32),
                "unknown_004c1bd0": random_.getrandbits(32),
            },
            extra=buffers,
            mocks=mocks,
        )

        allocation, buffers, mocks = tables(random_, (0x1006C,))
        allocated = random_.choice((0, 0x40, 0x1288))
        t.check(
            "game_unknown_0002b4a0_initialize",
            globals={
                "game_state_allocation": allocation,
                "cached_object_render_state_data": random_.getrandbits(32),
                "unknown_004c1bd0": random_.getrandbits(32),
                "game_seconds": random_.randbytes(8),
                "unknown_004ba034": random_.getrandbits(32),
                "unknown_004ba038": random_.getrandbits(32),
                "unknown_004ba03c": random_.getrandbits(32),
                "unknown_004ba054": random_.randbytes(16),
                "unknown_004ba064": random_.randbytes(16),
                "unknown_0050942c": random_.getrandbits(32),
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
            },
            memory={BASE + allocated: Memory(random_.randbytes(0x18 + 0x10))},
            extra=buffers,
            mocks=mocks,
            externals={
                "unknown_0003b8a0": lambda harness, arguments: 0,
                "unknown_000167a0": lambda harness, arguments: 0,
            },
        )

        count = random_.randint(0, 12)
        in_use = {index: random_.randint(1, 0x7FFF) for index in random_.sample(range(0x100), count)}
        t.check(
            "game_unknown_0002b4a0_initialize_for_new_map",
            globals={
                "cached_object_render_state_data": array(random_, 0x100, 0x100, in_use),
                "game_seconds": random_.randbytes(8),
                "unknown_004ba134": random_.getrandbits(32),
                "unknown_004b9970": random_.randbytes(0x30),
                "unknown_004c5018": random_.randbytes(0x6A8),
                "unknown_005093fc": random_.randbytes(1),
            },
        )
