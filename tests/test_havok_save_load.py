# what the physics do before and after the state of the game is written or read

import random

from test_data import array

OTHERS = (
    "unknown_001c2b10",
    "havok_unknown_001c29d0",
    "havok_unknown_001c2890",
    "unknown_001c2910",
    "unknown_001c2a10",
    "havok_unknown_001c39c0",
    "unknown_0030be40",
)


def some(random_, maximum_count: int, size: int):
    count = random_.randint(0, maximum_count)
    in_use = {index: random_.randint(1, 0x7FFF) for index in random_.sample(range(maximum_count), count)}
    return array(random_, maximum_count, size, in_use)


def run(t) -> None:
    random_ = random.Random(653)

    for n in range(200):
        flags = random_.choice((0, 1, 2, 3, random_.getrandbits(32)))
        for name in ("havok_before_save_or_load", "havok_after_save_or_load"):
            t.check(
                name,
                flags,
                globals={
                    "impact_data": some(random_, 0x20, 0xA0),
                    "impact_array_data": some(random_, 0x20, 0x40),
                    "unknown_0047f04c": random_.choice((0xFFFFFFFF, 0, 1, random_.getrandbits(32))),
                    "unknown_0047f058": random_.randbytes(1),
                    "unknown_0047f059": random_.randbytes(1),
                    "unknown_0047f048": random_.getrandbits(32),
                    "unknown_0051ecac": random_.getrandbits(32),
                },
                externals={other: (lambda harness, arguments: 0) for other in OTHERS},
            )
