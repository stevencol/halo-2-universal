# source/networking/simulation_view.c

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(447)

    for n in range(2500):
        now = random_.choice((5000, 100000, 0xFFFFFF00, random_.getrandbits(32)))
        view = bytearray(random_.randbytes(0x90))
        struct.pack_into("<i", view, 0x28, random_.choice((0, 0, 0, 1, 4, 7)))
        view[0x88] = random_.choice((0, 1, 1, 0x80))
        struct.pack_into("<I", view, 0x8C, (now - random_.choice((0, 1, 1999, 2000, 2001, 60000, -5))) & 0xFFFFFFFF)
        kept = random_.choice((0, 1, 0x80))
        t.check(
            "simulation_view_set_blocked",
            Buffer(bytes(view)),
            random_.choice((0, 1, 1, 1, 0x100)),
            globals={"unknown_00510548": bytes((kept,)), "unknown_0051054c": now if kept else random_.getrandbits(32)},
            externals={
                "GetTickCount": lambda harness, arguments, now=now: now,
                "unknown_00085a70": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2])),
            },
        )
