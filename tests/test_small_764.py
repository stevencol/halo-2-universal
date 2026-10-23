# five small functions of what the interface makes of its numbers of the
# network game

import random

NONE = 0xFFFFFFFF


def number(random_: random.Random) -> int:
    return random_.choice((*range(9), *range(9), NONE, 0x80000000, 0x100, 0x101, 0x104, random_.getrandbits(32)))


def run(t) -> None:
    random_ = random.Random(764)

    for n in range(600):
        t.check("user_interface_networking_unknown_00199951", number(random_))
        t.check(
            "user_interface_networking_unknown_00199971",
            externals={"unknown_0019989d": lambda harness, arguments, at=random_.choice((0, 1, 5, 6, 6, 7, 0x106, NONE)): (at, ())},
        )
        t.check("user_interface_networking_unknown_0019997f", number(random_))
        t.check("user_interface_networking_unknown_0019999e", number(random_))
        t.check("user_interface_networking_unknown_001999e3", number(random_))
