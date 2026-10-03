# source/objects/objects.c

import random
import struct

from difftest import Buffer
from test_data import DATA_ARRAY_SIZE, NONE, identifier, index

HEADER_SIZE = 0xC
OBJECT_SIZE = 0x200
NODE_MATRICES_OFFSET = 0x120
MATRIX_SIZE = 0x34
NODE_COUNT = 4


def world(random_: random.Random, object_count: int, maximum_count: int) -> tuple[Buffer, list[int]]:
    """The data array of the object headers, with its objects after it in the buffer, and the index of each object.

    An object can be attached to an object before it; each object has the
    matrices of some nodes.
    """
    headers_offset = DATA_ARRAY_SIZE
    bits_offset = headers_offset + maximum_count * HEADER_SIZE
    objects_offset = (bits_offset + ((maximum_count + 31) >> 5) * 4 + 15) & ~15
    places = sorted(random_.sample(range(maximum_count), object_count))

    array = bytearray(DATA_ARRAY_SIZE)
    array[0:7] = b"object\0"
    struct.pack_into(
        "<2iBBHIIiiihHII",
        array,
        0x20,
        maximum_count,
        HEADER_SIZE,
        0,
        1,
        0,
        0x64407440,
        0,
        0,
        max(places, default=-1) + 1,
        object_count,
        -0x7F00,
        0,
        headers_offset,
        bits_offset,
    )

    headers = bytearray(random_.randbytes(maximum_count * HEADER_SIZE))
    for n in range(maximum_count):
        struct.pack_into("<h", headers, n * HEADER_SIZE, 0)
    bits = bytearray(objects_offset - bits_offset)
    objects = bytearray()
    indices: list[int] = []
    pointers = [0x44, 0x48]
    for number, place in enumerate(places):
        identifier_ = identifier(random_)
        indices.append(index(place, identifier_))
        # the header: identifier, flags, type, cluster, size of the data, the data
        struct.pack_into(
            "<hBBhhI",
            headers,
            place * HEADER_SIZE,
            identifier_,
            random_.getrandbits(8),
            random_.randrange(0, 12),
            random_.randrange(0, 50),
            OBJECT_SIZE,
            objects_offset + number * OBJECT_SIZE,
        )
        pointers.append(headers_offset + place * HEADER_SIZE + 8)
        bits[place >> 3] |= 1 << (place & 7)

        datum = bytearray(random_.randbytes(OBJECT_SIZE))
        parent = random_.choice(indices[:-1]) if indices[:-1] and random_.random() < 0.6 else NONE
        struct.pack_into("<ib", datum, 0x14, parent, random_.randrange(0, NODE_COUNT))
        struct.pack_into("<3f", datum, 0x64, *(random_.uniform(-40, 40) for _ in range(3)))
        struct.pack_into("<h", datum, 0x116, NODE_MATRICES_OFFSET)
        for node in range(NODE_COUNT):
            matrix = [random_.choice((1.0, 1.0, 0.5, 2.5)), *(random_.uniform(-1, 1) for _ in range(9))]
            matrix += [random_.uniform(-40, 40) for _ in range(3)]
            struct.pack_into("<13f", datum, NODE_MATRICES_OFFSET + node * MATRIX_SIZE, *matrix)
        objects += datum

    return Buffer(bytes(array) + bytes(headers) + bytes(bits) + bytes(objects), pointers=tuple(pointers)), indices


def run(t) -> None:
    random_ = random.Random(8)

    for _ in range(40):
        maximum_count = random_.choice((4, 20, 64))
        object_count = random_.randint(1, maximum_count)
        data, indices = world(random_, object_count, maximum_count)
        globals_ = {"object_header_data": data}

        for object_index in random_.sample(indices, min(len(indices), 8)):
            t.check("object_wake", object_index, globals=globals_)
            t.check("object_get_world_matrix", object_index, Buffer(random_.randbytes(0x34), format="<13f"), globals=globals_)
            t.check("object_get_location", object_index, Buffer(bytes(8)), globals=globals_)
            t.check("object_get_ultimate_parent", object_index, globals=globals_)
            t.check("object_get_origin", object_index, Buffer(struct.pack("<3f", 9, 9, 9), format="<3f"), globals=globals_)
            for type_mask in (0xFFFFFFFF, 0, 1, 0x555, 0xAAA, random_.getrandbits(12)):
                t.check("object_try_and_get_and_verify_type", object_index, type_mask, globals=globals_)
                t.check("object_get_and_verify_type", object_index, type_mask, globals=globals_)
            t.check("object_get_first_unflagged_ancestor", object_index, globals=globals_)
            # the salt is not looked at: the absolute index alone, which can be 0
            t.check("object_get_first_unflagged_ancestor", object_index & 0xFFFF, globals=globals_)

        # an iterator, from the start and from any header
        for absolute_index in (0, 1, maximum_count - 1, maximum_count, *random_.sample(range(maximum_count), 3)):
            for type_mask, header_flags in ((0xFFFFFFFF, 0), (0x0F3, 0), (0xFFFFFFFF, 1), (0xFFF, 0x21), (0, 0)):
                iterator = struct.pack("<IBBhi", type_mask, header_flags, 0xCC, absolute_index, 0x55555555)
                t.check("object_iterator_next", Buffer(iterator), globals=globals_)

        # objects that are not there
        for object_index in (NONE, 0, index(maximum_count + 3, 5), indices[0] ^ 0x10000, indices[-1] ^ 0x7FFF0000):
            t.check("object_try_and_get_and_verify_type", object_index, 0xFFFFFFFF, globals=globals_)
            t.check("object_get_and_verify_type", object_index, 0xFFFFFFFF, globals=globals_)
        t.check("object_get_first_unflagged_ancestor", NONE, globals=globals_)
        t.check("object_get_ultimate_parent", NONE, globals=globals_)

    # ---------- a new iterator

    for type_mask in (0, 1, 0xFFFFFFFF, 0x0F3, 0x80000000):
        for header_flags in (0, 1, 0x21, 0xFF, 0x100, 0xABCDEF21):
            t.check("object_iterator_new", Buffer(random_.randbytes(0x14)), type_mask, header_flags)
