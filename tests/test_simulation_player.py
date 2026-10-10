# source/networking/simulation_player.c, source/game/player_action.c

import random
import struct

from difftest import Buffer

from test_simulation_world import a_world


def run(t) -> None:
    random_ = random.Random(481)

    for n in range(1500):
        t.check("player_action_clear", Buffer(random_.randbytes(0x5C + 4), format="<92B4B"))
        world = bytearray(random_.randbytes(0x40))
        struct.pack_into("<i", world, 0x8, random_.choice((1, 2, 3, 3, 4, 5, 5)))
        t.check(
            "simulation_player_new",
            Buffer(random_.randbytes(0x88 + 4)),
            Buffer(random_.randbytes(12)),
            Buffer(random_.randbytes(6)),
            Buffer(bytes(world)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
        )

    # ---------- what a player of the thing does, for the game

    for n in range(2500):
        thing = a_world(random_, 0)
        world = bytearray(thing.data)
        struct.pack_into("<i", world, 0x8, random_.choice((1, 2, 2, 3, 4, 5)))
        player = bytearray(random_.randbytes(0x88))
        struct.pack_into("<i", player, 0x8, random_.choice((0, 1, 2, 3, 4, 4, 5, 5, 6)))
        struct.pack_into("<I", player, 0x20, 0x88)
        player[0x18:0x1E] = bytes((random_.choice((1, 2)),)) * 5 + bytes((random_.choice((1, 2, 3)),))
        now = random_.choice((1000, 1000, 5))
        last = random_.choice((-1, now, now - 14, now - 15, now - 16, now - 100, now + 3))
        struct.pack_into("<i", player, 0x28, last)
        time = bytearray(random_.randbytes(0x10))
        struct.pack_into("<fI", time, 4, 1 / 30, now)
        has = random_.choice((0, 1, 1, 0x100))
        good = random_.choice((0, 1, 1, 1))
        packed = random_.randbytes(0x34)

        def gives(harness, arguments, has=has, packed=packed):
            harness.mu.mem_write(arguments[2], packed)
            return has, (arguments[0], arguments[1])

        def unpacks(harness, arguments):
            read = bytes(harness.mu.mem_read(arguments[0], 0x34))
            harness.mu.mem_write(arguments[1], (read * 2)[:0x5C])
            return 0, (read,)

        t.check(
            "simulation_player_get_action",
            Buffer(bytes(player) + bytes(world), pointers=(0x20,) + tuple(0x88 + pointer for pointer in thing.pointers)),
            Buffer(random_.randbytes(0x5C + 4)),
            globals={"game_time_globals": Buffer(bytes(time)), "unknown_004ced44": struct.pack("<f", 0.5)},
            externals={
                "unknown_0008b7f0": gives,
                "unknown_000825e0": unpacks,
                "unknown_0014d0a0": lambda harness, arguments, good=good: (good, (bytes(harness.mu.mem_read(arguments[0], 0x5C)),)),
            },
        )
