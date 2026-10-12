# three more small parts of the game's table

import random

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(645)

    def note(harness, arguments):
        return 0, ()

    for n in range(100):
        for name, pointer, size in (
            ("game_unknown_00225f80_initialize", "unknown_0051ebf8", 4),
            ("game_unknown_00134d90_initialize", "unknown_004e6740", 0x204),
            ("game_unknown_00210db0_initialize", "unknown_004f93a4", 0x40),
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
            "game_unknown_00225f80_initialize_for_new_map",
            globals={"unknown_0051ebf8": Buffer(random_.randbytes(4 + 0x10)), "unknown_004701ec": random_.getrandbits(32)},
        )
        t.check("game_unknown_00134d90_dispose", globals={"unknown_004e6740": random_.getrandbits(32)})
        t.check(
            "game_unknown_00134d90_initialize_for_new_map",
            globals={"unknown_004e6740": 0 if n % 5 == 0 else Buffer(random_.randbytes(0x204 + 0x10))},
        )
        t.check(
            "game_unknown_00210db0_initialize_for_new_structure_bsp",
            globals={"unknown_004f93a4": Buffer(random_.randbytes(0x40 + 0x10))},
            externals={"game_unknown_00210db0_unknown_00210f00": note, "game_unknown_00210db0_unknown_00210e20": note},
        )
        t.check("game_unknown_00210db0_dispose_from_old_structure_bsp", externals={"game_unknown_00210db0_unknown_00210e80": note})
