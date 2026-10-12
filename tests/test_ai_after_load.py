# the minds of the characters, after the state of the game is read

import random

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(654)

    for n in range(100):
        t.check(
            "ai_after_load",
            random_.getrandbits(32),
            globals={"actor_firing_position_owners": Buffer(random_.randbytes(0x3C + 0x10), places=(0x2C, 0x30))},
        )
