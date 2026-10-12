# the lists of objects that nothing uses are deleted

import random
import struct

from difftest import Buffer

from test_data import array

NONE = 0xFFFFFFFF
LISTS, OBJECTS = 16, 40


def run(t) -> None:
    random_ = random.Random(699)

    for n in range(300):
        in_use = {index: random_.randint(-0x8000, 0x7FFF) or 1 for index in random_.sample(range(LISTS), random_.choice((0, 1, 6, LISTS)))}
        headers = array(random_, LISTS, 0xC, in_use)
        objects = array(random_, OBJECTS, 0xC, {index: random_.randint(1, 0x7FFF) for index in range(OBJECTS)})
        header_bytes = bytearray(headers.data)
        object_bytes = bytearray(objects.data)
        # each list has its own objects, one after the other
        free = list(range(OBJECTS))
        random_.shuffle(free)
        for index in range(LISTS):
            some = [random_.getrandbits(16) << 16 | free.pop() for _ in range(random_.randint(0, 3)) if free]
            for at, one in enumerate(some):
                struct.pack_into("<I", object_bytes, 0x4C + (one & 0xFFFF) * 0xC + 8, some[at + 1] if at + 1 < len(some) else NONE)
            struct.pack_into("<hHI", header_bytes, 0x4C + index * 0xC + 4, random_.choice((0, 0, 1, 3, -1)), 0, some[0] if some else NONE)
        t.check(
            "object_lists_delete_unused",
            globals={
                "object_list_header_data": Buffer(bytes(header_bytes), pointers=headers.pointers),
                "list_object_data": Buffer(bytes(object_bytes), pointers=objects.pointers),
            },
        )
