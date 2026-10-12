# source/networking: a view of another console is made, and the console that is not the host looks for the host's

import random
import struct

from difftest import Buffer

from test_simulation_world import a_world

PLAYERS = 0xC34
WORLD = 0x40
OBSERVER = 0xA8 + 4 * 0x528
SESSION = 0x7620
CHANNELS = 8 * 0xF8


def note(harness, arguments):
    return 0, tuple(harness.normalize(argument) for argument in arguments)


def run(t) -> None:
    random_ = random.Random(582)

    for n in range(1200):
        world = bytearray(random_.randbytes(0x40))
        struct.pack_into("<i", world, 8, random_.choice((0, 1, 2, 3, 4, 5, 6)))
        t.check("simulation_unknown_00067e10", Buffer(bytes(world)))

        # the players of the simulation, the thing that is going, the observer, a session
        world_at, observer_at, session_at = PLAYERS, PLAYERS + WORLD, PLAYERS + WORLD + OBSERVER
        data = bytearray(random_.randbytes(session_at + SESSION))
        struct.pack_into("<I", data, 4, world_at)
        struct.pack_into("<I", data, 8, observer_at)
        struct.pack_into("<I", data, 0xC, session_at)
        data[world_at + 0x2F] = random_.choice((0, 1, 1, 1, 1, 0x80))
        struct.pack_into("<i", data, world_at + 8, random_.choice((2, 3, 4, 5)))
        channel_index = random_.choice((-1, 0, 1, 2, 3, 3))
        for index in range(4):
            at = observer_at + 0xA8 + index * 0x528
            struct.pack_into("<i", data, at, random_.choice((7, 7, 7, 7, 6, 0)))
            struct.pack_into("<i", data, at + 0xC, random_.randrange(8))
        channels = bytearray(random_.randbytes(CHANNELS))
        for index in range(8):
            struct.pack_into("<i", channels, index * 0xF8 + 0x54, random_.choice((5, 5, 5, 5, 4, 0)))
        view = random_.choice((0, 0x7777, 0x7777, 0x7777))
        externals = {
            "network_session_has_channel_peer": lambda harness, arguments, value=random_.choice((0, 1, 1, 1, 1, 0x100)): (
                value,
                (harness.normalize(arguments[0]), arguments[1]),
            ),
            "simulation_view_create": lambda harness, arguments: (
                0x5555,
                (arguments[0], bytes(harness.mu.mem_read(arguments[1], 6)), arguments[2], arguments[3]),
            ),
            "simulation_unknown_00067e10": lambda harness, arguments, value=random_.choice((0, 1)): (value, (harness.normalize(arguments[0]),)),
            "simulation_world_get_view_by_unknown14": lambda harness, arguments, view=view: (
                view,
                (harness.normalize(arguments[0]), bytes(harness.mu.mem_read(arguments[1], 6))),
            ),
            "simulation_world_unknown_0006acb0": lambda harness, arguments, view=view: (view, (harness.normalize(arguments[0]),)),
            "unknown_00085650": note,
        }
        t.check(
            "simulation_players_create_view",
            channel_index,
            Buffer(bytes(data), pointers=(4, 8, 0xC)),
            random_.randrange(16),
            Buffer(random_.randbytes(0x20)),
            globals={"unknown_004d87d4": Buffer(bytes(channels))},
            externals=externals,
        )

        # the console that does not rule the game
        thing = a_world(random_, 0, size=0x80)
        world = bytearray(thing.data)
        struct.pack_into("<i", world, 0x18, random_.choice((1, 2, 3, 4)))
        for at in thing.pointers[1::2]:
            struct.pack_into("<i", world, at - 0xC + 0x28, random_.choice((0, 0, 0, 4)))
        # the thing comes after the players in the buffer: what points into it moves with it
        for offset in thing.pointers:
            struct.pack_into("<I", world, offset, struct.unpack_from("<I", world, offset)[0] + PLAYERS)
        world_size = len(world)
        session = bytearray(random_.randbytes(SESSION))
        count = random_.choice((1, 2, 16))
        struct.pack_into("<i", session, 0x54, count)
        host = random_.choice((-1, 0, 0, count - 1, count))
        struct.pack_into("<i", session, 0x40, host)
        for index in range(16):
            session[0x72DC + index * 0x14 + 1] = random_.choice((0, 1, 1, 0x80))
            struct.pack_into("<i", session, 0x72DC + index * 0x14 + 4, random_.randrange(4))
        players = bytearray(random_.randbytes(PLAYERS))
        struct.pack_into("<I", players, 4, PLAYERS)
        struct.pack_into("<I", players, 0xC, PLAYERS + world_size)
        pointers = [4, 0xC] + [PLAYERS + offset for offset in thing.pointers]
        t.check(
            "simulation_players_update_views_client",
            Buffer(bytes(players) + bytes(world) + bytes(session), pointers=tuple(pointers)),
            externals={
                "simulation_world_unknown_0006b040": note,
                "simulation_players_create_view": lambda harness, arguments: (1, tuple(harness.normalize(argument) for argument in arguments)),
            },
        )
