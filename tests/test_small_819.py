# two small functions of the way to the main menu: a byte of the session
# set in some of its states, and a long of a list before a mark

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
NODE = 0x10


def run(t) -> None:
    random_ = random.Random(819)

    for n in range(400):
        session = bytearray(random_.randbytes(0x7420))
        struct.pack_into("<i", session, 0x741C, random_.choice((0, 1, 2, 3, 5, 8, 9, -1)))
        told = random_.randrange(2)
        t.check(
            "network_life_cycle_unknown_000642a0",
            Buffer(random_.randbytes(4)),
            globals={"unknown_00527330": bytes((random_.choice((0, 1, 1)),)), "network_life_cycle_session": Buffer(bytes(session))},
            externals={"network_session_leader_set_unknown49a1": lambda harness, arguments, told=told: (told, tuple(arguments))},
        )

        # up to five nodes, each with a thing of 0x10 bytes after them
        count = random_.randrange(1, 6)
        data = bytearray(random_.randbytes(count * NODE + count * 0x10))
        pointers = []
        for node in range(count):
            at = node * NODE
            struct.pack_into("<I", data, at, count * NODE + node * 0x10)
            pointers.append(at)
            if node + 1 < count:
                struct.pack_into("<I", data, at + 0x0C, (node + 1) * NODE)
                pointers.append(at + 0x0C)
            else:
                struct.pack_into("<I", data, at + 0x0C, 0)
            thing = count * NODE + node * 0x10
            struct.pack_into("<I", data, thing + 8, random_.choice((0xCABCABB0, 1, 2)))
        t.check("havok_unknown_001c55b0", Buffer(bytes(data), pointers=tuple(pointers)))
