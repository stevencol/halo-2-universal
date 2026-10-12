# what two parts of the game do after a state is loaded

import random

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(625)

    def note(harness, arguments):
        return 0, tuple(arguments)

    for n in range(300):
        t.check("game_unknown_001e6a40_after_load", random_.getrandbits(32), externals={"unknown_001e75d0": note})
        t.check(
            "game_unknown_001552e0_after_load",
            random_.getrandbits(32),
            globals={"unknown_004e8c34": Buffer(random_.randbytes(8))},
            externals={"unknown_00155380": note, "unknown_00155a30": note},
        )
