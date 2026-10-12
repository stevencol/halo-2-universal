# five short functions of parts of the game: the players, the looping sounds, the levels, and one more

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(612)

    for n in range(300):
        pointer = lambda: random_.choice((0, 0x1234, random_.getrandbits(32)))
        t.check("players_dispose", globals={"player_data": pointer(), "players_globals": pointer()})
        t.check(
            "players_dispose_from_old_map",
            globals={
                "unknown_004ed39c": bytes((random_.choice((0, 0, 0, 1, 0x80)),)),
                "player_data": Buffer(random_.randbytes(0x4C)),
                "players_globals": Buffer(random_.randbytes(0x130)),
            },
            externals={
                "unknown_001c7fe0": lambda harness, arguments: (0, ()),
            },
        )
        t.check("object_looping_sounds_dispose", globals={"unknown_004ed28c": pointer()})
        t.check(
            "levels_dispose_from_old_map",
            globals={"multiplayer_level_data": Buffer(random_.randbytes(0x4C)), "campaign_level_data": Buffer(random_.randbytes(0x4C))},
        )
        reals = struct.pack(
            "<7f", *(random_.choice((0.0, 1.0, 0.0011, -2.5, 4.17, random_.uniform(-10, 10))) for _ in range(7))
        )
        t.check(
            "game_unknown_001eb550_unknown_00211030",
            globals={
                "unknown_0051e9c4": Buffer(reals),
                "unknown_004f93ac": random_.getrandbits(32),
                "unknown_004f93b0": random_.getrandbits(32),
            },
        )
