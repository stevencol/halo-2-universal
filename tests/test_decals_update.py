# the decals, each frame

import random
import struct

from difftest import Buffer

from test_decals_walks import decals


def run(t) -> None:
    random_ = random.Random(663)

    for n in range(200):
        cache = bytearray(random_.randbytes(0x70))
        struct.pack_into("<i", cache, 0x38, random_.choice((0, 1, 0x7FFFFFFE, 0x7FFFFFFF, random_.getrandbits(31))))
        t.check(
            "decals_update",
            globals={"decal_data": decals(random_), "unknown_00509448": Buffer(bytes(cache))},
            externals={
                "unknown_0017cc60": lambda harness, arguments: 0,
                "lruv_wrap_ticks": lambda harness, arguments: 0,
            },
        )
