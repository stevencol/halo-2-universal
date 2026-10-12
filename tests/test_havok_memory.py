# two of the functions that the physics part starts with, and one more part of the table

import random

from difftest import PHYSICAL_ADDRESS, SCRATCH_ADDRESS, Buffer, Memory

from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(647)

    for n in range(100):
        allocated = random_.choice((0, 0x40, 0x1288))
        t.check(
            "havok_unknown_001edbc0",
            globals={
                "unknown_0051e9cc": random_.choice((0, 0, random_.getrandbits(32))),
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
            },
            memory={BASE + allocated: Memory(random_.randbytes(0x1F40 + 0x10))},
        )

        # the memory that the console gives is in the only buffer, at any place of its start
        memory = 0 if n % 6 == 0 else SCRATCH_ADDRESS + random_.choice((0, 4, 0x10, 0x7FC, 0xFFC, 0x1000))
        t.check(
            "havok_unknown_00146b30",
            globals={"unknown_00479888": random_.getrandbits(32)},
            extra=[Buffer(random_.randbytes(0x2100))],
            externals={
                "VirtualAlloc": lambda harness, arguments, memory=memory: memory,
                "GetLastError": lambda harness, arguments: 8,
            },
        )

        t.check(
            "game_unknown_0003f2b0_initialize_for_new_structure_bsp",
            globals={"unknown_00509438": random_.getrandbits(32)},
        )
