# three functions of the way to the main menu (guided by halo2-decompiled):
# the leaf and cluster of a point, whether a weapon is held by a player, and
# a new object header

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(847)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        bsp = bytearray(random_.randbytes(0xA0 + 4 * 8))
        struct.pack_into("<I", bsp, 0x30, 0xA0)
        struct.pack_into("<i", bsp, 0x2C, random_.choice((0, 3, 4)))
        struct.pack_into("<i", bsp, 0x9C, random_.choice((0, 2, 5)))
        leaf = random_.choice((NONE, 0, 1, 2, 3))
        t.check(
            "unknown_0002b720",
            Buffer(random_.randbytes(12)),
            Buffer(struct.pack("<I", random_.choice((NONE, 0, 2, 3, 9)))),
            Buffer(struct.pack("<I", random_.choice((NONE, 0, 1, 4, 9)))),
            globals={"global_structure_bsp": Buffer(bytes(bsp), pointers=(0x30,))},
            externals={"bsp3d_test_point": lambda harness, arguments, leaf=leaf: (leaf, (arguments[0], arguments[1]))},
        )

        table = some_objects(random_)
        objects = bytearray(table.data) + bytearray(random_.randbytes(0x100))
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            objects[at + 0x12C] = random_.choice((0, 1, 3))
            struct.pack_into("<II", objects, at + 0x13C, random_.choice((NONE, salted(random_.randrange(4)))), 0)
            struct.pack_into("<I", objects, at + 0x154, random_.choice((NONE, salted(random_.randrange(COUNT)))))
        game = bytearray(random_.randbytes(0x10))
        struct.pack_into("<i", game, 8, random_.choice((1, 2, 2)))
        told = random_.randrange(2)
        t.check(
            "weapon_unknown_00106030",
            salted(random_.randrange(COUNT)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers), "main_game_globals": Buffer(bytes(game))},
            externals={"game_engine_unknown_00159dd0": lambda harness, arguments, told=told: (told, (arguments[0] & 0xFF,))},
        )

        table = some_objects(random_)
        made = random_.choice((NONE, salted(random_.randrange(COUNT))))
        ok = random_.random() < 0.8
        t.check(
            "object_header_new",
            random_.choice((0, 4, 13, 0x40)) | random_.getrandbits(16) << 16,
            globals={"object_header_data": table, "unknown_004de2ec": random_.getrandbits(32)},
            externals={
                "datum_new": lambda harness, arguments, made=made: (made, tuple(arguments)),
                "memory_pool_block_allocate": lambda harness, arguments, ok=ok: (int(ok), tuple(arguments)),
                "datum_delete": lambda harness, arguments: (0, tuple(arguments)),
            },
        )
