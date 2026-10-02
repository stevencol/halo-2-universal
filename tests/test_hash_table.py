# source/memory/hash_table.c
#
# The table, its lists and its entries are one buffer: the pointers of the
# lists are places in it. The functions that give the hash of a key and that
# compare two keys are the test's

import random
import struct

from difftest import Buffer, Mock, mock_address

HEADER = 0x3C
LIST_COUNT, MAXIMUM_COUNT, SIZE, HASH, COMPARE, ALLOCATION, FIRST_FREE = 0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x38
ENTRY = 0xC


def hash_of(key: int) -> int:
    # keys that differ in their high bits only have the same hash
    return (key & 0xFFFF) * 2654435761 & 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(84)

    def hashed(call) -> int:
        (key,) = call.stack
        return hash_of(key)

    def same(call) -> int:
        key, other = call.stack
        # not 1: any value with something in its low byte
        return 0x180 if key == other else 0x100

    mocks = [Mock(hashed, stack_arguments=1), Mock(same, stack_arguments=2)]

    def table(list_count: int, maximum_count: int, size: int, keys: list, free_order: list | None = None) -> tuple:
        """A table with some keys in it; and where each entry is."""
        entry_size = size + ENTRY
        entries = HEADER + 4 * list_count
        blob = bytearray(random_.randbytes(entries + entry_size * maximum_count + 8))
        pointers = []
        blob[0x1F] = 0
        struct.pack_into("<3I", blob, LIST_COUNT, list_count, maximum_count, size)
        struct.pack_into("<3I", blob, HASH, mock_address(0), mock_address(1), 0x80AA0000)
        lists = [0] * list_count
        places = {}
        order = list(range(maximum_count))
        random_.shuffle(order)
        for key, index in zip(keys, order):
            place = entries + index * entry_size
            list_ = hash_of(key) % list_count
            # the last that is added is the first of its list
            struct.pack_into("<3I", blob, place, key, hash_of(key), lists[list_])
            if lists[list_]:
                pointers.append(place + 8)
            lists[list_] = place
            places[key] = place
        for list_, place in enumerate(lists):
            struct.pack_into("<I", blob, HEADER + 4 * list_, place)
            if place:
                pointers.append(HEADER + 4 * list_)
        free = 0
        for index in order[len(keys) :]:
            place = entries + index * entry_size
            struct.pack_into("<I", blob, place + 8, free)
            if free:
                pointers.append(place + 8)
            free = place
        struct.pack_into("<I", blob, FIRST_FREE, free)
        if free:
            pointers.append(FIRST_FREE)
        return Buffer(bytes(blob), pointers=tuple(pointers)), places

    def some_keys(count: int) -> list:
        keys = set()
        while len(keys) < count:
            # some with the same hash, and some in the same list
            keys.add(random_.choice((random_.randrange(40), random_.randrange(40) | 0x10000, random_.getrandbits(32))))
        return list(keys)

    # ---------- a new table

    for n in range(40):
        maximum_count = random_.choice((0, 1, 5, 20))
        size = random_.choice((0, 4, 7, 12))
        list_count = random_.choice((1, 3, 16))
        room = HEADER + 4 * list_count + (size + ENTRY) * maximum_count
        memory = Buffer(random_.randbytes(room + 8))
        allocator = Buffer(struct.pack("<3I", 4, mock_address(0), 0x11111111), pointers=(0,))
        gives = n % 8 != 7

        t.check(
            "hash_table_new",
            allocator,
            maximum_count,
            Buffer(random_.choice((b"font characters", b"", b"a name that is longer than thirty-one characters")) + bytes(1)),
            size,
            list_count,
            0x80111100,
            0x80222200,
            mocks=[Mock(lambda call, memory=memory, gives=gives: memory.address if gives else 0, stack_arguments=1, registers=("ecx",))],
            extra=[memory],
        )

    # ---------- an empty table

    for n in range(40):
        maximum_count = random_.choice((0, 1, 2, 5, 20))
        data, places = table(random_.choice((1, 3, 16)), maximum_count, random_.choice((0, 3, 4, 12)), some_keys(random_.randrange(maximum_count + 1)))
        t.check("hash_table_clear", data)

    # ---------- adding, finding and removing

    for n in range(300):
        maximum_count = random_.choice((1, 2, 5, 12))
        size = random_.choice((0, 3, 4, 12))
        list_count = random_.choice((1, 2, 3, 16))
        keys = some_keys(random_.randrange(maximum_count + 1))
        # a key that is there, one that is not, one with the hash of one that
        # is, and one in the list of one that is
        asked = [random_.getrandbits(32), random_.randrange(40)]
        if keys:
            asked += [random_.choice(keys), random_.choice(keys), random_.choice(keys) ^ 0x10000, random_.choice(keys) ^ 0x80000000]
        key = random_.choice(asked)
        data, places = table(list_count, maximum_count, size, keys)
        t.check("hash_table_find", data, key, mocks=mocks)
        data, places = table(list_count, maximum_count, size, keys)
        t.check("hash_table_remove", data, key, mocks=mocks)
        data, places = table(list_count, maximum_count, size, keys)
        t.check("hash_table_add", data, key, Buffer(random_.randbytes(size + 4)), mocks=mocks)
