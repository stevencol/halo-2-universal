# fourteen functions of objects that call ones that are known

import random
import struct

from difftest import Buffer

from test_lights_walks import given, lights
from test_object_types_more import after
from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
PLAYERS, PLAYER = 3, 0x21C
EFFECTS, EFFECT = 3, 0x190
TAGS, TAG = 4, 0x40


def run(t) -> None:
    random_ = random.Random(732)

    for n in range(200):
        # an object for what stands in for the functions that look for one: it is not in the table
        unit = bytearray(random_.randbytes(0x400))
        unit[0x10A] = random_.choice((4, 4, 0, 0xFF, 0xFB))
        struct.pack_into("<I", unit, 0x13C, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(PLAYERS))))
        struct.pack_into("<I", unit, 0x2E0, random_.choice((NONE, 100, 970, 1000)))

        def found(*before: Buffer):
            """What stands in for a function that looks for an object: it is after these buffers, or nowhere."""
            there = 0 if random_.random() < 0.25 else after(*before)
            return lambda harness, arguments: (there, tuple(arguments))

        object_index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        the_unit = [Buffer(bytes(unit))]

        t.check(
            "unit_get_player_index",
            object_index,
            extra=the_unit,
            externals={"object_try_and_get_and_verify_type": found()},
        )

        there = bytearray(random_.randbytes(0x40))
        there[0] = random_.choice((0, 1, 1, 1))
        struct.pack_into("<h", there, 2, random_.choice((4, 4, 4, 3)))
        there = Buffer(bytes(there))
        t.check(
            "unknown_00510c6c_get_unit",
            globals={"unknown_00510c6c": there},
            extra=the_unit,
            externals={"object_try_and_get_and_verify_type": found(there)},
        )

        def some_objects() -> Buffer:
            table = objects(random_)
            data = bytearray(table.data)
            for one in range(COUNT):
                data[0x4C + one * 12 + 3] = random_.choice((0, 1, 5, 31, 37, 0xFF))
                struct.pack_into("<I", data, FIRST + one * OBJECT, random_.getrandbits(16) << 16 | random_.randrange(TAGS))
                struct.pack_into("<h", data, FIRST + one * OBJECT + 0x116, random_.choice((0x200, 0x300)))
            return Buffer(bytes(data), pointers=table.pointers)

        matrix = Buffer(random_.randbytes(0x34))
        table = some_objects()
        identity = Buffer(random_.randbytes(0x34))
        t.check(
            "object_get_first_node_matrix",
            matrix,
            object_index,
            globals={"object_header_data": table, "global_identity4x3": identity},
            extra=the_unit,
            externals={"object_try_and_get_and_verify_type": found(matrix, table, identity)},
        )

        time = bytearray(random_.randbytes(0x24))
        struct.pack_into("<h", time, 2, random_.choice((30, 60)))
        struct.pack_into("<I", time, 8, random_.choice((0, 1000, 1030, 1031, 1060, 5000)))
        time = Buffer(bytes(time))
        t.check(
            "biped_unknown_00175f50",
            object_index,
            globals={"game_time_globals": time},
            extra=the_unit,
            externals={"object_try_and_get_and_verify_type": found(time)},
        )

        thing = Buffer(random_.randbytes(4) + struct.pack("<I", object_index))
        table = some_objects()
        t.check(
            "unknown_object_is_of_types",
            thing,
            random_.choice((0, 0xFFFFFFFF, 1, 2, 0x20, 0x23, random_.getrandbits(32))),
            globals={"object_header_data": table},
            extra=the_unit,
            externals={"object_get_and_verify_type": found(thing, table)},
        )

        says = random_.choice((0, 1))
        t.check(
            "object_unknown_0003d7c0",
            object_index,
            random_.choice((0, Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4)))),
            globals={"unknown_004c1bd0": random_.choice((object_index, object_index, random_.getrandbits(32)))},
            externals={"object_unknown_000b9ce0": lambda harness, arguments, says=says: (says, tuple(arguments))},
        )

        # what stands in for the walk through the objects: some, and then no more
        walked = [random_.getrandbits(32) for _ in range(random_.choice((0, 1, 3)))]

        def next_(harness, arguments, walked=walked):
            calls = sum(1 for call in harness._calls if call[0] == "object_iterator_next")
            start = harness.mu.mem_read(arguments[0], 16)
            record = (bytes(start[:5]), bytes(start[6:8]), bytes(start[12:16])) + ((bytes(start[8:12]),) if calls == 0 else ())
            if calls < len(walked):
                harness.mu.mem_write(arguments[0] + 8, struct.pack("<I", walked[calls]))
                return 1, record
            return 0, record

        each = lambda harness, arguments: (0, tuple(arguments))
        t.check("objects_get_first_of_type5", Buffer(random_.randbytes(4)), externals={"object_iterator_next": next_})
        t.check(
            "objects_call_unknown58",
            random_.getrandbits(32),
            externals={"object_iterator_next": next_, "object_types_call_unknown58": each},
        )
        t.check(
            "objects_call_unknown5C",
            random_.getrandbits(32),
            externals={"object_iterator_next": next_, "object_types_call_unknown5C": each},
        )

        # an effect whose object is a unit of a player
        effects = bytearray(random_.randbytes(0x4C + EFFECTS * EFFECT))
        struct.pack_into("<I", effects, 0x44, 0x4C)
        effects = Buffer(bytes(effects), pointers=(0x44,))
        players = bytearray(random_.randbytes(0x4C + PLAYERS * PLAYER))
        struct.pack_into("<I", players, 0x44, 0x4C)
        for index in range(PLAYERS):
            struct.pack_into("<h", players, 0x4C + index * PLAYER + 0x28, random_.choice((-1, 0, 1)))
        players = Buffer(bytes(players), pointers=(0x44,))
        t.check(
            "effect_unknown_0017afd0",
            random_.getrandbits(16) << 16 | random_.randrange(EFFECTS),
            random_.choice((NONE, 0, 1, 0xFFFF)),
            globals={"effect_data": effects, "player_data": players},
            extra=the_unit,
            externals={"object_try_and_get_and_verify_type": found(effects, players)},
        )

        t.check(
            "object_set_unknownD4",
            object_index,
            random_.getrandbits(32),
            globals={"object_header_data": some_objects()},
            externals={"object_types_call_unknown50": each},
        )
        t.check(
            "objects_compact_memory_pool",
            *(random_.getrandbits(32) for _ in range(6)),
            globals={"unknown_004de2ec": random_.getrandbits(32)},
            externals={"memory_pool_compact": each},
        )

        # the tags: each names another at 0x38, and that one two more at 4 and 0x14
        tags = bytearray(random_.randbytes(TAGS * 16 + TAGS * TAG))
        pointers = []
        for index in range(TAGS):
            at = TAGS * 16 + index * TAG
            struct.pack_into("<I", tags, index * 16 + 8, at)
            pointers.append(index * 16 + 8)
            struct.pack_into("<I", tags, at + 0x38, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(TAGS), random_.getrandbits(16) << 16 | random_.randrange(TAGS))))
            for offset in (4, 0x14):
                struct.pack_into("<I", tags, at + offset, random_.choice((NONE, 7, 7, 7)))
        count = random_.getrandbits(32)
        place = random_.getrandbits(32)

        def counted(harness, arguments, count=count, place=place):
            harness.mu.mem_write(arguments[1], struct.pack("<I", count))
            return place, (arguments[0],)

        t.check(
            "object_unknown_000be1d0",
            object_index,
            globals={"object_header_data": some_objects(), "global_tag_instances": Buffer(bytes(tags), pointers=tuple(pointers))},
            externals={"object_get_unknown116_and_count": counted, "object_types_call_unknown74": each},
        )

        table = lights(random_)
        maximum = struct.unpack_from("<i", table.data, 0x20)[0]
        t.check(
            "light_unknown_000c3220",
            random_.getrandbits(16) << 16 | random_.randrange(maximum),
            globals={"light_data": table},
            externals={"cluster_partition_remove": given},
        )
