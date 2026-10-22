# three small functions: sixteen things of six bytes read and written in a
# stream, and whether an online task went wrong

import random
import struct

from difftest import Buffer
from test_online_storage import tasks_table

NONE = 0xFFFFFFFF
HEADER = 0x40


def stream(random_: random.Random, size: int, position: int, error: int = 0, blank: bool = False) -> Buffer:
    """A stream (its structure, then its data) over `size` bytes, at a position in bits."""
    data = bytes(size) if blank else random_.randbytes(size)
    header = bytearray(random_.randbytes(HEADER))
    struct.pack_into("<I4i", header, 0, HEADER, size, random_.getrandbits(31), random_.getrandbits(31), position)
    header[0x14] = error
    struct.pack_into("<i", header, 0x18, 0)
    return Buffer(bytes(header) + data, pointers=(0,))


def run(t) -> None:
    random_ = random.Random(748)

    for n in range(300):
        data = bytearray(random_.randbytes(0x64))
        struct.pack_into("<I", data, 0, random_.choice((0, 1, 0xFFFF, 0x8001, random_.getrandbits(16), 0x10000, random_.getrandbits(32))))
        size = random_.choice((0x80, 0x80, 4, 20))
        t.check(
            "simulation_write_unknown_00087c00",
            Buffer(bytes(data)),
            stream(random_, size, random_.choice((0, 3, 8, 17)), blank=True),
            externals={"csprintf256": lambda harness, arguments: (0, (arguments[2],))},
        )

        size = random_.choice((0x80, 0x80, 4, 20))
        t.check(
            "simulation_read_unknown_00087c80",
            stream(random_, size, random_.choice((0, 3, 8, 17, size * 8 - 10)), error=random_.choice((0, 0, 0, 1))),
            Buffer(random_.randbytes(0x64)),
        )

        # a mask of 0 in the last 16 bits: the stream ends just at its end
        size = random_.choice((4, 20, 0x80))
        exact = stream(random_, size, size * 8 - 16, error=random_.choice((0, 0, 1)))
        data = bytearray(exact.data)
        data[-2:] = b"\0\0"
        t.check(
            "simulation_read_unknown_00087c80",
            Buffer(bytes(data), pointers=exact.pointers),
            Buffer(random_.randbytes(0x64)),
        )

        tasks, number = tasks_table(random_)
        result = random_.choice((0, 1, 0x1500F2, 0x80155100, 0x80004005, 0x80151000, 0x80155101, random_.getrandbits(32)))
        t.check(
            "online_task_failed",
            random_.choice((number, number, NONE, number ^ 0x10000, number & 0xFFFF | 0x7FFF0000)),
            globals={"online_tasks": tasks},
            externals={
                "online_task_continue": lambda harness, arguments, result=result: (
                    result,
                    (harness.normalize(arguments[0]),),
                ),
            },
        )
