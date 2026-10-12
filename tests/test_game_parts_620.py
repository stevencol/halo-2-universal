# two more parts of the game: game/game_unknown_0013bf00.c and game/game_unknown_00153870.c

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(620)

    for n in range(200):
        for name, pointer, size in (
            ("game_unknown_0013bf00_initialize", "unknown_00510c50", 0x24),
            ("game_unknown_00153870_initialize", "unknown_00510c5c", 0x2B0),
        ):
            allocated = random_.choice((0, 0x40, 0x1288))
            t.check(
                name,
                globals={
                    "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
                    pointer: random_.getrandbits(32),
                },
                memory={BASE + allocated: Memory(random_.randbytes(size + 0x10))},
            )
        game = bytearray(random_.randbytes(0x20))
        struct.pack_into("<i", game, 8, random_.choice((0, 1, 1, 2, 3)))
        for name in ("game_unknown_0013bf00_initialize_for_new_map", "game_unknown_0013bf00_dispose_from_old_map"):
            t.check(
                name,
                globals={"unknown_00510c50": Buffer(random_.randbytes(0x34)), "main_game_globals": Buffer(bytes(game))},
            )
