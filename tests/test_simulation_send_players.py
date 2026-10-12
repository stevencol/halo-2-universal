# source/networking: the players are told to the view of the other console, and a view's events are sent

import random
import struct

from difftest import Buffer

from test_simulation_world import a_world

PLAYERS = 0xC34
VIEW = 0x90
EVENTS = 0x4CD0


def run(t) -> None:
    random_ = random.Random(579)

    for n in range(1500):
        players = bytearray(random_.randbytes(PLAYERS))
        struct.pack_into("<I", players, 0x88, random_.choice((0, 1, 0x8001, 0xFFFF, random_.getrandbits(16), random_.getrandbits(32))))
        for index in range(16):
            players[0x8C + index * 0xB4 + 0xC] = random_.choice((0, 0, 1))
        players[0xC30] = random_.choice((0, 0, 1, 0x80))
        t.check(
            "simulation_players_take_update",
            Buffer(random_.randbytes(0xC0)),
            Buffer(random_.randbytes(0x60)),
            random_.choice((0, 0, 1, 0x100, 0xFFFFFF00)),
            Buffer(bytes(players)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
        )

        thing = a_world(random_, 0)
        world = bytearray(thing.data)
        struct.pack_into("<I", world, 0, 0x4321)
        for at in thing.pointers[1::2]:
            struct.pack_into("<i", world, at - 0xC + 0x24, random_.choice((-1, 2, 2, 5)))
            struct.pack_into("<I", world, at - 0xC + 0x20, 0x8765)
            struct.pack_into("<i", world, at - 0xC + 0x3C, random_.choice((-1, 0, 0, 3)))
        message = random_.randbytes(0xC8)

        def takes(harness, arguments, message=message, there=random_.choice((0, 1, 1, 0x100))):
            if there & 0xFF:
                harness.mu.mem_write(arguments[5], message[:4])
                harness.mu.mem_write(arguments[6], message[4:8])
                harness.mu.mem_write(arguments[0], message[8:])
            return there, (arguments[0] - arguments[5], arguments[0] - arguments[6], arguments[2] & 0xFF, arguments[3])

        def sends(harness, arguments):
            return 0, (arguments[0] & 0xFF, *arguments[1:6], bytes(harness.mu.mem_read(arguments[6], arguments[5])))

        t.check(
            "simulation_world_send_players",
            Buffer(bytes(world), pointers=thing.pointers),
            random_.choice((0, 1, 0x80)),
            externals={"simulation_players_take_update": takes, "network_observer_channel_send": sends},
        )

        # the things that happen, of a view
        data = bytearray(random_.randbytes(EVENTS + VIEW))
        struct.pack_into("<I", data, 0, EVENTS)
        data[5] = random_.choice((0, 1, 1, 1, 0x80))
        data[0xC] = random_.choice((0, 0, 1))
        time = random_.choice((-1, -1, 1000, random_.getrandbits(31)))
        struct.pack_into("<i", data, 0x4CCC, time)
        struct.pack_into("<H", data, EVENTS + 2, random_.choice((1, 2, 3, 3, 3, 4, 4, 5, 0x103)))
        struct.pack_into("<i", data, EVENTS + 0x3C, random_.choice((-1, 0, 0, 0, 3)))
        data[EVENTS + 0x75] = random_.choice((0, 1, 1, 1, 0x80))
        struct.pack_into("<i", data, EVENTS + 0x24, random_.choice((-1, 2, 2, 5)))
        struct.pack_into("<I", data, EVENTS + 0x20, 0x8765)
        limit = random_.choice((0, 100, 5000))
        waiting = {
            "network_observer_channel_unknown_000768b0": lambda harness, arguments, value=random_.choice((0, 1, 0x100)): (value, tuple(arguments)),
        }
        t.check(
            "simulation_view_events_message_waiting",
            Buffer(bytes(data[EVENTS:])),
            externals=waiting,
        )
        t.check(
            "simulation_view_events_update",
            Buffer(bytes(data), pointers=(0,)),
            globals={
                "unknown_00510ca0": bytes((random_.choice((0, 1, 1, 1, 0x80)),)),
                "unknown_00510cb1": bytes((random_.choice((0, 1, 0x80)),)),
                "unknown_004ced40": limit,
            },
            externals={
                **waiting,
                "network_time_since": lambda harness, arguments, value=random_.choice((limit - 1, limit, limit + 1, 0, -5)): (value & 0xFFFFFFFF, tuple(arguments)),
                "unknown_00086c50": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "network_observer_channel_set_pending": lambda harness, arguments: (0, tuple(arguments)),
            },
        )
