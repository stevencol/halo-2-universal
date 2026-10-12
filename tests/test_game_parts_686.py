# something between two sides is counted, and may make them enemies

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(686)

    for n in range(800):
        game = bytearray(random_.randbytes(0x1200))
        struct.pack_into("<i", game, 8, random_.choice((0, 1, 1, 1, 1, 1, 2)))
        time = random_.choice((0, 100, 5000, 0x7FFFFFF0))
        clock = bytearray(random_.randbytes(0x24))
        struct.pack_into("<h", clock, 2, random_.choice((30, 30, 60, 1, 0, 7, 13, 25)))
        struct.pack_into("<I", clock, 8, time)
        data = bytearray(random_.randbytes(0x400))
        struct.pack_into("<h", data, 0, random_.choice((0, 1, 3, 8, 8, -1)))
        for index in range(9):
            at = 4 + index * 0x14
            struct.pack_into("<2h", data, at, random_.randrange(3), random_.randrange(3))
            struct.pack_into("<2h", data, at + 4, random_.choice((-1, 1, 3, 5)), random_.choice((-1, 0, 900)))
            data[at + 8] = random_.choice((0, 1, 0x80))
            data[at + 9] = random_.choice((0, 1, 0x80))
            data[at + 0xA] = random_.choice((0, 1))
            struct.pack_into("<h", data, at + 0xC, random_.choice((0, 1, 2, 4, -1)))
            struct.pack_into("<i", data, at + 0x10, time - random_.choice((0, 1, 5, 6, 7, 12, 13, 300, -5)))
        t.check(
            "game_team_unknown_001df820",
            random_.getrandbits(16) << 16 | random_.randrange(3),
            random_.getrandbits(16) << 16 | random_.randrange(3),
            random_.getrandbits(16) << 16 | random_.choice((0, 1, 2, 3, 0xFFFF)),
            globals={
                "main_game_globals": Buffer(bytes(game)),
                "game_time_globals": Buffer(bytes(clock)),
                "unknown_004f55ec": Buffer(bytes(data)),
            },
            externals={
                "unknown_001c9830": lambda harness, arguments: (
                    0,
                    (arguments[0], arguments[1], arguments[2] & 0xFF, arguments[3] & 0xFF),
                )
            },
        )
