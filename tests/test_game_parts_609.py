# two more parts of the game: game/game_unknown_001eb550.c and five functions of sound_manager.c

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(609)

    def note(harness, arguments):
        return 0, ()

    for n in range(100):
        zero = {"global_zero_vector3d": Buffer(random_.randbytes(12))}
        numbers = {"unknown_004ea95c": random_.getrandbits(32), "unknown_004eca60": random_.randbytes(32)}
        for name, pointer, size, more in (
            ("game_unknown_001eb550_initialize", "unknown_0051e9c4", 0x1C, zero),
            ("sound_unknown_00183e40", "unknown_004ed280", 0x4204, numbers),
        ):
            allocated = random_.choice((0, 0x40, 0x1288))
            t.check(
                name,
                globals={
                    "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
                    pointer: random_.getrandbits(32),
                    **more,
                },
                memory={BASE + allocated: Memory(random_.randbytes(size + 0x10))},
            )
        t.check("game_unknown_001eb550_dispose", globals={"unknown_0051e9c4": random_.getrandbits(32)})
        t.check(
            "game_unknown_001eb550_initialize_for_new_map",
            globals={"unknown_0051e9c4": Buffer(random_.randbytes(0x2C)), **zero},
        )
        t.check("sound_unknown_00183f10", globals=numbers)
        t.check("sound_unknown_00183ec0", globals={"unknown_004ed280": Buffer(random_.randbytes(0x4214))})
        game = bytearray(random_.randbytes(0x20))
        game[0xC] = random_.choice((0, 1, 3, 4, 5, 6, 0x84, 0xFF))
        t.check(
            "sound_unknown_00183ee0",
            globals={"main_game_globals": Buffer(bytes(game))},
            externals={"unknown_00185a30": note, "unknown_00185630": note},
        )
        t.check("sound_unknown_00183f00", externals={"unknown_00185a30": note, "unknown_00185a70": note})
