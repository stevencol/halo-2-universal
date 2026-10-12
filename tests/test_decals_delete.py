# a decal is taken out of its list and deleted; and a chain of them

import random
import struct

from difftest import Buffer

from test_data import array

NONE = 0xFFFFFFFF
COUNT, SIZE = 16, 0x40


def decals(random_) -> Buffer:
    """Some marks in lists: each has one before and one after, or none, and its chain goes on only to later marks."""
    in_use = {index: random_.randint(1, 0x7FFF) for index in range(COUNT) if random_.random() < 0.9}
    table = array(random_, COUNT, SIZE, in_use)
    data = bytearray(table.data)

    def other(low: int = 0) -> int:
        if low >= COUNT or random_.random() < 0.4:
            return NONE
        return random_.getrandbits(16) << 16 | random_.randrange(low, COUNT)

    for index in range(COUNT):
        at = 0x4C + index * SIZE
        data[at + 3] = random_.randrange(7)
        struct.pack_into("<h", data, at + 0xA, random_.choice((-1, -1, 0, 0x1FF, random_.randrange(0x200))))
        struct.pack_into("<3I", data, at + 0x30, other(), other(), random_.getrandbits(32))
        struct.pack_into("<I", data, at + 0x3C, other(index + 1))
    return Buffer(bytes(data), pointers=table.pointers)


def run(t) -> None:
    random_ = random.Random(661)

    for n in range(400):
        for name in ("decal_delete", "decal_delete_chain"):
            t.check(
                name,
                random_.getrandbits(16) << 16 | random_.randrange(COUNT),
                globals={"decal_data": decals(random_), "decal_globals": Buffer(random_.randbytes(0x3C2C + 0x10))},
            )
