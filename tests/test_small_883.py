# one function of the way to the main menu (guided by halo2-decompiled): the
# matrix of a havok component from that of its main body or of its object

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(883)

    for n in range(400):
        component = bytearray(random_.randbytes(0x80))
        component[0x18] = random_.choice((0, 1, 2, 0xFF, 0x80, 5))
        struct.pack_into("<i", component, 0x74, random_.choice((0, 1, 3)))
        matrix = random_.randbytes(0x34)

        def fill(at, matrix=matrix):
            def mock(harness, arguments):
                harness.mu.mem_write(arguments[at], matrix)
                return 0, tuple(argument for place, argument in enumerate(arguments) if place != at)
            return mock

        t.check(
            "havok_unknown_001d4360",
            Buffer(bytes(component)),
            Buffer(random_.randbytes(0x34)),
            externals={
                "havok_component_get_rigid_body_matrix": fill(2),
                "object_get_world_matrix": fill(1),
                "havok_unknown_001d43d0": lambda harness, arguments: (
                    0,
                    (arguments[0], bytes(harness.mu.mem_read(arguments[1], 0x34)), arguments[2]),
                ),
            },
        )
