# the cache of the decals: its two functions, its idle and what is done after a load

import random
import struct

from difftest import Buffer

from test_data import array

NONE = 0xFFFFFFFF
BLOCKS = 0x400


def table(random_) -> bytes:
    """The mark of each block of the cache: one of the table, or none."""
    return b"".join(
        struct.pack("<I", random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(BLOCKS))))
        for _ in range(BLOCKS)
    )


def run(t) -> None:
    random_ = random.Random(660)

    for n in range(200):
        block_index = random_.getrandbits(16) << 16 | random_.randrange(BLOCKS)
        t.check(
            "decal_cache_delete_block",
            block_index,
            globals={"decal_cache_block_decal_indices": table(random_)},
            externals={"decal_delete_chain": lambda harness, arguments: 0},
        )

        indices = bytearray(table(random_))
        struct.pack_into("<I", indices, (block_index & 0xFFFF) * 4, random_.getrandbits(16) << 16 | random_.randrange(BLOCKS))
        decals = array(random_, BLOCKS, 0x40, {})
        data = bytearray(decals.data)
        for index in range(BLOCKS):
            data[0x4C + index * 0x40 + 2] = random_.choice((0, 1, 2, 3, 0xFC, 0xFF, random_.randrange(0x100)))
        t.check(
            "decal_cache_block_locked",
            block_index,
            globals={
                "decal_cache_block_decal_indices": bytes(indices),
                "decal_data": Buffer(bytes(data), pointers=decals.pointers),
            },
        )

        t.check(
            "decals_after_load",
            random_.getrandbits(32),
            globals={"unknown_00509448": Buffer(random_.randbytes(0x70), places=(0x20, 0x24))},
        )

        cache = bytearray(random_.randbytes(0x70))
        struct.pack_into("<i", cache, 0x38, random_.choice((0, 1, 0x7FFFFFFE, 0x7FFFFFFF, 0x7FFFFFFF, random_.getrandbits(31))))
        t.check(
            "decals_cache_idle",
            globals={"unknown_00509448": Buffer(bytes(cache))},
            externals={"lruv_wrap_ticks": lambda harness, arguments: 0},
        )
