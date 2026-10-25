# seven functions of the way to the main menu (guided by halo2-decompiled):
# a player for the entries, the resources of a graph and of those it
# inherits, the object of a havok component woken, whether a gamepad has a
# button, what the partition keeps of an object, whether geometry blocks are
# loaded, and an object that blends

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
COMPONENT = 0xA0


def run(t) -> None:
    random_ = random.Random(850)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        t.check(
            "unknown_0051f40c_player",
            random_.choice((NONE, 3, salted(1))),
            globals={
                "unknown_0051f408": bytes((random_.choice((0, 1)),)),
                "unknown_004850c8": random_.choice((0, 1)),
                "unknown_005234c4": random_.getrandbits(32),
                "unknown_005233ec": random_.randrange(4),
            },
            externals={"unknown_0051f40c_clear": lambda harness, arguments: (0, tuple(arguments))},
        )

        # a graph inheriting from two tags (whose definitions are graphs too, with no resources)
        graph = bytearray(random_.randbytes(0x60 + 2 * 0x20))
        struct.pack_into("<iI", graph, 0x4C, random_.choice((0, 1, 2)), 0x60)
        for entry in range(2):
            struct.pack_into("<I", graph, 0x60 + entry * 0x20 + 4, random_.choice((NONE, salted(0), salted(1))))
        tags = bytearray(random_.randbytes(0x20 + 2 * 0xB4))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x20 + tag * 0xB4)
            struct.pack_into("<i", tags, 0x20 + tag * 0xB4 + 0xAC, 0)
        t.check(
            "havok_unknown_001ddaf0",
            Buffer(bytes(graph), pointers=(0x50,)),
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18))},
            externals={"havok_unknown_001ddab0": lambda harness, arguments: (0, tuple(arguments))},
        )

        thing = bytearray(random_.randbytes(0x40))
        pairs = random_.randrange(4)
        struct.pack_into("<Ii", thing, 0x30, 0x40, pairs)
        table = b"".join(struct.pack("<iI", random_.choice((0x2001, 0x2002, 7)), salted(random_.randrange(2))) for _ in range(4))
        components = bytearray(random_.randbytes(0x50 + 2 * COMPONENT))
        struct.pack_into("<I", components, 0x44, 0x50)
        for component in range(2):
            struct.pack_into("<I", components, 0x50 + component * COMPONENT + 4, random_.getrandbits(32))
        t.check(
            "havok_unknown_001c4a20",
            Buffer(bytes(thing) + table, pointers=(0x30,)),
            globals={"havok_component_data": Buffer(bytes(components), pointers=(0x44,))},
            externals={"object_unknown_000bba20": lambda harness, arguments: (0, tuple(arguments))},
        )

        state = bytearray(random_.randbytes(0x1B0))
        state[1] = random_.choice((0, 1))
        for slot in range(4):
            struct.pack_into("<I", state, 0x14 + slot * 4, random_.choice((0, 0, 0x1234)))
        t.check("input_unknown_0023e400", random_.randrange(0x30), globals={"input_globals": bytes(state)})

        table = some_objects(random_)
        objects = bytearray(table.data)
        flags = random_.getrandbits(16)
        t.check(
            "object_unknown_000b88e0",
            salted(random_.randrange(COUNT)),
            Buffer(random_.randbytes(0x14)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
            externals={"object_unknown_0016b430": lambda harness, arguments, flags=flags: (flags, tuple(arguments))},
        )

        data = bytearray(random_.randbytes(0x30 + 3 * 0x5C))
        struct.pack_into("<i", data, 0x1C, random_.choice((0, 1, 3)))
        struct.pack_into("<I", data, 0x28, 0x30)
        tags = bytearray(random_.randbytes(0x10)) + data
        struct.pack_into("<I", tags, 8, 0x10)
        loaded = random_.random() < 0.6
        t.check(
            "unknown_0003e320",
            salted(0),
            Buffer(bytes(random_.choice((0, 1, 2, 0xFF)) for _ in range(3))),
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x10 + 0x28))},
            externals={"geometry_cache_block_load": lambda harness, arguments, loaded=loaded: (int(loaded), (arguments[0], arguments[1]))},
        )

        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<I", objects, at, salted(0))
            struct.pack_into("<hhhh", objects, at + 0x10E, 0x40, random_.choice((0, 0x20, 0x60)), random_.choice((-1, 0x80)), 0)
            struct.pack_into("<h", objects, at + 0x12A, 0x100)
        tags = bytearray(random_.randbytes(0x10 + 0x40))
        struct.pack_into("<I", tags, 8, 0x10)
        struct.pack_into("<I", tags, 0x10 + 0x38, random_.choice((NONE, 3)))
        t.check(
            "object_unknown_000ba350",
            salted(random_.randrange(COUNT)),
            f32(random_.choice((0.0, 0.5))),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers), "global_tag_instances": Buffer(bytes(tags), pointers=(8,))},
            externals={
                "havok_unknown_001cb5f0": lambda harness, arguments: (0, tuple(arguments)),
                "object_wake": lambda harness, arguments: (0, tuple(arguments)),
            },
        )
