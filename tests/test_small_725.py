# thirteen more small functions of actors

import random
import struct

from difftest import Buffer

from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
ACTORS, ACTOR = 4, 0x888
SQUADS, SQUAD = 3, 0x98
SWARMS, SWARM = 3, 0x34
CLUMPS, CLUMP = 3, 0x50


def run(t) -> None:
    random_ = random.Random(725)

    for n in range(200):
        # the actors are all in one list, in an order of its own
        order = list(range(ACTORS))
        random_.shuffle(order)
        salts = [random_.getrandbits(16) for _ in range(ACTORS)]
        whole = lambda index: salts[index] << 16 | index

        def an_object() -> int:
            return random_.getrandbits(16) << 16 | random_.randrange(COUNT)

        def actors() -> Buffer:
            data = bytearray(random_.randbytes(0x4C + ACTORS * ACTOR))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for place, index in enumerate(order):
                at = 0x4C + index * ACTOR
                later = whole(order[place + 1]) if place + 1 < ACTORS else NONE
                struct.pack_into("<I", data, at + 0x20, later)
                data[at + 7] = random_.choice((0, 0, 1))
                struct.pack_into("<I", data, at + 0x18, an_object())
                struct.pack_into("<I", data, at + 0x1C, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(SWARMS))))
                struct.pack_into("<I", data, at + 0x7C, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(CLUMPS))))
                for element in range(3):
                    struct.pack_into("<h", data, at + 0x1C4 + element * 12, random_.choice((-1, 0, 7)))
                    struct.pack_into("<h", data, at + 0x1C4 + element * 12 + 8, random_.choice((-3, 0, 1, 2, 50)))
                data[at + 0x225] = random_.choice((0, 1))
                data[at + 0x229] = random_.choice((0, 0, 1))
                struct.pack_into("<I", data, at + 0x26C, random_.choice((NONE, an_object())))
                struct.pack_into("<I", data, at + 0x338, random_.choice((NONE, random_.getrandbits(32))))
                data[at + 0x622] = random_.choice((0, 1, 1))
                for offset in (0x624, 0x628, 0x62C, 0x630):
                    struct.pack_into("<f", data, at + offset, random_.uniform(-8.0, 8.0))
            return Buffer(bytes(data), pointers=(0x44,))

        def some_objects() -> Buffer:
            table = objects(random_)
            data = bytearray(table.data)
            for one in range(COUNT):
                data[FIRST + one * OBJECT + 0x34C] = random_.choice((0, 0, 1, 0xFF))
            return Buffer(bytes(data), pointers=table.pointers)

        def table(count: int, size: int) -> Buffer:
            data = bytearray(random_.randbytes(0x4C + count * size))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(count):
                struct.pack_into("<I", data, 0x4C + index * size + 0x68 % size, whole(random_.randrange(ACTORS)))
            return Buffer(bytes(data), pointers=(0x44,))

        def ai() -> Buffer:
            data = bytearray(random_.randbytes(0x20))
            data[1] = random_.choice((0, 1, 1, 1))
            struct.pack_into("<I", data, 0x14, whole(order[0]))
            return Buffer(bytes(data))

        actor_index = whole(random_.randrange(ACTORS))
        both = lambda: {"actor_data": actors(), "object_header_data": some_objects()}
        low = lambda: random_.getrandbits(24) << 8 | random_.choice((0, 1, 1))

        t.check("actor_set_unknown810_bit11", actor_index, low(), globals=both())
        t.check("actor_set_unknown225", actor_index, low(), globals=both())
        t.check("actor_unknown_001f5560", actor_index, Buffer(random_.randbytes(12)), globals={"actor_data": actors()})
        t.check("actor_unknown_00203fb0", actor_index, globals={"actor_data": actors(), "ai_globals": ai()})
        t.check(
            "actor_iterator_new",
            random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(SQUADS))),
            Buffer(random_.randbytes(12)),
            globals={"ai_globals": ai(), "squad_data": table(SQUADS, SQUAD)},
        )
        iterator = struct.pack("<3I", random_.getrandbits(32), random_.getrandbits(32), random_.choice((NONE, actor_index, actor_index)))
        t.check("actor_iterator_next", Buffer(iterator), globals={"actor_data": actors(), "ai_globals": ai()})
        t.check("actor_unknown_00255520", actor_index, random_.getrandbits(32), globals=both())
        t.check("actor_unknown_00255660", actor_index, Buffer(random_.randbytes(0x1C)), globals={"actor_data": actors()})
        t.check("actor_unknown_00257010", actor_index, random_.getrandbits(32), globals={"actor_data": actors()})
        t.check(
            "actor_unknown_002562c0",
            actor_index,
            random_.getrandbits(32),
            globals={"actor_data": actors(), "swarm_data": table(SWARMS, SWARM)},
        )
        t.check(
            "actor_unknown_002570b0",
            actor_index,
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={
                "actor_data": actors(),
                "unknown_005047f4": Buffer(struct.pack("<h", random_.choice((1, 1, 0, 2))) + random_.randbytes(6)),
                "unknown_00470c40": random_.getrandbits(32),
                "unknown_00470c44": random_.getrandbits(32),
            },
        )
        t.check(
            "actor_unknown_00257a40",
            actor_index,
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={
                "actor_data": actors(),
                "clump_data": table(CLUMPS, CLUMP),
                "game_time_globals": Buffer(random_.randbytes(0x24)),
            },
        )
        t.check("actor_unknown_0025aba0", actor_index, globals={"actor_data": actors()})
