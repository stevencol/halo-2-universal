# five small functions: starting what is known of this console in the
# network, a text of the matchmaking, whether a task says an error, the
# least of some numbers by flags, and whether a point is made of numbers

import random
import struct

from difftest import Buffer, GlobalData

NONE = 0xFFFFFFFF
USERS = 8


def run(t) -> None:
    random_ = random.Random(779)

    for n in range(600):
        count = random_.randrange(USERS)
        # the tables of transport.c: the next one is filled with pointers to
        # functions, which are compared as what they point to
        places = tuple(range(0, USERS * 4, 4))
        t.check(
            "transport_security_initialize",
            globals={
                "unknown_004cf790": bytes((random_.choice((0, 1)),)),
                "unknown_004cf791": bytes((random_.choice((0, 1)),)),
                "unknown_004cf792": bytes((random_.choice((0, 1)),)),
                "unknown_004cf793": random_.randbytes(0x24),
                "unknown_004cf7b8": random_.randbytes(0x14),
                "unknown_004cf7cc": random_.randbytes(6),
                "unknown_004cf7d4": random_.randbytes(0x100),
                "unknown_004cf8d4": bytes((random_.choice((0, 1)),)),
                "unknown_004d8b1c": count,
                "unknown_004d8b20": GlobalData(random_.randbytes(USERS * 4), places=places),
                "unknown_004d8b40": GlobalData(random_.randbytes(USERS * 4), places=places),
                "unknown_004d8b60": GlobalData(random_.randbytes(USERS * 4), places=places),
                "unknown_004d8b80": random_.randbytes(USERS * 4),
            },
        )

        t.check(
            "online_matchmaking_unknown_0008ebc0",
            random_.choice((0, Buffer(random_.randbytes(0xC) + b"name of something\0"), Buffer(bytes(0x10)))),
        )

        # the tasks: a data array of four, 0x20 bytes each, with the
        # identifier of each in its first sixteen bits
        tasks = bytearray(random_.randbytes(0x4C + 4 * 0x20))
        struct.pack_into("<I", tasks, 0x24, 0x20)
        struct.pack_into("<I", tasks, 0x38, random_.choice((0, 2, 4)))
        struct.pack_into("<I", tasks, 0x44, 0x4C)
        identifiers = [random_.choice((0, 0x8001, 0x8002, 0x1234)) for _ in range(4)]
        for index, identifier in enumerate(identifiers):
            struct.pack_into("<H", tasks, 0x4C + index * 0x20, identifier)
        tasks = Buffer(bytes(tasks), pointers=(0x44,))
        place = random_.randrange(5)
        index = random_.choice((NONE, (identifiers[place % 4] << 16) | place, (0x8001 << 16) | place, random_.getrandbits(32)))

        def continued(harness, arguments, said=random_.choice((0x8015B108, 0x8015B108, 0, 0x1500F2, 0x8015B109)), tasks=tasks):
            return said, ((arguments[0] - tasks.address) if arguments[0] else None,)

        t.check(
            "online_task_unknown_00092750",
            index,
            globals={"online_tasks": tasks},
            externals={"online_task_continue": continued},
        )

        t.check(
            "game_engine_unknown_000a4950",
            Buffer(struct.pack("<I", random_.choice((0, 1, 2, 3, 4, 8, 0x10, 0x11, 0x1F, 0x20, 0x13, random_.getrandbits(32))))),
        )

        def coordinate() -> bytes:
            return random_.choice((
                struct.pack("<f", random_.uniform(-1000, 1000)),
                struct.pack("<f", 0.0),
                struct.pack("<I", 0x7F800000),
                struct.pack("<I", 0xFF800000),
                struct.pack("<I", 0x7FC00000),
                struct.pack("<I", 0x7F7FFFFF),
                struct.pack("<I", 0x00000001),
            ))

        t.check("valid_real_point3d", Buffer(coordinate() + coordinate() + coordinate()))
