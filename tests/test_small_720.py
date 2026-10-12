# eight small functions of players, tags and others

import random
import struct

from difftest import Buffer

from test_small_717 import TAG_SIZE, TAGS

NONE = 0xFFFFFFFF
PLAYERS, PLAYER = 4, 0x21C
THINGS, THING = 3, 0x1B0
TAG = 0x200


def some_tags(random_) -> Buffer:
    """Some tags of 0x200 bytes after their table, or nowhere; each has at 0x28 things that are after them all."""
    data = bytearray(random_.randbytes(TAGS * 16 + TAGS * TAG + 0x200))
    pointers = []
    for index in range(TAGS):
        at = TAGS * 16 + index * TAG
        if random_.random() < 0.2:
            struct.pack_into("<I", data, index * 16 + 8, 0)
            continue
        struct.pack_into("<I", data, index * 16 + 8, at)
        pointers.append(index * 16 + 8)
        struct.pack_into("<I", data, at + 0x28, TAGS * 16 + TAGS * TAG)
        pointers.append(at + 0x28)
        struct.pack_into("<f", data, at + 0x164, random_.uniform(-50.0, 50.0))
    return Buffer(bytes(data), pointers=tuple(pointers))


def run(t) -> None:
    random_ = random.Random(720)
    assert TAG_SIZE <= TAG

    for n in range(300):
        players = bytearray(random_.randbytes(0x4C + PLAYERS * PLAYER))
        struct.pack_into("<I", players, 0x24, PLAYER)
        struct.pack_into("<I", players, 0x44, 0x4C)
        for index in range(PLAYERS):
            struct.pack_into("<h", players, 0x4C + index * PLAYER + 0x28, random_.choice((-1, 0, 1, 2)))
        some_players = lambda: Buffer(bytes(players), pointers=(0x44,))
        player_index = random_.getrandbits(16) << 16 | random_.randrange(PLAYERS)

        t.check(
            "player_index_from_absolute_index",
            random_.choice((NONE, 0, 1, 2, 3)),
            globals={"player_data": some_players()},
        )
        t.check(
            "local_player_get_player_index",
            random_.choice((NONE, 0, 1, 2, 3)),
            globals={"players_globals": Buffer(random_.randbytes(0x90))},
        )
        for name in ("game_unknown_001e6a40_set_unknown195", "game_unknown_001e6a40_set_unknown19B"):
            t.check(
                name,
                player_index,
                globals={
                    "player_data": some_players(),
                    "game_time_globals": Buffer(random_.randbytes(0x24)),
                    "unknown_0051e9c0": Buffer(random_.randbytes(THINGS * THING + 8)),
                },
            )

        thing = bytearray(random_.randbytes(8))
        struct.pack_into("<h", thing, 4, random_.randrange(0x100))
        t.check(
            "game_unknown1138_test",
            Buffer(bytes(thing)),
            globals={"main_game_globals": Buffer(random_.choice((bytes(0x1160), random_.randbytes(0x1160))))},
        )

        tags = some_tags(random_)
        present = [index for index in range(TAGS) if index * 16 + 8 in tags.pointers]
        if present:
            t.check(
                "tag_get_unknown164_scaled",
                random_.getrandbits(16) << 16 | random_.choice(present),
                globals={"global_tag_instances": tags, "unknown_0051e9c4": Buffer(struct.pack("<f", random_.uniform(-4.0, 4.0)) + random_.randbytes(4))},
            )
        t.check(
            "tag_get_unknown28_unknown2C",
            random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(TAGS))),
            random_.randrange(5),
            globals={"global_tag_instances": some_tags(random_)},
        )

        # something with a block at 0x4C whose things are after it
        count = random_.choice((0, 1, 3))
        unknown = bytearray(random_.randbytes(0x54 + 3 * 0x20))
        struct.pack_into("<2I", unknown, 0x4C, count, 0x54)
        for index in range(3):
            named = random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(TAGS)))
            struct.pack_into("<I", unknown, 0x54 + index * 0x20 + 4, named)
        t.check(
            "unknown4C_get_tag",
            random_.choice((0, 1, 2, 3, 7)),
            Buffer(bytes(unknown), pointers=(0x50,)),
            globals={"global_tag_instances": some_tags(random_)},
        )
