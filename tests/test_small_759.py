# four small functions of the network game: a number of a player of a roster
# by one of the list of games, setting the number of 0x49C8 of the session,
# and two things that are asked of a session

import random
import struct

from difftest import Buffer
from test_network_hoppers import hopper

NONE = 0xFFFFFFFF
ROSTER = 0x308
SESSION = 0x7424
STATES = (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, NONE, 0x80000000)


def roster(random_: random.Random) -> bytes:
    data = bytearray(random_.randbytes(ROSTER))
    count = random_.choice((0, 1, 4, 16, random_.randint(0, 16)))
    struct.pack_into("<i", data, 0x184, count)
    for index in range(16):
        struct.pack_into("<i", data, 0x248 + index * 4, random_.choice((-1, 0, 1, 5, 20, 64, 127, random_.randrange(128))))
        struct.pack_into("<i", data, 0x2C8 + index * 4, random_.choice((0, 1, 2, 3, -1)))
    return bytes(data)


def run(t) -> None:
    random_ = random.Random(759)

    for n in range(300):
        one = bytearray(hopper(random_))
        for index in range(128):
            one[0x51C + index] = random_.choice((index, index, index + 1, min(127, index + 5), 127))
        found = Buffer(bytes(one))
        index = random_.choice((0, 1, 3, 15, NONE, random_.randrange(16)))
        t.check(
            "network_session_roster_player_unknown_00063220",
            random_.randrange(16),
            Buffer(roster(random_)),
            index,
            extra=[found],
            externals={
                "network_hoppers_get": lambda harness, arguments, index=index, there=random_.choice((0, 1, 1)): (
                    found.address if index != 0 or there else 0,
                    tuple(arguments),
                ),
            },
        )

        # the session of the network game
        session = bytearray(random_.randbytes(0x7420))
        struct.pack_into("<I", session, 0x741C, random_.choice(STATES))
        if random_.random() < 0.5:
            session[0x72D8:0x72DC] = session[0x50:0x54]
        struct.pack_into("<I", session, 0x4980, random_.choice((1, 1, 0)))
        t.check(
            "network_life_cycle_set_unknown49c8",
            random_.getrandbits(32),
            globals={
                "unknown_00527330": bytes((random_.choice((0, 1, 1, 1)),)),
                "network_life_cycle_session": Buffer(bytes(session)),
            },
            externals={
                "network_session_leader_set_unknown49c8": lambda harness, arguments, said=random_.choice((0, 1, 1)): (
                    said,
                    (harness.normalize(arguments[0]), arguments[1]),
                ),
                "unknown_00074aa0": lambda harness, arguments: (0, ()),
                "unknown_00065770": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
        )

        # something with its session at 4, and another thing
        thing = bytearray(8) + bytearray(random_.randbytes(SESSION))
        struct.pack_into("<I", thing, 4, 8)
        struct.pack_into("<I", thing, 8 + 0x741C, random_.choice(STATES))
        other = bytearray(random_.randbytes(0x174))
        struct.pack_into("<ii", other, 0x16C, random_.choice((0, 5, 10)), random_.choice((0, 5, 10, 11)))
        t.check(
            "network_life_cycle_unknown_0006dcc0",
            Buffer(bytes(thing), pointers=(4,)),
            Buffer(bytes(other)),
            externals={
                "network_hoppers_get": lambda harness, arguments, there=random_.choice((0, 1, 1)): (0x1234 if there else 0, tuple(arguments)),
            },
        )

        session = bytearray(random_.randbytes(SESSION))
        struct.pack_into("<I", session, 0x741C, random_.choice(STATES + (5, 5, 7)))
        session[0x7420] = random_.choice((0, 1))
        t.check(
            "network_life_cycle_unknown_0006e6b0",
            Buffer(bytes(session)),
            Buffer(bytes((random_.choice((0, 0, 1)),))),
            globals={"unknown_004cf95c": bytes((random_.choice((0, 1, 1)),))},
            externals={
                "network_session_host_unknown_0005aeb0": lambda harness, arguments, said=random_.choice((0, 1, 0x100)): (
                    said,
                    (harness.normalize(arguments[0]), arguments[1]),
                ),
            },
        )
