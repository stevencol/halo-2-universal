# source/sound/sound_speakers.c

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def real_bits(value: float) -> int:
    return struct.unpack("<I", struct.pack("<f", value))[0]


def run(t) -> None:
    random_ = random.Random(545)

    for n in range(1500):
        t.check(
            "sound_speakers_unknown_00192950",
            random_.choice((0, 1, 7, 15, 16, 31, random_.randrange(16))),
            globals={"unknown_00510c9c": Buffer(struct.pack("<H", random_.getrandbits(16)) + random_.randbytes(6))},
        )

        def loud(harness, arguments, seed=n):
            value = random.Random(seed * 11 + arguments[0]).choice((0.0, 1.0, 0.25, 0.5, -1.0, 2.0, 1e-30, float("inf")))
            return real_bits(value), (arguments[0], bytes(harness.mu.mem_read(arguments[1], 12)), arguments[2])

        direction = Buffer(random_.randbytes(12))
        unknown = random_.choice((0, 1, 1, 0xFF, random_.getrandbits(32)))
        t.check("sound_speakers_get_gains4", direction, Buffer(random_.randbytes(16 + 8)), unknown, externals={"unknown_00192b90": loud})
        t.check("sound_speakers_get_gains5", Buffer(random_.randbytes(20 + 8)), direction, unknown, externals={"unknown_00192b90": loud})
