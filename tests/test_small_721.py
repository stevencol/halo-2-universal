# thirteen small functions of actors and one of the time

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
ACTORS, ACTOR = 3, 0x888


def run(t) -> None:
    random_ = random.Random(721)

    for n in range(150):
        def actors() -> Buffer:
            data = bytearray(random_.randbytes(0x4C + ACTORS * ACTOR))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(ACTORS):
                at = 0x4C + index * ACTOR
                for offset in (0x338, 0x85C):
                    struct.pack_into("<I", data, at + offset, random_.choice((NONE, 0, random_.getrandbits(32))))
                struct.pack_into("<h", data, at + 0x61E, random_.choice((-5, 0, 7, 0x7000)))
            return Buffer(bytes(data), pointers=(0x44,))

        actor_index = random_.getrandbits(16) << 16 | random_.randrange(ACTORS)
        time = {"game_time_globals": Buffer(random_.randbytes(0x24))}

        for name, arguments in (
            ("actor_unknown_001a8380", 2),
            ("actor_unknown_001b2310", 2),
            ("actor_unknown_001ab3a0", 2),
            ("actor_unknown_001c04c0", 2),
            ("actor_unknown_001b5790", 2),
            ("actor_unknown_001b5dd0", 2),
            ("actor_unknown_001b6880", 3),
            ("actor_unknown_001be6b0", 2),
            ("actor_unknown_001c1640", 2),
            ("actor_unknown_001ad550", 1),
            ("actor_unknown_001b2da0", 1),
            ("actor_unknown_001e4260", 1),
        ):
            t.check(
                name,
                actor_index,
                *(random_.getrandbits(32) for _ in range(arguments - 1)),
                globals={"actor_data": actors(), **time},
            )
        t.check(
            "actor_unknown_001f3610",
            actor_index,
            random_.getrandbits(16) << 16 | random_.choice((0xFFFB, 0, 7, 8, 0x7FFF, 0x8000)),
            globals={"actor_data": actors()},
        )

        ticks = bytearray(random_.randbytes(0x24))
        struct.pack_into("<h", ticks, 2, random_.choice((30, 60, 0, -30)))
        seconds = random_.choice((0.0, 0.5, 1.0, 0.05, 0.25, random_.uniform(-100.0, 100.0), 1.0 / 60.0))
        t.check(
            "game_seconds_to_ticks",
            struct.unpack("<I", struct.pack("<f", seconds))[0],
            globals={"game_time_globals": Buffer(bytes(ticks))},
        )
