# source/networking/simulation_view.c

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(447)

    for n in range(2500):
        now = random_.choice((5000, 100000, 0xFFFFFF00, random_.getrandbits(32)))
        view = bytearray(random_.randbytes(0x90))
        struct.pack_into("<i", view, 0x28, random_.choice((0, 0, 0, 1, 4, 7)))
        view[0x88] = random_.choice((0, 1, 1, 0x80))
        struct.pack_into("<I", view, 0x8C, (now - random_.choice((0, 1, 1999, 2000, 2001, 60000, -5))) & 0xFFFFFFFF)
        kept = random_.choice((0, 1, 0x80))
        t.check(
            "simulation_view_set_blocked",
            Buffer(bytes(view)),
            random_.choice((0, 1, 1, 1, 0x100)),
            globals={"unknown_00510548": bytes((kept,)), "unknown_0051054c": now if kept else random_.getrandbits(32)},
            externals={
                "GetTickCount": lambda harness, arguments, now=now: now,
                "simulation_view_set_state": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2])),
            },
        )

    # ---------- a view goes to a state

    for n in range(3000):
        view = bytearray(random_.randbytes(0x90))
        at = random_.choice((0, 1, 2, 3, 4, 5, 6))
        with_ = random_.choice((-1, 0, 5, random_.getrandbits(31)))
        struct.pack_into("<i", view, 0x28, random_.choice((0, 0, 0, 1, 7)))
        struct.pack_into("<2i", view, 0x2C, at, with_)
        struct.pack_into("<Ii", view, 0x20, 0x12345678, random_.choice((-1, 0, 3, 3)))
        t.check(
            "simulation_view_set_state",
            Buffer(bytes(view)),
            random_.choice((0, 1, 2, 3, at, at + 1, at + 1, at + 2, 7, 0xFFFFFFFF)),
            random_.choice((0xFFFFFFFF, 0xFFFFFFFF, 0, 5, with_ & 0xFFFFFFFF, with_ & 0xFFFFFFFF, 0x80000000, random_.getrandbits(32))),
            externals={
                "network_observer_channel_send": lambda harness, arguments: (
                    0,
                    (arguments[0] & 0xFF, arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], bytes(harness.mu.mem_read(arguments[6], 8))),
                ),
                "unknown_000861c0": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
        )

    # ---------- a view that cannot go on, what the players of the other console do, and the state of the game

    VIEW, WORLD = 0x90, 0x1220

    def a_view() -> Buffer:
        """A view, and after it the thing that it is of."""
        data = bytearray(random_.randbytes(VIEW + WORLD))
        struct.pack_into("<I", data, 0xC, VIEW)
        struct.pack_into("<i", data, 0x28, random_.choice((0, 0, 0, 1, 7)))
        struct.pack_into("<2i", data, 0x80, random_.choice((0, 5, 100)), random_.choice((0, 5, 100)))
        struct.pack_into("<i", data, VIEW + 0x8, random_.choice((0, 1, 2, 3, 3, 3, 3, 4, 5)))
        struct.pack_into("<i", data, VIEW + 0x18, random_.choice((0, 4, 4, 4)))
        struct.pack_into("<i", data, VIEW + 0x28, random_.choice((0, 6, 101, 200)))
        struct.pack_into("<i", data, VIEW + 0x1200, random_.choice((-1, 0, 0, 100)))
        return Buffer(bytes(data), pointers=(0xC,))

    state = lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2]))

    for n in range(2500):
        t.check("simulation_view_fail", a_view(), random_.choice((2, 4, 5, 7)), externals={"simulation_view_set_state": state})
        t.check(
            "simulation_view_unknown_00085f50",
            a_view(),
            random_.choice((0, 0, 0, 0, 1, 0x80)),
            random_.choice((0, 4, 5, 6, 99, 100, 101, 199, 200, 0xFFFFFFFF)),
            random_.choice((0, 4, 5, 6, 99, 100, 101, 0xFFFFFFFF)),
            random_.getrandbits(4),
            Buffer(random_.randbytes(4 * 0x5C)),
            externals={
                "simulation_view_set_state": state,
                "simulation_world_set_player_actions": lambda harness, arguments: (
                    0,
                    (harness.normalize(arguments[0]), harness.normalize(arguments[1]), arguments[2], harness.normalize(arguments[3])),
                ),
            },
        )
        could = random_.choice((0, 1, 1, 0x100))
        t.check(
            "simulation_view_unknown_00085ed0",
            a_view(),
            random_.choice((0, 100, 0x3FE000)),
            random_.choice((0, 0, 1, 100, 0xFFFFFFFF, 0x80000000)),
            Buffer(random_.randbytes(0x80)),
            externals={
                "simulation_view_set_state": state,
                "simulation_world_receive_game_state": lambda harness, arguments, could=could: (
                    could,
                    (harness.normalize(arguments[0]), arguments[1], arguments[2], harness.normalize(arguments[3])),
                ),
                "simulation_world_finish_game_state": lambda harness, arguments, could=could: (could, (harness.normalize(arguments[0]), arguments[1])),
            },
        )

    # ---------- the number that the game goes on from

    for n in range(2500):
        view = a_view()
        data = bytearray(view.data)
        struct.pack_into("<i", data, VIEW + 0x18, random_.choice((2, 3, 3, 3, 3, 4)))
        data[VIEW + 0x24] = random_.choice((0, 1))
        could = random_.choice((0, 1, 1, 0x100))
        t.check(
            "simulation_view_unknown_00085dc0",
            Buffer(bytes(data), pointers=view.pointers),
            random_.choice((0, 5, 1000, 0x80000000, random_.getrandbits(32))),
            externals={
                "simulation_view_set_state": state,
                "unknown_0006a8a0": lambda harness, arguments, could=could: (could, (harness.normalize(arguments[0]),)),
                "simulation_world_clear_updates": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "simulation_world_unknown_00069350": lambda harness, arguments: (0, (arguments[0] & 0xFF, harness.normalize(arguments[1]))),
            },
        )
