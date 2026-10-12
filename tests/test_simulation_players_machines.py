# source/networking/simulation_players.c: the players of the consoles that are gone leave

import random
import struct

from difftest import Buffer

PLAYERS = 0xC34
WORLD = 0x7C + 16 * 0x88


def a_machine(random_) -> bytes:
    return bytes((random_.choice((1, 2)),)) * 5 + bytes((random_.choice((1, 2, 3)),))


def run(t) -> None:
    random_ = random.Random(586)

    for n in range(1500):
        data = bytearray(random_.randbytes(PLAYERS + WORLD))
        struct.pack_into("<I", data, 4, PLAYERS)
        struct.pack_into("<I", data, 0x1C, random_.choice((0, 1, 0x8001, 0xFFFF, random_.getrandbits(16))))
        for index in range(16):
            data[0x24 + index * 6 : 0x2A + index * 6] = a_machine(random_)
        struct.pack_into("<I", data, 0x88, random_.choice((0, 1, 0x8001, 0xFFFF, random_.getrandbits(16), random_.getrandbits(32))))
        for index in range(16):
            at = 0x8C + index * 0xB4
            data[at + 0xC] = random_.choice((0, 0, 0, 1))
            data[at + 0x14 : at + 0x1A] = a_machine(random_)
        struct.pack_into("<i", data, PLAYERS + 8, random_.choice((1, 2, 3, 4, 5)))
        for index in range(16):
            at = PLAYERS + 0x7C + index * 0x88
            struct.pack_into("<i", data, at, random_.choice((-1, 0, index)))
            data[at + 0x24] = random_.choice((0, 1))
            data[at + 0x25] = random_.choice((0, 1, 0x80))

        def leaves(harness, arguments):
            # the player has left: the game looks at no one twice, but what it has is not to stay as it was
            update = bytes(harness.mu.mem_read(arguments[1], 0x14))
            return 0, (harness.normalize(arguments[0]), update)

        t.check(
            "simulation_players_machines_changed",
            Buffer(bytes(data), pointers=(4,)),
            externals={"simulation_players_collection_apply_update": leaves},
        )
