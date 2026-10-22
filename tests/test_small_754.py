# five small functions of what a search of games of the online service found:
# the identifiers already seen, seeing a result, the counts and the best result

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
HEADER = 0xA4
RESULT = 0x204
RESULTS = 50
IDENTIFIERS = 16


def search(random_: random.Random, active: int = 1) -> tuple[bytes, tuple, list]:
    """A search, its 50 results and its list of identifiers, in one buffer;
    and the identifiers of the results."""
    data = bytearray(random_.randbytes(HEADER + RESULTS * RESULT + IDENTIFIERS * 8))
    data[0] = active
    data[0x14] = random_.choice((0, 1))
    struct.pack_into("<i", data, 0x1C, random_.choice((-1, 0, 3, random_.randrange(100))))
    struct.pack_into("<i", data, 0x34, random_.choice((0, 5, 10, 100)))
    struct.pack_into("<I", data, 0x8C, HEADER)
    count = random_.choice((0, 1, 3, 8, IDENTIFIERS))
    maximum = random_.choice((count, count + 1, IDENTIFIERS))
    struct.pack_into("<iiI", data, 0x94, count, maximum, 0)
    struct.pack_into("<I", data, 0xA0, HEADER + RESULTS * RESULT)
    identifiers = []
    for index in range(RESULTS):
        at = HEADER + index * RESULT
        data[at] = random_.choice((0, 1, 1))
        data[at + 1] = random_.choice((0, 1, 1))
        data[at + 2] = random_.choice((0, 0, 1))
        data[at + 0x74] = random_.choice((0, 1, 1, 1))
        struct.pack_into("<i", data, at + 0x1F8, random_.choice((0, 5, 10, 100, random_.randrange(200))))
        identifiers.append(bytes(data[at + 0x1C : at + 0x24]))
    # some of the identifiers are already in the list
    list_at = HEADER + RESULTS * RESULT
    for index in range(min(count, IDENTIFIERS)):
        if random_.random() < 0.5:
            data[list_at + index * 8 : list_at + index * 8 + 8] = random_.choice(identifiers)
    return bytes(data), (0x8C, 0xA0), identifiers


def run(t) -> None:
    random_ = random.Random(754)

    for n in range(200):
        data, pointers, identifiers = search(random_)
        identifier = random_.choice(identifiers + [random_.randbytes(8)])
        t.check("network_search_has_identifier", Buffer(data, pointers=pointers), Buffer(identifier))

        data, pointers, identifiers = search(random_)
        index = random_.randrange(RESULTS)
        t.check("network_search_result_see", Buffer(data[HEADER + index * RESULT : HEADER + (index + 1) * RESULT]), Buffer(data, pointers=pointers))

        data, pointers, identifiers = search(random_, random_.choice((0, 1, 1, 1)))
        index = random_.randrange(RESULTS)
        thing = bytearray(random_.randbytes(12))
        if random_.random() < 0.7:
            thing[4:12] = data[HEADER + index * RESULT + 0xBC : HEADER + index * RESULT + 0xC4]
        t.check("network_search_see", Buffer(data, pointers=pointers), Buffer(bytes(thing)))

        data, pointers, identifiers = search(random_, random_.choice((0, 1, 1, 1)))
        t.check(
            "network_search_get_counts",
            Buffer(data, pointers=pointers),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
        )

        data, pointers, identifiers = search(random_, random_.choice((0, 1, 1, 1)))
        data = bytearray(data)
        # all the results ready, a quarter of them or fewer
        ready = random_.choice((1.0, 0.5, 0.25, 0.2, 0.1))
        for index in range(RESULTS):
            data[HEADER + index * RESULT + 0x74] = 1 if random_.random() < ready else 0
        now = random_.getrandbits(32)
        started = (now - random_.choice((0, 1, 999, 1000, 1001, 5000))) & 0xFFFFFFFF
        struct.pack_into("<I", data, 4, started)
        own_time = random_.choice((0, 0, 1, 0x80))
        t.check(
            "network_search_best_result",
            Buffer(bytes(data), pointers=pointers),
            random_.choice((Buffer(random_.randbytes(0x44)), Buffer(random_.randbytes(0x44)), 0)),
            globals={
                "unknown_004ce394": 1000,
                "unknown_00510548": bytes((own_time,)),
                "unknown_0051054c": now,
            },
            externals={"GetTickCount": lambda harness, arguments, now=now: now},
        )
