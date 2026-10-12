# the time of the game starts and stops, and four more short functions of parts of the game

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(608)

    for n in range(200):
        for name, pointer, size in (
            ("game_time_initialize", "game_time_globals", 0x24),
            ("game_unknown_001e6a40_initialize", "unknown_0051e9c0", 0x6C8),
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
        struct.pack_into("<h", game, 0xE, random_.choice((30, 30, 60, 15, 1, 0, -30, 0x7FFF)))
        t.check(
            "game_time_initialize_for_new_map",
            globals={"game_time_globals": Buffer(random_.randbytes(0x34)), "main_game_globals": Buffer(bytes(game))},
        )
        time = bytearray(random_.randbytes(0x24))
        time[0] = random_.choice((0, 1, 0x80))
        t.check("game_time_dispose_from_old_map", globals={"game_time_globals": random_.choice((0, 1, 1)) and Buffer(bytes(time))})
        t.check("saved_game_files_dispose_from_old_map", globals={"saved_game_files_string_list_index": random_.getrandbits(32)})
        t.check("sound_unknown_00225700", globals={"unknown_00502124": random_.randbytes(16)})
        t.check("game_unknown_001e6a40_initialize_for_new_map", globals={"unknown_0051e9c0": Buffer(random_.randbytes(0x6D8))})
        t.check(
            "game_unknown_001e6a40_dispose_from_old_map",
            externals={"unknown_001e75d0": lambda harness, arguments: (0, (arguments[0],))},
        )
