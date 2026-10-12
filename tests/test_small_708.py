# seven small functions

import random
import struct

from difftest import Buffer

from test_data import array

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(708)

    for n in range(200):
        clock = bytearray(random_.randbytes(0x24))
        clock[0] = random_.choice((0, 1, 1, 0x80))
        clock[1] = random_.choice((0, 1, 0x80))
        struct.pack_into("<fi", clock, 4, random_.choice((1 / 30, 1 / 60, 0.0, 2.5)), random_.choice((0, 1, 900, -5, 0x7FFFFFFF)))
        time = {"game_time_globals": Buffer(bytes(clock))}
        t.check("game_time_get_seconds", globals={"game_time_globals": 0} if n % 9 == 0 else time)
        t.check("game_time_get", globals=time)
        t.check("game_time_get_paused", globals=time)

        players = bytearray(random_.randbytes(0x130))
        for user in range(4):
            struct.pack_into("<I", players, 0xC + user * 4, random_.choice((NONE, random_.getrandbits(32))))
        t.check(
            "local_user_has_player",
            random_.choice((NONE, 0, 1, 2, 3)),
            globals={"players_globals": Buffer(bytes(players))},
        )

        ai = bytearray(random_.randbytes(0x374))
        ai[1] = random_.choice((0, 1, 1, 0x80))
        t.check(
            "swarm_iterator_new",
            Buffer(random_.randbytes(0x14 + 4)),
            globals={"ai_globals": Buffer(bytes(ai)), "swarm_data": random_.getrandbits(32)},
        )

        t.check(
            "clump_unknown_00268c60",
            random_.getrandbits(16) << 16 | random_.randrange(0x14),
            globals={"clump_data": array(random_, 0x14, 0x50, {}), **time},
        )

        game = bytearray(random_.randbytes(0x1200))
        struct.pack_into("<I", game, 0x180, random_.choice((9, 9, 0, 8, random_.getrandbits(32))))
        t.check("game_unknown_00240140", globals={"main_game_globals": Buffer(bytes(game))})
