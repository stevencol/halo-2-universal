# four short functions of two parts of the game: the rasterizer and one more

import random

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(617)

    def note(harness, arguments):
        return 0, ()

    for n in range(200):
        t.check(
            "game_unknown_0002b4a0_dispose",
            globals={"cached_object_render_state_data": random_.getrandbits(32)},
            externals={"game_unknown_0002b4a0_unknown_0003b950": note},
        )
        table = bytearray(random_.randbytes(0x4C))
        table[0x29] = random_.choice((0, 1, 0x80))
        t.check(
            "game_unknown_0002b4a0_dispose_from_old_map",
            globals={"cached_object_render_state_data": random_.choice((0, 1, 1, 1)) and Buffer(bytes(table))},
        )
        t.check(
            "rasterizer_dispose_from_old_map",
            globals={"unknown_00485a80": random_.getrandbits(32)},
            externals={"rasterizer_text_cache_flush": note},
        )
        t.check(
            "rasterizer_unknown_000226a0",
            externals={"unknown_00015b70": note, "unknown_0001e1c0": note, "unknown_00036ab0": note},
        )
