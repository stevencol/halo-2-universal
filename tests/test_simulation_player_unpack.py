# source/networking/simulation_player.c: what a player does is made whole again from a message

import hashlib
import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(590)

    def clears(harness, arguments):
        # what makes it nothing: every byte is set, by what was there
        was = bytes(harness.mu.mem_read(arguments[0], 0x5C))
        harness.mu.mem_write(arguments[0], (hashlib.sha256(was).digest() * 3)[:0x5C])
        return 0, ()

    for n in range(4000):
        packed = bytearray(random_.randbytes(0x34))
        for offset in (0x18, 0x19, 0x1A, 0x1B, 0x30, 0x31):
            packed[offset] = random_.choice((0, 0, 1, 0x80))
        for offset in (0x20, 0x24):
            struct.pack_into("<f", packed, offset, random_.choice((0.0, 0.0, -0.0, 1.0, 0.25, -1.0, float("nan"))))
        struct.pack_into("<i", packed, 0x28, random_.choice((-1, 5, 5, 0x1234)))
        t.check(
            "simulation_player_action_unpack",
            Buffer(bytes(packed)),
            Buffer(random_.randbytes(0x5C)),
            externals={
                "player_action_clear": clears,
                "unknown_000a58d0": lambda harness, arguments, value=random_.choice((-1, 9, 9, 0x4321)): (value & 0xFFFFFFFF, (arguments[0],)),
            },
        )
