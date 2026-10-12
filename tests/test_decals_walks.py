# two walks over the decals: after a load, and when their bits are cleared

import random
import struct

from difftest import Buffer

from test_data import array

COUNT, SIZE = 0x40, 0x40


def decals(random_) -> Buffer:
    """Some marks: the first of a chain, or not; with a block of the cache; and with their bits."""
    count = random_.choice((0, 1, COUNT, random_.randint(0, COUNT)))
    in_use = {index: random_.randint(-0x8000, 0x7FFF) or 1 for index in random_.sample(range(COUNT), count)}
    table = array(random_, COUNT, SIZE, in_use)
    data = bytearray(table.data)
    data[0x29] = random_.choice((0, 1, 1, 1, 0x80))
    for index in range(COUNT):
        at = 0x4C + index * SIZE
        own = (in_use.get(index, 0) & 0xFFFF) << 16 | index
        data[at + 2] = random_.choice((0, 1, 2, 3, 0xFC, 0xFF, random_.randrange(0x100)))
        struct.pack_into("<I", data, at + 4, random_.getrandbits(16) << 16 | random_.randrange(0x400))
        struct.pack_into("<I", data, at + 0x38, random_.choice((own, own, own ^ 1, own ^ 0x10000, random_.getrandbits(32))))
    return Buffer(bytes(data), pointers=table.pointers)


def run(t) -> None:
    random_ = random.Random(662)

    for n in range(200):
        t.check(
            "decals_after_load_cache_blocks",
            random_.getrandbits(32),
            globals={"decal_data": decals(random_), "decal_cache_block_decal_indices": random_.randbytes(0x1000)},
        )
        t.check(
            "decals_clear_flags",
            random_.choice((0, 1, 1, 0x100, 0x80)),
            globals={"decal_data": decals(random_), "decal_globals": Buffer(random_.randbytes(0x3C2C + 0x10))},
        )
