# three medium functions of the way to the main menu: the element of the
# structure bsp an object matches, an entry of some bits by its kind, and one
# more transparent plane

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(815)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<I", objects, at, salted(random_.randrange(2)))
            objects[at + 0xAA] = random_.choice((0, 1))
            objects[at + 0xAB] = random_.choice((0, 1))
            struct.pack_into("<Ih", objects, at + 0xA4, random_.choice((5, 6)), random_.choice((0, 1)))
        tags = bytearray(random_.randbytes(0x20 + 2 * 0x10))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x20 + tag * 0x10)
            tags[0x20 + tag * 0x10 + 3] = random_.choice((0, 1, 1))
        bsp = bytearray(random_.randbytes(0x240 + 4 * 0x5C))
        struct.pack_into("<iI", bsp, 0x22C, random_.choice((0, 1, 4)), 0x240)
        for element in range(4):
            at = 0x240 + element * 0x5C
            struct.pack_into("<Ih", bsp, at, random_.choice((5, 6)), random_.choice((0, 1)))
            bsp[at + 6] = random_.choice((0, 1))
            bsp[at + 7] = random_.choice((0, 1))
        t.check(
            "object_unknown_000bfd20",
            salted(random_.randrange(COUNT)),
            globals={
                "object_header_data": Buffer(bytes(objects), pointers=table.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18)),
                "global_structure_bsp": Buffer(bytes(bsp), pointers=(0x230,)),
            },
        )

        # the bits: a base (0x00), a definition (0x04) whose small numbers place the entries
        data = bytearray(random_.randbytes(0x10 + 0x10 + 0x100))
        struct.pack_into("<II", data, 0, 0x20, 0x10)
        data[9] = random_.choice((0, 1, 2, 3, 4))
        struct.pack_into("<bb", data, 0x10, random_.choice((0, 4)), random_.choice((0, 8)))
        struct.pack_into("<h", data, 0x16, random_.choice((0, 16)))
        struct.pack_into("<i", data, 0x1C, random_.choice((0, 8)))
        t.check(
            "unit_unknown_0020aa70",
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(12)),
            Buffer(bytes(data), pointers=(0, 4)),
            random_.getrandbits(16) << 16 | random_.randrange(4),
            globals={"global_zero_vector3d": Buffer(random_.randbytes(12))},
        )

        count = random_.choice((0, 1, 3, 0x400, 0x401))
        planes = bytearray(random_.randbytes(0x70 * 4))
        size = random_.choice((0, 3, 8, 16))
        t.check(
            "rasterizer_debug_add_plane",
            size,
            Buffer(struct.pack("<3I", *(f32(random_.uniform(-5, 5)) for _ in range(3)))),
            random_.getrandbits(32),
            Buffer(random_.randbytes(16)),
            globals={
                "unknown_004b6280": count,
                "unknown_004b6288": Buffer(bytes(planes)),
                "unknown_004b6291": bytes((random_.choice((0, 1)),)),
                "unknown_00485618": struct.pack("<3I", *(f32(random_.uniform(-5, 5)) for _ in range(3))),
                "unknown_00485624": struct.pack("<3I", *(f32(random_.uniform(-1, 1)) for _ in range(3))),
            },
        )
