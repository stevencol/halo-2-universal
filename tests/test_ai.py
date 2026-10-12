# the part of the game that the minds of the characters are

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, Memory, Mock, mock_address

from test_effects_initialize import tables
from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000

OTHERS = (
    "actors_initialize",
    "swarms_initialize",
    "props_initialize",
    "squads_initialize",
    "command_scripts_initialize",
    "vocalizations_initialize",
    "unknown_00292130",
    "unknown_001a6d80",
)


def run(t) -> None:
    random_ = random.Random(635)

    for n in range(100):
        t.check(
            "ai_unknown_001c7fa0",
            globals={
                "ai_globals": Buffer(random_.randbytes(0x374 + 0x10)),
                "unknown_004f55cc": Buffer(random_.randbytes(0x38 + 0x10)),
            },
        )

    for n in range(40):
        allocation, buffers, mocks = tables(random_, (0x690, 0xF00, 0x440C, 0x1E0))
        allocated = random_.choice((0, 0x40, 0x1288))
        t.check(
            "ai_initialize",
            globals={
                "game_state_allocation": allocation,
                **{
                    name: random_.getrandbits(32)
                    for name in (
                        "ai_globals",
                        "unknown_004f55cc",
                        "unknown_005044cc",
                        "unknown_005044d0",
                        "unknown_005047f4",
                        "clump_data",
                        "joint_state_data",
                        "dynamic_firing_point_data",
                        "flock_data",
                    )
                },
                "unknown_004f5768": random_.randbytes(2),
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
            },
            memory={BASE + allocated: Memory(random_.randbytes(0x400 + 0x10))},
            extra=buffers,
            mocks=mocks,
            externals={name: (lambda harness, arguments: 0) for name in OTHERS},
        )

    for n in range(100):
        # the table, and after it what gave its memory, with its two functions
        table = bytearray(random_.randbytes(0x3C + 12))
        struct.pack_into("<I", table, 0x34, 0x3C)
        struct.pack_into("<3I", table, 0x3C, 0x40, mock_address(0), mock_address(1))
        t.check(
            "ai_dispose",
            globals={"actor_firing_position_owners": Buffer(bytes(table), pointers=(0x34, 0x3C))},
            mocks=[
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
        )
