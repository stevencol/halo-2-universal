# six small functions of the way to the main menu: eight numbers set,
# whether a player may with a number, a short of the table or of an object,
# the definition of a tag or of an element of it, how long a vector is, and
# counts of some things made one less

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
DEFINITION = 0x60


def run(t) -> None:
    random_ = random.Random(825)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def tags(definitions: list[bytes]) -> Buffer:
        data = bytearray(random_.randbytes(0x10 * len(definitions)))
        for tag in range(len(definitions)):
            struct.pack_into("<I", data, tag * 0x10 + 8, 0x10 * len(definitions) + tag * DEFINITION)
        return Buffer(bytes(data) + b"".join(definitions), pointers=tuple(tag * 0x10 + 8 for tag in range(len(definitions))))

    for n in range(400):
        t.check("unknown_0023ecd0", globals={"unknown_00470a3c": random_.randbytes(32)})

        data = bytearray(random_.randbytes(0x14))
        struct.pack_into("<hh", data, 0x10, random_.choice((0, 1, 2)), random_.choice((0, 1, 2, 3)))
        player = bytes((0, 0, random_.choice((0, 0x20, 0xFF, 0xDF)), 0))
        globals_ = bytearray(random_.randbytes(0x10))
        struct.pack_into("<h", globals_, 8, random_.choice((0, 1, 1)))
        t.check(
            "unknown_0017b270",
            Buffer(bytes(data)),
            Buffer(player),
            random_.choice((0, 1, 2)),
            globals={"players_globals": Buffer(bytes(globals_))},
        )

        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            struct.pack_into("<I", objects, FIRST + index * OBJECT, salted(random_.randrange(2)))
        definitions = []
        for tag in range(2):
            definition = bytearray(random_.randbytes(DEFINITION))
            struct.pack_into("<I", definition, 0x04, random_.choice((NONE, salted(1))))
            struct.pack_into("<I", definition, 0x38, salted(random_.randrange(2)))
            definitions.append(bytes(definition))
        t.check(
            "object_unknown_0003d3e0",
            salted(random_.randrange(COUNT)),
            random_.choice((0, 1, 0x100)),
            globals={
                "object_header_data": Buffer(bytes(objects), pointers=table.pointers),
                "global_tag_instances": tags(definitions),
                "unknown_004c1bd4": random_.getrandbits(16).to_bytes(2, "little"),
            },
        )

        # two tags; the first has three elements (0x20 bytes) after the definitions
        definitions = [bytearray(random_.randbytes(DEFINITION)) for _ in range(2)]
        struct.pack_into("<iI", definitions[0], 0x4C, random_.choice((0, 2, 3)), 0)
        struct.pack_into("<iI", definitions[1], 0x4C, 0, 0)
        elements = bytearray(random_.randbytes(3 * 0x20))
        for element in range(3):
            struct.pack_into("<I", elements, element * 0x20 + 4, random_.choice((NONE, salted(1), salted(0))))
        instances = tags([bytes(definition) for definition in definitions])
        data = bytearray(instances.data) + elements
        struct.pack_into("<I", data, 0x20 + 0x50, 0x20 + 2 * DEFINITION)
        t.check(
            "havok_unknown_001c6bb0",
            Buffer(struct.pack("<Ih", salted(random_.randrange(2) if random_.random() < 0.3 else 0), random_.choice((-1, 0, 1, 2, 3))) + b"\0\0"),
            globals={"global_tag_instances": Buffer(bytes(data), pointers=instances.pointers + (0x20 + 0x50,))},
        )

        vector = [random_.uniform(-1.2, 1.2) * random_.choice((1, 0.3)) for _ in range(3)]
        t.check("havok_unknown_00246eb0", Buffer(random_.randbytes(0x28) + struct.pack("<3f", *vector)))

        # things: a byte at 0x02 whose low three bits count them, from 0x1C on (to 0x34)
        entries = bytearray(random_.randbytes(0x50 + 4 * 0x10))
        struct.pack_into("<I", entries, 0x44, 0x50)
        data = bytearray(random_.randbytes(0x38 + 8 * 0x10))
        data[2] = random_.getrandbits(5) << 3 | random_.choice((0, 1, 3, 4, 5, 7))
        pointers = []
        for thing in range(1, 8):
            struct.pack_into("<I", data, 0x18 + thing * 4, 0x38 + thing * 0x10)
            pointers.append(0x18 + thing * 4)
            struct.pack_into("<I", data, 0x38 + thing * 0x10 + 8, salted(random_.randrange(4)))
        t.check(
            "unknown_002ae8e0",
            Buffer(bytes(data), pointers=tuple(pointers)),
            globals={"unknown_00502104": Buffer(bytes(entries), pointers=(0x44,))},
        )
