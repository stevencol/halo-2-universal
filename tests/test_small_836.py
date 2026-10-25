# five functions of the way to the main menu: two things that take memory
# of the state of the game, a new thread of the scripts, the object that has
# an object, and the matrices of a tree of nodes

import random
import struct

from difftest import Buffer, Memory

from test_game_state import BASE, game_state_globals
from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
THREAD = 0x418
NODE = 0x60


def run(t) -> None:
    random_ = random.Random(836)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        allocated = random_.choice((0, 0x40, 0x400))
        t.check(
            "unknown_005093e0_initialize",
            globals={"game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated), "unknown_005093e0": random_.getrandbits(32)},
            memory={BASE + allocated: Memory(random_.randbytes(0x400))},
        )
        t.check(
            "unknown_005093e4_initialize",
            globals={"game_state_globals": game_state_globals(random_, base=BASE, cpu_allocation_size=allocated), "unknown_005093e4": random_.getrandbits(32)},
            memory={BASE + allocated: Memory(random_.randbytes(0x40))},
        )

        threads = bytearray(random_.randbytes(0x50 + 2 * THREAD))
        struct.pack_into("<I", threads, 0x44, 0x50)
        scenario = bytearray(random_.randbytes(0x1C0 + 3 * 0x28))
        struct.pack_into("<I", scenario, 0x1BC, 0x1C0)
        for script in range(3):
            struct.pack_into("<h", scenario, 0x1C0 + script * 0x28 + 0x20, random_.choice((0, 1, 2)))
        made = random_.choice((NONE, salted(random_.randrange(2))))
        t.check(
            "hs_thread_new",
            random_.choice((NONE, 0, 1, 2)),
            random_.getrandbits(8),
            globals={"hs_thread_data": Buffer(bytes(threads), pointers=(0x44,)), "global_scenario": Buffer(bytes(scenario), pointers=(0x1BC,))},
            externals={"datum_new": lambda harness, arguments, made=made: (made, tuple(arguments))},
        )

        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<I", objects, at, salted(random_.randrange(2)))
            objects[at + 4] = random_.choice((0, 1))
            struct.pack_into("<I", objects, at + 0x154, salted(random_.randrange(COUNT)))
        tags = bytearray(random_.randbytes(0x20 + 2 * 0x10))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x20 + tag * 0x10)
            tags[0x20 + tag * 0x10 + 2] = random_.choice((0, 0x10, 0x10))
        told = random_.randrange(2)
        t.check(
            "object_unknown_000b8ca0",
            salted(random_.randrange(COUNT)),
            globals={"object_header_data": Buffer(bytes(objects), pointers=table.pointers), "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18))},
            externals={"object_unknown_0010cf50": lambda harness, arguments, told=told: (told, tuple(arguments))},
        )

        # five nodes in a tree from node 0: each with its first child at 0x06 and the next at 0x08
        count = 5
        nodes = bytearray(random_.randbytes(0x50 + count * NODE))
        struct.pack_into("<iI", nodes, 0x48, random_.choice((3, 5)), 0x50)
        for node in range(count):
            struct.pack_into("<hh", nodes, 0x50 + node * NODE + 6, -1, -1)
        for node in range(1, count):
            parent = 0x50 + random_.randrange(node) * NODE
            struct.pack_into("<h", nodes, 0x50 + node * NODE + 8, struct.unpack_from("<h", nodes, parent + 6)[0])
            struct.pack_into("<h", nodes, parent + 6, node)
        t.check(
            "unknown_0016d8d0",
            Buffer(bytes(nodes), pointers=(0x4C,)),
            Buffer(struct.pack("<13f", *(random_.uniform(-2, 2) for _ in range(13)))),
            0,
            random_.choice((2, 5, 9)),
            Buffer(struct.pack(f"<{13 * count}f", *(random_.uniform(-2, 2) for _ in range(13 * count)))),
        )
