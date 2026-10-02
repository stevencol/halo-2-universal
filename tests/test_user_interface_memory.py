# source/interface/user_interface_memory.c
#
# The pool is the test's: what is compared is what the functions ask of
# memory_pool.c, which the tests of that file check

import random
import struct

from difftest import Buffer, GlobalRange
from test_async import calls_so_far

NONE = 0xFFFFFFFF
POOL_SIZE = 0x40


def run(t) -> None:
    random_ = random.Random(38)

    # ---------- the pool

    def new_pool(pool, give):
        def handler(harness, arguments):
            # the name is in the game and in source/, at different places
            allocation, size, name = arguments
            text = bytes(harness.mu.mem_read(name, 0x20)).split(b"\0", 1)[0]
            return (pool.address if give else 0), (harness.normalize(allocation), size, text)

        return handler

    for configuration in (*range(-1, 8), NONE - 3, 0x100):
        for give in (True, False):
            pool = Buffer(random_.randbytes(POOL_SIZE))
            t.check(
                "user_interface_memory_pool_new",
                configuration & 0xFFFFFFFF,
                globals={"user_interface_memory_pool": 0x1234, "user_interface_allocation": GlobalRange(4)},
                externals={"memory_pool_new": new_pool(pool, give)},
                extra=[pool],
            )

    for pool in (0, 0x60F00000):
        t.check(
            "user_interface_memory_pool_delete",
            globals={"user_interface_memory_pool": pool},
            externals={"memory_pool_delete": lambda harness, arguments: 0},
        )

    # ---------- its memory

    # there is room at once, after the interface lets go of something, or not at all
    for succeeds_at in (0, 1, 2):
        for _ in range(4):

            def allocate(harness, arguments, succeeds_at=succeeds_at):
                # the place for the address is in the stack, which is not at
                # the same place in the game and in source/
                pool, reference, *rest = arguments
                if calls_so_far(harness, "memory_pool_block_allocate") == succeeds_at:
                    harness.mu.mem_write(reference, struct.pack("<I", 0x60F01230))
                    return 1, (pool, *rest)
                return 0, (pool, *rest)

            t.check(
                "user_interface_memory_allocate",
                random_.getrandbits(16),
                random_.getrandbits(32),
                random_.getrandbits(32),
                globals={"user_interface_memory_pool": 0x60F00000},
                externals={"memory_pool_block_allocate": allocate, "unknown_00147ebe": lambda harness, arguments: 0},
            )

    def free(harness, arguments):
        pool, reference = arguments
        return 0, (pool, struct.unpack("<I", harness.mu.mem_read(reference, 4))[0])

    for _ in range(4):
        t.check(
            "user_interface_memory_free",
            0x60F00000 + random_.getrandbits(12),
            globals={"user_interface_memory_pool": 0x60F00000},
            externals={"memory_pool_block_free": free},
        )
