# five medium functions of the way to the main menu: the table of
# unknown_0051f40c made ready, one more entry of those of the rasterizer, the
# next bit of a list, and a particle system added to and taken from a list

import random
import struct

from difftest import Buffer, Pointer

NONE = 0xFFFFFFFF
SYSTEM = 0x54


def run(t) -> None:
    random_ = random.Random(804)

    def word(value: int) -> int:
        return value & 0xFFFFFFFF

    def systems(count: int) -> Buffer:
        data = bytearray(random_.randbytes(0x50 + count * SYSTEM))
        struct.pack_into("<I", data, 0x44, 0x50)
        for index in range(count):
            struct.pack_into("<H", data, 0x50 + index * SYSTEM, random_.choice((0x1234, 0x5678, 0x8001)))
        return data

    for n in range(400):
        last = random_.choice((0, 1, 5, 6))
        t.check(
            "unknown_0051f40c_update",
            globals={
                "rasterizer_display_initialized": word(random_.choice((-1, 0, 5, 6, 7))),
                "unknown_00485898": random_.choice((0, 3, 4, 5, 7, 8)),
                "unknown_005234b0": last,
                "unknown_0051f408": bytes((random_.choice((0, 1)),)),
                "unknown_0051f409": bytes((random_.choice((0, 1)),)),
                "unknown_005234c4": random_.choice((NONE, 0, 3)),
                "unknown_0051f40c": random_.randbytes(0x3FE0),
            },
        )

        data = bytearray(random_.randbytes(0x2D8))
        index = random_.randrange(16)
        struct.pack_into("<i", data, 0x2D0, index)
        values = [random_.choice((0, 1, 2)) for _ in range(4)]
        struct.pack_into("<3i", data, 0x210 + index * 12, *(random_.choice((value, value, 9)) for value in values[:3]))
        struct.pack_into("<i", data, 0xCC + index * 4, random_.choice((values[3], 9)))
        t.check("rasterizer_unknown_0001c620", Buffer(bytes(data)), values[1], values[0], values[2], values[3])

        count = random_.choice((0, 1, 3, 4))
        array = struct.pack("<4I", *(random_.choice((0, 0, 1, 0x80000000, random_.getrandbits(32))) for _ in range(4)))
        iterator = struct.pack("<hhII", count, random_.choice((-1, 0, 1, 3)), random_.choice((0, 0, 4, random_.getrandbits(32))), 0x10)
        t.check("bit_vector_iterator_next", Buffer(iterator + array, pointers=(8,)), Buffer(random_.randbytes(4)))

        # a list of the particle systems: each has the one before at 0x3C and after at 0x38
        table = systems(4)
        order = random_.sample(range(4), random_.choice((0, 1, 2, 3)))
        salts = [struct.unpack_from("<H", table, 0x50 + index * SYSTEM)[0] for index in range(4)]
        ids = [salts[index] << 16 | index for index in range(4)]
        for index in range(4):
            struct.pack_into("<II", table, 0x50 + index * SYSTEM + 0x38, NONE, NONE)
        for position, index in enumerate(order):
            before = ids[order[position - 1]] if position else NONE
            after = ids[order[position + 1]] if position + 1 < len(order) else NONE
            struct.pack_into("<II", table, 0x50 + index * SYSTEM + 0x38, after, before)
        first = ids[order[0]] if order else NONE
        last_ = ids[order[-1]] if order else NONE
        outside = [index for index in range(4) if index not in order] or [random_.randrange(4)]

        data_array = Buffer(bytes(table), pointers=(0x44,))
        element = random_.choice(outside)
        t.check(
            "particle_system_list_add",
            Pointer(data_array, 0x50 + element * SYSTEM),
            Buffer(struct.pack("<I", last_)),
            Buffer(struct.pack("<I", first)),
            globals={"particle_system_data": data_array},
        )

        data_array = Buffer(bytes(table), pointers=(0x44,))
        element = random_.choice(order) if order else random_.randrange(4)
        t.check(
            "particle_system_list_remove",
            Pointer(data_array, 0x50 + element * SYSTEM),
            Buffer(struct.pack("<I", random_.choice((last_, last_, ids[element])))),
            Buffer(struct.pack("<I", random_.choice((first, first, ids[element])))),
            globals={"particle_system_data": data_array},
        )
