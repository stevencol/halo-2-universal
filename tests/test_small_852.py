# four functions of the way to the main menu (guided by halo2-decompiled):
# the strings of the nodes of the scripts, the geometry of one of sixteen
# things, whether the geometry of a tag is predicted, and whether a biped
# moves

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
NODE = 0x14


def run(t) -> None:
    random_ = random.Random(852)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        nodes = bytearray(random_.randbytes(0x60 + 6 * NODE))
        struct.pack_into("<i", nodes, 0x24, NODE)
        struct.pack_into("<i", nodes, 0x38, random_.choice((0, 3, 6)))
        struct.pack_into("<II", nodes, 0x44, 0x60, 0x50)
        struct.pack_into("<I", nodes, 0x50, random_.getrandbits(6))
        for node in range(6):
            struct.pack_into("<h", nodes, 0x60 + node * NODE + 4, random_.choice((9, 9, 2)))
        scenario = bytearray(random_.randbytes(0x1B8))
        t.check(
            "unknown_002a09c0",
            globals={"script_node_data": Buffer(bytes(nodes), pointers=(0x44, 0x48)), "global_scenario": Buffer(bytes(scenario))},
        )

        entries = bytearray(random_.randbytes(16 * 0x20))
        for entry in range(16):
            struct.pack_into("<IiI", entries, entry * 0x20 + 4, random_.choice((NONE, salted(0))), 0, random_.randrange(2) << 9 | random_.randrange(2) << 18)
        tags = bytearray(random_.randbytes(0x10 + 0x30 + 2 * 0x5C))
        struct.pack_into("<II", tags, 8, 0x10, 0)
        struct.pack_into("<I", tags, 0x10 + 0x28, 0x40)
        bsp = bytearray(random_.randbytes(0x150 + 2 * 0x58 + 2 * 0xB0 + 3 * 0xC8))
        struct.pack_into("<I", bsp, 0xA0, 0x150 + 2 * 0x58)
        struct.pack_into("<I", bsp, 0x13C, 0x150 + 2 * 0x58 + 2 * 0xB0)
        struct.pack_into("<I", bsp, 0x144, 0x150)
        for place in range(2):
            struct.pack_into("<h", bsp, 0x150 + place * 0x58 + 0x34, random_.randrange(3))
        loaded = random_.random() < 0.6
        t.check(
            "unknown_000449e0",
            random_.randrange(16) | random_.getrandbits(16) << 16,
            random_.choice((0, 1)),
            random_.choice((0, 1)),
            globals={
                "unknown_004ba138": bytes(entries),
                "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x10 + 0x28)),
                "global_structure_bsp": Buffer(bytes(bsp), pointers=(0xA0, 0x13C, 0x144)),
            },
            externals={"geometry_cache_block_load": lambda harness, arguments, loaded=loaded: (int(loaded), (arguments[0], arguments[1]))},
        )

        # a definition with three sections (0x5C) and two groups of parts
        definition = bytearray(random_.randbytes(0x30 + 3 * 0x5C + 2 * 0x10 + 4 * 0x10))
        sections = 0x30
        groups = sections + 3 * 0x5C
        parts = groups + 2 * 0x10
        struct.pack_into("<iIiI", definition, 0x1C, 2, 0x10 + groups, 3, 0x10 + sections)
        struct.pack_into("<i", definition, 0x24, random_.choice((0, 1, 3)))
        for group in range(2):
            struct.pack_into("<iI", definition, groups + group * 0x10 + 8, random_.choice((0, 1, 2)), 0x10 + parts + group * 0x20)
        for part in range(4):
            struct.pack_into("<h", definition, parts + part * 0x10 + 4, random_.randrange(3))
        tags = bytearray(random_.randbytes(0x10)) + definition
        struct.pack_into("<I", tags, 8, 0x10)
        pointers = [8, 0x10 + 0x20, 0x10 + 0x28] + [0x10 + groups + group * 0x10 + 0x0C for group in range(2)]
        flip = random_.randrange(2)

        def predict(harness, arguments, flip=flip):
            return (int((arguments[0] >> 2 & 1) != flip or arguments[0] % 3 == 0), (arguments[0],))

        t.check(
            "unknown_0016e950",
            salted(0),
            random_.choice((0, 1, 2)) | random_.getrandbits(16) << 16,
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=tuple(pointers))},
            externals={"geometry_cache_block_predict": predict},
        )

        table = some_objects(random_)
        objects = bytearray(table.data) + bytearray(random_.randbytes(0x400))
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            objects[at + 0x34B] = random_.choice((0, 0, 1))
            struct.pack_into("<I", objects, at + 0xB4, random_.choice((NONE, salted(0))))
            objects[at + 0x3DC] = random_.choice((0, 3, 4, 5, 6))
            struct.pack_into("<3f", objects, at + 0x1B0, *(random_.choice((0.0, 0.0, 0.001, 1.0)) for _ in range(3)))
        components = bytearray(random_.randbytes(0x50 + 0xA0 + 0x50))
        struct.pack_into("<I", components, 0x44, 0x50)
        struct.pack_into("<I", components, 0x50 + 4, random_.getrandbits(32))
        struct.pack_into("<I", components, 0x50 + 0x70, 0xF0)
        components[0xF0 + 0x44] = random_.choice((0, 1))
        told = random_.randrange(2)
        t.check(
            "biped_unknown_000ddfb0",
            salted(random_.randrange(COUNT)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers), "havok_component_data": Buffer(bytes(components), pointers=(0x44, 0x50 + 0x70))},
            externals={"unit_unknown_000e4050": lambda harness, arguments, told=told: (told, tuple(arguments))},
        )
