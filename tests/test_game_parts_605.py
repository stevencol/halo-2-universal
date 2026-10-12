# two parts of the game whose four functions are alike: objects/object_unknown_0010a980.c, game/game_unknown_001ec420.c

import random

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000
PARTS = (("object_unknown_0010a980", 0x304), ("game_unknown_001ec420", 0x24))


def run(t) -> None:
    random_ = random.Random(605)

    for n in range(100):
        for name, size in PARTS:
            allocated = random_.choice((0, 0x40, 0x1288))
            t.check(
                f"{name}_initialize",
                globals={
                    "game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated),
                    f"{name}_globals": random_.getrandbits(32),
                },
                memory={BASE + allocated: Memory(random_.randbytes(size + 0x10))},
            )
            t.check(f"{name}_dispose", globals={f"{name}_globals": random_.getrandbits(32)})
            t.check(f"{name}_initialize_for_new_map", globals={f"{name}_globals": Buffer(random_.randbytes(size + 0x10))})
            t.check(f"{name}_dispose_from_old_map", globals={f"{name}_globals": Buffer(random_.randbytes(size + 0x10))})
