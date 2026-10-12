# three more functions of game/game_unknown_0016efb0.c

import random
import struct

from difftest import Buffer, GlobalData


def run(t) -> None:
    random_ = random.Random(623)

    def place(harness, arguments):
        return 0, tuple(harness.normalize(argument) for argument in arguments)

    flag = lambda: bytes((random_.choice((0, 1, 0x80)),))
    for n in range(400):
        things = lambda: {"unknown_004e9bd4": GlobalData(random_.randbytes(0xD60))}
        t.check(
            "game_unknown_0016efb0_initialize_for_new_map",
            globals={**things(), "unknown_004ea934": flag(), "unknown_00510c70": Buffer(random_.randbytes(0x18))},
            externals={"unknown_0016f4b0": place},
        )

        players = bytearray(random_.randbytes(0x130))
        for index in range(4):
            struct.pack_into("<i", players, 0xC + index * 4, random_.choice((-1, -1, 0x12340000 + index)))
        kept = bytearray(random_.randbytes(0x24))
        kept[5] = random_.choice((0, 0, 1, 0x80))
        time = struct.pack("<BBhfi", 1, 0, 30, 1 / 30, random_.choice((0, 0, 1, 500, -1)))
        t.check(
            "game_unknown_0016efb0_unknown_0016f120",
            globals={
                **things(),
                "players_globals": Buffer(bytes(players)),
                "unknown_00510c50": random_.choice((0, 1, 1, 1)) and Buffer(bytes(kept)),
                "game_time_globals": Buffer(time + random_.randbytes(0x18)),
                "unknown_004ea935": flag(),
            },
            externals={"scenario_location_from_point": place},
        )

        game = bytearray(random_.randbytes(0x20))
        struct.pack_into("<i", game, 8, random_.choice((0, 1, 2, 2, 3)))
        t.check(
            "game_unknown_0016efb0_unknown_0016f200",
            globals={
                "main_game_globals": Buffer(bytes(game)),
                "unknown_004ea935": flag(),
                "unknown_004ea936": flag(),
                **{name: random_.getrandbits(32) for name in ("unknown_004e6470", "unknown_004e6474", "unknown_004e64a0", "unknown_004e64a4", "unknown_004e64ac")},
            },
        )
