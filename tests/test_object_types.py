# source/objects/object_types.c

import random
import struct

from difftest import SCRATCH_ADDRESS, Buffer, Mock, mock_address

from test_units import COUNT, OBJECT, some_objects

FIRST = 0x4C + COUNT * 12
TYPE = 0x84 + 5 * 4
TYPES = 4
# where a type has each of the functions that are given an object and nothing else
OFFSETS = (0x54, 0x20, 0x24, 0x38, 0x3C, 0x50, 0x78, 0x40, 0x44, 0x30, 0x58, 0x5C, 0x70, 0x74, 0x7C)
# those that are given things after the object, and how many
MORE = {0x30: 1, 0x58: 1, 0x5C: 1, 0x70: 1, 0x74: 2, 0x7C: 2}


def run(t) -> None:
    random_ = random.Random(599)

    for n in range(4500):
        offset = OFFSETS[n % len(OFFSETS)]
        # what the types are: a buffer that is no argument, which the harness puts after the table of the objects
        table = some_objects(random_)
        base = SCRATCH_ADDRESS + ((len(table.data) + 64 + 15) & ~15)
        definitions = bytearray(random_.randbytes(TYPES * TYPE))
        for index in range(TYPES):
            at = index * TYPE
            struct.pack_into("<I", definitions, at + offset, random_.choice((0, mock_address(index), mock_address(index))))
            chain = random_.sample(range(TYPES), random_.choice((0, 1, 2, 4)))
            for place in range(5):
                struct.pack_into(
                    "<I",
                    definitions,
                    at + 0x84 + place * 4,
                    base + chain[place] * TYPE if place < len(chain) else 0,
                )
        t.check(
            f"object_types_call_unknown{offset:X}",
            random_.getrandbits(16) << 16 | random_.randrange(COUNT),
            *(random_.getrandbits(32) for _ in range(MORE.get(offset, 0))),
            globals={
                "object_header_data": table,
                "object_type_definitions": struct.pack("<13I", *(base + index * TYPE for index in range(TYPES)), *range(9)),
            },
            extra=[Buffer(bytes(definitions))],
            mocks=[Mock(lambda call: 0, stack_arguments=1 + MORE.get(offset, 0)) for index in range(TYPES)],
        )
