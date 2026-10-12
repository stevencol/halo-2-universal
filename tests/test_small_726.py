# twelve more small functions of actors

import random
import struct

from difftest import Buffer

from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
ACTORS, ACTOR = 4, 0x888
JOINTS, JOINT = 3, 0xBC
POINTS, POINT = 2, 0x484
SCRIPTS, SCRIPT = 3, 0xD4
CLUMPS, CLUMP = 3, 0x50
PROPS, PROP = 3, 0xC4


def run(t) -> None:
    random_ = random.Random(726)

    for n in range(200):
        # the actors are all in one list through 0x80, in an order of its own
        order = list(range(ACTORS))
        random_.shuffle(order)
        salts = [random_.getrandbits(16) for _ in range(ACTORS)]
        whole = lambda index: salts[index] << 16 | index
        joints = [random_.getrandbits(16) << 16 | index for index in range(JOINTS)]
        points = [random_.getrandbits(16) << 16 | index for index in range(POINTS)]
        number = random_.getrandbits(32)

        def actors() -> Buffer:
            data = bytearray(random_.randbytes(0x4C + ACTORS * ACTOR))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for place, index in enumerate(order):
                at = 0x4C + index * ACTOR
                struct.pack_into("<I", data, at + 0x80, whole(order[place + 1]) if place + 1 < ACTORS else NONE)
                struct.pack_into("<h", data, at + 4, random_.choice((15, 0, 3, 3)))
                data[at + 7] = random_.choice((0, 0, 1))
                data[at + 9] = random_.choice((0, 0, 0, 1))
                data[at + 0xA] = random_.choice((0, 1, 1))
                for element in range(4):
                    struct.pack_into(
                        "<hhI",
                        data,
                        at + 0x194 + element * 12,
                        random_.choice((-1, 0, 2)),
                        random_.randrange(4),
                        random_.choice(joints),
                    )
                data[at + 0x267] = random_.choice((0, 1))
                data[at + 0x268] = random_.choice((0, 1))
                struct.pack_into("<I", data, at + 0x338, random_.choice((number, random_.getrandbits(32))))
                struct.pack_into("<I", data, at + 0x3F4, random_.choice((NONE, *points)))
                if random_.random() < 0.5:
                    struct.pack_into("<I", data, at + 0x402 + random_.randrange(4) * 6, number)
                struct.pack_into("<h", data, at + 0x41C, random_.choice((1, 2, 3)))
                struct.pack_into("<h", data, at + 0x420, random_.choice((0, 1, 1, 2)))
                struct.pack_into("<I", data, at + 0x424, random_.choice((number, random_.getrandbits(32))))
                struct.pack_into("<h", data, at + 0x684, random_.choice((-1, 0, 0, 1)))
                for offset in (0x698, 0x69C):
                    struct.pack_into("<i", data, at + offset, random_.choice((-7, 0, 1, 2, 30, 100000)))
                data[at + 0x6F8] = random_.choice((0, 1, 1, 1))
                struct.pack_into("<I", data, at + 0x858, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(SCRIPTS))))
            return Buffer(bytes(data), pointers=(0x44,))

        def table(count: int, size: int, fill=None) -> Buffer:
            data = bytearray(random_.randbytes(0x4C + count * size))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(count):
                if fill:
                    fill(data, 0x4C + index * size)
            return Buffer(bytes(data), pointers=(0x44,))

        def a_script(data, at) -> None:
            struct.pack_into("<h", data, at + 0x30, random_.choice((0, 1, 2, -1)))
            struct.pack_into("<f", data, at + 0x60, random_.choice((0.0, -1.0, 0.5, 3.0, float("nan"))))

        def a_clump(data, at) -> None:
            data[at + 0x1C] = random_.choice((0, 1, 1, 1))
            struct.pack_into("<I", data, at + 0x18, random_.choice((NONE, whole(random_.randrange(ACTORS)))))

        def a_prop(data, at) -> None:
            struct.pack_into("<I", data, at + 0x1C, random_.choice((NONE, whole(random_.randrange(ACTORS)), whole(random_.randrange(ACTORS)))))
            data[at + 0x23] = random_.choice((0, 0, 1))
            data[at + 0x25] = random_.choice((0, 1, 1))

        def some_objects() -> Buffer:
            objects_ = objects(random_)
            data = bytearray(objects_.data)
            for one in range(COUNT):
                struct.pack_into("<I", data, FIRST + one * OBJECT + 0x12C, random_.choice((NONE, whole(random_.randrange(ACTORS)), whole(random_.randrange(ACTORS)))))
            return Buffer(bytes(data), pointers=objects_.pointers)

        actor_index = whole(random_.randrange(ACTORS))
        time = lambda: Buffer(random_.randbytes(2) + struct.pack("<h", random_.choice((30, 60, 0))) + random_.randbytes(0x20))

        t.check(
            "actor_unknown_0025bba0",
            actor_index,
            random_.getrandbits(32),
            globals={"actor_data": actors(), "game_time_globals": time()},
        )
        nothing = random_.choice((NONE, NONE, number))
        t.check(
            "actor_unknown_00262890",
            actor_index,
            random_.choice((number, number, nothing, random_.getrandbits(32))),
            globals={"actor_data": actors(), "unknown_00470fa0": nothing},
        )
        t.check(
            "actor_set_dynamic_firing_point",
            actor_index,
            random_.choice(points),
            globals={"actor_data": actors(), "dynamic_firing_point_data": table(POINTS, POINT)},
        )
        t.check("actor_find_unknown194", actor_index, random_.choice(joints), globals={"actor_data": actors()})
        t.check(
            "actor_unknown_0026ed40",
            actor_index,
            random_.getrandbits(16) << 16 | random_.randrange(4),
            globals={"actor_data": actors(), "joint_state_data": table(JOINTS, JOINT)},
        )
        t.check(
            "actor_unknown_0026ecc0",
            actor_index,
            Buffer(random_.randbytes(0x1C)),
            random_.getrandbits(16) << 16 | random_.randrange(4),
            globals={"actor_data": actors(), "joint_state_data": table(JOINTS, JOINT)},
        )
        t.check(
            "object_set_actor_unknownB",
            random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(COUNT), random_.getrandbits(16) << 16 | random_.randrange(COUNT))),
            random_.getrandbits(24) << 8 | random_.choice((0, 1, 0xFF)),
            globals={"actor_data": actors(), "object_header_data": some_objects()},
        )
        t.check("actor_unknown_00298b60", actor_index, globals={"actor_data": actors()})
        t.check("actor_unknown_00297600", actor_index, globals={"actor_data": actors()})
        t.check(
            "actor_unknown_0025aad0",
            actor_index,
            Buffer(random_.randbytes(4)),
            globals={"actor_data": actors(), "command_script_data": table(SCRIPTS, SCRIPT, a_script)},
        )
        t.check(
            "clump_unknown_002687a0",
            random_.getrandbits(16) << 16 | random_.randrange(CLUMPS),
            globals={"actor_data": actors(), "clump_data": table(CLUMPS, CLUMP, a_clump), "game_time_globals": time()},
        )
        thing = bytearray(random_.randbytes(0x18))
        struct.pack_into("<I", thing, 8, random_.getrandbits(16) << 16 | random_.randrange(PROPS))
        struct.pack_into("<I", thing, 0x14, random_.choice((NONE, random_.getrandbits(32))))
        t.check(
            "unknown_prop_unknown_0025da40",
            Buffer(bytes(thing)),
            globals={"actor_data": actors(), "prop_data": table(PROPS, PROP, a_prop)},
        )
