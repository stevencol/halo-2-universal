# five small functions: a bit of main_game_globals, a point and a number,
# the language of the game, the folder of the fonts, and the point of a
# local player

import random
import struct

from difftest import Buffer, GlobalData

NONE = 0xFFFFFFFF
LOCAL_PLAYERS, LOCAL_PLAYER = 4, 0x358


def run(t) -> None:
    random_ = random.Random(780)

    for n in range(600):
        game = bytearray(random_.randbytes(0x1200))
        for index in range(16):
            struct.pack_into("<I", game, 0x1178 + index * 4, random_.choice((0, 0xFFFFFFFF, 1, 0x80000000, random_.getrandbits(32))))
        thing = bytearray(random_.randbytes(8))
        struct.pack_into("<h", thing, 4, random_.choice((0, 1, 31, 32, 33, 255, 511, random_.randrange(512))))
        t.check("game_unknown1178_test", Buffer(bytes(thing)), globals={"main_game_globals": Buffer(bytes(game))})

        t.check(
            "game_unknown_0011c7d0",
            random_.choice((0, Buffer(random_.randbytes(12)))),
            random_.choice((0, Buffer(random_.randbytes(4)))),
            Buffer(random_.randbytes(0x10)),
        )

        t.check(
            "get_current_language",
            globals={"global_language": random_.choice((NONE, NONE, 0, 1, 8))},
            externals={
                "XGetLanguage": lambda harness, arguments, said=random_.randrange(10): (said, ()),
                "language_from_xbox_language": lambda harness, arguments, said=random_.randrange(9): (said, tuple(arguments)),
            },
        )

        def printed(harness, arguments):
            # the folder of the fonts, as csnzprintf makes it
            text = b"d:\\maps\\fonts\\\0"
            harness.mu.mem_write(arguments[0], text)
            format_ = bytes(harness.mu.mem_read(arguments[2], 9))
            argument = bytes(harness.mu.mem_read(arguments[3], 9))
            # unknown_0055e718 points to a text that is not at the same
            # address in the game and in source/: the text is compared
            # here, and the global is not given to the call
            at = harness.globals["unknown_0055e718"] if harness._external_side == "original" else harness.variables["unknown_0055e718"]
            (pointer,) = struct.unpack("<I", harness.mu.mem_read(at, 4))
            return arguments[0], (arguments[1], format_, argument, bytes(harness.mu.mem_read(pointer, 9)))

        def added(harness, arguments):
            return 0, (bytes(harness.mu.mem_read(arguments[1], 15)),)

        t.check(
            "fonts_get_folder",
            Buffer(random_.randbytes(0x110 + 4)),
            externals={"csnzprintf": printed, "file_path_add_name": added},
        )

        t.check(
            "game_unknown_0016efb0_get_point",
            random_.choice((NONE, 0, 1, 2, 3)),
            globals={
                "unknown_004e9bd4": GlobalData(random_.randbytes(LOCAL_PLAYERS * LOCAL_PLAYER)),
                "global_structure_bsp_index": struct.pack("<h", random_.choice((-1, 0, 2))),
            },
        )
