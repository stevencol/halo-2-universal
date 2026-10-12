# source/objects/object_types.c

import random
import struct

from difftest import SCRATCH_ADDRESS, Buffer, Mock, mock_address

from test_units import COUNT, OBJECT, some_objects

FIRST = 0x4C + COUNT * 12
TYPE = 0x84 + 5 * 4
TYPES = 4


def run(t) -> None:
    random_ = random.Random(599)

    for n in range(1500):
        # what the types are: a buffer that is no argument, which the harness puts after the table of the objects
        table = some_objects(random_)
        base = SCRATCH_ADDRESS + ((len(table.data) + 64 + 15) & ~15)
        definitions = bytearray(random_.randbytes(TYPES * TYPE))
        for index in range(TYPES):
            at = index * TYPE
            struct.pack_into("<I", definitions, at + 0x54, random_.choice((0, mock_address(index), mock_address(index))))
            chain = random_.sample(range(TYPES), random_.choice((0, 1, 2, 4)))
            for place in range(5):
                struct.pack_into(
                    "<I",
                    definitions,
                    at + 0x84 + place * 4,
                    base + chain[place] * TYPE if place < len(chain) else 0,
                )
        t.check(
            "object_types_call_unknown54",
            random_.getrandbits(16) << 16 | random_.randrange(COUNT),
            globals={
                "object_header_data": table,
                "object_type_definitions": struct.pack("<13I", *(base + index * TYPE for index in range(TYPES)), *range(9)),
            },
            extra=[Buffer(bytes(definitions))],
            mocks=[Mock(lambda call: 0, stack_arguments=1) for index in range(TYPES)],
        )
