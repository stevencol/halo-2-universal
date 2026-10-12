# every object gets its part of the physics again

import random
import struct

from difftest import Buffer

from test_units import COUNT, some_objects

NOTHING = lambda harness, arguments: 0


def run(t) -> None:
    random_ = random.Random(688)

    for n in range(200):
        # the two walks do not go over the same objects, so that some keep what they had
        walks = [random_.sample(range(COUNT), random_.choice((0, 1, 3, COUNT))) for _ in range(2)]
        salts = [random_.getrandbits(16) << 16 for _ in range(COUNT)]

        def next_(harness, arguments, walks=walks, salts=salts):
            asked = sum(1 for call in harness._calls if call[0] == "object_iterator_next")
            some = walks[0] if asked <= len(walks[0]) else walks[1]
            at = struct.unpack("<h", harness.mu.mem_read(arguments[0] + 6, 2))[0]
            harness.mu.mem_write(arguments[0] + 6, struct.pack("<h", at + 1))
            if at >= len(some):
                return 0, ()
            harness.mu.mem_write(arguments[0] + 8, struct.pack("<I", salts[at] | some[at]))
            table_at = struct.unpack("<I", harness.mu.mem_read(harness.global_addresses["object_header_data"], 4))[0]
            entries = struct.unpack("<I", harness.mu.mem_read(table_at + 0x44, 4))[0]
            return struct.unpack("<I", harness.mu.mem_read(entries + some[at] * 12 + 8, 4))[0], ()

        clock = bytearray(random_.randbytes(0x24))
        t.check(
            "havok_unknown_001c2a10",
            globals={
                "object_header_data": some_objects(random_),
                "game_time_globals": Buffer(bytes(clock)),
                "unknown_0047f054": random_.getrandbits(32),
            },
            externals={
                "object_iterator_next": next_,
                "unknown_00146bf0": NOTHING,
                "unknown_001c35f0": NOTHING,
                "unknown_00278f00": NOTHING,
                "unknown_001c50c0": NOTHING,
            },
        )
