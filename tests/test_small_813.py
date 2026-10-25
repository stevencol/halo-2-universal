# four medium functions of the way to the main menu: three things of Havok
# made nothing, the entry of the scenario of a cluster, the player a player
# without a unit follows, and whether an object is a weapon in a hand

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
PLAYER = 0x21C


def run(t) -> None:
    random_ = random.Random(813)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        t.check("havok_unknown_001ccba0", Buffer(random_.randbytes(0x60)))

        count = random_.choice((0, 1, 3, 4))
        scenario = bytearray(random_.randbytes(0x20 + 4 * 8))
        struct.pack_into("<iI", scenario, 0x08, count, 0x20)
        scenario[0x12] = random_.choice((0, 2))
        for entry in range(4):
            struct.pack_into("<I", scenario, 0x20 + entry * 8 + 4, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(2))))
        bsp = bytearray(random_.randbytes(0xB0 + 3 * 0xB0))
        struct.pack_into("<I", bsp, 0xA0, 0xB0)
        for cluster in range(3):
            bsp[0xB0 + cluster * 0xB0 + 0x6E] = random_.choice((0xFF, 0, 1, 3, 5))
        tags = bytearray(random_.randbytes(0x20 + 2 * 0xB0))
        loaded = random_.random() < 0.85
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x20 + tag * 0xB0 if loaded else 0)
            struct.pack_into("<I", tags, 0x20 + tag * 0xB0 + 4, random_.choice((NONE, 1)))
            tags[0x20 + tag * 0xB0 + 0x10] = random_.choice((0, 0x20))
        t.check(
            "scenario_unknown_0014b360",
            random_.choice((NONE, 0, 1, 2)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(1)),
            Buffer(random_.randbytes(12)),
            globals={
                "global_scenario": Buffer(bytes(scenario), pointers=(0x0C,)),
                "global_structure_bsp": Buffer(bytes(bsp), pointers=(0xA0,)),
                "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18) if loaded else ()),
            },
        )

        # players: a data array with salts; each may follow another (0x1B0)
        players = bytearray(random_.randbytes(0x50 + 4 * PLAYER))
        struct.pack_into("<i", players, 0x24, PLAYER)
        struct.pack_into("<i", players, 0x38, 4)
        struct.pack_into("<I", players, 0x44, 0x50)
        salts = [random_.choice((0x1234, 0x5678)) for _ in range(4)]
        for player in range(4):
            at = 0x50 + player * PLAYER
            struct.pack_into("<H", players, at, salts[player])
            players[at + 2] = random_.choice((0, 2))
            players[at + 3] = random_.getrandbits(8)
            struct.pack_into("<h", players, at + 0x28, random_.choice((-1, -1, 0)))
            struct.pack_into("<I", players, at + 0x2C, random_.choice((NONE, NONE, 3)))
            struct.pack_into("<i", players, at + 0x190, random_.choice((0, 100, 500)))
            struct.pack_into("<I", players, at + 0x1B0, random_.choice((NONE, salts[(player + 1) % 4] << 16 | (player + 1) % 4, 0x1111 << 16 | 2)))
            players[at + 0xC4] = random_.choice((0, 1, 1))
        time = bytearray(random_.randbytes(0x24))
        struct.pack_into("<h", time, 2, 30)
        struct.pack_into("<I", time, 8, random_.choice((100, 299, 300, 600)))
        t.check(
            "player_unknown_00162c50",
            salted(random_.randrange(4)),
            Buffer(random_.randbytes(4)),
            globals={"player_data": Buffer(bytes(players), pointers=(0x44,)), "game_time_globals": Buffer(bytes(time))},
        )

        table = some_objects(random_)
        objects = bytearray(table.data)
        ids = [salted(index) for index in range(COUNT)]
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            objects[at + 0xAA] = random_.choice((0, 1, 2))
            objects[at + 0x212] = random_.choice((0, 1, 0xFF))
            objects[at + 0x213] = random_.choice((0, 1, 0xFF))
            for weapon in range(4):
                struct.pack_into("<I", objects, at + 0x218 + weapon * 4, random_.choice(ids + [NONE]))
        t.check(
            "unit_unknown_00101690",
            random_.choice(ids),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
        )
