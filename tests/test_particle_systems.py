# source/effects/particle_systems.c: the particles make their four tables

import random

from test_effects_initialize import tables

NAMES = ("particle_system_data", "particle_data", "particle_emitter_data", "particle_location_data")


def run(t) -> None:
    random_ = random.Random(628)

    for n in range(60):
        allocation, buffers, mocks = tables(random_, (0x2A5C, 0x100CC, 0x4C6C, 0x346C))
        t.check(
            "particle_systems_initialize",
            globals={"game_state_allocation": allocation, **{name: random_.getrandbits(32) for name in NAMES}},
            extra=buffers,
            mocks=mocks,
        )
