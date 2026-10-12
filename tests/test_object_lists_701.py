# how many objects of a list count

import random
import struct

from difftest import Buffer

from test_data import array
from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
LISTS, REFERENCES = 8, 30
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(701)

    for n in range(300):
        table = some_objects(random_)
        data = bytearray(table.data)
        for index in range(COUNT):
            data[FIRST + index * OBJECT + 0x10A] = random_.choice((0, 4, 4, 0xFB, 0xFF, random_.randrange(0x100)))

        headers = array(random_, LISTS, 0xC, {index: random_.randint(1, 0x7FFF) for index in range(LISTS)})
        references = array(random_, REFERENCES, 0xC, {index: random_.randint(1, 0x7FFF) for index in range(REFERENCES)})
        header_bytes = bytearray(headers.data)
        reference_bytes = bytearray(references.data)
        free = list(range(REFERENCES))
        random_.shuffle(free)
        for index in range(LISTS):
            some = [random_.getrandbits(16) << 16 | free.pop() for _ in range(random_.randint(0, 6)) if free]
            for at, one in enumerate(some):
                struct.pack_into(
                    "<2I",
                    reference_bytes,
                    0x4C + (one & 0xFFFF) * 0xC + 4,
                    random_.getrandbits(16) << 16 | random_.randrange(COUNT),
                    some[at + 1] if at + 1 < len(some) else NONE,
                )
            struct.pack_into("<I", header_bytes, 0x4C + index * 0xC + 8, some[0] if some else NONE)
        t.check(
            "object_list_count",
            random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(LISTS), random_.randrange(LISTS))),
            globals={
                "object_list_header_data": Buffer(bytes(header_bytes), pointers=headers.pointers),
                "list_object_data": Buffer(bytes(reference_bytes), pointers=references.pointers),
                "object_header_data": Buffer(bytes(data), pointers=table.pointers),
            },
        )
