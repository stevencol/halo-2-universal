# one widget of an object is deleted

import random
import struct

from difftest import Buffer, Mock, mock_address

from test_units import COUNT, OBJECT, some_objects
from test_widgets_delete import FIRST, NONE, WIDGETS, widgets


def run(t) -> None:
    random_ = random.Random(672)

    for n in range(500):
        object_index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        table = some_objects(random_)
        data = bytearray(table.data)
        first = random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(WIDGETS), random_.randrange(WIDGETS)))
        struct.pack_into("<I", data, FIRST + (object_index & 0xFFFF) * OBJECT + 0xDC, first)

        # what the widgets of the object are in the tables of their kinds: one of them is asked for, or another
        some = widgets(random_)
        theirs, at = [], first
        while at != NONE:
            what, at = struct.unpack_from("<2I", some.data, 0x4C + (at & 0xFFFF) * 0xC + 4)
            theirs.append(what)
        index = random_.choice(theirs + theirs + [NONE, random_.getrandbits(32)])

        kinds = bytearray(random_.randbytes(3 * 0x38))
        for kind in range(3):
            struct.pack_into("<I", kinds, kind * 0x38 + 0x1C, mock_address(kind))
        t.check(
            "widget_delete",
            object_index,
            index,
            globals={
                "object_header_data": Buffer(bytes(data), pointers=table.pointers),
                "widget_data": some,
                "widget_type_definitions": bytes(kinds),
            },
            mocks=[Mock(lambda call: 0, stack_arguments=1) for _ in range(3)],
        )
