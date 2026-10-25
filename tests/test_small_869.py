# two functions of the way to the main menu (guided by halo2-decompiled): a
# tick of two recent amounts of damage, and what is done with the fog that
# is coming

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(869)

    for n in range(400):
        definition = random_.choice((None, Buffer(random_.randbytes(0x30) + struct.pack(
            "<4f",
            random_.choice((0.0, 0.1, 0.5, 2.0, 0.05)),
            random_.choice((0.0, 0.00005, 0.5, 2.0)),
            random_.choice((0.0, 0.1, 1.0, 2.0)),
            random_.choice((0.0, 0.00005, 0.5, 2.0)),
        ))))
        times = bytearray(random_.randbytes(0x24))
        struct.pack_into("<h", times, 2, random_.choice((30, 60, 30)))
        t.check(
            "unit_unknown_000d8b50",
            Buffer(bytes((random_.choice((0xFF, 0, 1, 5, 0x3C, 0x7E, 0x7F, 0x40)),))),
            definition if definition else 0,
            Buffer(struct.pack("<f", random_.choice((0.0, 0.1, 0.5, 1.0, -0.2, 0.01)))),
            Buffer(struct.pack("<f", random_.choice((0.0, 0.1, 0.5, 1.0, -0.2, 0.01)))),
            random_.choice((0, 1, 0x100)),
            globals={"game_time_globals": Buffer(bytes(times))},
        )

        fog = bytearray(random_.randbytes(0x120))
        intensities = (0.0, 0.0, 0.5, 1.0, -1.0)
        struct.pack_into("<f", fog, 0xAC, random_.choice((0.0, 1.0, 1.0, 0.5, -0.5)))
        struct.pack_into("<2f", fog, 0xB0, random_.uniform(0, 100), random_.uniform(0, 100))
        struct.pack_into("<f", fog, 0x10C, random_.choice((1.0, 1.0, 1.0, 0.5)))
        struct.pack_into("<f", fog, 0x118, random_.choice((0.0, 0.0, 0.0, 1.0)))
        for layer in range(3):
            struct.pack_into("<2f", fog, 0x1C + layer * 0x18 + 0x0C, random_.choice(intensities), random_.uniform(0, 100))
        t.check("rasterizer_unknown_00130de0", Buffer(bytes(fog)))
