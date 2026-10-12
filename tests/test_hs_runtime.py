# source/hs/hs_runtime.c: what runs the scripts makes its tables

import random

from test_effects_initialize import tables


def run(t) -> None:
    random_ = random.Random(627)

    def note(harness, arguments):
        return 0, tuple(harness.normalize(argument) for argument in arguments)

    for n in range(40):
        allocation, buffers, mocks = tables(random_, (0x51E74, 0x61CC))
        t.check(
            "hs_runtime_initialize",
            globals={
                "game_state_allocation": allocation,
                "hs_thread_data": random_.getrandbits(32),
                "hs_global_data": random_.getrandbits(32),
            },
            extra=buffers,
            mocks=mocks,
            externals={"data_delete_all": note, "datum_new_at_absolute_index": note, "unknown_0020a1a0": note},
        )
