# the last function of the part of the decals

import random


def run(t) -> None:
    random_ = random.Random(667)

    for n in range(100):
        t.check(
            "decals_activation_changed",
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            externals={"unknown_0023aad0": lambda harness, arguments: 0},
        )
