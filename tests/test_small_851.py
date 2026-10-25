# three functions of the way to the main menu (guided by halo2-decompiled):
# the animations of a graph and of those it inherits asked for, how the
# members of the session are, and bits of the numbers of the actors

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
MEMBER = 0x10C


def run(t) -> None:
    random_ = random.Random(851)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        graph = bytearray(random_.randbytes(0x60 + 2 * 0x20))
        struct.pack_into("<iI", graph, 0x4C, random_.choice((1, 2, 2)), 0x60)
        for entry in range(2):
            struct.pack_into("<I", graph, 0x60 + entry * 0x20 + 4, random_.choice((NONE, salted(0), salted(1))))
        tags = bytearray(random_.randbytes(0x20 + 2 * 0x10))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x20 + tag * 0x10)
        t.check(
            "havok_unknown_001ddd00",
            Buffer(bytes(graph), pointers=(0x50,)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.choice((0, 1, 1)),
            random_.choice((0, 1)),
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18))},
            externals={"havok_unknown_001ddc70": lambda harness, arguments: (0, (arguments[0] & 0xFF,) + tuple(arguments[1:5]) + (arguments[5] & 0xFF,))},
        )

        session = bytearray(random_.randbytes(0x7420))
        struct.pack_into("<i", session, 0x741C, random_.choice((2, 3, 5, 9)))
        struct.pack_into("<i", session, 0x54, random_.choice((0, 1, 3, 4)))
        for member in range(4):
            struct.pack_into("<ii", session, 0xE0 + member * MEMBER, random_.choice((1, 2, 3, 4, 4, 0)), random_.choice((5, 50, 120)))
        found = random_.random() < 0.85
        buffer = Buffer(bytes(session))

        def get(harness, arguments, buffer=buffer, found=found):
            if found:
                harness.mu.mem_write(arguments[0], buffer.address.to_bytes(4, "little"))
            return (int(found), ())

        t.check(
            "network_session_get_members_status",
            random_.choice((0, Buffer(random_.randbytes(4)))),
            extra=[buffer],
            externals={"network_life_cycle_get_state_session": get},
        )

        # actors: a data array of four, each with a number at 0x254
        actors = bytearray(random_.randbytes(0x60 + 4 * 0x260))
        struct.pack_into("<iiiI", actors, 0x24, 0x260, 0, 0, 0)
        struct.pack_into("<i", actors, 0x38, random_.choice((0, 2, 4)))
        struct.pack_into("<II", actors, 0x44, 0x60, 0x50)
        struct.pack_into("<I", actors, 0x50, random_.getrandbits(4))
        for actor in range(4):
            at = 0x60 + actor * 0x260
            struct.pack_into("<H", actors, at, random_.choice((0x1234, 0)))
            actors[at + 9] = random_.choice((0, 1))
            struct.pack_into("<h", actors, at + 0x254, random_.choice((-1, 0, 5, 31, 32, 40)))
        globals_ = bytearray(random_.randbytes(0x10))
        globals_[1] = random_.choice((0, 1, 1))
        t.check(
            "ai_unknown_001c9b50",
            Buffer(random_.randbytes(8)),
            random_.choice((0, 33, 40, 64)),
            globals={"ai_globals": Buffer(bytes(globals_)), "actor_data": Buffer(bytes(actors), pointers=(0x44, 0x48))},
        )
