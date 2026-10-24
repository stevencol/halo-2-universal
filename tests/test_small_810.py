# four medium functions of the way to the main menu: bits of how an object
# is, shorts of an element of a tag of an object, a point near a polygon, and
# the settings of a kind

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
BIG = 0x400
SETTINGS = ("unknown_004ba020", "unknown_004ba021", "unknown_004ba022", "unknown_004ba023", "unknown_004ba024", "unknown_004ba025")


def run(t) -> None:
    random_ = random.Random(810)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def objects(fill) -> Buffer:
        table = some_objects(random_)
        headers = bytearray(table.data[:FIRST])
        data = bytearray()
        for index in range(COUNT):
            struct.pack_into("<I", headers, 0x4C + index * 12 + 8, FIRST + index * BIG)
            headers[0x4C + index * 12 + 2] = random_.choice((0, 0x10, 0xEF))
            one = bytearray(random_.randbytes(BIG))
            fill(one)
            data += one
        return Buffer(bytes(headers) + bytes(data), pointers=table.pointers)

    for n in range(400):
        def fill(one):
            one[0xAA] = random_.choice((0, 0, 11, 11, 3))
            struct.pack_into("<I", one, 4, random_.getrandbits(32) & random_.getrandbits(32))

        t.check("object_unknown_0016b430", salted(random_.randrange(COUNT)), globals={"object_header_data": objects(fill)})

        # definitions 0 to 3 name at 0x38 the tag 4, with elements (0x38) at 0x54 and entries (0x14) at their 0x18
        tags = bytearray(random_.randbytes(0x60 + 4 * 0x40 + 0x80 + 2 * 0x38 + 8 * 0x14))
        definitions = 0x60
        tag = 0x60 + 4 * 0x40
        elements = tag + 0x80
        entries = elements + 2 * 0x38
        pointers = []
        for index in range(4):
            struct.pack_into("<I", tags, index * 0x10 + 8, definitions + index * 0x40)
            struct.pack_into("<I", tags, definitions + index * 0x40 + 0x38, 4)
            pointers.append(index * 0x10 + 8)
        struct.pack_into("<I", tags, 4 * 0x10 + 8, tag)
        pointers.append(4 * 0x10 + 8)
        struct.pack_into("<iI", tags, tag + 0x54, 0, elements)
        struct.pack_into("<i", tags, tag + 0x70, random_.choice((0, 1, 4, 8)))
        pointers.append(tag + 0x58)
        for element in range(2):
            for at in range(8):
                tags[elements + element * 0x38 + 4 + at] = random_.choice((0xFF, 0, 3, 7))
            struct.pack_into("<I", tags, elements + element * 0x38 + 0x18, entries)
            pointers.append(elements + element * 0x38 + 0x18)

        def fill(one):
            struct.pack_into("<I", one, 0, salted(random_.randrange(4)))
            one[0xB1] = random_.choice((0xFF, 0, 1))

        # the block at 0x54 is the pointer at 0x58: the count goes before it
        tags_ = bytearray(tags)
        struct.pack_into("<I", tags_, tag + 0x54, elements)
        pointers = [p if p != tag + 0x58 else tag + 0x54 for p in pointers]
        t.check(
            "object_unknown_0004c0d0",
            salted(random_.randrange(COUNT)),
            Buffer(random_.randbytes(0x170)),
            random_.choice((0, 1, 1, 2)),
            globals={"object_header_data": objects(fill), "global_tag_instances": Buffer(bytes(tags_), pointers=tuple(pointers))},
        )

        count = random_.choice((0, 3, 4))
        square = [(-1.0, -1.0), (1.0, -1.0), (1.0, 1.0), (-1.0, 1.0)][:count]
        if n % 6 == 0 and count:
            square[1] = square[0]
        if random_.random() < 0.5:
            square.reverse()
        points = b"".join(struct.pack("<2I", f32(x), f32(y)) for x, y in square) + bytes(8)
        point = struct.pack("<2I", f32(random_.choice((0.0, 1.0, 1.5, -1.2, random_.uniform(-2, 2)))), f32(random_.choice((0.0, 1.1, random_.uniform(-2, 2)))))
        t.check(
            "ai_point_near_polygon2d",
            Buffer(points),
            Buffer(point),
            f32(random_.choice((0.0, 0.1, 0.5, 1.0))),
            random_.getrandbits(16) << 16 | count,
        )

        t.check(
            "game_unknown_0002b4a0_set_kind",
            random_.choice((0, 1, 2, 3, 4, 5)),
            globals={
                "unknown_004670e4": random_.getrandbits(32),
                "unknown_004ba01c": random_.getrandbits(32),
                "unknown_004ba028": random_.getrandbits(32),
                "unknown_004ba02c": random_.getrandbits(32),
                **{name: bytes((random_.getrandbits(8),)) for name in SETTINGS},
            },
        )
