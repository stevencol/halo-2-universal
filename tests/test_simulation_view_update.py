# source/networking/simulation_view.c: a view does its work

import random
import struct

from difftest import Buffer

VIEW = 0x90
DATA = 0x6068


def run(t) -> None:
    random_ = random.Random(5782)

    for n in range(3000):
        data = bytearray(random_.randbytes(VIEW + DATA))
        struct.pack_into("<i", data, 0x28, random_.choice((0, 0, 0, 0, 4)))
        struct.pack_into("<H", data, 2, random_.choice((0, 1, 2, 3, 3, 4, 4, 5, 0x103)))
        struct.pack_into("<I", data, 8, VIEW)
        for offset in (0x3A, 0x5079, 0x6064):
            data[VIEW + offset] = random_.choice((0, 0, 0, 1, 0x80))

        def state(harness, arguments, kind=random_.choice((None, None, 0, 3, 5))):
            # going to another state can leave the view of another kind
            if kind is not None:
                harness.mu.mem_write(arguments[0] + 2, struct.pack("<H", kind))
            return 0, (harness.normalize(arguments[0]), arguments[1], arguments[2])

        t.check(
            "simulation_view_update",
            Buffer(bytes(data), pointers=(8,)),
            externals={
                "simulation_view_set_state": state,
                "simulation_view_events_update": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
        )
