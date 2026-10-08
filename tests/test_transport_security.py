# source/networking/transport_security.c

import random
import struct

from difftest import Buffer, GlobalData

FAILED = 0x80004005


def run(t) -> None:
    random_ = random.Random(316)

    def keys() -> GlobalData:
        data = bytearray(random_.randbytes(0x100))
        for index in range(8):
            data[index * 0x20] = random_.choice((1, 1, 0x80, 0))
        return GlobalData(bytes(data))

    # ---------- what a session is known by, and its key: they can be in the
    # stack, so what is told of them is compared by what they are

    for n in range(160):
        result = random_.choice((0, 0, 0, 1, FAILED))
        t.check(
            "transport_security_set_key",
            random_.randrange(8),
            Buffer(random_.randbytes(16)),
            random_.choice((0, 0, 1, random_.getrandbits(32))),
            random_.choice((0, 1, 0x80, 0x1FF, 0x100)),
            Buffer(random_.randbytes(8)),
            globals={"unknown_004cf7d4": keys()},
            externals={
                "unknown_003cd0e1": lambda harness, arguments: 0,
                "unknown_003cd0d6": lambda harness, arguments, result=result: (
                    result,
                    (bytes(harness.mu.mem_read(arguments[0], 8)), bytes(harness.mu.mem_read(arguments[1], 16))),
                ),
            },
        )

    for n in range(200):
        made = random_.randbytes(24)
        # the library always makes them here: what the game does when it
        # cannot is done with what there was in the stack
        makes = random_.choice((0, 0, 0, 1, FAILED))
        result = random_.choice((0, 0, 0, 1, FAILED))

        def make(harness, arguments, made=made, makes=makes):
            harness.mu.mem_write(arguments[0], made[:8])
            harness.mu.mem_write(arguments[1], made[8:])
            return makes, ()

        t.check(
            "transport_security_create_key",
            random_.choice((0, 0, 0, 1, random_.getrandbits(32))),
            random_.randrange(8),
            random_.choice((0, 1, 1, 0x80, 0x100)),
            globals={"unknown_004cf7d4": keys()},
            externals={
                "unknown_003cd0cb": make,
                "unknown_003cd0e1": lambda harness, arguments: 0,
                "unknown_003cd0d6": lambda harness, arguments, result=result: (
                    result,
                    (bytes(harness.mu.mem_read(arguments[0], 8)), bytes(harness.mu.mem_read(arguments[1], 16))),
                ),
            },
        )

    # ---------- what this console is known by

    for asked in (1, 0, 0x80):
        for known in (1, 0):
            t.check(
                "transport_security_forget_address",
                random_.getrandbits(32),
                globals={"unknown_004cf791": bytes((asked,)), "unknown_004cf792": bytes((known,))},
            )

    for n in range(200):
        address = random_.randbytes(0x24)
        status = random_.choice((0, 0, 1, 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 0x84, 0x8000, 0x8002, 0xFFFFFFFF))

        def says(harness, arguments, address=address, status=status):
            # where it goes is in the stack
            harness.mu.mem_write(arguments[0], address)
            return status, ()

        t.check(
            "transport_security_get_address",
            globals={
                "unknown_004cf791": bytes((random_.choice((0, 0, 0, 1, 0x80)),)),
                "unknown_004cf792": bytes((random_.choice((0, 1)),)),
                "unknown_004cf793": random_.randbytes(0x24),
                "unknown_004cf7b8": random_.randbytes(0x14),
            },
            externals={"unknown_003cd17d": says},
        )
