# the levels and the looping sounds of the objects make their tables of data

import random

from difftest import PHYSICAL_ADDRESS, Memory

from test_effects_initialize import tables
from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(615)

    for n in range(60):
        allocation, buffers, mocks = tables(random_, (0xF0, 0x1E4))
        t.check(
            "levels_initialize",
            globals={
                "heap_allocation": allocation,
                "campaign_level_data": random_.getrandbits(32),
                "multiplayer_level_data": random_.getrandbits(32),
            },
            extra=buffers,
            mocks=mocks,
        )
        allocation, buffers, mocks = tables(random_, (0x60CC,))
        allocated = random_.choice((0, 0x40, 0x1288))
        t.check(
            "object_looping_sounds_initialize",
            globals={
                "game_state_allocation": allocation,
                "unknown_004ed28c": random_.getrandbits(32),
                "unknown_004ed288": random_.getrandbits(32),
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
            },
            memory={BASE + allocated: Memory(random_.randbytes(0x244 + 0x10))},
            extra=buffers,
            mocks=mocks,
        )

        allocation, buffers, mocks = tables(random_, (0x2210,))
        allocated = random_.choice((0, 0x40, 0x1288))
        t.check(
            "players_initialize",
            globals={
                "game_state_allocation": allocation,
                "player_data": random_.getrandbits(32),
                "players_globals": random_.getrandbits(32),
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
            },
            memory={BASE + allocated: Memory(random_.randbytes(0x130 + 0x10))},
            extra=buffers,
            mocks=mocks,
        )
