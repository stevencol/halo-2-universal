# six small functions of the game engine

import random
import struct

from difftest import Buffer, Mock, mock_address

from test_object_types_more import after

KINDS = 8
NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(705)

    for n in range(200):
        globals_ = bytearray(random_.randbytes(0xC9C))
        kind = random_.randrange(KINDS)
        struct.pack_into("<I", globals_, 0xC14, kind)
        engine_globals = Buffer(bytes(globals_))
        game = bytearray(random_.randbytes(0x1200))
        game[0xC] = random_.choice((0, 1, 4, 4, 5))
        game_globals = Buffer(bytes(game))

        # what runs the kind of game: where its functions are, and after it the 36 of them, of which two are mocks
        engine = bytearray(random_.randbytes(8 + 36 * 4))
        struct.pack_into("<I", engine, 0, 8)
        struct.pack_into("<I", engine, 8 + 14 * 4, mock_address(0))
        struct.pack_into("<I", engine, 8 + 35 * 4, mock_address(1))
        kinds = bytearray(KINDS * 4)
        if random_.random() < 0.7:
            # the buffers of the globals come first: that of the engine is after the two
            struct.pack_into("<I", kinds, kind * 4, after(engine_globals, game_globals))
        answer = random_.choice((0, 1))
        options = {
            "globals": {
                "unknown_004e9ae8": engine_globals,
                "main_game_globals": game_globals,
                "unknown_0055e4d0": bytes(kinds),
            },
            "extra": [Buffer(bytes(engine), pointers=(0,))],
            "mocks": [
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call, answer=answer: answer, stack_arguments=2, registers=("ecx",)),
            ],
            "externals": {"unknown_001e97b0": lambda harness, arguments: 0},
        }
        t.check("game_engine_running", **options)
        t.check("game_engine_unknown_0015dd10", **options)
        t.check("game_engine_unknown_0015ea50", random_.choice((NONE, random_.getrandbits(32))), **options)
        t.check("game_engine_unknown_0015eaf0", **options)
        t.check("game_engine_unknown_0015fe50", random_.getrandbits(32), **options)
        t.check("game_engine_unknown_0015ff50", random_.getrandbits(32), random_.getrandbits(32), **options)
