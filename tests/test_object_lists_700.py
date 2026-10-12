# a list of objects is made, gets an object, loses one, and is walked

import random
import struct

from difftest import Buffer

from test_data import array

NONE = 0xFFFFFFFF
LISTS, OBJECTS = 16, 40


def lists(random_, full: bool = False) -> tuple:
    """The headers and the objects of some lists, each list with its own objects; and the objects of each."""
    count = LISTS if full else random_.choice((0, 1, 6, LISTS - 1))
    in_use = {index: random_.randint(-0x8000, 0x7FFF) or 1 for index in random_.sample(range(LISTS), count)}
    headers = array(random_, LISTS, 0xC, in_use)
    used = random_.choice((OBJECTS, OBJECTS - 3, 20))
    objects = array(random_, OBJECTS, 0xC, {index: random_.randint(1, 0x7FFF) for index in random_.sample(range(OBJECTS), used)})
    header_bytes = bytearray(headers.data)
    object_bytes = bytearray(objects.data)
    free = list(range(OBJECTS))
    random_.shuffle(free)
    things = {}
    for index in range(LISTS):
        some = [random_.getrandbits(16) << 16 | free.pop() for _ in range(random_.randint(0, 3)) if free]
        things[index] = [random_.getrandbits(32) for _ in some]
        for at, one in enumerate(some):
            struct.pack_into(
                "<2I", object_bytes, 0x4C + (one & 0xFFFF) * 0xC + 4, things[index][at], some[at + 1] if at + 1 < len(some) else NONE
            )
        struct.pack_into(
            "<hhI", header_bytes, 0x4C + index * 0xC + 4, random_.choice((0, 0, 1, 3)), len(some), some[0] if some else NONE
        )
    return (
        Buffer(bytes(header_bytes), pointers=headers.pointers),
        Buffer(bytes(object_bytes), pointers=objects.pointers),
        things,
    )


def run(t) -> None:
    random_ = random.Random(700)

    for n in range(300):
        headers, objects, things = lists(random_, full=n % 3 == 0)
        t.check("object_list_new", globals={"object_list_header_data": headers, "list_object_data": objects})

        headers, objects, things = lists(random_)
        list_index = random_.getrandbits(16) << 16 | random_.randrange(LISTS)
        t.check(
            "object_list_add",
            list_index,
            random_.getrandbits(32),
            globals={"object_list_header_data": headers, "list_object_data": objects},
        )

        headers, objects, things = lists(random_)
        mine = things[list_index & 0xFFFF]
        t.check(
            "object_list_remove",
            list_index,
            random_.choice(mine + mine + [random_.getrandbits(32)]),
            globals={"object_list_header_data": headers, "list_object_data": objects},
        )

        headers, objects, things = lists(random_)
        first = headers.data[0x4C + (list_index & 0xFFFF) * 0xC + 8 : 0x4C + (list_index & 0xFFFF) * 0xC + 12]
        t.check("object_list_get_next", Buffer(first + random_.randbytes(4)), globals={"list_object_data": objects})
