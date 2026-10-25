# four small functions of the way to the main menu: a procedure called for
# the entries of a list whose masks match, and three keys of a Havok entity
# set

import random
import struct

from difftest import Buffer, Mock, mock_address

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(844)

    for n in range(400):
        count = random_.randrange(6)
        # a list of 0x18 bytes, its shorts at 0x18 and its longs at 0x28
        data = bytearray(random_.randbytes(0x18 + 0x10 + 0x18))
        struct.pack_into("<iHhII", data, 0, 6, count, 0, 0x18, 0x28)
        for entry in range(6):
            struct.pack_into("<H", data, 0x18 + entry * 2, random_.choice((0, 1, 2, 0x8000, 0xFFFF)))
        t.check(
            "game_engine_list_iterate",
            Buffer(bytes(data), pointers=(8, 12)),
            random_.choice((0, 1, 2, 0xFFFF, 0x1FFFF, 0x12340001)),
            mock_address(0),
            mocks=[Mock(lambda call: 0, stack_arguments=2)],
        )

        for name, key in (("unknown30_set_2001", 0x2001), ("unknown30_set_2002", 0x2002), ("unknown30_set_2003", 0x2003)):
            pairs = random_.randrange(4)
            entity = bytearray(random_.randbytes(0x40))
            struct.pack_into("<Ii", entity, 0x30, 0x40, pairs)
            table = b"".join(struct.pack("<iI", random_.choice((key, 0x2001, 0x2002, 7)), random_.getrandbits(32)) for _ in range(4))
            t.check(
                name,
                Buffer(bytes(entity) + table, pointers=(0x30,)),
                random_.getrandbits(32),
                externals={
                    "hkEntity_property_erase": lambda harness, arguments: (0, (arguments[0], arguments[2])),
                    "hkEntity_property_put": lambda harness, arguments: (0, tuple(arguments)),
                },
            )
