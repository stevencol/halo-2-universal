# the lists of objects make their tables; three memories of the console are given back

import random

from test_effects_initialize import tables


def run(t) -> None:
    random_ = random.Random(626)

    for n in range(60):
        allocation, buffers, mocks = tables(random_, (0x294,))
        t.check(
            "object_lists_initialize",
            globals={
                "game_state_allocation": allocation,
                "object_list_header_data": random_.getrandbits(32),
                "list_object_data": random_.getrandbits(32),
            },
            extra=buffers,
            mocks=mocks,
            externals={
                "unknown_00296270": lambda harness, arguments, made=random_.choice((0, 0x12340000)): (
                    made,
                    (bytes(harness.mu.mem_read(arguments[0], 12)), arguments[1], arguments[2]),
                ),
            },
        )

    for n in range(300):
        memory = lambda: random_.choice((0, 0x12340000, 0x12340000))
        t.check(
            "game_unknown_0002b4a0_unknown_0003b950",
            globals={"unknown_004c1a5c": memory(), "unknown_004c1a74": memory(), "unknown_004c1a8c": memory()},
            externals={
                "VirtualFree": lambda harness, arguments, done=random_.choice((0, 1, 1)): (done, tuple(arguments)),
                "GetLastError": lambda harness, arguments: (0, ()),
            },
        )
