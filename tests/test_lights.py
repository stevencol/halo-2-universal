# the lights make their table and start a new map

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

from test_data import array
from test_effects_initialize import tables
from test_game_state import game_state_globals
from test_object_types_more import after

BASE = PHYSICAL_ADDRESS + 0x60000


def fill(harness, arguments) -> tuple:
    """What to compare of a call to cluster_partition_initialize: that it is given the three things, and the name as its text."""
    name = bytes(harness.mu.mem_read(arguments[1], 0x20)).split(b"\0")[0]
    return 0, (arguments[0] == harness.global_addresses["light_cluster_partition"], name, arguments[2])


def some(random_, maximum_count: int, size: int) -> Buffer:
    count = random_.randint(0, min(maximum_count, 6))
    in_use = {index: random_.randint(1, 0x7FFF) for index in random_.sample(range(maximum_count), count)}
    return array(random_, maximum_count, size, in_use)


def run(t) -> None:
    random_ = random.Random(638)

    for n in range(40):
        allocation, buffers, mocks = tables(random_, (0x17458,))
        allocated = random_.choice((0, 0x40, 0x1288))
        t.check(
            "lights_initialize",
            globals={
                "game_state_allocation": allocation,
                "light_data": random_.getrandbits(32),
                "light_globals": random_.getrandbits(32),
                "light_cluster_partition": random_.randbytes(12),
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
            },
            memory={BASE + allocated: Memory(random_.randbytes(0xC + 0x10))},
            extra=buffers,
            mocks=mocks,
            externals={"cluster_partition_initialize": fill},
        )

    for n in range(40):
        lights = some(random_, 0x15E, 0x110)
        globals_ = Buffer(random_.randbytes(0xC + 0x10))
        numbers = Buffer(random_.randbytes(0x800 + 0x10))
        first = some(random_, 0x20, 0x10)
        second = some(random_, 0x30, 0x14)
        t.check(
            "lights_initialize_for_new_map",
            globals={
                "light_data": lights,
                "light_globals": globals_,
                "light_cluster_partition": struct.pack(
                    "<3I",
                    after(lights, globals_),
                    after(lights, globals_, numbers),
                    after(lights, globals_, numbers, first),
                ),
            },
            extra=[numbers, first, second],
        )
