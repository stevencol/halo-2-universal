# source/networking/simulation_player.c: the part of what a player does that names an object

import random
import struct

from difftest import Buffer

REALS = (0.0, 0.0, -0.0, 1.0, 0.25, -1.0, float("nan"))


def run(t) -> None:
    random_ = random.Random(591)

    for n in range(4000):
        number = lambda harness, arguments, value=random_.choice((-1, 9, 9, 0x4321)): (value & 0xFFFFFFFF, (arguments[0],))
        packed = struct.pack(
            "<ffiI",
            random_.choice(REALS),
            random_.choice(REALS),
            random_.choice((-1, 5, 5, 0x1234)),
            random_.getrandbits(32),
        )
        t.check(
            "simulation_player_action_unpack_object",
            Buffer(packed),
            Buffer(random_.randbytes(0x24)),
            externals={"simulation_gamestate_entity_get_object": number},
        )

        unknown34 = bytearray(random_.randbytes(0x24))
        struct.pack_into("<i", unknown34, 0, random_.choice((-1, 5, 5, 0x1234)))
        struct.pack_into("<f", unknown34, 0x1C, random_.choice(REALS))
        struct.pack_into("<f", unknown34, 0x20, random_.choice(REALS))
        world = bytearray(random_.randbytes(0x40))
        struct.pack_into("<i", world, 8, random_.choice((1, 2, 3, 4, 5)))
        t.check(
            "simulation_player_action_pack_object",
            Buffer(bytes(unknown34)),
            Buffer(random_.randbytes(0x10)),
            globals={"unknown_004cf770": random_.randbytes(12) + bytes(4), "unknown_004cf770+0xc": Buffer(bytes(world))},
            externals={
                "object_try_and_get_and_verify_type": lambda harness, arguments, there=random_.choice((0, 0x1234, 0x1234)): (
                    there,
                    (arguments[0], arguments[1]),
                ),
                "simulation_object_get_created_entity": number,
                "simulation_object_get_remote_entity": number,
            },
        )
