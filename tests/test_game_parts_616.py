# the engine of the game takes its memory, and three more start functions of parts of the game

import random

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

from test_game_state import game_state_globals

BASE = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(616)

    for n in range(100):
        own = {"unknown_004e8c38": random_.randbytes(0x50C)}
        for name, more, size in (
            ("game_engine_initialize", {"unknown_004e9ae8": random_.getrandbits(32), "unknown_00502258": random_.randbytes(0x9C)}, 0xC9C),
            ("game_unknown_0016efb0_initialize", {"unknown_00510c70": random_.getrandbits(32)}, 0x14),
            (
                "game_unknown_001552e0_initialize",
                {**own, "unknown_004e8c34": random_.getrandbits(32), "unknown_00510c6c": random_.getrandbits(32)},
                0x7C,
            ),
        ):
            allocated = random_.choice((0, 0x40, 0x1288))
            t.check(
                name,
                globals={"game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated), **more},
                memory={BASE + allocated: Memory(random_.randbytes(size + 0x10))},
            )
        t.check(
            "game_unknown_001552e0_dispose_from_old_map",
            globals={**own, "unknown_004e8c34": Buffer(random_.randbytes(8))},
        )
