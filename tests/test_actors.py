# the actors make their table

import random

from difftest import PHYSICAL_ADDRESS, Memory

from test_effects_initialize import tables
from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def new_table(harness, arguments) -> tuple:
    """What to compare of a call to hash_table_new: its name as its text, which is not at the same address in both."""
    name = bytes(harness.mu.mem_read(arguments[2], 0x20)).split(b"\0")[0]
    return (*map(harness.normalize, arguments[:2]), name, *map(harness.normalize, arguments[3:]))


def run(t) -> None:
    random_ = random.Random(634)

    for n in range(300):
        key = random_.choice((random_.getrandbits(32), random_.randrange(0x200)))
        t.check("actor_firing_position_owner_hash", key)
        other = key if n % 3 == 0 else random_.choice((random_.getrandbits(32), key ^ (1 << random_.randrange(32))))
        t.check("actor_firing_position_owner_compare", key, other)

    for n in range(20):
        allocation, buffers, mocks = tables(random_, (0x8886C,))
        allocated = random_.choice((0, 0x40, 0x1288))
        table = random_.choice((0, random_.getrandbits(32)))
        t.check(
            "actors_initialize",
            globals={
                "game_state_allocation": allocation,
                "actor_data": random_.getrandbits(32),
                "unknown_004f93a0": random_.getrandbits(32),
                "actor_firing_position_owners": random_.getrandbits(32),
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
            },
            memory={BASE + allocated: Memory(random_.randbytes(0x640 + 0x10))},
            extra=buffers,
            mocks=mocks,
            externals={"hash_table_new": lambda harness, arguments, table=table: (table, new_table(harness, arguments))},
        )
