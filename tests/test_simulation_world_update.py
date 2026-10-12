# source/networking/simulation_world.c: the thing that the simulation has going does its work

import random
import struct

from difftest import Buffer

from test_simulation_world import a_world

STEPS = (
    "simulation_world_unknown_00069c50",
    "simulation_world_unknown_00069c80",
    "simulation_world_unknown_00069d50",
    "simulation_world_unknown_00069dd0",
    "simulation_world_update_players_unknown25",
    "simulation_world_unknown_00069eb0",
    "simulation_world_unknown_00069f10",
    "simulation_world_unknown_00069f90",
    "simulation_world_unknown_00069fe0",
    "simulation_world_send_players",
)


def run(t) -> None:
    random_ = random.Random(578)

    for n in range(3000):
        thing = a_world(random_, 0)
        world = bytearray(thing.data)
        struct.pack_into("<i", world, 0x8, random_.choice((0, 1, 2, 3, 4, 5)))
        struct.pack_into("<i", world, 0x18, random_.choice((0, 1, 2, 3, 4, 5, 6, 7)))

        def step(harness, arguments, after=None):
            before = struct.unpack("<i", harness.mu.mem_read(arguments[0] + 0x18, 4))[0]
            if after is not None:
                harness.mu.mem_write(arguments[0] + 0x18, struct.pack("<i", after))
            return 0, (harness.normalize(arguments[0]), before, *arguments[1:])

        externals = {
            name: (lambda harness, arguments, after=random_.choice((None, None, 1, 2, 3, 4, 5, 6)): step(harness, arguments, after))
            for name in STEPS
        }
        externals["simulation_view_update"] = lambda harness, arguments: (0, (harness.normalize(arguments[0]),))
        t.check(
            "simulation_world_update",
            Buffer(bytes(world), pointers=thing.pointers),
            externals=externals,
        )
