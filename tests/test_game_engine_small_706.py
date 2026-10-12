# seven more small functions of the game engine

import random
import struct

from difftest import Buffer, Mock, mock_address

from test_object_types_more import after

KINDS = 8
NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(706)

    for n in range(200):
        globals_ = bytearray(random_.randbytes(0xC9C))
        kind = random_.randrange(KINDS)
        struct.pack_into("<I", globals_, 0xC14, kind)
        engine_globals = Buffer(bytes(globals_))
        game = bytearray(random_.randbytes(0x1200))
        game_globals = Buffer(bytes(game))
        players = bytearray(random_.randbytes(0x130))
        for user in range(4):
            struct.pack_into("<I", players, 0xC + user * 4, random_.choice((NONE, random_.getrandbits(32))))
        players_globals = Buffer(bytes(players))

        engine = bytearray(random_.randbytes(8 + 36 * 4))
        struct.pack_into("<I", engine, 0, 8)
        struct.pack_into("<I", engine, 8 + 2 * 4, mock_address(0))
        struct.pack_into("<I", engine, 8 + 13 * 4, mock_address(1))
        kinds = bytearray(KINDS * 4)
        if random_.random() < 0.7:
            struct.pack_into("<I", kinds, kind * 4, after(engine_globals, game_globals, players_globals))
        options = {
            "globals": {
                "unknown_004e9ae8": engine_globals,
                "main_game_globals": game_globals,
                "players_globals": players_globals,
                "unknown_0055e4d0": bytes(kinds),
                "unknown_004b9ed8": random_.choice((NONE, 0, 1, 2, 3)),
            },
            "extra": [Buffer(bytes(engine), pointers=(0,))],
            "mocks": [
                Mock(lambda call: 0, stack_arguments=0, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
        }
        t.check("game_engine_dispose_from_old_map", **options)
        t.check("game_engine_unknown_00157a40", **options)
        t.check("game_engine_unknown_001587b0", random_.getrandbits(16) << 16 | random_.randrange(0x10), **options)
        t.check("game_engine_unknown_001595d0", **options)
        t.check("game_engine_unknown_00162030", **options)
        t.check("game_engine_unknown_00163040", random_.randrange(8), **options)
        t.check("game_engine_unknown_00163080", **options)
