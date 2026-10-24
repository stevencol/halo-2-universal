# ten small functions of the game engine: a player without a unit, an entry
# by a short, bits of the kinds of game, a pair of a tag of the game globals,
# the bits of the shorts at 0x0C and 0x0E, a definition of an object and
# sixteen longs

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
PLAYER = 0x21C
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(795)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def engine() -> Buffer:
        data = bytearray(random_.randbytes(0xC9C))
        struct.pack_into("<i", data, 0xC14, random_.randrange(4))
        struct.pack_into("<HH", data, 0x0C, random_.choice((0, 1, 0x80, 0xFF, random_.getrandbits(16))), random_.getrandbits(16))
        struct.pack_into("<i", data, 0xC18, random_.choice((0, 1, 3, 8)))
        for entry in range(8):
            struct.pack_into("<h", data, 0xC20 + entry * 0x10, random_.choice((-1, 0, 2, 5)))
        return Buffer(bytes(data))

    def kinds() -> bytes:
        return struct.pack("<4I", *(random_.choice((0, 0x1234, 0x1234)) for _ in range(4)))

    def game() -> Buffer:
        data = bytearray(random_.randbytes(0x1200))
        struct.pack_into("<i", data, 0x180, random_.choice((7, 7, 9, 9, 2)))
        struct.pack_into("<I", data, 0x184, random_.choice((0, 1, 2, 3)))
        struct.pack_into("<i", data, 0x1B8, random_.choice((-1, 0, 1)))
        struct.pack_into("<i", data, 0x1C4, random_.choice((0, 1, 1)))
        struct.pack_into("<I", data, 0x22C, random_.choice((0, 1, 2, 3)))
        return Buffer(bytes(data))

    def players() -> Buffer:
        data = bytearray(random_.randbytes(0x50 + 4 * PLAYER))
        struct.pack_into("<I", data, 0x44, 0x50)
        for player in range(4):
            struct.pack_into("<I", data, 0x50 + player * PLAYER + 0x2C, random_.choice((NONE, 5)))
            struct.pack_into("<h", data, 0x50 + player * PLAYER + 0x1AC, random_.choice((0, 0, 1)))
        return Buffer(bytes(data), pointers=(0x44,))

    def speakers() -> Buffer:
        return Buffer(struct.pack("<H", random_.getrandbits(16)))

    for n in range(400):
        t.check("game_engine_unknown_0015db30", salted(random_.randrange(4)), globals={"main_game_globals": game(), "player_data": players()})
        t.check(
            "game_engine_unknown_0015e410",
            random_.getrandbits(16) << 16 | random_.choice((0xFFFF, 0, 2, 5, 7)),
            globals={"unknown_004e9ae8": engine(), "unknown_0055e4d0": kinds()},
        )
        t.check(
            "game_engine_unknown_0015eb20",
            random_.choice((NONE, 0, 1, 7, 15, 31, 33)),
            globals={"unknown_004e9ae8": engine(), "unknown_0055e4d0": kinds(), "main_game_globals": game(), "unknown_00510c9c": speakers()},
        )

        # the game globals name a tag whose block at 0x08 has an element with pairs at 0x68
        tag = random_.randrange(4)
        tags = bytearray(random_.randbytes(0x40 + 0x10 + 0x70 + 4 * 8))
        struct.pack_into("<I", tags, tag * 0x10 + 8, 0x40)
        struct.pack_into("<I", tags, 0x40 + 0x0C, 0x50)
        struct.pack_into("<iI", tags, 0x50 + 0x68, random_.choice((0, 1, 4, -1)), 0xC0)
        values = [random_.choice((3, 5, 9)) for _ in range(4)]
        for index in range(4):
            struct.pack_into("<I", tags, 0xC0 + index * 8 + 4, values[index])
        game_globals = bytearray(random_.randbytes(0x170))
        struct.pack_into("<I", game_globals, 0x16C, salted(tag))
        t.check(
            "game_engine_unknown_0015eb80",
            random_.choice((3, 5, 9, 11)),
            globals={
                "global_game_globals": Buffer(bytes(game_globals)),
                "global_tag_instances": Buffer(bytes(tags), pointers=(tag * 0x10 + 8, 0x40 + 0x0C, 0x50 + 0x6C)),
            },
        )

        t.check(
            "game_engine_unknown_0015fef0",
            random_.choice((0, 1, 7, 15, 31, 0x100)),
            globals={"unknown_004e9ae8": engine(), "unknown_0055e4d0": kinds(), "main_game_globals": game(), "unknown_00510c9c": speakers()},
        )
        for name in ("game_engine_unknown_00161e10", "game_engine_unknown_00161e60"):
            t.check(
                name,
                random_.choice((NONE, 0, 1, 7, 8, 3)),
                globals={"unknown_004e9ae8": engine(), "unknown_0055e4d0": kinds(), "main_game_globals": game()},
            )
        t.check("game_engine_unknown_00161eb0", random_.choice((NONE, 0, 1, 6, 7, 3)), globals={"unknown_004e9ae8": engine()})

        table = some_objects(random_)
        objects = bytearray(table.data)
        tags = bytearray(random_.randbytes(0x40 + 4 * 0x2A0))
        for tag in range(4):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x40 + tag * 0x2A0)
            struct.pack_into("<h", tags, 0x40 + tag * 0x2A0 + 0x290, random_.choice((0, 1, 1)))
        for index in range(COUNT):
            struct.pack_into("<I", objects, FIRST + index * OBJECT, salted(random_.randrange(4)))
        t.check(
            "game_engine_unknown_00162b10",
            random_.choice((NONE, salted(random_.randrange(COUNT)))),
            globals={
                "main_game_globals": game(),
                "object_header_data": Buffer(bytes(objects), pointers=table.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=tuple(tag * 0x10 + 8 for tag in range(4))),
            },
        )

        t.check("game_engine_unknown_001630b0", random_.getrandbits(32), Buffer(random_.randbytes(0x40)))
