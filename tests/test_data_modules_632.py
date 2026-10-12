# the squads and the swarms make their tables

import random

from difftest import PHYSICAL_ADDRESS, Memory

from test_effects_initialize import tables
from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(632)

    for n in range(60):
        allocation, buffers, mocks = tables(random_, (0xC760, 0x163C))
        t.check(
            "squads_initialize",
            globals={
                "game_state_allocation": allocation,
                "squad_data": random_.getrandbits(32),
                "squad_group_data": random_.getrandbits(32),
                "unknown_00557c74": random_.randbytes(20),
            },
            extra=buffers,
            mocks=mocks,
        )
        allocation, buffers, mocks = tables(random_, (0x6D0,))
        allocated = random_.choice((0, 0x40, 0x1288))
        t.check(
            "swarms_initialize",
            globals={
                "game_state_allocation": allocation,
                "swarm_data": random_.getrandbits(32),
                "unknown_005044c4": random_.getrandbits(32),
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
            },
            memory={BASE + allocated: Memory(random_.randbytes(0x190 + 0x10))},
            extra=buffers,
            mocks=mocks,
        )
