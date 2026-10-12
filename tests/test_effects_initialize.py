# source/effects: the effects, the contrails and the decals make their tables of data

import random
import struct

from difftest import PHYSICAL_ADDRESS, SCRATCH_ADDRESS, Buffer, Memory, Mock, mock_address

from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def an_allocation() -> Buffer:
    """Something that gives memory: where its functions are, and the two of them, which the test stands in for."""
    return Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,))


def tables(random_, sizes: tuple) -> tuple:
    """The memory for some tables, after the buffer of what gives it; and the function that gives it, by the size that is asked for, or gives none."""
    allocation = an_allocation()
    at = SCRATCH_ADDRESS + ((len(allocation.data) + 64 + 15) & ~15)
    given, buffers = {}, []
    for size in sizes:
        buffers.append(Buffer(random_.randbytes(size)))
        given[size] = random_.choice((at, at, at, 0))
        at += (size + 64 + 15) & ~15
    mocks = [
        Mock(lambda call, given=given: given[call.stack[0]], stack_arguments=1, registers=("ecx",)),
        Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
    ]
    return allocation, buffers, mocks


def run(t) -> None:
    random_ = random.Random(614)

    def note(harness, arguments):
        return 0, ()

    for n in range(40):
        allocation, buffers, mocks = tables(random_, (0x1906C, 0x788C))
        t.check(
            "effects_initialize",
            globals={
                "game_state_allocation": allocation,
                "effect_data": random_.getrandbits(32),
                "effect_location_data": random_.getrandbits(32),
            },
            extra=buffers,
            mocks=mocks,
            externals={"unknown_00173de0": note},
        )
        allocation, buffers, mocks = tables(random_, (0x486C, 0xE0CC))
        t.check(
            "contrails_initialize",
            globals={
                "game_state_allocation": allocation,
                "contrail_data": random_.getrandbits(32),
                "contrail_point_data": random_.getrandbits(32),
            },
            extra=buffers,
            mocks=mocks,
        )
        allocation, buffers, mocks = tables(random_, (0x100CC,))
        allocated = random_.choice((0, 0x40, 0x1288))
        t.check(
            "decals_initialize",
            globals={
                "game_state_allocation": allocation,
                "decal_data": random_.getrandbits(32),
                "decal_globals": random_.getrandbits(32),
                "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
            },
            memory={BASE + allocated: Memory(random_.randbytes(0x3C2C + 0x10))},
            extra=buffers,
            mocks=mocks,
            externals={"unknown_00043890": note},
        )
