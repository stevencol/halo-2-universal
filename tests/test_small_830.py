# five small functions of the way to the main menu: whether a vector is one
# long, some bits of flags, how much of a count of the voice is left, how
# much of something there is at a value, and something that starts

import random
import struct

from difftest import Buffer, f32

NONE = 0xFFFFFFFF
PLAYER = 0x21C


def run(t) -> None:
    random_ = random.Random(830)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        vector = random_.choice(([1.0, 0.0, 0.0], [0.6, 0.8, 0.0], [0.6, 0.8, 0.02], [0.0, 0.0, 0.0]))
        if random_.random() < 0.5:
            vector = [random_.uniform(-1, 1) for _ in range(3)]
            length = sum(value * value for value in vector) ** 0.5 or 1
            vector = [value / length for value in vector]
        vector = [value * random_.choice((1, 1, 1.0004, 0.9996, 2)) for value in vector]
        data = struct.pack("<3f", *vector) if n % 11 else struct.pack("<3I", 0x7F800000, 0, 0)
        t.check("real_math_unknown_000a0190", Buffer(data))

        t.check("unknown_0016b500", random_.getrandbits(32))

        data = bytearray(random_.randbytes(0xC18))
        struct.pack_into("<i", data, 0xC14, random_.randrange(4))
        players = bytearray(random_.randbytes(0x50 + 4 * PLAYER))
        struct.pack_into("<I", players, 0x44, 0x50)
        for player in range(4):
            struct.pack_into("<h", players, 0x50 + player * PLAYER + 0x28, random_.randrange(4))
        time = bytearray(random_.randbytes(0x24))
        ticks = random_.choice((30, 60))
        struct.pack_into("<hf", time, 2, ticks, 1 / ticks)
        t.check(
            "game_voice_unknown_00159170",
            salted(random_.randrange(4)),
            globals={
                "unknown_004e9ae8": Buffer(bytes(data)),
                "unknown_0055e4d0": struct.pack("<16I", *(random_.choice((0, 1, 1)) for _ in range(16))),
                "unknown_004e9af0": bytes(random_.choice((0, 3, 7, 8, 9, 15)) for _ in range(4)),
                "player_data": Buffer(bytes(players), pointers=(0x44,)),
                "game_time_globals": Buffer(bytes(time)),
            },
        )

        # a range from 1 to 5, rising for 1 and falling for 1.5
        data = bytearray(random_.randbytes(0x18)) + struct.pack("<6f", 1.0, 2.0, 0.5, 6.0, 1.5, 0.5)
        globals_ = bytearray(random_.randbytes(8))
        globals_[5] = random_.choice((0, 0, 1))
        t.check(
            "unknown_00249bb0",
            Buffer(bytes(data)),
            f32(random_.choice((0.5, 1.0, 1.5, 2.0, 3.0, 3.5, 4.5, 5.0, 6.0, 7.0, random_.uniform(0, 7)))),
            globals={"unknown_00510c50": random_.choice((0, Buffer(bytes(globals_)), Buffer(bytes(globals_))))},
        )

        t.check(
            "unit_unknown_001f03e0",
            Buffer(random_.randbytes(0x74)),
            globals={
                "global_zero_vector3d": Buffer(random_.randbytes(12)),
                "global_up3d": Buffer(random_.randbytes(12)),
                "global_identity4x3": Buffer(random_.randbytes(0x34)),
                "unknown_0047d8e0": random_.randbytes(2),
            },
        )
