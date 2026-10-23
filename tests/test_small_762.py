# five small functions of what the interface asks of the network game and
# of its two sessions

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(762)

    def says(*values):
        return lambda harness, arguments, said=random_.choice(values): (said, tuple(arguments))

    for n in range(600):
        t.check(
            "user_interface_networking_unknown_00199dc9",
            random_.getrandbits(32),
            random_.choice((0, 1, NONE)),
            random_.getrandbits(32),
            externals={
                "user_interface_networking_unknown_00199d7c": says(0, 3, 4, 4, 5, 6, 6, 7, 8),
                "network_life_cycle_unknown_000594a0": says(0, 1),
            },
        )

        membership = bytearray(random_.randbytes(0x10D0))
        struct.pack_into("<I", membership, 0x8, random_.choice((0, 1, 4, 15, NONE, random_.getrandbits(32))))
        struct.pack_into("<I", membership, 0x10CC, random_.choice((0, 1, 4, 15, 16, random_.getrandbits(32))))
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

        established = lambda harness, arguments, so=random_.choice((0, 1, 1, 1)): (so, (arguments[0] == found.address,))
        for name, getter in (
            ("user_interface_networking_get_player_count", "network_life_cycle_get_session"),
            ("user_interface_networking_get_other_player_count", "network_life_cycle_get_other_session"),
            ("user_interface_networking_get_membership_unknown8", "network_life_cycle_get_session"),
        ):
            t.check(
                name,
                extra=[membership, found],
                externals={
                    getter: session_of,
                    "network_session_established": established,
                    "network_session_get_membership": members,
                },
            )

        t.check(
            "user_interface_networking_set_unknown49c4",
            externals={
                "network_life_cycle_local_peer_is_host": says(0, 1),
                "network_life_cycle_set_unknown49c4": says(0, 1),
            },
        )
