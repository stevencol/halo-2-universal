# source/networking/simulation_world.c

import random
import struct

from difftest import Buffer

WORLD = 0x80
VIEW = 0x40
OTHER = 0x28


def a_world(random_, channel: int) -> Buffer:
    """What the game in the network has going, with some of its fifteen views after it, and after each what it has at 0xC."""
    data = bytearray(random_.randbytes(WORLD))
    struct.pack_into("<I", data, 8, random_.choice((0, 1, 1, 1, 1, 0x1234)))
    pointers = []
    chance = random_.choice((0.1, 0.5, 0.9))
    for index in range(15):
        if random_.random() >= chance:
            struct.pack_into("<I", data, 0x40 + index * 4, 0)
            continue
        at = len(data)
        view = bytearray(random_.randbytes(VIEW))
        struct.pack_into("<H", view, 2, random_.choice((0, 1, 1, 1, 2, 2, 3, 3, 4, 5, 0x101, 0x21, random_.getrandbits(8))))
        struct.pack_into("<i", view, 0x3C, random_.choice((channel, channel, random_.randrange(4), -1)))
        struct.pack_into("<I", view, 0xC, at + VIEW)
        other = bytearray(random_.randbytes(OTHER))
        other[0x24] = random_.choice((0, 1, 1, 0x80))
        data += view + other
        struct.pack_into("<I", data, 0x40 + index * 4, at)
        pointers += [0x40 + index * 4, at + 0xC]
    return Buffer(bytes(data), pointers=tuple(pointers))


def run(t) -> None:
    random_ = random.Random(444)

    for n in range(2500):
        channel = random_.randrange(4)
        mask = random_.choice((0xFFFFFFFF, 0xFFFFFFFF, 1 << random_.randrange(6), 0x14, 0, random_.getrandbits(32)))
        index = random_.choice((0, 0, 0, 3, 14, 15, 16, -1, -5, random_.randrange(15)))
        t.check(
            "simulation_world_next_view",
            Buffer(struct.pack("<Ii", mask, index)),
            a_world(random_, channel),
            Buffer(random_.randbytes(4)),
        )
        t.check("simulation_world_get_channel_view", a_world(random_, channel), random_.choice((channel, channel, 0xFFFFFFFF, 7)))

    # ---------- what the thing has of each player, its first view of two kinds, and memory that is given back

    for n in range(1500):
        players = bytearray(random_.randbytes(0x7C + 16 * 0x88))
        for player in range(16):
            if random_.random() < 0.4:
                struct.pack_into("<i", players, 0x7C + player * 0x88, -1)
            players[0x7C + player * 0x88 + 0x25] = random_.choice((0, 0, 1, 1, 0x80))
        t.check(
            "simulation_world_player_unknown25",
            random_.choice((random_.randrange(16), random_.randrange(16) | 0x30000, random_.randrange(16) | 0xFFFF0000)),
            Buffer(bytes(players)),
        )
        t.check("simulation_world_players_clear_unknown25", Buffer(bytes(players)))
        t.check("simulation_world_unknown_0006acb0", a_world(random_, random_.randrange(4)))
        t.check(
            "simulation_world_unknown_0006aae0",
            Buffer(random_.randbytes(0x1210)),
            externals={"texture_cache_return_memory": lambda harness, arguments: 0},
        )
