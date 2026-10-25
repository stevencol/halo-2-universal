# six functions of the way to the main menu (guided by halo2-decompiled): a
# looping sound of an object, a bitmap of a group, the entries of an object
# cleared, how a scenery is pathfound, a function for every object, and an
# object woken

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(846)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        entries = bytearray(random_.randbytes(0x50 + 4 * 0x18))
        struct.pack_into("<I", entries, 0x44, 0x50)
        made = random_.choice((NONE, salted(random_.randrange(4))))
        t.check(
            "sound_unknown_0018a5a0",
            random_.choice((NONE, 5)),
            random_.choice((NONE, 7)),
            f32(random_.uniform(0, 2)),
            globals={"unknown_004ed28c": Buffer(bytes(entries), pointers=(0x44,))},
            externals={"datum_new": lambda harness, arguments, made=made: (made, tuple(arguments))},
        )

        group = bytearray(random_.randbytes(0x10 + 0x50))
        struct.pack_into("<I", group, 8, 0x10)
        struct.pack_into("<iI", group, 0x10 + 0x44, random_.choice((0, 2, 5)), 0x60)
        picked = random_.choice((NONE, 0, 1, 4, 6, -2))
        t.check(
            "game_unknown_00137610",
            salted(0),
            random_.getrandbits(16),
            random_.getrandbits(16),
            globals={"global_tag_instances": Buffer(bytes(group), pointers=(8, 0x10 + 0x48))},
            externals={"tag_unknown_00137590": lambda harness, arguments, picked=picked: (picked & 0xFFFFFFFF, (arguments[0], arguments[1] & 0xFFFF, arguments[2] & 0xFFFF))},
        )

        count = random_.randrange(6)
        slots = bytearray(random_.randbytes(0x3FE0))
        for slot in range(6):
            struct.pack_into("<I", slots, slot * 0x20, random_.getrandbits(32) & ~(0xFFF << 6 | 0x10) | random_.choice((0, 0x10)) | random_.randrange(3) << 6)
        t.check(
            "unknown_0051f40c_clear_object",
            random_.choice((NONE, 0, salted(random_.randrange(3)))),
            globals={"unknown_0051f40c": bytes(slots), "unknown_005233ec": count},
            externals={"unknown_0051f40c_clear": lambda harness, arguments: (0, tuple(arguments))},
        )

        tags = bytearray(random_.randbytes(0x10 + 0xC0))
        struct.pack_into("<I", tags, 8, 0x10)
        placement = bytearray(random_.randbytes(0x50))
        struct.pack_into("<h", placement, 0x4C, random_.choice((0, 1, 2, 3, 4, -1)))
        t.check(
            "scenery_get_pathfinding_policy",
            random_.choice((NONE, salted(0))),
            Buffer(bytes(placement)),
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=(8,))},
        )

        table = some_objects(random_)
        objects = bytearray(table.data)
        struct.pack_into("<i", objects, 0x38, COUNT)
        for index in range(COUNT):
            struct.pack_into("<H", objects, 0x4C + index * 12, random_.choice((0, 0x1234)))
        t.check(
            "object_unknown_000bfcc0",
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
            externals={"object_unknown_000bfd20": lambda harness, arguments: (0, tuple(arguments))},
        )

        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            objects[0x4C + index * 12 + 2] = random_.choice((0, 1, 0x80, 0x81, 5))
        t.check(
            "object_unknown_000bba20",
            salted(random_.randrange(COUNT)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers)},
            externals={
                "object_get_ultimate_parent": lambda harness, arguments: (salted(random_.randrange(COUNT)) if False else (arguments[0] & 0xFFFF0000) | ((arguments[0] + 1) % COUNT), tuple(arguments)),
                "object_wake": lambda harness, arguments: (0, tuple(arguments)),
                "object_unknown_000bc7b0": lambda harness, arguments: (0, tuple(arguments)),
            },
        )
