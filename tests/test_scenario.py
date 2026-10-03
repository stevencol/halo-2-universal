# source/scenario/scenario.c: unloading the scenario and where a point is

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
LEAF_COUNT = 12


def run(t) -> None:
    random_ = random.Random(138)

    def tags_unload(harness, arguments):
        # what the scenario index is when the tags are unloaded
        (index,) = struct.unpack("<I", harness.mu.mem_read(harness.global_addresses["global_scenario_index"], 4))
        return 0, (index,)

    for _ in range(20):
        t.check(
            "scenario_unload",
            globals={
                "global_scenario_index": struct.pack("<I", random_.getrandbits(32)),
                "global_structure_bsp_index": struct.pack("<H", random_.getrandbits(16)),
                "global_scenario": random_.getrandbits(32),
                "global_game_globals": random_.getrandbits(32),
                "global_structure_bsp": random_.getrandbits(32),
                "global_structure_lightmap": random_.getrandbits(32),
                "global_collision_bsp": random_.getrandbits(32),
                "global_bsp3d": random_.getrandbits(32),
            },
            externals={"scenario_tags_unload": tags_unload},
        )

    for _ in range(300):
        # the structure bsp has its leaves after it
        bsp = bytearray(random_.randbytes(0x40 + LEAF_COUNT * 8))
        struct.pack_into("<iI", bsp, 0x2C, LEAF_COUNT, 0x40)
        leaf = random_.choice((NONE, 0, 1, LEAF_COUNT - 1, random_.randrange(LEAF_COUNT)))
        structure_bsp_index = random_.choice((0xFFFF, 0, 1, 2, 0x7FFF, 0x8000))

        def test_point(harness, arguments, leaf=leaf):
            return leaf

        t.check(
            "scenario_location_from_point",
            Buffer(random_.randbytes(8)),
            Buffer(struct.pack("<3f", *(random_.uniform(-50, 50) for _ in range(3)))),
            globals={
                "global_structure_bsp_index": struct.pack("<H", structure_bsp_index),
                "global_structure_bsp": Buffer(bytes(bsp), pointers=(0x30,)),
                "global_bsp3d": Buffer(random_.randbytes(16)),
            },
            externals={"bsp3d_test_point": test_point},
        )
