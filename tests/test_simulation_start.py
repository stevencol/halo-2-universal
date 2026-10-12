# source/networking/simulation.c: the simulation starts and ends, with the game and with each map

import random
import struct

from difftest import Buffer, GlobalData, GlobalRange


def run(t) -> None:
    random_ = random.Random(575)

    def record(*names):
        return lambda harness, arguments: (0, tuple(harness.normalize(argument) for argument in arguments))

    for n in range(300):
        def fills(harness, arguments):
            harness.mu.mem_write(arguments[2], struct.pack("<i", 0x11))
            harness.mu.mem_write(arguments[1], struct.pack("<i", 0x1A))
            tables = bytes(harness.mu.mem_read(arguments[0], 0x108))
            return 0, (harness.normalize(arguments[0]), tables)

        there = {
            "unknown_004cf770": GlobalData(bytes((random_.choice((0, 1)),)) + random_.randbytes(15), places=(0xC,)),
            "unknown_004cf780": GlobalData(random_.randbytes(4), places=(0,)),
            "unknown_004cf784": GlobalData(random_.randbytes(4), places=(0,)),
        }
        t.check(
            "simulation_initialize",
            globals={
                **there,
                "unknown_00544b20": GlobalRange(0x1220),
                "unknown_00545d40": GlobalRange(0xC34),
                "unknown_00546974": GlobalData(random_.randbytes(0x108)),
            },
            externals={"unknown_00082240": fills},
        )
        t.check("simulation_dispose", globals=there)

        off = bytes((random_.choice((0, 0, 0, 1)),))
        game = bytearray(random_.randbytes(0x20))
        game[0xC] = random_.choice((0, 1, 3, 4, 5, 6, 0x84, 0xFF))
        world = random_.getrandbits(32)

        def what(harness, arguments):
            return 0, (arguments[0], bytes(harness.mu.mem_read(arguments[0], 0x40)))

        t.check(
            "simulation_initialize_for_new_map",
            globals={
                "unknown_004ed39c": off,
                "unknown_004cf770": random_.randbytes(12) + struct.pack("<I", world),
                "unknown_004cf780": Buffer(random_.randbytes(0x40)),
                "unknown_004cf784": random_.getrandbits(32),
                "unknown_004d87e8": random_.getrandbits(32),
                "main_game_globals": Buffer(bytes(game)),
            },
            externals={
                "simulation_world_initialize": what,
                "simulation_players_reset": what,
                "simulation_players_initialize_for_new_map": what,
                "unknown_00195a40": lambda harness, arguments: (0, ()),
            },
        )

        has = bytearray(random_.randbytes(0x100))
        given = random_.choice((False, True, True))
        struct.pack_into("<I", has, 8, 0x40 if given else 0)
        players = Buffer(bytes(has), pointers=(8,) if given else ())

        def ends(harness, arguments, players=players):
            return 0, (arguments[0], bytes(harness.mu.mem_read(players.address, 0x100)))

        t.check(
            "simulation_dispose_from_old_map",
            globals={
                "unknown_004ed39c": off,
                "unknown_004cf770": random_.randbytes(12) + struct.pack("<I", world),
                "unknown_004cf780": players,
            },
            externals={
                "simulation_world_dispose": ends,
                "unknown_00195e90": lambda harness, arguments: (0, ()),
            },
        )
