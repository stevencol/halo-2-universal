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
                "simulation_player_actions_take": gives,
                "simulation_player_action_unpack": unpacks,
                "player_action_valid": lambda harness, arguments, good=good: (good, (bytes(harness.mu.mem_read(arguments[0], 0x5C)),)),
            },
        )

    # ---------- whether what a player does is good

    t.check("player_action_valid", 0)
    reals = (0.0, 0.0, 0.0, 1.0, -1.0, 0.5, -0.5, 1.0000001, -1.0000001, 2.0, 1e30, float("inf"), float("-inf"), float("nan"), -0.0, 1e-40)
    for n in range(20000):
        action = bytearray(0x5C)
        struct.pack_into("<6f", action, 0x4, *(random_.choice((0.0, 0.5, 1.0, -0.25)) for value in range(2)), random_.choice((0.0, 0.5, -1.0, 1.0)), random_.choice((0.0, 0.5, -1.0, 1.0)), random_.choice((0.0, 0.5, 1.0)), 0.25)
        struct.pack_into("<2b2h", action, 0x20, random_.choice((-1, 0, 3)), random_.choice((-1, 1, 2)), random_.choice((-1, 0, 1)), random_.choice((-1, 0, 3)))
        struct.pack_into("<h2xii", action, 0x28, random_.choice((0, 0, 1, 8)), random_.choice((5, 5, -1)), 7)
        struct.pack_into("<i", action, 0x34, random_.choice((-1, 3, 3)))
        struct.pack_into("<H2x2f", action, 0x4C, random_.choice((0, 0, 1, 2)), random_.choice((0.0, 0.0, 0.5, 1.0)), random_.choice((0.0, 0.0, 0.5)))
        action[0x1D] = random_.choice((0, 0, 1))
        # and then something of it is another
        for change in range(random_.choice((0, 0, 1, 1, 2))):
            what = random_.randrange(8)
            if what == 0:
                struct.pack_into("<f", action, random_.choice((0x4, 0x8, 0xC, 0x10, 0x14, 0x18, 0x50, 0x54)), random_.choice(reals))
            elif what == 1:
                struct.pack_into("<b", action, random_.choice((0x20, 0x21)), random_.choice((-1, -2, 0, 1, 3, 4, 127, -128)))
            elif what == 2:
                struct.pack_into("<h", action, random_.choice((0x22, 0x24, 0x28)), random_.choice((-1, -2, 0, 1, 2, 3, 4, 8, 9, 0x7FFF)))
            elif what == 3:
                action[0x1D] = random_.getrandbits(8)
            elif what == 4:
                struct.pack_into("<i", action, random_.choice((0x2C, 0x34)), random_.choice((-1, 0, 5)))
            elif what == 5:
                action[0x4C] = random_.getrandbits(8)
            else:
                action[random_.randrange(0x5C)] = random_.getrandbits(8)
        t.check("player_action_valid", Buffer(bytes(action)))
