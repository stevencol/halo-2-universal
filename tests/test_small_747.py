# three more functions of the table of players kept in two lists

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
PLAYERS, PLAYER = 350, 0x68


def run(t) -> None:
    random_ = random.Random(747)

    for n in range(300):
        t.check(
            "network_recent_players_initialize",
            globals={
                "unknown_004cf98c": random_.randbytes(PLAYERS * PLAYER),
                "unknown_004cf970": random_.getrandbits(32),
                "unknown_004cf974": random_.getrandbits(32),
                "unknown_004cf978": random_.getrandbits(32),
                "unknown_004cf97c": random_.getrandbits(32),
                "unknown_004cf980": random_.getrandbits(32),
                "unknown_004cf984": random_.getrandbits(32),
                "unknown_004cf988": random_.getrandbits(32),
            },
        )

        # the players in the order of who they are
        count = random_.choice((0, 1, 2, 3, 7, 100, PLAYERS, random_.randrange(PLAYERS + 1)))
        prefix = random_.randbytes(random_.choice((0, 8, 11)))
        names = sorted(prefix + random_.randbytes(12 - len(prefix)) for index in range(count))
        data = bytearray(random_.randbytes(PLAYERS * PLAYER))
        for index, name in enumerate(names):
            data[index * PLAYER : index * PLAYER + 12] = name
        if names and random_.random() < 0.6:
            wanted = random_.choice(names)
        else:
            wanted = prefix + random_.randbytes(12 - len(prefix))
        t.check(
            "network_recent_players_find",
            Buffer(wanted),
            random_.choice((Buffer(random_.randbytes(4)), 0)),
            globals={"unknown_004cf98c": bytes(data), "unknown_004cf978": count},
        )

        # the second list, from one player to the next, with no loops
        data = bytearray(random_.randbytes(PLAYERS * PLAYER))
        chain = random_.sample(range(PLAYERS), random_.choice((1, 2, 5, 20)))
        for position, index in enumerate(chain):
            at = index * PLAYER
            following = chain[position + 1] if position + 1 < len(chain) else 0xFFFF
            struct.pack_into("<H", data, at + 0x5A, following)
            struct.pack_into("<I", data, at + 0x60, random_.choice((0x10, 0x10, 0x10, 0x14, 0, 0x11, 0x13, random_.getrandbits(32))))
            if random_.random() < 0.15:
                struct.pack_into("<II", data, at, 0, 0)
            if random_.random() < 0.15:
                struct.pack_into("<H", data, at + 6, 0xFEFE)
            data[at + 8] = random_.choice((0, 0, 0, 4, 1, 2, 3))
            struct.pack_into("<H", data, at + 0xC, random_.choice((1, 1, 0x8000, 0)))
        start = random_.choice((chain[0], chain[0], chain[-1], NONE))
        t.check(
            "network_recent_players_iterate_second",
            random_.choice((Buffer(random_.randbytes(0x58)), 0)),
            Buffer(struct.pack("<I", start)),
            globals={"unknown_004cf98c": bytes(data)},
        )
