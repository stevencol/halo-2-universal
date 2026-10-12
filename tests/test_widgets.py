# the widgets and the table of their kinds

import random
import struct

from difftest import Buffer, Mock, mock_address

from test_data import array
from test_effects_initialize import tables

TYPES, TYPE_SIZE = 3, 0x38


def kinds(random_, offset: int, first_mock: int) -> tuple:
    """The table of the kinds, some of them with a function at an offset; and the mocks of those functions."""
    table = bytearray(random_.randbytes(TYPES * TYPE_SIZE))
    mocks = []
    for index in range(TYPES):
        if random_.random() < 0.7:
            struct.pack_into("<I", table, index * TYPE_SIZE + offset, mock_address(first_mock + len(mocks)))
            mocks.append(Mock(lambda call: 0, stack_arguments=0))
        else:
            struct.pack_into("<I", table, index * TYPE_SIZE + offset, 0)
    return bytes(table), mocks


def run(t) -> None:
    random_ = random.Random(641)

    for n in range(100):
        allocation, buffers, mocks = tables(random_, (0x354,))
        table, more = kinds(random_, 8, len(mocks))
        t.check(
            "widgets_initialize",
            globals={
                "game_state_allocation": allocation,
                "widget_data": random_.getrandbits(32),
                "widget_type_definitions": table,
            },
            extra=buffers,
            mocks=mocks + more,
        )

    for n in range(100):
        count = random_.randint(0, 0x40)
        in_use = {index: random_.randint(1, 0x7FFF) for index in random_.sample(range(0x40), count)}
        table, mocks = kinds(random_, 0xC, 0)
        t.check(
            "widgets_initialize_for_new_map",
            globals={"widget_data": array(random_, 0x40, 0xC, in_use), "widget_type_definitions": table},
            mocks=mocks,
        )
        table, mocks = kinds(random_, 0x10, 0)
        t.check(
            "widgets_dispose_from_old_map",
            globals={"widget_data": Buffer(random_.randbytes(0x4C + 0x10)), "widget_type_definitions": table},
            mocks=mocks,
        )
