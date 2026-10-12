# the minds of the characters, each tick

import random
import struct

from difftest import Buffer

OTHERS = (
    "unknown_001c81c0",
    "unknown_00201c80",
    "unknown_00268510",
    "unknown_001e2f50",
    "unknown_0020fb30",
    "unknown_00293830",
    "unknown_001e3240",
)


def run(t) -> None:
    random_ = random.Random(655)

    for n in range(400):
        ai = bytearray(random_.randbytes(0x374 + 0x10))
        for offset in (0, 1, 2):
            ai[offset] = random_.choice((0, 1, 1, 0x80))
        struct.pack_into("<h", ai, 0x22, random_.choice((0, 1, 2, -1, -0x8000, 0x7FFF, random_.randint(-5, 300))))
        game = bytearray(random_.randbytes(0x1200))
        game[0xC] = random_.choice((0, 1, 2, 4, 4, 5))
        t.check(
            "ai_update",
            globals={"ai_globals": Buffer(bytes(ai)), "main_game_globals": Buffer(bytes(game))},
            externals={name: (lambda harness, arguments: 0) for name in OTHERS},
        )
