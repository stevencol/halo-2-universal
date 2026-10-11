# source/interface/user_interface_networking.c, and network_life_cycle_session_get_unknown4999

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(537)

    def told(harness, arguments):
        return 0, tuple(arguments)

    for n in range(600):
        t.check("user_interface_networking_unknown_001999bf", random_.choice((0, 1, 2, 3, 4, NONE, random_.getrandbits(32))))
        t.check(
            "user_interface_networking_unknown_001999d7",
            externals={"unknown_0019989d": lambda harness, arguments, at=random_.choice((0, 1, 2, 3, 4, 6, NONE)): (at, ())},
        )
        t.check(
            "user_interface_networking_unknown_00199e2e",
            random_.choice((0, 1)),
            externals={"network_life_cycle_unknown_00059360": told, "network_life_cycle_unknown_000593a0": told},
        )
        t.check(
            "user_interface_networking_unknown_0019a015",
            externals={
                "network_life_cycle_session_is_open": lambda harness, arguments, so=random_.choice((0, 1, 1)): (so, ()),
                "unknown_00199fd6": lambda harness, arguments, more=random_.choice((0, 1, 16, NONE, 0x80000000)): (more, ()),
            },
        )

        session = bytearray(random_.randbytes(0x7420))
        struct.pack_into("<I", session, 0x741C, random_.choice((0, 1, 2, 3, 4, 8, 9, NONE, 0x80000000)))
        session[0x4998] = random_.choice((0, 1, 1, 0xFF))
        t.check(
            "network_life_cycle_session_get_unknown4999",
            globals={
                "unknown_00527330": bytes((random_.choice((0, 1, 1, 1)),)),
                "network_life_cycle_session": Buffer(bytes(session)),
            },
        )
