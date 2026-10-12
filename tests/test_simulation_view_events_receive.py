# source/networking/simulation_view.c: a view takes the things that happen that another console sent

import random
import struct

from difftest import Buffer

VIEW = 0x90
DATA = 0x6060 + 0x10


def run(t) -> None:
    random_ = random.Random(587)

    for n in range(3000):
        data = bytearray(random_.randbytes(VIEW + DATA))
        number = random_.choice((0, 5, 100))
        struct.pack_into("<i", data, 0x30, number)
        struct.pack_into("<I", data, 8, VIEW)
        events = VIEW + 0x6060
        data[events + 6] = random_.choice((0, 1, 1, 1, 0x80))
        waits = random_.choice((0, 1, 7))
        struct.pack_into("<i", data, events + 8, waits)
        t.check(
            "simulation_view_events_receive",
            Buffer(bytes(data), pointers=(8,)),
            random_.choice((number, number, number, number - 1, number + 1, -1)),
            random_.choice((waits, waits, 0, waits + 1, -1)),
            Buffer(random_.randbytes(0x20)),
            externals={
                "unknown_001988e0": lambda harness, arguments: (0, (harness.normalize(arguments[0]), harness.normalize(arguments[1]))),
                "unknown_001973f0": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
        )
