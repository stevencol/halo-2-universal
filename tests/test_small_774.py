# five small functions of something of 0x2F0 bytes for each local player
# (0x0051e994) and near it

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
PLAYER, PLAYERS = 0x2F0, 4


def run(t) -> None:
    random_ = random.Random(774)

    for n in range(600):
        player = bytearray(random_.randbytes(PLAYER))
        first = random_.choice((0, 0, 1, 5, 15, 16, random_.randrange(16)))
        struct.pack_into("<ii", player, 0x2E8, first, random_.choice((0, 1, 3, 15 - first, 16 - first, random_.randrange(17))))
        t.check("game_unknown_001a1c8e_add_value", Buffer(bytes(player)), random_.getrandbits(32))

        t.check(
            "game_unknown_001a1c8e_initialize",
            globals={"unknown_0051e994": random_.getrandbits(32)},
            externals={
                "game_state_malloc": lambda harness, arguments, said=random_.choice((0, 0x12345678)): (said, tuple(arguments)),
            },
        )

        def real() -> float:
            return random_.choice((0.0, -0.0, -1.0, 0.5, 1.0, 63.9, 64.0, 64.5, 100.0, -100.0, float("nan"), random_.uniform(-10, 80)))

        t.check(
            "game_unknown_001a1c8e_reals_to_bytes",
            Buffer(struct.pack("<ff", real(), real())),
            Buffer(random_.randbytes(4)),
        )

        t.check(
            "game_unknown_001a1c8e_reset_player",
            random_.randrange(PLAYERS),
            globals={"unknown_0051e994": Buffer(random_.randbytes(PLAYERS * PLAYER + 8))},
        )

        data = bytearray(random_.randbytes(12))
        struct.pack_into("<H", data, 8, random_.choice((0, 1, 2, 4, 8, 6, 12, 0x10, 0xFFF0, random_.getrandbits(16))))
        t.check("game_unknown_001a1c8e_flag_index", Buffer(bytes(data)))
