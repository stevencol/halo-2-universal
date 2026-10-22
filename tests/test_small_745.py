# four small functions of the life cycle of the network game that end tasks

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
KEYS, KEY = 8, 0x20


def task(random_: random.Random) -> int:
    return random_.choice((NONE, NONE, 0, 1, 0x10002, random_.getrandbits(32)))


def something(random_: random.Random, size: int = 0xA0) -> bytearray:
    data = bytearray(random_.randbytes(size))
    struct.pack_into("<I", data, 0xC, task(random_))
    struct.pack_into("<I", data, 0x14, task(random_))
    return data


CLOSES = {"online_task_close": lambda harness, arguments: (0, tuple(arguments))}


def run(t) -> None:
    random_ = random.Random(745)

    for n in range(300):
        t.check(
            "network_life_cycle_unknown_00072d30",
            globals={
                "unknown_0047708c": bytes(something(random_)),
                "unknown_0047712c": bytes(something(random_)),
                "unknown_00477088": bytes((random_.choice((0, 1, 0x80)),)),
            },
            externals=CLOSES,
        )

        t.check("network_life_cycle_unknown_00073310", Buffer(bytes(something(random_, 0x20))), externals=CLOSES)

        data = something(random_, 0x30)
        data[0xA] = random_.choice((0, 1, 1, 0x80))
        struct.pack_into("<I", data, 0x2C, random_.randrange(KEYS))
        keys = bytearray(random_.randbytes(KEYS * KEY))
        for index in range(KEYS):
            keys[index * KEY] = random_.choice((0, 1, 0x80))
        t.check(
            "network_life_cycle_unknown_00072df0",
            Buffer(bytes(data)),
            globals={"unknown_004cf7d4": bytes(keys)},
            externals={
                **CLOSES,
                "XNetUnregisterKey": lambda harness, arguments: (
                    0,
                    (arguments[0] - harness.global_addresses["unknown_004cf7d4"],),
                ),
            },
        )

        t.check(
            "network_life_cycle_unknown_00073dc0",
            random_.getrandbits(32),
            globals={
                "unknown_00510518": random_.choice((0, 0, 1, NONE, random_.getrandbits(32))),
                "unknown_00510530": task(random_),
                "unknown_00510514": random_.getrandbits(32),
            },
            externals=CLOSES,
        )
