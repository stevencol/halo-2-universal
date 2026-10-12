# the records of what the characters say

import random

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

from test_data import array
from test_effects_initialize import tables
from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(633)

    for n in range(60):
        allocation, buffers, mocks = tables(random_, (0x53C,))
        allocated = random_.choice((0, 0x40, 0x1288))
        t.check(
            "vocalizations_initialize",
            globals={
                "game_state_allocation": allocation,
                "vocalization_record_data": random_.getrandbits(32),
                "unknown_004f939c": random_.getrandbits(32),
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
            },
            memory={BASE + allocated: Memory(random_.randbytes(0xFB8 + 0x10))},
            extra=buffers,
            mocks=mocks,
        )

    for n in range(120):
        in_use = {
            index: random_.randint(1, 0x7FFF) for index in random_.sample(range(0xF), random_.randint(0, 0xF))
        }
        t.check(
            "vocalizations_initialize_for_new_map",
            globals={
                "vocalization_record_data": array(random_, 0xF, 0x54, in_use),
                "unknown_004f939c": Buffer(random_.randbytes(0xFB8 + 0x10)),
            },
        )
