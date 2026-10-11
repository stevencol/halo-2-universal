# source/interface/user_interface.c, and rasterizer_text_end

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(539)

    for n in range(600):
        t.check(
            "rasterizer_text_end",
            globals={"rasterizer_text_unknown_004c1a18": bytes((random_.choice((0, 1, 1, 0x80)),))},
            externals={"unknown_0001e930": lambda harness, arguments: (0, tuple(arguments))},
        )
        t.check(
            "user_interface_unknown_00147da5",
            random_.choice((NONE, 0, 1, 2, 3)),
            externals={
                "user_interface_controller_find_voice_index": lambda harness, arguments, index=random_.choice((NONE, 0, 1, 2, 3)): (index, tuple(arguments)),
                "unknown_00148e6d": lambda harness, arguments, so=random_.choice((0, 1)): (so, tuple(arguments)),
            },
        )
        game = bytearray(random_.randbytes(0x1200))
        struct.pack_into("<I", game, 8, random_.choice((1, 2, 3, 3, 3)))
        t.check(
            "user_interface_unknown_0014a224",
            globals={"main_game_globals": Buffer(bytes(game))},
            externals={
                "game_in_progress": lambda harness, arguments, so=random_.choice((0, 1, 1, 1)): (so, ()),
                "unknown_00147f4f": lambda harness, arguments, at=random_.choice((0, 5, 6, 6, 7, 9, 9, 10, NONE)): (at, ()),
            },
        )

    # ---------- whether the interface shows a thing

    SCREEN = 0x620
    STATES = (0, 6, 7, 7, 8, 8, 9, 0xF0, 0xF0, 0xEF, NONE)
    for n in range(2000):
        identifier = random_.choice((0x3A, 0x23, 0x38, 0x1C, random_.getrandbits(32)))
        data = bytearray(random_.randbytes(0x10 + 2 * SCREEN))
        pointers = []
        for index in range(2):
            screen = 0x10 + index * SCREEN
            struct.pack_into("<I", data, screen + 0x70, random_.choice(STATES))
            struct.pack_into("<I", data, screen + 0x610, random_.choice((identifier, identifier, random_.getrandbits(32))))
            if random_.random() < 0.8:
                struct.pack_into("<I", data, 8 + index * 4, screen)
                pointers.append(8 + index * 4)
            else:
                struct.pack_into("<I", data, 8 + index * 4, 0)
        channel = Buffer(bytes(data), pointers=tuple(pointers))

        screen = Buffer(random_.randbytes(0x70) + struct.pack("<I", random_.choice(STATES)) + random_.randbytes(8))
        t.check("user_interface_screen_unknown_00149278", random_.choice((screen, screen, screen, 0)))
        t.check(
            "user_interface_unknown_001492d6",
            random_.choice((4, 0, 1)),
            random_.choice((0, 1, 5)),
            identifier,
            extra=[channel],
            externals={
                "unknown_00148262": lambda harness, arguments, there=random_.random() < 0.9: (
                    channel.address if there else 0,
                    tuple(arguments),
                ),
            },
        )
        t.check(
            "user_interface_unknown_00149318",
            externals={
                "user_interface_unknown_001492d6": lambda harness, arguments, which=random_.choice((0x3A, 0x23, 0x38, 0, 0)): (
                    arguments[2] == which,
                    tuple(arguments),
                ),
            },
        )
