# what a creature keeps of its swarm starts with nothing

import random

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(652)

    for n in range(200):
        t.check(
            "swarm_unknown_0028da50",
            Buffer(random_.randbytes(0x90)),
            globals={"unknown_00468788": Buffer(random_.choice((bytes(12), random_.randbytes(12))))},
        )
