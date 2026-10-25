# four small functions of the way to the main menu: whether two vectors
# are one long and perpendicular, a real for two numbers, the letters of a
# language, and one more entry of a table

import random
import struct

from difftest import Buffer, f32

NUMBERS = (0x09000020, 0x0900001F, 0x090006B2, 0x0400000C, 0x0A000014, 0x09000015, 0x09000016, 0x0A000017, 7)


def run(t) -> None:
    random_ = random.Random(832)

    def unit_vector() -> list[float]:
        vector = [random_.uniform(-1, 1) for _ in range(3)]
        length = sum(value * value for value in vector) ** 0.5 or 1
        return [value / length for value in vector]

    for n in range(400):
        forward = unit_vector()
        up = random_.choice((unit_vector(), [forward[1], -forward[0], 0.0]))
        if random_.random() < 0.3:
            up = [value * 1.01 for value in up]
        if random_.random() < 0.2:
            forward = [value * 0.99 for value in forward]
        t.check("real_math_unknown_000a74c0", Buffer(struct.pack("<3f", *forward)), Buffer(struct.pack("<3f", *up)))

        t.check(
            "unit_unknown_001137b0",
            random_.choice(NUMBERS),
            random_.choice(NUMBERS),
            random_.choice((0, 0, 1)),
            random_.choice((0, 0, 1)),
            f32(random_.choice((0.0, 0.1, 0.2, 0.3))),
        )

        t.check("language_get_code", random_.choice((-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9)))

        table = bytearray(random_.randbytes(0x6C08 + 0x40 * 8))
        index = random_.randrange(5)
        struct.pack_into("<i", table, 0, index)
        struct.pack_into("<i", table, 0x6C04, random_.randrange(8))
        data = bytearray(random_.randbytes(0x19C + 16 * 8))
        struct.pack_into("<i", data, 0x198, random_.choice((0, 1, 3, 16)))
        t.check(
            "particle_systems_unknown_00172bb0",
            Buffer(bytes(table)),
            Buffer(bytes(data)),
            random_.getrandbits(32),
            random_.getrandbits(32),
        )
