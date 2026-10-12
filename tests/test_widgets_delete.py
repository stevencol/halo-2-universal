# the widgets of an object are deleted

import random
import struct

from difftest import Buffer, Mock, mock_address

from test_data import array
from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
WIDGETS = 0x40


def widgets(random_) -> Buffer:
    """Some widgets: the next of each is a later one, or none."""
    in_use = {index: random_.randint(1, 0x7FFF) for index in range(WIDGETS) if random_.random() < 0.8}
    table = array(random_, WIDGETS, 0xC, in_use)
    data = bytearray(table.data)
    for index in range(WIDGETS):
        later = NONE if index + 1 >= WIDGETS or random_.random() < 0.4 else (
            random_.getrandbits(16) << 16 | random_.randrange(index + 1, WIDGETS)
        )
        struct.pack_into(
            "<hII",
            data,
            0x4C + index * 0xC + 2,
            random_.randrange(3),
            random_.choice((NONE, random_.getrandbits(32), random_.getrandbits(32))),
            later,
        )
    return Buffer(bytes(data), pointers=table.pointers)


def run(t) -> None:
    random_ = random.Random(670)

    for n in range(300):
        table = some_objects(random_)
        data = bytearray(table.data)
        for index in range(COUNT):
            struct.pack_into(
                "<I",
                data,
                FIRST + index * OBJECT + 0xDC,
                random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(WIDGETS))),
            )
        kinds = bytearray(random_.randbytes(3 * 0x38))
        for kind in range(3):
            struct.pack_into("<I", kinds, kind * 0x38 + 0x1C, mock_address(kind))
        t.check(
            "widgets_delete",
            random_.getrandbits(16) << 16 | random_.randrange(COUNT),
            globals={
                "object_header_data": Buffer(bytes(data), pointers=table.pointers),
                "widget_data": widgets(random_),
                "widget_type_definitions": bytes(kinds),
            },
            mocks=[Mock(lambda call: 0, stack_arguments=1) for _ in range(3)],
        )
