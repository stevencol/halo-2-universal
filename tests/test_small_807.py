# five medium functions of the way to the main menu: something of the sound
# made from global vectors, two tags of the definition of an object, twenty
# bytes of an object, two reals by a havok component and the state flags of
# the rasterizer

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
BIG = 0x300


def run(t) -> None:
    random_ = random.Random(807)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    def objects_with(size: int, fill) -> tuple[bytes, tuple]:
        table = some_objects(random_)
        headers = bytearray(table.data[:FIRST])
        objects = bytearray()
        for index in range(COUNT):
            struct.pack_into("<I", headers, 0x4C + index * 12 + 8, FIRST + index * size)
            headers[0x4C + index * 12 + 3] = random_.choice((0, 1, 1, 2))
            one = bytearray(random_.randbytes(size))
            fill(one)
            objects += one
        return bytes(headers) + bytes(objects), table.pointers

    for n in range(400):
        t.check(
            "sound_unknown_0016f3c0",
            Buffer(random_.randbytes(0x48)),
            globals={
                "unknown_00468788": Buffer(random_.randbytes(12)),
                "global_zero_vector3d": Buffer(random_.randbytes(12)),
                "global_forward3d": Buffer(random_.randbytes(12)),
                "global_up3d": Buffer(random_.randbytes(12)),
                "global_structure_bsp_index": struct.pack("<h", random_.choice((-1, 0, 2))),
                "unknown_0054e854": random_.getrandbits(32),
            },
        )

        # definitions (tags 0 to 3) name at 0x38 a tag 4 to 7, which names one of 8 to 11 at 0x04
        tags = bytearray(random_.randbytes(0xC0 + 12 * 0x80))
        for tag in range(12):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0xC0 + tag * 0x80)
        for tag in range(4):
            struct.pack_into("<I", tags, 0xC0 + tag * 0x80 + 0x38, random_.choice((NONE, salted(4 + tag))))
        for tag in range(4, 8):
            struct.pack_into("<I", tags, 0xC0 + tag * 0x80 + 0x04, random_.choice((NONE, salted(8 + tag - 4))))
            struct.pack_into("<I", tags, 0xC0 + tag * 0x80 + 0x0C, random_.choice((NONE, 7)))
        for tag in range(8, 12):
            struct.pack_into("<ii", tags, 0xC0 + tag * 0x80 + 0x74, random_.choice((0, 1)), 0)
            struct.pack_into("<i", tags, 0xC0 + tag * 0x80 + 0x7C, random_.choice((0, 2, -1)))
        instances = lambda: Buffer(bytes(tags), pointers=tuple(tag * 0x10 + 8 for tag in range(12)))
        data, pointers = objects_with(0x260, lambda one: struct.pack_into("<I", one, 0, salted(random_.randrange(4))))
        t.check(
            "object_unknown_000bf9a0",
            salted(random_.randrange(COUNT)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            globals={"object_header_data": Buffer(data, pointers=pointers), "global_tag_instances": instances()},
        )

        data, pointers = objects_with(0x400, lambda one: struct.pack_into("<h", one, 0x33E, 0x200))
        t.check("object_unknown_00113870", salted(random_.randrange(COUNT)), globals={"object_header_data": Buffer(data, pointers=pointers)})

        def fill(one):
            struct.pack_into("<I", one, 0, salted(random_.randrange(4)))
            struct.pack_into("<I", one, 0x134, random_.choice((0, 2)))
            struct.pack_into("<I", one, 0x248, random_.choice((NONE, NONE, 4)))

        data, pointers = objects_with(BIG, fill)
        definitions = bytearray(random_.randbytes(0x40 + 4 * 0x200))
        for tag in range(4):
            struct.pack_into("<I", definitions, tag * 0x10 + 8, 0x40 + tag * 0x200)
            struct.pack_into("<I", definitions, 0x40 + tag * 0x200 + 0x1EC, random_.choice((0, 1 << 19)))
        components = bytearray(random_.randbytes(0x50 + 4 * 0xA0))
        struct.pack_into("<I", components, 0x44, 0x50)
        for component in range(4):
            struct.pack_into("<I", components, 0x50 + component * 0xA0 + 8, salted(random_.randrange(COUNT)))
        t.check(
            "havok_unknown_001cf8b0",
            salted(random_.randrange(4)),
            Buffer(random_.randbytes(0x10)),
            Buffer(random_.randbytes(0x44)),
            globals={
                "havok_component_data": Buffer(bytes(components), pointers=(0x44,)),
                "object_header_data": Buffer(data, pointers=pointers),
                "global_tag_instances": Buffer(bytes(definitions), pointers=tuple(tag * 0x10 + 8 for tag in range(4))),
            },
        )

        last = random_.choice((0, 3, 5))
        names = ("rasterizer_flags_group2_00485060", "rasterizer_flags_group2_00485068", "rasterizer_flags_group2_0048507c",
                 "rasterizer_flags_group2_00485080", "rasterizer_flags_group2_0048508c", "rasterizer_flags_group2_00485090",
                 "rasterizer_flags_group2_004850ac", "rasterizer_flags_group2_004850b0", "rasterizer_flags_group2_004850bc",
                 "rasterizer_flags_group2_004850c0", "rasterizer_state_flags_00485b78", "rasterizer_state_flags_00485b7c",
                 "rasterizer_state_flags_00485b80", "rasterizer_state_flags_00485b84", "rasterizer_state_flags_00485b88")
        t.check(
            "rasterizer_unknown_0001bd50",
            random_.choice((0, last, 3, 5)),
            globals={"unknown_00485a58": last, **{name: random_.getrandbits(32) for name in names}},
        )
