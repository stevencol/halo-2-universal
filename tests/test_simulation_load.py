# source/networking/simulation.c: the simulation around a loaded state, and what it watches outside the ticks

import random
import struct

from difftest import Buffer

from test_simulation_world import a_world


def run(t) -> None:
    random_ = random.Random(576)

    def note(harness, arguments):
        game = struct.unpack("<I", harness.mu.mem_read(harness.global_addresses["main_game_globals"], 4))[0]
        kind = harness.mu.mem_read(game + 0xC, 1)[0] if game else None
        there = bytes(harness.mu.mem_read(harness.global_addresses["unknown_004cf770"], 12))
        return 0, (tuple(harness.normalize(argument) for argument in arguments), kind, there)

    for n in range(1200):
        there = bytearray(random_.randbytes(12))

        t.check(
            "simulation_before_load",
            random_.getrandbits(32),
            globals={"unknown_004cf770": bytes(there) + bytes(4)},
        )

        world = bytearray(random_.randbytes(0x40))
        struct.pack_into("<i", world, 8, random_.choice((0, 1, 1, 3)))
        has = bytearray(random_.randbytes(0x100))
        given = random_.choice((False, True))
        struct.pack_into("<I", has, 8, 0x40 if given else 0)
        game = bytearray(random_.randbytes(0x20))
        game[0xC] = random_.choice((0, 1, 1, 4, 5, 6, 0xFF))
        voice = bytearray(0xC18)
        struct.pack_into("<i", voice, 0xC14, random_.randrange(16))
        t.check(
            "simulation_after_load",
            random_.choice((0, 1, 2, 4, 4, 5, 0xFFFFFFFF, random_.getrandbits(32))),
            globals={
                "unknown_004ed39c": bytes((random_.choice((0, 0, 0, 1)),)),
                "unknown_004cf770": bytes(there) + bytes(4),
                "unknown_004cf770+0xc": Buffer(bytes(world)),
                "unknown_004cf780": Buffer(bytes(has), pointers=(8,) if given else ()),
                "unknown_004cf784": random_.getrandbits(32),
                "unknown_004d87e8": random_.getrandbits(32),
                "main_game_globals": Buffer(bytes(game)),
                "unknown_004e9ae8": Buffer(bytes(voice)),
                "unknown_0055e4d0": b"".join(struct.pack("<I", random_.choice((0, 1, 0x100))) for _ in range(16)),
            },
            externals={
                name: note
                for name in (
                    "network_life_cycle_end",
                    "simulation_world_unknown_0006b040",
                    "simulation_world_dispose",
                    "unknown_00195e90",
                    "unknown_00162060",
                    "unknown_00162420",
                    "unknown_000bb7f0",
                    "unknown_00183f10",
                    "simulation_world_initialize",
                    "simulation_players_reset",
                    "simulation_players_initialize_for_new_map",
                    "unknown_00195a40",
                    "simulation_world_players_clear",
                    "simulation_players_after_load",
                    "simulation_world_unknown_0006a770",
                )
            },
        )

        # outside the ticks
        thing = a_world(random_, 0)
        world = bytearray(thing.data)
        struct.pack_into("<i", world, 0x8, random_.choice((0, 1, 3, 3, 5)))
        struct.pack_into("<i", world, 0x18, random_.choice((1, 2, 3, 4)))
        world[0x2C] = random_.choice((0, 0, 1))
        thing = Buffer(bytes(world), pointers=thing.pointers)
        there = bytearray(random_.randbytes(0x10))
        there[0] = random_.choice((0, 1, 1, 1, 1, 1))
        there[1] = random_.choice((0, 0, 0, 1))
        there[2] = random_.choice((0, 0, 0, 0, 0, 1))
        there[9] = random_.choice((0, 0, 0, 1))
        struct.pack_into("<I", there, 0xC, 0)
        game = bytearray(random_.randbytes(0x1124))
        game[0x1120] = random_.choice((0, 1, 1, 1, 1, 1))
        kept = random_.choice((0, 1, 0x80))
        now = random_.getrandbits(32)

        def fails_after(harness, arguments, fails=random_.choice((False, False, False, True))):
            if fails:
                harness.mu.mem_write(harness.global_addresses["unknown_004cf770"] + 2, b"\x01")
            return 0, (tuple(harness.normalize(argument) for argument in arguments),)

        globals_ = {
            "unknown_004cf770": bytes(there),
            "unknown_004cf770+0xc": thing,
            "unknown_004cf780": 0x4321,
            "main_game_globals": random_.choice((0, 1, 1, 1, 1, 1, 1, 1)) and Buffer(bytes(game)),
            "unknown_00510548": bytes((kept,)),
            "unknown_0051054c": now if kept else random_.getrandbits(32),
        }
        externals = {
            "GetTickCount": lambda harness, arguments, now=now: now,
            "simulation_world_unknown_0006b040": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            "simulation_world_set_unknown18": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
            "simulation_world_restart": fails_after,
            "main_game_restart": lambda harness, arguments: (0, ()),
            "unknown_00083220": lambda harness, arguments, well=random_.choice((0, 1, 1, 1, 0x100)): (well, (arguments[0],)),
            "simulation_world_update": fails_after,
            "game_is_authoritative": lambda harness, arguments, value=random_.choice((0, 1, 1)): value,
            "game_finished_wait_over": lambda harness, arguments, value=random_.choice((0, 1, 1, 0x100)): value,
            "game_is_distributed": lambda harness, arguments, value=random_.choice((0, 0, 1)): value,
            "main_game_goto_main_menu": lambda harness, arguments: (0, (arguments[0],)),
        }
        t.check("simulation_idle", globals=globals_, externals=externals)
        t.check("simulation_unknown_000687b0", globals=globals_, externals=externals)
