# four medium functions of the way to the main menu: an effect taken out of
# the entries of its object, whether a point is in the world, one of the
# things of Havok made from its values, and whether an object is seen by
# some clusters

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
BIG = 0x200
EFFECT = 0x50


def run(t) -> None:
    random_ = random.Random(805)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        # effects (a data array of 0x50 bytes) and an object with entries at its short at 0x11E
        effects = bytearray(random_.randbytes(0x50 + 4 * EFFECT))
        struct.pack_into("<i", effects, 0x24, EFFECT)
        struct.pack_into("<i", effects, 0x38, random_.choice((2, 4)))
        struct.pack_into("<I", effects, 0x44, 0x50)
        salts = [random_.choice((0, 0x1234)) for _ in range(4)]
        object_found = random_.random() < 0.8
        for index in range(4):
            struct.pack_into("<H", effects, 0x50 + index * EFFECT, salts[index])
            struct.pack_into("<I", effects, 0x50 + index * EFFECT + 0x48, 7)
        effect = random_.choice((NONE, salts[0] << 16 | 0, salts[1] << 16 | 1, 0x5555 << 16 | 2, salts[3] << 16 | 3))
        object_ = bytearray(random_.randbytes(BIG))
        struct.pack_into("<hh", object_, 0x11C, random_.choice((0, 8, 24, 32, -8)), 0x140)
        for entry in range(4):
            object_[0x140 + entry * 8] = random_.choice((2, 2, 1))
            struct.pack_into("<I", object_, 0x140 + entry * 8 + 4, random_.choice((effect, effect, 5)))
        # the object is kept after the effects, in the same buffer
        at = len(effects)
        data_array = Buffer(bytes(effects) + bytes(object_), pointers=(0x44,))
        t.check(
            "effect_unknown_00177590",
            effect,
            globals={"effect_data": data_array},
            externals={
                "object_try_and_get_and_verify_type": lambda harness, arguments, data_array=data_array, at=at, object_found=object_found: (
                    data_array.address + at if object_found else 0,
                    tuple(arguments),
                )
            },
        )

        point = [f32(random_.choice((0.0, 1.5, -50000.0, 50000.0, 50001.0, -50001.0, random_.uniform(-60000, 60000)))) for _ in range(3)]
        if n % 7 == 0:
            point[random_.randrange(3)] = random_.choice((0x7F800000, 0xFF800000, 0x7FC00000))
        t.check("valid_world_real_point3d", Buffer(struct.pack("<3I", *point)))

        t.check(
            "havok_unknown_001c6470",
            Buffer(random_.randbytes(0x20)),
            random_.choice((NONE, 0, 5)),
            random_.getrandbits(32),
            random_.choice((NONE, 0xFFFF0003, 0x00020003, random_.getrandbits(32))),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
        )

        table = some_objects(random_)
        headers = bytearray(table.data)
        for index in range(COUNT):
            headers[0x4C + index * 12 + 3] = random_.choice((0, 1, 7, 7, 9, 12))
            struct.pack_into("<h", headers, 0x4C + index * 12 + 4, random_.choice((-1, 0, 5, 33, 63)))
            struct.pack_into("<I", headers, FIRST + index * OBJECT + 4, random_.choice((0, 2, 0xFFFFFFFD)))
        said = random_.choice((0, 1))
        value = random_.choice((0, 1))

        def told(harness, arguments, said=said, value=value):
            harness.mu.mem_write(arguments[1], bytes((value,)))
            return said, (arguments[0], harness.mu.mem_read(arguments[2], 8))

        t.check(
            "object_unknown_000b6d60",
            salted(random_.randrange(COUNT)),
            Buffer(random_.randbytes(8)),
            globals={"object_header_data": Buffer(bytes(headers), pointers=table.pointers)},
            externals={"object_types_call_unknown48": told},
        )
