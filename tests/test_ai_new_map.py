# the minds of the characters, for a new map

import random
import struct

from difftest import Buffer

from test_data import array

TABLES = (
    ("actor_data", 0x100, 0x888),
    ("swarm_data", 0x20, 0x34),
    ("prop_data", 0x100, 0xC4),
    ("prop_ref_data", 0x400, 0x3C),
    ("tracking_data", 0x64, 0x124),
    ("clump_data", 0x14, 0x50),
    ("joint_state_data", 0x14, 0xBC),
    ("dynamic_firing_point_data", 0xF, 0x484),
    ("command_script_data", 0x28, 0xD4),
    ("joint_command_script_data", 0xA, 0x8C),
    ("flock_data", 0xA, 0x28),
)


def run(t) -> None:
    random_ = random.Random(636)

    for n in range(100):
        t.check("ai_unknown_00295970", globals={"unknown_005047f4": Buffer(random_.randbytes(0x24 + 0x10))})

    for n in range(30):
        game = bytearray(random_.randbytes(0x1200))
        struct.pack_into("<i", game, 8, random_.choice((0, 1, 1, 2, 3)))
        data = {}
        for name, maximum_count, size in TABLES:
            count = random_.randint(0, min(maximum_count, 6))
            in_use = {index: random_.randint(1, 0x7FFF) for index in random_.sample(range(maximum_count), count)}
            data[name] = array(random_, maximum_count, size, in_use)
        t.check(
            "ai_initialize_for_new_map",
            globals={
                "main_game_globals": Buffer(bytes(game)),
                "ai_globals": Buffer(random_.randbytes(0x374 + 0x10)),
                "unknown_005044cc": Buffer(random_.randbytes(0x20 + 0x10)),
                "unknown_005044d0": Buffer(random_.randbytes(0x10 + 0x10)),
                "unknown_005047f4": Buffer(random_.randbytes(0x24 + 0x10)),
                "actor_firing_position_owners": Buffer(random_.randbytes(0x40)),
                "unknown_00502428": random_.getrandbits(32),
                "unknown_0050242c": random_.getrandbits(32),
                **data,
            },
            externals={
                name: (lambda harness, arguments: 0)
                for name in (
                    "unknown_001c8010",
                    "hash_table_clear",
                    "unknown_00200b30",
                    "vocalizations_initialize_for_new_map",
                )
            },
        )
