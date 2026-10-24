# thirteen small functions: whether two angles are valid, the end of
# something of online_unknown_000b36c0.c, two of texts, the iterators of the
# objects of each cluster, the bit 0x40 of the byte at 0xC0 of an object, a
# long of the tag of an object and a child of the object of a thing

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(784)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def told(harness, arguments):
        return 0, tuple(arguments)

    def text() -> bytes:
        return bytes(random_.choice(b"ab\x80") for n in range(random_.choice((0, 1, 2, 3, 4, 5, 6, 9, random_.randrange(300)))))

    def references(count: int, size: int) -> tuple[bytes, tuple[int, ...]]:
        """a data array of so many references (each with an object at 4 and the next at 8)"""
        data = bytearray(random_.randbytes(0x50 + count * size))
        struct.pack_into("<I", data, 0x24, size)
        struct.pack_into("<I", data, 0x44, 0x50)
        for index in range(count):
            later = random_.choice([NONE] + [random_.getrandbits(16) << 16 | later for later in range(index + 1, count)])
            struct.pack_into("<I", data, 0x50 + index * size + 8, later)
        return bytes(data), (0x44,)

    for n in range(400):
        angles = [
            f32(random_.choice((0.0, -0.0, 1.0, 3.0, 6.2831855, 6.3, -0.1, 1.5707964, -1.5707964, 1.6, -1.6, random_.uniform(-7, 7))))
            for _ in range(2)
        ]
        if n % 7 == 0:
            angles[random_.randrange(2)] = random_.choice((0x7F800000, 0xFF800000, 0x7FC00000, 0x7F800001))
        t.check("valid_real_euler_angles2d", Buffer(struct.pack("<2I", *angles)))

        online = lambda: {
            "unknown_00510580": bytes((random_.choice((0, 1, 0x80)),)),
            "unknown_00510584": random_.choice((NONE, 0, 3)),
            "unknown_005107d8": random_.choice((0, 1, 2, 5, NONE)),
            "unknown_0054761c": bytes((random_.choice((0, 1)),)),
            "unknown_00547620": random_.choice((0, 1, 2, 5, 14, 15, NONE)),
        }
        externals = {"online_unknown_000b36c0_unregister": told, "online_task_close": told}
        t.check("online_unknown_000b36c0_unknown_000b4020", globals=online(), externals=externals)
        t.check("online_unknown_000b36c0_unknown_000b4050", globals=online(), externals=externals)

        first = text()
        second = random_.choice((first, first, first[:-1] + b"c" if first else b"c", text(), first[:-1]))
        t.check("cseries_unknown_000b6190", Buffer(first + b"\0"), Buffer(second + b"\0"))
        t.check("cseries_unknown_000b6720", Buffer(text() + b"\0" + random_.randbytes(4)))

        # the partitions: the first reference of each of eight clusters, and the references
        for name, size in (("collideable_object_cluster_partition", 0x10), ("noncollideable_object_cluster_partition", 0x14)):
            data, pointers = references(6, size)
            firsts = struct.pack("<8I", *(random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(6))) for _ in range(8)))
            partition = {f"{name}+0": Buffer(firsts), f"{name}+4": Buffer(data, pointers=pointers)}
            kind = "noncollideable" if name.startswith("non") else "collideable"
            first_arguments = (
                Buffer(random_.randbytes(4)),
                random_.getrandbits(16) << 16 | random_.randrange(8),
                Buffer(random_.randbytes(4), places=(0,)),
            )
            if kind == "collideable":
                t.check("cluster_get_first_collideable_object", *first_arguments, globals=partition)
            else:
                t.check("cluster_get_first_noncollideable_object", *first_arguments, globals=partition)
            if kind == "collideable":
                t.check(
                    "cluster_get_next_collideable_object",
                    Buffer(struct.pack("<I", random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(6))))),
                    Buffer(random_.randbytes(4), places=(0,)),
                    globals=partition,
                )

        # an iterator, its partition and the references of its second table
        iterator = bytearray(random_.randbytes(0x70))
        struct.pack_into("<II", iterator, 0, 0x10, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(6))))
        struct.pack_into("<I", iterator, 0x18, 0x70)
        data, pointers = references(6, 0x0C)
        data = bytearray(data)
        struct.pack_into("<I", data, 0x44, 0x70 + 0x50)
        t.check("object_get_next_cluster", Buffer(bytes(iterator) + bytes(data), pointers=(0, 0x18, 0x70 + 0x44)))

        # objects with havok components
        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            struct.pack_into("<I", objects, FIRST + index * OBJECT + 0xB4, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(4))))
        havok = bytearray(random_.randbytes(0x50 + 4 * 0xA0))
        struct.pack_into("<I", havok, 0x44, 0x50)
        globals_ = {"object_header_data": Buffer(bytes(objects), pointers=table.pointers), "havok_component_data": Buffer(bytes(havok), pointers=(0x44,))}
        object_index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        t.check("object_unknown_000b8840", object_index, globals=globals_, externals={"unknown_001d1260": told})
        t.check("object_unknown_000b8890", object_index, globals=globals_, externals={"unknown_001d1540": told})

        # the tag of an object, with a block of elements of 0x18 bytes at 0x94
        tag = random_.randrange(4)
        tags = bytearray(random_.randbytes(0x140))
        struct.pack_into("<I", tags, tag * 0x10 + 8, 0x40)
        struct.pack_into("<iI", tags, 0x40 + 0x94, random_.choice((0, 1, 3, 4, -1)), 0xE0)
        objects = bytearray(table.data)
        index = random_.randrange(COUNT)
        struct.pack_into("<I", objects, FIRST + index * OBJECT, random_.getrandbits(16) << 16 | tag)
        t.check(
            "object_unknown_000b8c40",
            random_.getrandbits(16) << 16 | index,
            random_.getrandbits(16) << 16 | word(random_.choice((-1, 0, 1, 2, 3, 4, -0x8000))) & 0xFFFF,
            globals={
                "object_header_data": Buffer(bytes(objects), pointers=table.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=(tag * 0x10 + 8, 0x40 + 0x98)),
            },
        )

        # the children of an object: each child's next is a later one
        objects = bytearray(table.data)
        tags_ = [random_.choice((1, 2)) for _ in range(COUNT)]
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            later = [random_.getrandbits(16) << 16 | later for later in range(index + 1, COUNT)]
            struct.pack_into("<I", objects, at, tags_[index])
            struct.pack_into("<I", objects, at + 0x0C, random_.choice([NONE] + later))
            struct.pack_into("<I", objects, at + 0x10, random_.choice([NONE, random_.getrandbits(16) << 16 | random_.randrange(COUNT)]))
            struct.pack_into("<I", objects, at + 0xD4, random_.choice((NONE, NONE, 0x1234)))
        owner = random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(COUNT)))
        thing = bytearray(random_.randbytes(0x14))
        struct.pack_into("<I", thing, 4, random_.choice((1, 2, 3)))
        t.check(
            "simulation_unknown_000a3a20",
            random_.getrandbits(32),
            Buffer(bytes(thing)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
            externals={"simulation_gamestate_entity_get_object": lambda harness, arguments, owner=owner: (owner, (arguments[0],))},
        )
