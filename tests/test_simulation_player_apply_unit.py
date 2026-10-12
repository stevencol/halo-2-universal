# source/networking/simulation_player.c: where a unit is, taken from a message

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(594)

    def note(harness, arguments):
        return 0x5A5A, tuple(harness.normalize(argument) for argument in arguments)

    for n in range(4000):
        table = some_objects(random_)
        data = bytearray(table.data)
        mover = random_.choice((7, 7, 0x1234))
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<I", data, at + 0x13C, random_.choice((7, 7, 9)))
            data[at + 0x10A] = random_.choice((0, 0, 4, 0xFB, 0xFF))
            rider = random_.choice((None, None, random_.randrange(COUNT)))
            struct.pack_into("<I", data, at + 0x248, 0xFFFFFFFF if rider is None else (random_.getrandbits(16) << 16 | rider))
            if random_.choice((False, True)):
                struct.pack_into("<I", data, at + 0x14, 0xFFFFFFFF)
        unit = random_.choice((0xFFFFFFFF, random_.getrandbits(16) << 16 | random_.randrange(COUNT), random_.getrandbits(16) << 16 | random_.randrange(COUNT)))
        t.check(
            "simulation_player_apply_unit_state",
            Buffer(random_.randbytes(0x40)),
            mover,
            globals={"object_header_data": Buffer(bytes(data), pointers=table.pointers)},
            externals={
                "simulation_gamestate_entity_get_object": lambda harness, arguments, unit=unit: (unit, (arguments[0],)),
                "unknown_000a9f70": note,
                "unknown_000aa260": note,
            },
        )
