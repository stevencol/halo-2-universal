# friends that are not allies, and a change between two sides that is counted down

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(683)

    def side() -> int:
        return random_.getrandbits(16) << 16 | random_.choice((0xFFFF, 0, 15, 16, 0x8000, random_.randrange(16), random_.randrange(4)))

    def game() -> Buffer:
        data = bytearray(random_.randbytes(0x1200))
        struct.pack_into("<i", data, 8, random_.choice((0, 1, 1, 1, 1, 2, 3)))
        return Buffer(bytes(data))

    for n in range(600):
        team, other = side(), side()
        t.check(
            "game_team_is_friend_and_not_ally",
            team,
            random_.choice((other, other, team)),
            globals={"main_game_globals": game(), "unknown_004f55ec": Buffer(random_.randbytes(0xE4 + 0x10))},
        )

        data = bytearray(random_.randbytes(0xE4 + 0x10))
        struct.pack_into("<h", data, 0, random_.choice((0, 1, 3, 7, 8, 8, 9, -1)))
        for index in range(9):
            struct.pack_into("<2h", data, 4 + index * 0x14, random_.randrange(4), random_.randrange(4))
        t.check(
            "game_team_unknown_001df6c0",
            random_.getrandbits(16) << 16 | random_.randrange(4),
            random_.getrandbits(16) << 16 | random_.randrange(4),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={"main_game_globals": game(), "unknown_004f55ec": Buffer(bytes(data))},
            externals={"game_team_change_apply": lambda harness, arguments: 0},
        )
