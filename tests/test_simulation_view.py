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
                "simulation_view_set_state": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2])),
            },
        )

    # ---------- a view goes to a state

    for n in range(3000):
        view = bytearray(random_.randbytes(0x90))
        at = random_.choice((0, 1, 2, 3, 4, 5, 6))
        with_ = random_.choice((-1, 0, 5, random_.getrandbits(31)))
        struct.pack_into("<i", view, 0x28, random_.choice((0, 0, 0, 1, 7)))
        struct.pack_into("<2i", view, 0x2C, at, with_)
        struct.pack_into("<Ii", view, 0x20, 0x12345678, random_.choice((-1, 0, 3, 3)))
        t.check(
            "simulation_view_set_state",
            Buffer(bytes(view)),
            random_.choice((0, 1, 2, 3, at, at + 1, at + 1, at + 2, 7, 0xFFFFFFFF)),
            random_.choice((0xFFFFFFFF, 0xFFFFFFFF, 0, 5, with_ & 0xFFFFFFFF, with_ & 0xFFFFFFFF, 0x80000000, random_.getrandbits(32))),
            externals={
                "network_observer_channel_send": lambda harness, arguments: (
                    0,
                    (arguments[0] & 0xFF, arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], bytes(harness.mu.mem_read(arguments[6], 8))),
                ),
                "unknown_000861c0": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
        )
