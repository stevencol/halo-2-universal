# eight small functions of the game engine: a time and a bit of an index of
# the players, a kind and a count, whether the index of unknown_004b9ed8 has
# its player, bits of the kind of game and the long at 0x7DC of an index

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
PLAYER = 0x21C


def run(t) -> None:
    random_ = random.Random(794)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def engine() -> Buffer:
        data = bytearray(random_.randbytes(0xC9C))
        struct.pack_into("<i", data, 0xC14, random_.randrange(4))
        struct.pack_into("<h", data, 0x6C, random_.choice((0, 1, 1, 2)))
        struct.pack_into("<h", data, 0xE0, random_.choice((-1, 0, 1, 5)))
        struct.pack_into("<i", data, 0xC04, random_.choice((0, 1, 1)))
        for index in range(4):
            struct.pack_into("<I", data, 0xE4 + index * 4, f32(random_.choice((0.0, 0.5, 1.0, 2.0))))
        return Buffer(bytes(data))

    def kinds() -> bytes:
        return struct.pack("<4I", *(random_.choice((0, 0x1234)) for _ in range(4)))

    def game() -> Buffer:
        data = bytearray(random_.randbytes(0x1200))
        data[0x0C] = random_.choice((0, 4, 4, 5))
        struct.pack_into("<i", data, 0x180, random_.choice((1, 3, 7, 9, 2)))
        struct.pack_into("<i", data, 0x190, random_.choice((0, 1)))
        struct.pack_into("<I", data, 0x22C, random_.choice((0, 1, 0x10, 0x11, 0x20, random_.getrandbits(32))))
        return Buffer(bytes(data))

    def players() -> tuple[Buffer, Buffer]:
        globals_ = struct.pack("<3I4I", 0, 0, 0, *(random_.choice((NONE, salted(random_.randrange(4)))) for _ in range(4)))
        data = bytearray(random_.randbytes(0x50 + 4 * PLAYER))
        struct.pack_into("<I", data, 0x44, 0x50)
        for player in range(4):
            struct.pack_into("<I", data, 0x50 + player * PLAYER + 0x2C, random_.choice((NONE, 5)))
        return Buffer(globals_), Buffer(bytes(data), pointers=(0x44,))

    for n in range(400):
        globals_, data = players()
        t.check(
            "game_engine_unknown_001584c0",
            random_.choice((NONE, 0, 1, 2, 3)),
            globals={"unknown_004e9ae8": engine(), "players_globals": globals_, "player_data": data, "main_game_globals": game()},
        )
        t.check(
            "game_engine_unknown_00158850",
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            globals={"unknown_004e9ae8": engine(), "main_game_globals": game()},
        )
        globals_, data = players()
        t.check(
            "game_engine_unknown_00159610",
            globals={
                "unknown_004e9ae8": engine(),
                "unknown_0055e4d0": kinds(),
                "unknown_004b9ed8": random_.choice((NONE, 0, 1, 3)),
                "players_globals": globals_,
                "player_data": data,
            },
        )
        t.check("game_engine_unknown_00159d40", globals={"main_game_globals": game()})
        t.check("game_engine_unknown_00159d80", globals={"main_game_globals": game()})
        t.check(
            "game_engine_unknown_00159dd0",
            random_.choice((0, 1, 5, 15, 16, 31, 0x100)),
            globals={
                "unknown_004e9ae8": engine(),
                "unknown_0055e4d0": kinds(),
                "main_game_globals": game(),
                "unknown_00510c9c": Buffer(struct.pack("<H", random_.getrandbits(16))),
            },
        )
        table = some_objects(random_)
        t.check(
            "game_engine_unknown_0015b220",
            salted(random_.randrange(COUNT)),
            random_.choice((0, 1, 7)),
            globals={"object_header_data": table, "unknown_004e9ae8": engine(), "main_game_globals": game()},
        )
        t.check("game_engine_unknown_0015b2f0", globals={"unknown_004e9ae8": engine(), "unknown_0055e4d0": kinds(), "main_game_globals": game()})
