# the thing that gives the physics its second memory

import random

from difftest import Buffer, GlobalRange

THING = 0x105410


def run(t) -> None:
    random_ = random.Random(649)

    for n in range(40):
        memory = (random_.getrandbits(27) << 4) + random_.choice((0, 0, 1, 4, 8, 0xC, 0xF, random_.randrange(16)))
        # nothing is made when the place of the thing is the address zero
        place = (
            (-THING) & 0xFFFFFFFF
            if n % 8 == 7
            else Buffer(bytes(THING) + random_.randbytes(0x40), places=(THING,))
        )
        t.check(
            "havok_unknown_00146b80",
            globals={
                "unknown_00510c44": memory,
                "unknown_00479888": place,
                "unknown_0047989c": random_.getrandbits(32),
                "unknown_00510c48": random_.randbytes(1),
                "unknown_004576a4": GlobalRange(4),
            },
        )
