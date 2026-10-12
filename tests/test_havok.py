# the physics part starts and stops; the random numbers start; a byte of the shell's memory

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, Memory, Mock

from test_data_modules_631 import a_table
from test_game_state import game_state_globals
from test_object_types_more import after

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(646)

    def note(harness, arguments):
        return 0, ()

    for n in range(100):
        allocated = random_.choice((0, 0x40, 0x1288))
        t.check(
            "havok_initialize",
            globals={
                "unknown_0051e9a0": random_.getrandbits(32),
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
            },
            memory={BASE + allocated: Memory(random_.randbytes(4 + 0x10))},
            externals={
                name: note
                for name in (
                    "unknown_001edbc0",
                    "impacts_initialize",
                    "havok_components_initialize",
                    "unknown_00146b30",
                    "unknown_00146b80",
                )
            },
        )

        # the memory of the console is in a buffer after the table; the number before it says where it starts
        table = a_table(random_)
        block = bytearray(random_.randbytes(0x60))
        struct.pack_into("<i", block, 0x1C, random_.choice((0, 4, 0x10, 0x20)))
        freed = random_.choice((0, 1, 1))
        t.check(
            "havok_dispose",
            globals={"havok_component_data": table, "unknown_00479888": after(table) + 0x20},
            extra=[Buffer(bytes(block))],
            mocks=[
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
            externals={
                "unknown_00146de0": note,
                "VirtualFree": lambda harness, arguments, freed=freed: freed,
                "GetLastError": lambda harness, arguments: 5,
                "impacts_dispose": note,
            },
        )

        t.check(
            "havok_initialize_for_new_map",
            globals={"unknown_0051e9cc": Buffer(random_.randbytes(2000 * 4 + 0x10))},
        )

        allocated = random_.choice((0, 0x40, 0x1288))
        time, rand, ticks = (random_.getrandbits(32) for _ in range(3))
        t.check(
            "random_math_initialize",
            globals={
                "global_random_seed_reference": random_.getrandbits(32),
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
            },
            memory={BASE + allocated: Memory(random_.randbytes(8 + 0x10))},
            externals={
                "time": lambda harness, arguments, time=time: time,
                "rand": lambda harness, arguments, rand=rand: rand,
                "GetTickCount": lambda harness, arguments, ticks=ticks: ticks,
            },
        )

        t.check("shell_memory_initialize_for_new_map", globals={"shell_memory_globals": random_.randbytes(16)})
