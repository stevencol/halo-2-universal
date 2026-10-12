# a change between two sides is put in the rows of bits

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(684)

    for n in range(300):
        t.check(
            "game_team_bit_set",
            Buffer(random_.randbytes(0x20 + 8)),
            random_.randrange(0x100),
            random_.choice((0, 1, 0x80, 0x100, 0xFFFFFF00)),
        )

    for n in range(600):
        element = bytearray(random_.randbytes(0x14 + 4))
        struct.pack_into("<2h", element, 0, *(random_.choice((0, 15, 16, -1, 0x7FFF, random_.randrange(16), random_.randrange(16))) for _ in range(2)))
        element[0xA] = random_.choice((0, 1, 1, 0x80))
        # a side below zero is before the rows: the bits are in the middle of what is kept, and more
        t.check(
            "game_team_change_apply",
            Buffer(bytes(element)),
            random_.choice((0, 1, 1, 0x80)) | random_.getrandbits(24) << 8,
            random_.choice((0, 0, 1, 0x80)) | random_.getrandbits(24) << 8,
            globals={"unknown_004f55ec": Buffer(random_.randbytes(0x400))},
            externals={
                "unknown_001c9830": lambda harness, arguments: (
                    0,
                    (arguments[0], arguments[1], arguments[2] & 0xFF, arguments[3] & 0xFF),
                )
            },
        )
