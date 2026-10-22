# five small functions of the numbers that the network game keeps of its players

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
PLACES = 5
PLACE_SIZE = 0x99C
RECORD_SIZE = 0x98
KINDS = 6
LEVELS = 0x80
# the smallest and the largest of each kind, as the game has them
LIMITS = ((0, 0x3FFFFFFF), (0, 0x7F), (0, 0x7F), (0, 0x7FFFFFFF), (0, 0x7FFFFFFF), (0, 0x7FFFFFFF))


def records(random_: random.Random) -> bytes:
    data = bytearray(random_.randbytes(PLACES * PLACE_SIZE))
    for place in range(PLACES):
        for record in range(16):
            at = place * PLACE_SIZE + record * RECORD_SIZE
            if at + 8 <= len(data):
                struct.pack_into("<I", data, at, random_.choice((3, 3, 4, 0, 2, NONE)))
            for kind in range(0x13):
                value = at + 8 + kind * 8
                if value + 8 <= len(data):
                    data[value] = random_.choice((0, 1, 1, 0x80))
    return bytes(data)


def limits(random_: random.Random) -> bytes:
    if random_.random() < 0.6:
        pairs = LIMITS
    else:
        pairs = []
        for kind in range(KINDS):
            low, high = sorted(random_.randrange(-1000, 1000) for n in range(2))
            pairs.append((low, high))
    return b"".join(struct.pack("<ii", low, high) for low, high in pairs)


def levels(random_: random.Random) -> list[int]:
    start = random_.randrange(-50, 50)
    values = [start]
    for n in range(LEVELS - 1):
        values.append(values[-1] + random_.choice((0, 1, 5, 100, 1000)))
    if random_.random() < 0.1:
        values = [random_.getrandbits(32) - (1 << 31) for n in range(LEVELS)]
    return values


def value_for(random_: random.Random, table: list[int]) -> int:
    choice = random_.random()
    if choice < 0.6:
        index = random_.randrange(LEVELS)
        return (table[index] + random_.randrange(-3, 4)) & 0xFFFFFFFF
    if choice < 0.8:
        index = random_.randrange(LEVELS - 1)
        return ((table[index] + table[index + 1]) // 2 + random_.randrange(-1, 2)) & 0xFFFFFFFF
    return random_.choice((0, NONE, 0x7FFFFFFF, 0x80000000, random_.getrandbits(32)))


def number(random_: random.Random) -> int:
    return random_.choice((0, 1, 5, 0x7F, 0x80, NONE, 0x7FFFFFFF, 0x80000000, random_.getrandbits(32), random_.randrange(300)))


def run(t) -> None:
    random_ = random.Random(744)

    for n in range(300):
        tick = random_.getrandbits(32)
        t.check(
            "online_statistics_unknown_000739a0",
            random_.choice((NONE, NONE, 0, 1, 2, 3, 4, 5)),
            random_.randrange(16),
            globals={
                "unknown_00509470": records(random_),
                "unknown_00510548": bytes((random_.choice((0, 0, 1, 0x80)),)),
                "unknown_0051054c": random_.getrandbits(32),
            },
            externals={"GetTickCount": lambda harness, arguments, tick=tick: tick},
        )

        table = levels(random_)
        t.check(
            "online_statistics_level",
            value_for(random_, table),
            random_.choice((0, 1, 2, 50, 0x7E, 0x7F, 0x80, random_.randrange(LEVELS), NONE)),
            globals={"unknown_004ce7b4": struct.pack(f"<{LEVELS}I", *(value & 0xFFFFFFFF for value in table))},
        )

        # a number just at the middle of the level below the largest and the next
        table = sorted(set(random_.randrange(-5000, 5000) for n in range(LEVELS * 2)))[:LEVELS]
        index = random_.randrange(LEVELS - 1)
        middle = (table[index] + table[index + 1]) // 2 if table[index] + table[index + 1] >= 0 else -(-(table[index] + table[index + 1]) // 2)
        t.check(
            "online_statistics_level",
            (middle + random_.choice((0, 0, 1, -1))) & 0xFFFFFFFF,
            index + 1,
            globals={"unknown_004ce7b4": struct.pack(f"<{LEVELS}I", *(value & 0xFFFFFFFF for value in table))},
        )

        for name in ("online_statistics_set", "online_statistics_add", "online_statistics_set_maximum"):
            record = random_.randrange(16)
            place = random_.randrange(PLACES)
            kind = random_.randrange(KINDS)
            if (place * PLACE_SIZE + 8 + (record * 0x13 + KINDS) * 8) > PLACES * PLACE_SIZE:
                record = 0
            value = number(random_)
            data = bytearray(records(random_))
            bounds = limits(random_)
            if random_.random() < 0.3:
                # what is there is already the number, as it is kept
                low, high = struct.unpack_from("<ii", bounds, kind * 8)
                signed = value - (1 << 32) if value & 0x80000000 else value
                at = place * PLACE_SIZE + 8 + (record * 0x13 + kind) * 8
                struct.pack_into("<i", data, at + 4, min(max(signed, low), high))
            t.check(
                name,
                record,
                place,
                kind,
                value,
                globals={"unknown_00509470": bytes(data), "unknown_0046722c": bounds},
            )
