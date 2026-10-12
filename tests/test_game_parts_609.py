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
            ("breakable_surfaces_initialize", "unknown_004ed280", 0x4204, numbers),
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
        t.check("breakable_surfaces_unknown_00183f10", globals=numbers)
        t.check("breakable_surfaces_initialize_for_new_map", globals={"unknown_004ed280": Buffer(random_.randbytes(0x4214))})
        game = bytearray(random_.randbytes(0x20))
        game[0xC] = random_.choice((0, 1, 3, 4, 5, 6, 0x84, 0xFF))
        t.check(
            "breakable_surfaces_unknown_00183ee0",
            globals={"main_game_globals": Buffer(bytes(game))},
            externals={"breakable_surfaces_unknown_00185a30": note, "unknown_00185630": note},
        )
        t.check("breakable_surfaces_unknown_00183f00", externals={"breakable_surfaces_unknown_00185a30": note, "breakable_surfaces_unknown_00185a70": note})


def run_610(t) -> None:
    random_ = random.Random(610)

    for n in range(300):
        game = bytearray(random_.randbytes(0x20))
        game[0xC] = random_.choice((0, 1, 3, 4, 4, 5))
        numbers = b"".join(struct.pack("<I", random_.choice((0xFFFFFFFF, 0xFFFFFFFF, 7, 0x1234))) for _ in range(8))
        t.check(
            "breakable_surfaces_unknown_00185a30",
            globals={
                "main_game_globals": Buffer(bytes(game)),
                "unknown_004ea95c": random_.getrandbits(32),
                "unknown_004eca60": numbers,
            },
            externals={"unknown_000b5920": lambda harness, arguments: (0, (arguments[0],))},
        )
        t.check(
            "breakable_surfaces_unknown_00185a70",
            globals={
                "unknown_004ea960": random_.randbytes(0x100),
                "unknown_004eaa60": random_.randbytes(0x2000),
                "unknown_004eca80": random_.randbytes(0x800),
            },
        )


_run_609 = run


def run(t) -> None:
    _run_609(t)
    run_610(t)
