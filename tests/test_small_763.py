# five small functions of what the interface asks of the network game: if it
# can go on, three numbers, the players of its two sessions and finding one

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
PLAYERS, PLAYER = 0x10D4, 0x13C
MEMBERSHIP = 0x4C


def players_mask(random_: random.Random) -> int:
    return random_.choice((0, 0, 1, 0x8000, 0xFFFF, 0x10000, random_.getrandbits(16), random_.getrandbits(32)))


def run(t) -> None:
    random_ = random.Random(763)

    def says(*values):
        return lambda harness, arguments, said=random_.choice(values): (said, tuple(arguments))

    for n in range(600):
        t.check(
            "user_interface_networking_unknown_0019a203",
            globals={"unknown_004d8ba0": bytes((random_.choice((0, 1, 1, 1)),))},
            externals={
                "network_life_cycle_local_peer_is_host": says(0, 1, 1),
                "network_life_cycle_get_state": says(*range(10), 8, 8, NONE, 0x80000000),
                "user_interface_networking_hopper_unknown_00193470": says(0, 1),
            },
        )

        first, second = Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4))
        t.check(
            "user_interface_networking_get_unknown4d08",
            first,
            second,
            externals={
                "network_life_cycle_get_unknown4d08": lambda harness, arguments, said=random_.choice((0, 1)): (
                    said,
                    (arguments[0] == first.address, arguments[1] == second.address),
                ),
            },
        )

        membership = bytearray(random_.randbytes(0x10D4))
        struct.pack_into("<I", membership, 0x10D0, players_mask(random_))
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
        index = random_.choice((NONE, 0, 1, 2, 7, 15, 16, 17, 0xFFFF, 0x10000, 0x10003, 0xFFFF000F, random_.getrandbits(32)))
        for name, getter in (
            ("user_interface_networking_has_player", "network_life_cycle_get_session"),
            ("user_interface_networking_other_has_player", "network_life_cycle_get_other_session"),
        ):
            t.check(
                name,
                index,
                extra=[membership, found],
                externals={
                    getter: session_of,
                    "network_session_established": established,
                    "network_session_get_membership": members,
                },
            )

        session = bytearray(random_.randbytes(MEMBERSHIP + PLAYERS + 16 * PLAYER))
        struct.pack_into("<I", session, MEMBERSHIP + 0x10D0, players_mask(random_))
        session = Buffer(bytes(session))
        identifier = Buffer(random_.randbytes(0xC))
        matching = set(random_.sample(range(16), random_.choice((0, 1, 1, 2, 5, 16))))

        def equal(harness, arguments, session=session, identifier=identifier, matching=matching):
            place = arguments[0] - (session.address + MEMBERSHIP + PLAYERS)
            return (place // PLAYER in matching and place % PLAYER == 0), (place, arguments[1] == identifier.address, arguments[2] & 0xFF)

        t.check(
            "user_interface_networking_find_player",
            random_.choice((session, session, session, 0)),
            random_.choice((identifier, identifier, identifier, 0)),
            externals={
                "network_session_established": lambda harness, arguments, so=random_.choice((0, 1, 1, 1)), session=session: (
                    so,
                    (arguments[0] == session.address,),
                ),
                "network_player_identifier_equal": equal,
            },
        )
