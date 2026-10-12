# source/networking/simulation_player.c: where the unit of a player is, for a message

import hashlib
import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

DATUM = 0x21C


def run(t) -> None:
    random_ = random.Random(592)

    def fills(*places):
        def stand_in(harness, arguments):
            for place, argument in zip(places, arguments[1:]):
                harness.mu.mem_write(argument, hashlib.sha256(bytes((place,)) + struct.pack("<I", arguments[0])).digest()[:12])
            return 0, (arguments[0],)

        return stand_in

    for n in range(3000):
        table = some_objects(random_)
        data = bytearray(table.data)
        unit = random_.randrange(COUNT)
        unit_index = random_.choice((0xFFFFFFFF, random_.getrandbits(16) << 16 | unit, random_.getrandbits(16) << 16 | unit))
        if unit_index != 0xFFFFFFFF:
            # what the unit is attached to has it at 0x248, or something else
            parent = struct.unpack_from("<I", data, 0x4C + COUNT * 12 + unit * OBJECT + 0x14)[0]
            if parent != 0xFFFFFFFF and random_.choice((False, True, True)):
                struct.pack_into("<I", data, 0x4C + COUNT * 12 + (parent & 0xFFFF) * OBJECT + 0x248, unit_index)
        datums = bytearray(random_.randbytes(4 * DATUM))
        player = random_.randrange(4)
        struct.pack_into("<I", datums, player * DATUM + 0x2C, unit_index)
        array = bytearray(random_.randbytes(0x4C))
        struct.pack_into("<I", array, 0x44, 0x4C)
        t.check(
            "simulation_player_get_unit_state",
            random_.getrandbits(16) << 16 | player,
            Buffer(random_.randbytes(0x40)),
            globals={
                "object_header_data": Buffer(bytes(data), pointers=table.pointers),
                "player_data": Buffer(bytes(array) + bytes(datums), pointers=(0x44,)),
            },
            externals={
                "unknown_000a5980": lambda harness, arguments, value=random_.choice((-1, 9, 9, 0x4321)): (value & 0xFFFFFFFF, (arguments[0],)),
                "object_get_origin": fills(1),
                "object_get_orientation": fills(2, 3),
                "object_get_velocities": fills(4, 5),
            },
        )
