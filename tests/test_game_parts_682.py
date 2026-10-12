# whether two sides are enemies, or friends

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(682)

    def side() -> int:
        return random_.getrandbits(16) << 16 | random_.choice((0xFFFF, 0, 15, 16, 0x8000, random_.randrange(16), random_.randrange(16)))

    for n in range(600):
        game = bytearray(random_.randbytes(0x1200))
        struct.pack_into("<i", game, 8, random_.choice((0, 1, 1, 1, 2, 2, 3)))
        enemies = random_.choice((0, 1))
        for name in ("game_team_is_enemy", "game_team_is_friend"):
            t.check(
                name,
                side(),
                side(),
                globals={
                    "main_game_globals": Buffer(bytes(game)),
                    "unknown_004f55ec": Buffer(random_.randbytes(0xE4 + 0x10)),
                },
                externals={
                    "game_engine_teams_are_enemies": lambda harness, arguments, enemies=enemies: (
                        enemies,
                        (arguments[0] & 0xFFFF, arguments[1] & 0xFFFF),
                    )
                },
            )
