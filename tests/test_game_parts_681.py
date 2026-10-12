# the part with the sixteen rows of bits, each tick

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(681)

    for n in range(400):
        data = bytearray(random_.randbytes(0xE4 + 0x10))
        struct.pack_into("<h", data, 0, random_.choice((0, -1, 1, 3, 9)))
        for index in range(9):
            at = 4 + index * 0x14
            struct.pack_into("<h", data, at + 6, random_.choice((0, 1, 30, -1)))
            struct.pack_into("<h", data, at + 0xC, random_.choice((0, 1, 1, 2, 5, -1)))
            struct.pack_into("<h", data, at + 0xE, random_.choice((0, 1, 1, 1, 2, 90, -1, -0x8000)))
        t.check(
            "game_unknown_001df460_update",
            globals={"unknown_004f55ec": Buffer(bytes(data))},
            externals={"game_team_change_apply": lambda harness, arguments: 0},
        )
