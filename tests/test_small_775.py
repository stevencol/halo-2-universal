# five small functions: two more of the values of a local player
# (0x0051e994), whether an object goes fast enough, and whether an object
# can be taken hold of (it seems)

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
PLAYER = 0x2F0
OBJECTS, OBJECT = 4, 0x140
TAGS = 4


def run(t) -> None:
    random_ = random.Random(775)

    def player() -> Buffer:
        data = bytearray(random_.randbytes(PLAYER))
        first = random_.choice((0, 0, 1, 5, 15, 16, random_.randrange(17)))
        second = random_.choice((0, 0, 1, 3, 15 - first, 16 - first, random_.randrange(17)))
        struct.pack_into("<ii", data, 0x2E8, first, max(second, 0))
        return Buffer(bytes(data))

    def some_object() -> bytearray:
        data = bytearray(random_.randbytes(OBJECT))
        struct.pack_into("<I", data, 0, random_.getrandbits(16) << 16 | random_.randrange(TAGS))
        data[0xAA] = random_.choice((0, 1, 1, 2, 3, 12, 12, 13))
        data[0xC1] = random_.choice((0, 1, 0xFE, 0xFF))
        data[0x10A] = random_.choice((0, 4, 3, 0xFB, 0xFF))
        struct.pack_into("<I", data, 0x13C, random_.choice((NONE, NONE, 0, 5)))
        return data

    def real() -> float:
        return random_.choice((0.0, 1.0, -1.0, 3.0, 5.0, 10.0, float("nan"), random_.uniform(-20, 20)))

    for n in range(600):
        t.check("game_unknown_001a1c8e_add_first_value", player(), random_.getrandbits(32))
        t.check(
            "game_unknown_001a1c8e_remove_value",
            player(),
            random_.choice((NONE, 0x80000000, 0, 1, 4, 15, 16, 17, random_.randrange(32))),
        )

        # the objects: a header of 0x4C bytes with a pointer to the entries
        # at 0x44, entries of 12 bytes with a pointer to the object at 8,
        # and the objects
        entries_at = 0x4C
        objects_at = entries_at + OBJECTS * 12
        table = bytearray(random_.randbytes(objects_at + OBJECTS * OBJECT))
        struct.pack_into("<I", table, 0x44, entries_at)
        pointers = [0x44]
        for index in range(OBJECTS):
            struct.pack_into("<I", table, entries_at + index * 12 + 8, objects_at + index * OBJECT)
            pointers.append(entries_at + index * 12 + 8)
            table[objects_at + index * OBJECT : objects_at + (index + 1) * OBJECT] = some_object()
        globals_ = bytearray(random_.randbytes(0x240))
        struct.pack_into("<I", globals_, 0x134, 0x200)
        minimum = random_.choice((0.0, 2.0, 4.0, 9.0, float("nan")))
        struct.pack_into("<f", globals_, 0x23C, minimum)
        other = bytearray(random_.randbytes(0x298))
        struct.pack_into("<f", other, 0x294, random_.choice((0.0, 1.0, 4.0, 9.0, float("nan"))))
        velocity = struct.pack("<fff", real(), real(), real())
        if random_.random() < 0.3:
            # just faster than the number of the globals, but not by 1.05
            velocity = struct.pack("<fff", minimum * 1.02, 0.0, 0.0)

        def velocities(harness, arguments, velocity=velocity):
            harness.mu.mem_write(arguments[1], velocity)
            return 0, (arguments[0], arguments[2])

        t.check(
            "game_unknown_001a1c8e_object_fast_enough",
            random_.getrandbits(16) << 16 | random_.randrange(OBJECTS),
            globals={
                "object_header_data": Buffer(bytes(table), pointers=tuple(pointers)),
                "global_game_globals": Buffer(bytes(globals_), pointers=(0x134,)),
                "unknown_00510c94": Buffer(bytes(other)),
            },
            externals={"object_get_velocities": velocities},
        )

        thing = Buffer(bytes(some_object()))
        tags = bytearray(random_.randbytes(TAGS * 16 + TAGS * 0xC0))
        tag_pointers = []
        for index in range(TAGS):
            at = TAGS * 16 + index * 0xC0
            struct.pack_into("<I", tags, index * 16 + 8, at)
            struct.pack_into("<I", tags, at + 0xBC, random_.choice((0, 0x40, 0xFFFFFFBF, 0xFFFFFFFF, random_.getrandbits(32))))
            tag_pointers.append(index * 16 + 8)
        t.check(
            "game_unknown_001a1c8e_object_unknown_001a2084",
            random_.getrandbits(32),
            extra=[thing],
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=tuple(tag_pointers))},
            externals={
                "object_try_and_get_and_verify_type": lambda harness, arguments, there=random_.choice((0, 1, 1, 1)), thing=thing: (
                    thing.address if there else 0,
                    tuple(arguments),
                ),
            },
        )
