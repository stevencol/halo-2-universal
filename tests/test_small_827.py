# four small functions of the way to the main menu: some tables made
# empty, whether a unit has waited enough, and how much of a count of the
# voice is left

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
PLAYER = 0x21C


def run(t) -> None:
    random_ = random.Random(827)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def time() -> Buffer:
        data = bytearray(random_.randbytes(0x24))
        ticks = random_.choice((30, 60))
        struct.pack_into("<hf", data, 2, ticks, 1 / ticks)
        return Buffer(bytes(data))

    def voice() -> dict:
        data = bytearray(random_.randbytes(0xC18))
        struct.pack_into("<i", data, 0xC14, random_.randrange(4))
        return {
            "unknown_004e9ae8": Buffer(bytes(data)),
            "unknown_0055e4d0": struct.pack("<16I", *(random_.choice((0, 1, 1)) for _ in range(16))),
            "unknown_004e9af0": bytes(random_.choice((0, 3, 7, 8, 9, 15, 0xFF)) for _ in range(4)),
            "game_time_globals": time(),
        }

    for n in range(400):
        t.check(
            "unknown_0003fd70",
            globals={
                "unknown_004c6700": random_.randbytes(8 * 0x408),
                "unknown_004c8740": random_.randbytes(64),
                "unknown_0050943c": random_.getrandbits(32),
                "unknown_00509440": random_.getrandbits(32),
            },
        )

        # units are 0x250 bytes here: the bytes from 0x250 on are in the next one (or after the last)
        table = some_objects(random_)
        objects = bytearray(table.data) + bytearray(random_.randbytes(0x400))
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            objects[at + 0xC1] = random_.choice((0, 1, 2))
            objects[at + 0x348] = random_.choice((0, 1, 3))
            objects[at + 0x399] = random_.choice((0, 5, 6, 11, 0x80))
            objects[at + 0x3DC] = random_.choice((0, 1, 2, 3))
        t.check(
            "unit_unknown_000e4050",
            salted(random_.randrange(COUNT)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers), "game_time_globals": time()},
        )

        t.check("game_voice_unknown_001591e0", random_.randrange(4), globals=voice())

        players = bytearray(random_.randbytes(0x50 + 4 * PLAYER))
        struct.pack_into("<I", players, 0x44, 0x50)
        for player in range(4):
            struct.pack_into("<h", players, 0x50 + player * PLAYER + 0x28, random_.choice((-1, 0, 1, 2, 3)))
        t.check(
            "game_voice_unknown_00161b60",
            random_.choice((NONE, salted(random_.randrange(4)))),
            globals={**voice(), "player_data": Buffer(bytes(players), pointers=(0x44,))},
        )
