# the minds of the characters, when a map goes

import random
import struct

from difftest import Buffer

TABLES = (
    "squad_data",
    "squad_group_data",
    "clump_data",
    "joint_state_data",
    "command_script_data",
    "joint_command_script_data",
    "dynamic_firing_point_data",
    "prop_data",
    "prop_ref_data",
    "tracking_data",
    "actor_data",
    "vocalization_record_data",
)


def globals_(random_) -> dict:
    ai = bytearray(random_.randbytes(0x374 + 0x10))
    ai[1] = random_.choice((0, 1, 1, 0x80))
    return {"ai_globals": Buffer(bytes(ai))}


def run(t) -> None:
    random_ = random.Random(637)

    for n in range(200):
        t.check(
            "ai_dispose_from_old_map",
            globals={**globals_(random_), **{name: Buffer(random_.randbytes(0x4C + 0x10)) for name in TABLES}},
            externals={
                "swarms_dispose_from_old_map": lambda harness, arguments: 0,
                "unknown_00292e00": lambda harness, arguments: 0,
            },
        )
        t.check(
            "ai_dispose_from_old_structure_bsp",
            globals=globals_(random_),
            externals={"unknown_00292f60": lambda harness, arguments: 0},
        )
