# source/networking/simulation_players.c: the views of the simulation are kept up

import random
import struct

from difftest import SCRATCH_ADDRESS, Buffer, GlobalRange

from test_simulation_world import a_world

PLAYERS = 0xC34
SESSION = 0x7620
TAIL = 0x50A0
TABLES = {name: GlobalRange(4) for name in ("unknown_00450c94", "unknown_00450cd0", "unknown_00450cf4", "unknown_00450d1c")}


def note(harness, arguments):
    return 0, tuple(harness.normalize(argument) for argument in arguments)


def run(t) -> None:
    random_ = random.Random(583)

    for n in range(1200):
        # the players of the simulation, the thing that is going with its views, a session, and what a view has at 0x8
        thing = a_world(random_, 0, size=0x80)
        world = bytearray(thing.data)
        world_size = len(world)
        session_at = PLAYERS + world_size
        tail_at = session_at + SESSION
        struct.pack_into("<i", world, 0x18, random_.choice((1, 2, 3, 4)))
        for offset in thing.pointers:
            struct.pack_into("<I", world, offset, struct.unpack_from("<I", world, offset)[0] + PLAYERS)
        pointers = [4, 0xC] + [PLAYERS + offset for offset in thing.pointers]
        places = []
        for at in thing.pointers[1::2]:
            view = at - 0xC
            struct.pack_into("<i", world, view + 0x28, random_.choice((0, 0, 4)))
            struct.pack_into("<i", world, view + 0x1C, random_.randrange(16))
            struct.pack_into("<i", world, view + 0x24, random_.choice((-1, 2)))
            if random_.choice((False, True)):
                struct.pack_into("<I", world, view + 8, tail_at)
                pointers.append(PLAYERS + view + 8)
                places = [tail_at + offset for offset in (0x4, 0x30, 0x5070, 0x5098)]
            else:
                struct.pack_into("<I", world, view + 8, 0)
        session = bytearray(random_.randbytes(SESSION))
        count = random_.choice((0, 1, 3, 16))
        struct.pack_into("<i", session, 0x54, count)
        struct.pack_into("<i", session, 0x72D8, random_.randrange(max(count, 1)))
        for index in range(16):
            session[0x72DC + index * 0x14 + 1] = random_.choice((0, 1, 1, 0x80))
            struct.pack_into("<i", session, 0x72DC + index * 0x14 + 4, random_.randrange(4))
        players = bytearray(random_.randbytes(PLAYERS))
        struct.pack_into("<I", players, 4, PLAYERS)
        struct.pack_into("<I", players, 0xC, session_at)
        t.check(
            "simulation_players_update_views_host",
            Buffer(
                bytes(players) + bytes(world) + bytes(session) + random_.randbytes(TAIL),
                pointers=tuple(pointers),
                places=tuple(places),
            ),
            random_.getrandbits(32),
            globals={"unknown_004d87ec": 0x1111, "unknown_004d87f0": 0x2222, **TABLES},
            externals={
                "simulation_view_release_channel": note,
                "simulation_view_detach": note,
                "datum_delete": note,
                "simulation_players_create_view": lambda harness, arguments: (1, tuple(harness.normalize(argument) for argument in arguments)),
            },
        )

        # what chooses between the two
        has_world = random_.choice((0, 1, 1, 1, 1, 1, 1, 1))
        has_session = random_.choice((0, 1, 1, 1, 1, 1, 1, 1))
        data = bytearray(random_.randbytes(PLAYERS + 0x40 + SESSION))
        world_at, session_at = PLAYERS, PLAYERS + 0x40
        struct.pack_into("<I", data, 4, world_at if has_world else 0)
        struct.pack_into("<I", data, 0xC, session_at if has_session else 0)
        struct.pack_into("<i", data, world_at + 8, random_.choice((1, 2, 3, 4, 5, 5, 5)))
        struct.pack_into("<i", data, session_at + 0x741C, random_.choice((0, 3, 5, 5, 5, 8)))

        def flag(*values):
            value = random_.choice(values)
            return lambda harness, arguments, value=value: (value, tuple(harness.normalize(argument) for argument in arguments))

        def membership(harness, arguments, local=random_.getrandbits(32), host=random_.getrandbits(32)):
            harness.mu.mem_write(arguments[0], struct.pack("<I", local))
            harness.mu.mem_write(arguments[2], struct.pack("<I", host))
            return SCRATCH_ADDRESS + session_at + 0x4C, (harness.normalize(arguments[1]),)

        t.check(
            "simulation_players_update_views",
            Buffer(bytes(data), pointers=tuple(([4] if has_world else []) + ([0xC] if has_session else []))),
            globals={
                "unknown_00527330": bytes((random_.choice((0, 1, 1, 1, 1, 0x80)),)),
                "unknown_00527334": random_.choice((3, 3, 8, 8, 8, 0, 4)),
            },
            externals={
                "network_session_unknown_00058d20": flag(0, 1, 0x100),
                "network_session_established": flag(0, 1, 1, 1, 0x100),
                "simulation_world_unknown_0006b040": note,
                "simulation_unknown_00067e10": flag(0, 1),
                "unknown_0006ae20": flag(0, 1, 1, 1, 0x100),
                "unknown_0006aee0": flag(0, 1, 1, 1, 0x100),
                "network_session_unknown_00058d90": flag(0, 1, 0x100),
                "simulation_world_unknown_0006b310": note,
                "simulation_world_unknown_0006b350": note,
                "network_session_get_membership": membership,
                "unknown_00083f00": flag(0, 1, 1, 1, 0x100),
                # its second argument is one that it does not look at: source/ does not pass it
                "simulation_players_update_views_host": lambda harness, arguments, value=random_.choice((0, 1, 0x100)): (
                    value,
                    (harness.normalize(arguments[0]),),
                ),
                "simulation_players_update_views_client": flag(0, 1, 0x100),
            },
        )
