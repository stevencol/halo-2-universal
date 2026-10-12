# the objects of the type 7 forget two shorts when the structure bsp goes

import random
import struct

from test_units import COUNT, some_objects


def run(t) -> None:
    random_ = random.Random(657)

    for n in range(200):
        some = random_.sample(range(COUNT), random_.choice((0, 1, 3, COUNT)))
        salts = [random_.getrandbits(16) << 16 for _ in range(COUNT)]

        def next_(harness, arguments, some=some, salts=salts):
            # the objects of this case, one after the other: which one is next is kept in the iterator
            at = struct.unpack("<h", harness.mu.mem_read(arguments[0] + 6, 2))[0]
            harness.mu.mem_write(arguments[0] + 6, struct.pack("<h", at + 1))
            if at >= len(some):
                return 0, ()
            harness.mu.mem_write(arguments[0] + 8, struct.pack("<I", salts[at] | some[at]))
            table_at = struct.unpack("<I", harness.mu.mem_read(harness.global_addresses["object_header_data"], 4))[0]
            entries = struct.unpack("<I", harness.mu.mem_read(table_at + 0x44, 4))[0]
            return struct.unpack("<I", harness.mu.mem_read(entries + some[at] * 12 + 8, 4))[0], ()

        t.check(
            "game_unknown_00210db0_unknown_00210e80",
            globals={"object_header_data": some_objects(random_)},
            externals={"object_iterator_next": next_},
        )
