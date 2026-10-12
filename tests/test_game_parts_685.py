# the time of a change between two sides starts again

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(685)

    for n in range(600):
        game = bytearray(random_.randbytes(0x1200))
        struct.pack_into("<i", game, 8, random_.choice((0, 1, 1, 1, 1, 2)))
        data = bytearray(random_.randbytes(0xE4 + 0x10))
        struct.pack_into("<h", data, 0, random_.choice((0, 1, 3, 8, 9, -1)))
        for index in range(9):
            at = 4 + index * 0x14
            struct.pack_into("<2h", data, at, random_.randrange(3), random_.randrange(3))
            struct.pack_into("<h", data, at + 6, random_.choice((-1, 0, 30, 900)))
            data[at + 8] = random_.choice((0, 1, 0x80))
            data[at + 9] = random_.choice((0, 1, 0x80))
            struct.pack_into("<h", data, at + 0xC, random_.choice((0, 1, 4, -1)))
        t.check(
            "game_team_unknown_001df950",
            random_.getrandbits(16) << 16 | random_.randrange(3),
            random_.getrandbits(16) << 16 | random_.randrange(3),
            globals={"main_game_globals": Buffer(bytes(game)), "unknown_004f55ec": Buffer(bytes(data))},
        )
