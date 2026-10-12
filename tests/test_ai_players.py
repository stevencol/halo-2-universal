# what the minds keep is as it starts, and the players are told to it

import random
import struct

from difftest import Buffer

from test_data import array

NONE = 0xFFFFFFFF


def things(random_) -> Buffer:
    """The two things, with a player or with none."""
    data = bytearray(random_.randbytes(0x38 + 0x10))
    for index in range(2):
        struct.pack_into("<I", data, index * 0x1C, random_.choice((NONE, NONE, random_.getrandbits(32))))
    return Buffer(bytes(data))


def game(random_) -> Buffer:
    data = bytearray(random_.randbytes(0x1200))
    struct.pack_into("<i", data, 8, random_.choice((0, 1, 1, 1, 2, 3)))
    return Buffer(bytes(data))


def run(t) -> None:
    random_ = random.Random(656)

    for n in range(300):
        t.check(
            "ai_unknown_001c80f0",
            random_.getrandbits(32),
            globals={"unknown_004f55cc": things(random_), "main_game_globals": game(random_)},
        )

    for n in range(150):
        count = random_.choice((0, 1, 2, 3, 16))
        in_use = {index: random_.randint(-0x8000, 0x7FFF) or 1 for index in random_.sample(range(16), count)}
        t.check(
            "ai_unknown_001c8010",
            globals={
                "ai_globals": Buffer(random_.randbytes(0x374 + 0x10)),
                "unknown_004f55cc": things(random_),
                "main_game_globals": game(random_),
                "player_data": array(random_, 16, 0x21C, in_use),
            },
        )
