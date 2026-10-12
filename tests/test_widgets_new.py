# the widgets of an object are made

import random
import struct

from difftest import Buffer, Mock, mock_address

from test_data import array
from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
WIDGETS = 8
TAGS = 4
DEFINITION, ELEMENTS = TAGS * 16, TAGS * 16 + 0xB0
GROUPS = (0x616E7421, 0x636C7764, 0x7464746C)


def run(t) -> None:
    random_ = random.Random(671)

    for n in range(400):
        object_index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        definition_index = random_.randrange(TAGS)
        table = some_objects(random_)
        data = bytearray(table.data)
        struct.pack_into(
            "<I", data, FIRST + (object_index & 0xFFFF) * OBJECT, random_.getrandbits(16) << 16 | definition_index
        )

        # the tags, the definition of the object and its widgets
        count = random_.choice((0, -1, 1, 2, 5))
        tags = bytearray(random_.randbytes(ELEMENTS + max(count, 1) * 8))
        struct.pack_into("<I", tags, definition_index * 16 + 8, DEFINITION)
        struct.pack_into("<iI", tags, DEFINITION + 0x9C, count, ELEMENTS)
        for index in range(max(count, 0)):
            struct.pack_into(
                "<2I",
                tags,
                ELEMENTS + index * 8,
                random_.choice(GROUPS + GROUPS + (random_.getrandbits(32),)),
                random_.choice((NONE, random_.getrandbits(32), random_.getrandbits(32), random_.getrandbits(32))),
            )

        # the kinds: some make their own, and some of those cannot
        kinds = bytearray(random_.randbytes(3 * 0x38))
        mocks = []
        for kind in range(3):
            struct.pack_into("<I", kinds, kind * 0x38, GROUPS[kind])
            if random_.random() < 0.75:
                struct.pack_into("<I", kinds, kind * 0x38 + 0x18, mock_address(len(mocks)))
                made = random_.choice((NONE, random_.getrandbits(32), random_.getrandbits(32)))
                mocks.append(Mock(lambda call, made=made: made, stack_arguments=2))
            else:
                struct.pack_into("<I", kinds, kind * 0x38 + 0x18, 0)

        in_use = {index: random_.randint(1, 0x7FFF) for index in random_.sample(range(WIDGETS), random_.choice((0, 3, 6, WIDGETS)))}
        t.check(
            "widgets_new",
            object_index,
            globals={
                "object_header_data": Buffer(bytes(data), pointers=table.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=(definition_index * 16 + 8, DEFINITION + 0xA0)),
                "widget_data": array(random_, WIDGETS, 0xC, in_use),
                "widget_type_definitions": bytes(kinds),
            },
            mocks=mocks,
        )
