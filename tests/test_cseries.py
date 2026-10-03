# source/cseries/cseries.c (csnzprintf is in test_map_files.py)

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(107)

    def string(text: bytes) -> Buffer:
        return Buffer(text + b"\0" + random_.randbytes(8))

    # ---------- how long a string is, up to a number

    for text in (b"", b"a", b"abc", b"x" * 40, b"\xff\x80 ok"):
        for maximum in (0, 1, 2, 3, 4, 39, 40, 41, 1000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF):
            t.check("csstrnlen", string(text), maximum)

    # ---------- what is added after a string

    for start in (b"", b"z:\\", b"abc", b"x" * 30, b"y" * 62, b"w" * 63):
        for size in (64, 40, 35, 32, 31):
            for format_, argument in ((b"%scache%03d", string(b"maps\\")), (b"%s", string(b"x" * 20)), (b"%05d|", -42), (b"", 0), (b"no", 0)):
                if len(start) >= size:
                    continue
                buffer = start + b"\0" + random_.randbytes(64 - len(start) - 1) + random_.randbytes(8)
                t.check("csnzappendf", Buffer(buffer), size, string(format_), argument)

    # ---------- memory of the console

    for size in (0, 1, 0x1000, 0x123456, 0xFFFFFFFF):
        for memory in (0, 0x80123000):
            t.check(
                "system_allocate",
                size,
                externals={
                    "VirtualAlloc": lambda harness, arguments, memory=memory: memory,
                    "GetLastError": lambda harness, arguments: 8,
                },
            )
