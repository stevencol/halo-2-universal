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
                "user_interface_networking_unknown_00199fd6": lambda harness, arguments, more=random_.choice((0, 1, 16, NONE, 0x80000000)): (more, ()),
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

    # ---------- three more short ones

    for n in range(600):
        session = bytearray(random_.randbytes(0x7420))
        struct.pack_into("<I", session, 0x741C, random_.choice((0, 1, 2, 3, 4, 8, 9, NONE, 0x80000000)))
        t.check(
            "network_life_cycle_session_get_unknown49C8",
            globals={
                "unknown_00527330": bytes((random_.choice((0, 1, 1, 1)),)),
                "network_life_cycle_session": Buffer(bytes(session)),
            },
        )

        membership = bytearray(random_.randbytes(0x10D0))
        struct.pack_into("<I", membership, 0x10CC, random_.choice((0, 1, 4, 15, 16, random_.randrange(17))))
        membership = Buffer(bytes(membership))
        found = Buffer(random_.randbytes(0x10))

        def session_of(harness, arguments, there=random_.choice((0, 1, 1, 1))):
            harness.mu.mem_write(arguments[0], struct.pack("<I", found.address))
            return there, ()

        def members(harness, arguments):
            # the places for the two numbers are the function's own
            harness.mu.mem_write(arguments[0], struct.pack("<I", 3))
            harness.mu.mem_write(arguments[2], struct.pack("<I", 5))
            return membership.address, (arguments[1] == found.address,)

        t.check(
            "user_interface_networking_unknown_00199fd6",
            extra=[membership, found],
            externals={
                "network_life_cycle_get_session": session_of,
                "network_session_established": lambda harness, arguments, so=random_.choice((0, 1, 1, 1)): (so, (arguments[0] == found.address,)),
                "network_session_get_membership": members,
            },
        )
        t.check(
            "random_seed_from_time",
            externals={
                "time": lambda harness, arguments, now=random_.getrandbits(32): (now, tuple(arguments)),
                "rand": lambda harness, arguments, random=random_.getrandbits(15): (random, ()),
                "GetTickCount": lambda harness, arguments, ticks=random_.getrandbits(32): (ticks, ()),
            },
        )
