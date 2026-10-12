# four more parts of the game: sound_manager.c and three files of game/

import random

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(607)

    def note(harness, arguments):
        return 0, tuple(arguments)

    for n in range(100):
        for name, pointer, size in (
            ("sound_unknown_00185ab0", "unknown_004ed284", 0x264),
            ("game_unknown_001df460_initialize", "unknown_004f55ec", 0xE4),
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
        t.check(
            "sound_unknown_00185b00",
            globals={"unknown_004ed284": Buffer(random_.randbytes(0x264))},
            externals={"unknown_001872d0": note},
        )
        t.check("game_unknown_001df460_initialize_for_new_map", globals={"unknown_004f55ec": Buffer(random_.randbytes(0xF4))})
        t.check(
            "game_unknown_0011be20_initialize_for_new_map",
            globals={"unknown_004f93b8": random_.randbytes(0xD0C)},
            externals={"unknown_00211cc0": note},
        )
        t.check("game_unknown_0011be20_dispose_from_old_map", globals={"unknown_004f93b8": random_.randbytes(0xD0C)})

        three = {name: random_.getrandbits(32) for name in ("unknown_0051e9ec", "unknown_0051e9f0", "unknown_0051e9f4")}
        allocated = random_.choice((0, 0x40, 0x1288))
        t.check(
            "game_unknown_0020aee0_initialize",
            globals={**three, "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated)},
            externals={
                "VirtualAlloc": lambda harness, arguments, memory=random_.choice((0, 0x12340000, 0x12340000)): (memory, tuple(arguments)),
                "GetLastError": note,
            },
        )
        t.check(
            "game_unknown_0020aee0_dispose",
            globals=three,
            externals={
                "VirtualFree": lambda harness, arguments, done=random_.choice((0, 1, 1)): (done, tuple(arguments)),
                "GetLastError": note,
            },
        )
        t.check(
            "game_unknown_0020aee0_initialize_for_new_map",
            globals={"unknown_0051e9ec": Buffer(random_.randbytes(0x1890))},
        )
