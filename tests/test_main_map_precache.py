# source/main/main_map_precache.c

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(547)

    for n in range(600):
        name = Buffer(b"maps\\03a_oldmombasa.map" + bytes(8))

        def which(harness, arguments, there=random_.choice((0, 1, 1, 1)), priority=random_.getrandbits(32), only=random_.choice((0, 1, 1, 0xFF))):
            # the fourth byte of the last part is not set by the game
            harness.mu.mem_write(arguments[0], struct.pack("<IIB", name.address, priority, only))
            return there, ()

        status = lambda harness, arguments, at=random_.choice((0, 0, 1, 2, 3, 4)): (at, (arguments[0] == name.address,))
        t.check(
            "main_map_precache_update",
            extra=[name],
            externals={
                "unknown_00163680": which,
                "cached_map_file_status": status,
                "cached_map_file_request": lambda harness, arguments: (1, (arguments[0] == name.address, arguments[1])),
            },
        )
        t.check(
            "main_map_precache_copy_idle",
            externals={"cached_map_file_copy_get_progress": lambda harness, arguments, so=random_.choice((0, 1)): (so, tuple(arguments))},
        )
        t.check(
            "main_map_precache_next_map_not_ready",
            extra=[name],
            externals={
                "unknown_00163610": lambda harness, arguments, there=random_.choice((0, 1, 1, 1)): (name.address if there else 0, ()),
                "cached_map_file_status": status,
            },
        )
