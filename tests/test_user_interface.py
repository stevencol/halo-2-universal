# source/interface/user_interface.c, and rasterizer_text_end

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(539)

    for n in range(600):
        t.check(
            "rasterizer_text_end",
            globals={"rasterizer_text_unknown_004c1a18": bytes((random_.choice((0, 1, 1, 0x80)),))},
            externals={"unknown_0001e930": lambda harness, arguments: (0, tuple(arguments))},
        )
        t.check(
            "user_interface_unknown_00147da5",
            random_.choice((NONE, 0, 1, 2, 3)),
            externals={
                "user_interface_controller_find_voice_index": lambda harness, arguments, index=random_.choice((NONE, 0, 1, 2, 3)): (index, tuple(arguments)),
                "unknown_00148e6d": lambda harness, arguments, so=random_.choice((0, 1)): (so, tuple(arguments)),
            },
        )
        game = bytearray(random_.randbytes(0x1200))
        struct.pack_into("<I", game, 8, random_.choice((1, 2, 3, 3, 3)))
        t.check(
            "user_interface_unknown_0014a224",
            globals={"main_game_globals": Buffer(bytes(game))},
            externals={
                "game_in_progress": lambda harness, arguments, so=random_.choice((0, 1, 1, 1)): (so, ()),
                "unknown_00147f4f": lambda harness, arguments, at=random_.choice((0, 5, 6, 6, 7, 9, 9, 10, NONE)): (at, ()),
            },
        )
