# source/networking/simulation_players.c: what the simulation has of the players, for a new map and after a state is loaded

import random
import struct

from difftest import SCRATCH_ADDRESS, Buffer

PLAYERS = 0xC34
WORLD = 0x40
SESSION = 0x7620
OTHER = 0x90
GAME = 0x1124


def a_machine(random_) -> bytes:
    return bytes((random_.choice((1, 2)),)) * 5 + bytes((random_.choice((1, 2, 3)),))


def run(t) -> None:
    random_ = random.Random(580)

    for n in range(1500):
        # the players of the simulation, then the thing that is going, a session and what the game in the network has
        data = bytearray(random_.randbytes(PLAYERS + WORLD + SESSION + OTHER))
        world_at, session_at, other_at = PLAYERS, PLAYERS + WORLD, PLAYERS + WORLD + SESSION
        struct.pack_into("<I", data, 4, world_at)
        has_session = random_.choice((False, True))
        struct.pack_into("<I", data, 0xC, session_at if has_session else 0)
        struct.pack_into("<i", data, session_at + 0x741C, random_.choice((1, 3, 8)))
        struct.pack_into("<i", data, session_at + 0x4C, random_.choice((0, 7, 0x1234)))
        count = random_.choice((0, 1, 2, 5, 16))
        struct.pack_into("<i", data, session_at + 0x4C + 8, count)
        struct.pack_into("<i", data, session_at + 0x72D8, random_.randrange(max(count, 1)))
        game = bytearray(random_.randbytes(GAME))
        struct.pack_into("<i", game, 8, random_.choice((0, 1, 2, 3)))
        struct.pack_into("<I", game, 0x270, random_.choice((0, 1, 0x8001, 0xFFFF, random_.getrandbits(16))))
        for index in range(16):
            game[0x274 + index * 6 : 0x27A + index * 6] = a_machine(random_)
        game[0x2D4] = random_.choice((0, 1, 1, 0x80))
        game[0x2D5:0x2DB] = a_machine(random_)

        # the session that the game in the network gives: the one here, in some state or in none
        found = random_.choice((0, 0, 1, 1, 0x100))
        state = 3 if has_session else random_.choice((0, 3, 3))
        membership = 7 if has_session else random_.choice((-1, 7, 7))

        def session(harness, arguments, found=found, state=state, membership=membership):
            if found & 0xFF:
                harness.mu.mem_write(SCRATCH_ADDRESS + session_at + 0x741C, struct.pack("<i", state))
                harness.mu.mem_write(SCRATCH_ADDRESS + session_at + 0x4C, struct.pack("<i", membership))
                harness.mu.mem_write(arguments[0], struct.pack("<I", SCRATCH_ADDRESS + session_at))
            return found, ()

        t.check(
            "simulation_players_initialize_for_new_map",
            Buffer(bytes(data), pointers=(4, 0xC) if has_session else (4,)),
            globals={
                "main_game_globals": Buffer(bytes(game)),
                "unknown_004d8ba0": bytes((random_.choice((0, 1, 1, 0x80)),)),
                "unknown_00527330": bytes((1,)),
                "unknown_00527378": SCRATCH_ADDRESS + other_at,
            },
            externals={"network_life_cycle_get_any_session": session},
        )

        # after a state was loaded
        data = bytearray(random_.randbytes(PLAYERS + WORLD))
        struct.pack_into("<I", data, 4, PLAYERS)
        data[PLAYERS + 0xD : PLAYERS + 0x13] = a_machine(random_)
        globals_ = bytearray(random_.randbytes(0x98))
        struct.pack_into("<I", globals_, 0x2C, random_.choice((0, 1, 0x8001, 0xFFFF, random_.getrandbits(16))))
        for index in range(16):
            globals_[0x30 + index * 6 : 0x36 + index * 6] = a_machine(random_)
        globals_[0x90] = random_.choice((0, 1, 1, 0x80))

        def which(harness, arguments):
            return 0, (bytes(harness.mu.mem_read(arguments[0], 6)) if arguments[0] else None,)

        def all_(harness, arguments):
            return 0, (arguments[0], bytes(harness.mu.mem_read(arguments[1], 0x60)))

        def refresh(harness, arguments):
            return 0, (harness.normalize(arguments[0]), bytes(harness.mu.mem_read(arguments[0], PLAYERS + WORLD)))

        exists = globals_[0x90]
        t.check(
            "simulation_players_after_load",
            Buffer(bytes(data), pointers=(4,)),
            random_.choice((0, 1, 2, 4, 4, 5, 0xFFFFFFFF, 0xFFFFFFFB)),
            globals={"players_globals": Buffer(bytes(globals_))},
            externals={"unknown_0014c880": which, "unknown_0014c630": all_, "simulation_players_refresh": refresh},
        )
