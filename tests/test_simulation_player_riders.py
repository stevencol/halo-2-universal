# source/networking/simulation_player.c: the weapons in hand of the units that ride with an object

import random
import struct

from difftest import Buffer

OBJECT = 0x250
COUNT = 6
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(593)

    for n in range(3000):
        # the table of the objects; the last one is what the others are attached to, one after the other
        headers = bytearray(random_.randbytes(FIRST))
        struct.pack_into("<I", headers, 0x44, 0x4C)
        pointers = [0x44]
        objects = bytearray()
        top = COUNT - 1
        riders = random_.sample(range(top), random_.choice((0, 1, 2, top)))
        for index in range(COUNT):
            struct.pack_into("<I", headers, 0x4C + index * 12 + 8, FIRST + index * OBJECT)
            pointers.append(0x4C + index * 12 + 8)
            one = bytearray(random_.randbytes(OBJECT))
            salt = random_.getrandbits(16) << 16
            if index == top:
                struct.pack_into("<I", one, 0x14, 0xFFFFFFFF)
                struct.pack_into("<I", one, 0x10, (salt | riders[0]) if riders else 0xFFFFFFFF)
            else:
                struct.pack_into("<I", one, 0x14, salt | top)
                struct.pack_into("<I", one, 0x10, 0xFFFFFFFF)
                if index in riders:
                    place = riders.index(index)
                    struct.pack_into("<I", one, 0xC, (salt | riders[place + 1]) if place + 1 < len(riders) else 0xFFFFFFFF)
            one[0xAA] = random_.choice((0, 0, 1, 1, 2, 3))
            struct.pack_into("<i", one, 0x24C, random_.choice((-1, 5, 5, 5)))
            one[0x212] = random_.choice((0, 1, 3, 0xFF))
            one[0x213] = random_.choice((0, 1, 3, 0xFF))
            objects += one
        start = random_.randrange(COUNT)
        object_index = random_.choice((0xFFFFFFFF, random_.getrandbits(16) << 16 | start, random_.getrandbits(16) << 16 | start))
        t.check(
            "simulation_player_get_rider_weapons",
            object_index,
            Buffer(struct.pack("<2I", random_.choice((0xFFFFFFFF, 0xFFFFFFFF, 7)), random_.choice((0xFFFFFFFF, 0xFFFFFFFF, 7)))),
            globals={"object_header_data": Buffer(bytes(headers) + bytes(objects), pointers=tuple(pointers))},
            externals={
                "unit_get_ultimate_unknown24C": lambda harness, arguments, value=random_.choice((object_index, object_index, 0x1234)): (
                    value,
                    (arguments[0],),
                ),
            },
        )
