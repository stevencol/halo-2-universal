# four medium functions of the way to the main menu: two things of Havok
# made nothing, the players' globals by the players in use, a number of an
# element of a tag and a new entry of the things of game_unknown_00131ff0.c

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
PLAYER = 0x21C


def run(t) -> None:
    random_ = random.Random(803)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    for n in range(400):
        t.check("havok_unknown_001ccc50", Buffer(random_.randbytes(0x60)))

        count = random_.choice((0, 1, 3, 4))
        players = bytearray(random_.randbytes(0x50 + 4 * PLAYER + 4))
        struct.pack_into("<iiI", players, 0x24, PLAYER, 0, 0)
        struct.pack_into("<i", players, 0x38, count)
        struct.pack_into("<II", players, 0x44, 0x50, 0x50 + 4 * PLAYER)
        struct.pack_into("<I", players, 0x50 + 4 * PLAYER, random_.getrandbits(4))
        for player in range(4):
            at = 0x50 + player * PLAYER
            players[at + 2] = random_.choice((0, 8, 0xF7, 0xFF))
            struct.pack_into("<I", players, at + 0x2C, random_.choice((NONE, NONE, 3)))
        t.check(
            "players_unknown_00152340",
            globals={
                "players_globals": Buffer(random_.randbytes(0x40)),
                "player_data": Buffer(bytes(players), pointers=(0x44, 0x48)),
            },
        )

        # a tag with elements of 0x3C bytes at 0x3C, each with entries of 0x20 at 0x34
        tags = bytearray(random_.randbytes(0x40 + 0x50 + 3 * 0x3C + 8 * 0x20))
        loaded = n % 9 != 0
        struct.pack_into("<I", tags, 8, 0x40 if loaded else 0)
        struct.pack_into("<iI", tags, 0x40 + 0x3C, random_.choice((0, 1, 3, -1)), 0x90)
        pointers = [8, 0x40 + 0x40] if loaded else [0x40 + 0x40]
        for element in range(3):
            at = 0x90 + element * 0x3C
            struct.pack_into("<hh", tags, at + 0x20, random_.choice((0, 2, -3)), random_.choice((0, -1, 1, 3)))
            struct.pack_into("<iI", tags, at + 0x34, random_.choice((0, 1)), 0x90 + 3 * 0x3C)
            pointers.append(at + 0x38)
        for entry in range(8):
            struct.pack_into("<h", tags, 0x90 + 3 * 0x3C + entry * 0x20, random_.choice((-1, 0, 4, 7)))
        t.check(
            "tag_unknown_00137590",
            random_.choice((NONE, random_.getrandbits(16) << 16)),
            random_.getrandbits(16) << 16 | random_.choice((0, 1, 2, 5, 7)),
            random_.getrandbits(16) << 16 | random_.randrange(8),
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=tuple(pointers))},
        )

        thing = bytearray(random_.randbytes(0x2AE0))
        struct.pack_into("<H", thing, 0xA5E, random_.choice((0, 1, 0x1FF, 0x200, 0x201, 0xFFFF, random_.randrange(0x200))))
        t.check(
            "game_unknown_00131ff0_new_entry",
            Buffer(bytes(thing)),
            random_.getrandbits(16) << 16 | random_.randrange(128),
        )
