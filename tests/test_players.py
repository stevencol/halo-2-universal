# source/game/players.c, and two short functions of cseries.c and simulation_player.c

import random
import struct

from difftest import Buffer, Mock, mock_address

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(538)

    for n in range(1500):
        players = bytearray(random_.randbytes(0x2C))
        for index in range(4):
            struct.pack_into("<I", players, 0xC + index * 4, random_.choice((NONE, NONE, 0, 0x10003, random_.getrandbits(32))))
        globals_ = {"players_globals": Buffer(bytes(players))}
        t.check("players_first_local_player", globals=globals_)
        t.check("players_next_local_player", random_.choice((NONE, 0, 1, 2, 3, 4, 5, 100)), globals=globals_)

        size = random_.choice((1, 4, 0x10, 0x44))
        count = random_.choice((0, 1, 2, 5, 8, NONE, 0xFFFFFFF0))
        # what makes an element takes it in a register; the calls are compared
        t.check(
            "array_construct_elements",
            Buffer(random_.randbytes(8 * 0x44)),
            size,
            count,
            mock_address(0),
            mocks=[Mock(lambda call: call.registers["ecx"], registers=("ecx",))],
        )

        actions = bytearray(random_.randbytes(0x18 + 16 * 0x34))
        struct.pack_into("<I", actions, 0x14, random_.choice((0, 0xFFFF, random_.getrandbits(16), random_.getrandbits(32))))
        t.check(
            "simulation_player_actions_take",
            random_.randrange(16),
            Buffer(bytes(actions)),
            Buffer(random_.randbytes(0x34)),
        )
