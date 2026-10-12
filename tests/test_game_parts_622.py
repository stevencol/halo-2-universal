# the players and the looping sounds of the objects, for a new map

import random

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(622)

    def note(harness, arguments):
        return 0, tuple(arguments)

    def table(harness, arguments):
        return 0, (harness.normalize(arguments[0]), harness.mu.mem_read(arguments[0] + 0x29, 1)[0])

    for n in range(300):
        t.check(
            "players_initialize_for_new_map",
            globals={
                "unknown_004ed39c": bytes((random_.choice((0, 0, 0, 1, 0x80)),)),
                "player_data": Buffer(random_.randbytes(0x4C)),
                "players_globals": Buffer(random_.randbytes(0x140)),
            },
            externals={"data_delete_all": table, "ai_unknown_001c7fe0": note},
        )
        t.check(
            "object_looping_sounds_initialize_for_new_map",
            globals={
                "unknown_004ed28c": random_.choice((0, 1, 1, 1)) and Buffer(random_.randbytes(0x4C)),
                "unknown_004ed288": Buffer(random_.randbytes(0x254)),
            },
            externals={"data_delete_all": table, "sound_console_set_gains": lambda harness, arguments: (0, (bytes(harness.mu.mem_read(arguments[0], 16)),)), "sound_fade_in_for_new_map": note},
        )
        t.check(
            "object_looping_sounds_unknown_00188980",
            globals={"unknown_004ed288": random_.choice((0, 1, 1, 1)) and Buffer(random_.randbytes(0x254))},
            externals={"unknown_0018bb80": note},
        )
