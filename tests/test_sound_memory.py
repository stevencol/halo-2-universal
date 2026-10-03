# source/sound/sound_memory.c

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(109)

    POOL, OTHER_POOL = 0x80100000, 0x80200000

    def pools() -> dict:
        return {
            "sound_memory_pool": POOL,
            "sound_memory_other_pool": OTHER_POOL,
            "sound_memory_use_other_pool": bytes((random_.choice((0, 1, 0x80)),)),
        }

    for n in range(120):
        given = random_.choice((0x80345000, 0))
        could = random_.choice((1, 0, 0x100))

        def allocate(harness, arguments, given=given, could=could):
            pool, reference, size, unknown4, unknown8 = arguments
            if could & 0xFF:
                harness.mu.mem_write(reference, struct.pack("<I", given))
            return could, (pool, size, unknown4, unknown8)

        t.check(
            "sound_memory_allocate",
            random_.choice((0, 1, 0x200, 0x1000, 0xFFFFFFFF)),
            random_.getrandbits(32),
            globals=pools(),
            externals={"memory_pool_block_allocate": allocate},
        )

    for n in range(60):
        t.check(
            "sound_memory_free",
            random_.choice((0, 0x80345000, 0x80345010)),
            random_.getrandbits(32),
            globals=pools(),
            externals={
                "memory_pool_block_free": lambda harness, arguments: (
                    0,
                    (arguments[0], struct.unpack("<I", harness.mu.mem_read(arguments[1], 4))[0]),
                ),
            },
        )
