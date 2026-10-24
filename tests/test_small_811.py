# four medium functions of the way to the main menu: the next element of an
# iterator of the AI, who holds what an object is held by, a real of an
# object between two by the time, and four reals of two ranges and a depth

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
BIG = 0x260


def run(t) -> None:
    random_ = random.Random(811)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        # a data array of elements of 0x10 bytes and an iterator over it
        count = random_.choice((0, 2, 4, 6))
        array = bytearray(random_.randbytes(0x50 + 6 * 0x10 + 4))
        struct.pack_into("<ii", array, 0x20, 6, 0x10)
        struct.pack_into("<i", array, 0x38, count)
        struct.pack_into("<II", array, 0x44, 0x50, 0x50 + 6 * 0x10)
        struct.pack_into("<I", array, 0x50 + 6 * 0x10, random_.getrandbits(6))
        for index in range(6):
            array[0x50 + index * 0x10 + 9] = random_.choice((0, 1))
        iterator = bytearray(random_.randbytes(0x18))
        struct.pack_into("<IIii", iterator, 0, 0, 0x18, 0, random_.choice((-1, -1, 0, 2, 5)))
        iterator[0x10] = random_.choice((0, 1))
        data = Buffer(bytes(iterator) + bytes(array), pointers=(4, 0x18 + 0x44, 0x18 + 0x48), places=(0,))
        data_ = bytearray(data.data)
        struct.pack_into("<II", data_, 0x18 + 0x44, 0x18 + 0x50, 0x18 + 0x50 + 6 * 0x10)
        t.check(
            "ai_iterator_next_unknown_001e46c0",
            Buffer(bytes(data_), pointers=(4, 0x18 + 0x44, 0x18 + 0x48), places=(0,)),
            globals={"ai_globals": Buffer(bytes((0, random_.choice((0, 1, 1)))) + random_.randbytes(14))},
        )

        table = some_objects(random_)
        headers = bytearray(table.data[:FIRST])
        objects = bytearray()
        for index in range(COUNT):
            struct.pack_into("<I", headers, 0x4C + index * 12 + 8, FIRST + index * BIG)
            headers[0x4C + index * 12 + 3] = random_.choice((0, 1, 2))
            one = bytearray(random_.randbytes(BIG))
            struct.pack_into("<I", one, 0, salted(random_.randrange(4)))
            struct.pack_into("<I", one, 0x14, random_.choice((NONE, salted(random_.randrange(COUNT)))))
            struct.pack_into("<I", one, 0x24C, random_.choice((NONE, 9)))
            objects += one
        tags = bytearray(random_.randbytes(0x40 + 4 * 0xC0))
        for tag in range(4):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x40 + tag * 0xC0)
            struct.pack_into("<I", tags, 0x40 + tag * 0xC0 + 0xBC, random_.choice((0, 1 << 26)))
        object_data = Buffer(bytes(headers) + bytes(objects), pointers=table.pointers)
        found = random_.choice((0, 1, 1))
        t.check(
            "unit_unknown_00101fb0",
            salted(random_.randrange(COUNT)),
            globals={
                "object_header_data": object_data,
                "global_tag_instances": Buffer(bytes(tags), pointers=tuple(tag * 0x10 + 8 for tag in range(4))),
            },
            externals={
                "object_try_and_get_and_verify_type": lambda harness, arguments, object_data=object_data, found=found: (
                    object_data.address + FIRST + (arguments[0] & 0xFFFF) * BIG if found else 0,
                    tuple(arguments),
                )
            },
        )

        globals_ = bytearray(random_.randbytes(0x304))
        object_index = salted(random_.randrange(COUNT))
        for entry in range(32):
            struct.pack_into("<I", globals_, 0x280 + entry * 4, random_.choice((NONE, object_index, object_index, 5)))
            struct.pack_into("<IIIii", globals_, entry * 0x14, random_.choice((1, 2)), f32(random_.uniform(-5, 5)), f32(random_.uniform(-5, 5)),
                             random_.choice((0, 90, 100)), random_.choice((0, 10, 30)))
        time = bytearray(random_.randbytes(0x24))
        struct.pack_into("<I", time, 8, 100)
        t.check(
            "object_unknown_0010a980_get_real",
            random_.choice((NONE, object_index)),
            random_.choice((1, 2, 3)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            globals={"object_unknown_0010a980_globals": Buffer(bytes(globals_)), "game_time_globals": Buffer(bytes(time))},
        )

        t.check(
            "rasterizer_unknown_000350e0",
            Buffer(struct.pack("<4I", *(f32(random_.uniform(-2, 2)) for _ in range(4)))),
            Buffer(random_.randbytes(16)),
            Buffer(struct.pack("<2I", f32(random_.uniform(0, 1)), f32(random_.uniform(0, 1)))),
            f32(random_.choice((0.1, 1.0, 50.0, 1000.0, 5000.0, -1.0))),
            globals={
                "rasterizer_unknown_00485ad4": f32(random_.choice((0.06, 0.1, 1.0, -1.0))),
                "rasterizer_unknown_00485ad8": f32(random_.choice((100.0, 1024.0, 4096.0))),
            },
        )
