# the components of the physics and the impacts make their tables

import random

from test_effects_initialize import tables


def run(t) -> None:
    random_ = random.Random(630)

    for n in range(60):
        allocation, buffers, mocks = tables(random_, (0x1409B,))
        t.check(
            "havok_components_initialize",
            globals={"heap_allocation": allocation, "havok_component_data": random_.getrandbits(32)},
            extra=buffers,
            mocks=mocks,
        )
        allocation, buffers, mocks = tables(random_, (0x1450, 0x850))
        t.check(
            "impacts_initialize",
            globals={
                "heap_allocation": allocation,
                "impact_data": random_.getrandbits(32),
                "impact_array_data": random_.getrandbits(32),
            },
            extra=buffers,
            mocks=mocks,
        )
