# source/units/units.c, and what simulation_player.c asks of a unit

import random
import struct

from difftest import Buffer

OBJECT = 0x250
COUNT = 5


def some_objects(random_) -> Buffer:
    """The table of the objects: its entries, of 12 bytes, and the objects after them; each is attached to a later one or to none."""
    headers = bytearray(random_.randbytes(0x4C + COUNT * 12))
    struct.pack_into("<I", headers, 0x44, 0x4C)
    pointers = [0x44]
    objects = bytearray()
    for index in range(COUNT):
        struct.pack_into("<I", headers, 0x4C + index * 12 + 8, len(headers) + len(objects))
        pointers.append(0x4C + index * 12 + 8)
        one = bytearray(random_.randbytes(OBJECT))
        later = [later for later in range(index + 1, COUNT)]
        parent = random_.choice(later + [None, None]) if later else None
        struct.pack_into("<I", one, 0x14, 0xFFFFFFFF if parent is None else (random_.getrandbits(16) << 16 | parent))
        one[0xAA] = random_.choice((0, 1, 1, 2, 3))
        one[0x212] = random_.choice((0, 1, 3, 0xFF))
        one[0x213] = random_.choice((0, 1, 3, 0xFF))
        objects += one
    return Buffer(bytes(headers) + bytes(objects), pointers=tuple(pointers))


def run(t) -> None:
    random_ = random.Random(5912)

    for n in range(2000):
        table = some_objects(random_)
        t.check(
            "unit_get_weapon",
            random_.getrandbits(16) << 16 | random_.randrange(COUNT),
            random_.choice((0, 1, 3, 0xFFFF, 0xABCDFFFF, 0x12340002)),
            globals={"object_header_data": table},
        )

        # the unit has the object at 0x24C, or another
        data = bytearray(table.data)
        index = random_.randrange(COUNT)
        object_index = random_.choice((0xFFFFFFFF, random_.getrandbits(16) << 16 | index, random_.getrandbits(16) << 16 | index))
        if object_index != 0xFFFFFFFF and random_.choice((False, True, True)):
            at = index
            while True:
                where = 0x4C + COUNT * 12 + at * OBJECT
                parent = struct.unpack_from("<I", data, where + 0x14)[0]
                if parent == 0xFFFFFFFF:
                    break
                at = parent & 0xFFFF
            struct.pack_into("<I", data, where + 0x24C, object_index)
        t.check(
            "simulation_player_get_unit_weapons",
            Buffer(random_.randbytes(8)),
            object_index,
            globals={"object_header_data": Buffer(bytes(data), pointers=table.pointers)},
            externals={
                "unit_get_weapon": lambda harness, arguments, value=random_.getrandbits(32): (
                    value ^ (arguments[1] & 0xFFFF),
                    (arguments[0], arguments[1] & 0xFFFF),
                ),
            },
        )
