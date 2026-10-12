# source/networking/simulation_gamestate_entities.c, and the chain of the units

import random
import struct

from difftest import Buffer, Mock, mock_address

from test_units import COUNT, OBJECT, some_objects

FIRST = 0x4C + COUNT * 12
WORLD = 0x40
MEMORY = 0xA0AC
ENTITIES = (3, 0x155, 0x3FF)


def run(t) -> None:
    random_ = random.Random(595)

    for n in range(3000):
        # the thing that is going and its memory: what tells of the things that are is at the start of it
        thing = bytearray(random_.randbytes(WORLD + MEMORY))
        struct.pack_into("<I", thing, 4, WORLD)
        struct.pack_into("<I", thing, WORLD + 0x2098 + 0xC, WORLD)
        numbers = {}
        for place in ENTITIES:
            numbers[place] = random_.getrandbits(22) << 10 | place
            thing[WORLD + 0x44 + place * 8] = random_.choice((0, 1, 4, 5, 0xFB, 0xFF))
            at = WORLD + 0x2098 + 0x14 + place * 0x20
            struct.pack_into("<I", thing, at, random_.choice((numbers[place], numbers[place], numbers[place] ^ 0x400)))
            struct.pack_into("<h", thing, at + 4, random_.randrange(4))
            struct.pack_into("<I", thing, at + 8, random_.choice((0xFFFFFFFF, 0x12340005, 0x12340005)))
        globals_ = {
            "unknown_004cf770": random_.randbytes(12) + bytes(4),
            "unknown_004cf770+0xc": Buffer(bytes(thing), pointers=(4, WORLD + 0x2098 + 0xC)),
        }

        # the kinds: each has its functions, and the eighth is the one that the test stands in for
        kinds = bytearray(random_.randbytes(0x84 + 4 + 0x20))
        for kind in range(4):
            struct.pack_into("<I", kinds, 4 + kind * 4, 0x84)
        struct.pack_into("<I", kinds, 0x84, 0x88)
        struct.pack_into("<I", kinds, 0x88 + 7 * 4, mock_address(0))
        t.check(
            "simulation_gamestate_entity_get_object",
            random_.choice((0xFFFFFFFF, *numbers.values(), *numbers.values())),
            globals={**globals_, "unknown_004cf784": Buffer(bytes(kinds), pointers=(4, 8, 12, 16, 0x84))},
            mocks=[Mock(lambda call, value=random_.choice((0, 1, 1, 0x100)): value, stack_arguments=1, registers=("ecx",))],
        )

        table = some_objects(random_)
        data = bytearray(table.data)
        for index in range(COUNT):
            struct.pack_into("<I", data, FIRST + index * OBJECT + 0xD4, random_.choice((0xFFFFFFFF, *numbers.values(), *numbers.values())))
            # the chain of the units: to a later one, or to none
            later = list(range(index + 1, COUNT))
            link = random_.choice(later + [None]) if later else None
            struct.pack_into("<I", data, FIRST + index * OBJECT + 0x24C, 0xFFFFFFFF if link is None else (random_.getrandbits(16) << 16 | link))
        objects = {"object_header_data": Buffer(bytes(data), pointers=table.pointers)}
        an_object = random_.choice((0xFFFFFFFF, random_.getrandbits(16) << 16 | random_.randrange(COUNT), random_.getrandbits(16) << 16 | random_.randrange(COUNT)))
        t.check("simulation_object_get_created_entity", an_object, globals={**globals_, **objects})
        t.check("simulation_object_get_remote_entity", an_object, globals={**globals_, **objects})
        t.check("unit_get_ultimate_unknown24C", random_.getrandbits(16) << 16 | random_.randrange(COUNT), globals=objects)
