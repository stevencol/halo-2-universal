# the command scripts and the props make their tables; the impacts give theirs back

import random
import struct

from difftest import Buffer, Mock, mock_address

from test_effects_initialize import tables


def a_table(random_) -> Buffer:
    """A table of data, and after it what its memory came from, with its two functions; or one that nothing gave."""
    data = bytearray(random_.randbytes(0x4C + 12))
    given = random_.choice((False, True, True))
    struct.pack_into("<I", data, 0x30, 0x4C if given else 0)
    struct.pack_into("<3I", data, 0x4C, 0x50, mock_address(0), mock_address(1))
    return Buffer(bytes(data), pointers=(0x30, 0x4C) if given else (0x4C,))


def run(t) -> None:
    random_ = random.Random(631)

    for n in range(60):
        allocation, buffers, mocks = tables(random_, (0x2174, 0x5C8))
        t.check(
            "command_scripts_initialize",
            globals={
                "game_state_allocation": allocation,
                "command_script_data": random_.getrandbits(32),
                "joint_command_script_data": random_.getrandbits(32),
            },
            extra=buffers,
            mocks=mocks,
        )
        allocation, buffers, mocks = tables(random_, (0xC46C, 0xF0CC, 0x726C))
        t.check(
            "props_initialize",
            globals={
                "game_state_allocation": allocation,
                **{name: random_.getrandbits(32) for name in ("prop_data", "prop_ref_data", "tracking_data")},
            },
            extra=buffers,
            mocks=mocks,
        )

    for n in range(300):
        t.check(
            "impacts_dispose",
            globals={"impact_data": a_table(random_), "impact_array_data": a_table(random_)},
            mocks=[
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
        )
