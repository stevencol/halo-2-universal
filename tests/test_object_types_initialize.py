# source/objects/object_types.c: the types of object are made into a list

import random
import struct

from difftest import SCRATCH_ADDRESS, Buffer, Mock, mock_address

TYPE = 0xC8
TYPES = 13
OTHERS = 4


def run(t) -> None:
    random_ = random.Random(604)

    for n in range(600):
        # the thirteen types, and four that no object is of by itself: a type is of some of those, and of itself last
        definitions = bytearray(random_.randbytes((TYPES + OTHERS) * TYPE))
        for index in range(TYPES + OTHERS):
            at = index * TYPE
            struct.pack_into("<I", definitions, at + 0x10, random_.choice((0, mock_address(index), mock_address(index))))
            struct.pack_into("<I", definitions, at + 0xC4, 0)
            if index < TYPES:
                chain = sorted(random_.sample(range(TYPES, TYPES + OTHERS), random_.choice((0, 1, 2, 4)))) + [index]
            else:
                chain = sorted(random_.sample(range(TYPES, index + 1), random_.choice((0, 1)))) + [index]
            for place in range(16):
                struct.pack_into(
                    "<I",
                    definitions,
                    at + 0x84 + place * 4,
                    SCRATCH_ADDRESS + chain[place] * TYPE if place < len(chain) else 0,
                )
        t.check(
            "object_types_initialize",
            globals={
                "object_type_definitions": struct.pack("<13I", *(SCRATCH_ADDRESS + index * TYPE for index in range(TYPES))),
                "object_type_definition_list": random_.getrandbits(32),
            },
            extra=[Buffer(bytes(definitions))],
            mocks=[Mock(lambda call: 0) for index in range(TYPES + OTHERS)],
        )
