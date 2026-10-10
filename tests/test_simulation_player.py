# source/networking/simulation_player.c, source/game/player_action.c

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(481)

    for n in range(1500):
        t.check("player_action_clear", Buffer(random_.randbytes(0x5C + 4), format="<92B4B"))
        world = bytearray(random_.randbytes(0x40))
        struct.pack_into("<i", world, 0x8, random_.choice((1, 2, 3, 3, 4, 5, 5)))
        t.check(
            "simulation_player_new",
            Buffer(random_.randbytes(0x88 + 4)),
            Buffer(random_.randbytes(12)),
            Buffer(random_.randbytes(6)),
            Buffer(bytes(world)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
        )
