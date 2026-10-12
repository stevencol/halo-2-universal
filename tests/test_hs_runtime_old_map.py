# what runs the scripts, when a map goes

import random

from difftest import Buffer

from test_data import array

GLOBALS = 0x440


def run(t) -> None:
    random_ = random.Random(697)

    for n in range(120):
        some = random_.sample(range(GLOBALS), random_.choice((0, 5, 200))) + random_.sample(range(0x410, GLOBALS), random_.choice((0, 3, 20)))
        in_use = {index: random_.randint(-0x8000, 0x7FFF) or 1 for index in some}
        t.check(
            "hs_runtime_dispose_from_old_map",
            globals={
                "hs_thread_data": Buffer(random_.randbytes(0x4C + 0x10)),
                "hs_global_data": array(random_, GLOBALS, random_.choice((8, 0xC)), in_use),
                "unknown_004f9388": random_.randbytes(1),
            },
        )
