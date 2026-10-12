# source/networking/simulation_players.c: what the simulation is at, as a number

import random
import struct

from difftest import Buffer

PLAYERS = 0xC34
WORLD = 0x40
SESSION = 0x7420


def run(t) -> None:
    random_ = random.Random(585)

    for n in range(4000):
        has_world = random_.choice((0, 1, 1, 1, 1, 1, 1, 1, 1, 1))
        has_session = random_.choice((0, 1, 1, 1, 1, 1))
        data = bytearray(random_.randbytes(PLAYERS + WORLD + SESSION))
        world_at, session_at = PLAYERS, PLAYERS + WORLD
        struct.pack_into("<I", data, 4, world_at if has_world else 0)
        struct.pack_into("<I", data, 0xC, session_at if has_session else 0)
        struct.pack_into("<i", data, world_at + 8, random_.choice((1, 2, 2, 3, 3, 4, 5)))
        struct.pack_into("<i", data, world_at + 0x18, random_.choice((0, 1, 2, 3, 4, 5, 6, 7, -1)))
        struct.pack_into("<i", data, session_at + 0x741C, random_.choice((0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, -1)))
        t.check(
            "simulation_players_get_status",
            Buffer(bytes(data), pointers=tuple(([4] if has_world else []) + ([0xC] if has_session else []))),
            globals={
                "unknown_004e9bb8": random_.choice((0, 0, 0, 0x12345678)),
                "unknown_00527330": bytes((random_.choice((0, 1, 1, 1, 1, 1, 0x80)),)),
                "unknown_00527334": random_.choice((3, 3, 8, 8, 8, 0, 4)),
            },
            externals={
                "cached_map_file_status": lambda harness, arguments, value=random_.choice((0, 1, 2, 3, 4)): (value, (arguments[0],)),
            },
        )
