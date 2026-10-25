# one function of the way to the main menu (guided by halo2-decompiled): the
# animations of a unit starting over

import random
import struct

from difftest import Buffer

from test_small_859 import BIG, big_objects
from test_units import COUNT


def run(t) -> None:
    random_ = random.Random(881)

    for n in range(400):
        units, pointers = big_objects(random_)
        index = random_.randrange(COUNT)
        at = 0x4C + COUNT * 12 + index * BIG
        struct.pack_into("<I", units, at, random_.randrange(2))
        struct.pack_into("<hh", units, at + 0x12A, 0x300, 0)
        struct.pack_into("<h", units, at + 0x33E, 0x200)
        tags = bytearray(random_.randbytes(0x20 + 2 * 0x40))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 16 + 8, 0x20 + tag * 0x40)
        told = {name: (lambda harness, arguments: (0, tuple(arguments)))
                for name in ("weapon_unknown_0010f040", "unit_unknown_0010e920", "unit_unknown_0010dbc0", "object_unknown_00113870")}
        t.check(
            "unit_unknown_0010f260",
            random_.getrandbits(16) << 16 | index,
            globals={
                "object_header_data": Buffer(bytes(units), pointers=tuple(pointers)),
                "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18)),
            },
            externals={
                **told,
                "tag_unknown_0016da90": lambda harness, arguments, entries=(random_.randrange(-1, 9), random_.randrange(-1, 9)): (
                    entries[arguments[1] == 0x090001A3] & 0xFFFFFFFF,
                    tuple(arguments),
                ),
            },
        )
