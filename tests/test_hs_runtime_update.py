# the threads of the scripts, each tick

import random
import struct

from difftest import Buffer

from test_data import array

THREADS, SIZE = 12, 0x418


def run(t) -> None:
    random_ = random.Random(698)

    for n in range(300):
        time = random_.choice((0, 100, 5000))
        clock = bytearray(random_.randbytes(0x24))
        struct.pack_into("<I", clock, 8, time)
        in_use = {index: random_.randint(-0x8000, 0x7FFF) or 1 for index in random_.sample(range(THREADS), random_.choice((0, 1, 5, THREADS)))}
        threads = array(random_, THREADS, SIZE, in_use)
        data = bytearray(threads.data)
        for index in range(THREADS):
            at = 0x4C + index * SIZE
            data[at + 2] = random_.choice((0, 1, 2, 4, 4))
            struct.pack_into("<i", data, at + 8, random_.choice((0, time, time + 1, time - 1, -1, 0x7FFFFFFF)))
        # one of the threads may stop the scripts
        stops = random_.choice((None, None, 0, 2))

        def runs(harness, arguments, stops=stops):
            ran = sum(1 for call in harness._calls if call[0] == "unknown_00209850")
            if ran == stops:
                harness.mu.mem_write(harness.global_addresses["unknown_004f9388"], b"\0")
            return 0

        t.check(
            "hs_runtime_update",
            globals={
                "unknown_004f9388": random_.choice((b"\1", b"\1", b"\1", b"\0", b"\x80")),
                "game_time_globals": Buffer(bytes(clock)),
                "hs_thread_data": Buffer(bytes(data), pointers=threads.pointers),
            },
            externals={"unknown_00209850": runs, "unknown_001deed0": lambda harness, arguments: 0},
        )
