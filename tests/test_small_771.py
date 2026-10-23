# three small functions: where the thing of a player is, whether something
# is of the fifth kind with one of three numbers, and a copy of 0x130 bytes
# that is given to unknown_0019d650

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
PLAYERS, PLAYER = 4, 0x21C


def run(t) -> None:
    random_ = random.Random(771)

    for n in range(600):
        data = bytearray(random_.randbytes(0x4C + PLAYERS * PLAYER))
        struct.pack_into("<I", data, 0x44, 0x4C)
        for index in range(PLAYERS):
            at = 0x4C + index * PLAYER
            struct.pack_into("<I", data, at + 0x2C, random_.choice((NONE, NONE, 0x10003, random_.getrandbits(32))))
            struct.pack_into("<I", data, at + 0x30, random_.choice((NONE, NONE, 0x20005, random_.getrandbits(32))))
        origin = Buffer(random_.randbytes(12))
        t.check(
            "player_get_object_origin",
            random_.getrandbits(16) << 16 | random_.randrange(PLAYERS),
            origin,
            globals={"player_data": Buffer(bytes(data), pointers=(0x44,))},
            externals={"object_get_origin": lambda harness, arguments, origin=origin: (0, (arguments[0], arguments[1] == origin.address))},
        )

        unknown = bytearray(random_.randbytes(12))
        struct.pack_into("<I", unknown, 0, random_.choice((5, 5, 5, 4, 6, 0, NONE)))
        struct.pack_into("<I", unknown, 8, random_.choice((0, 1, 2, 3, 4, 12, 13, 14, NONE, random_.getrandbits(32))))
        t.check("levels_unknown_0019b70b", Buffer(bytes(unknown)))

        t.check(
            "levels_unknown_0019d620",
            Buffer(random_.randbytes(0x130)),
            externals={"unknown_0019d650": lambda harness, arguments: (0, (bytes(harness.mu.mem_read(arguments[0], 0x130)),))},
        )
