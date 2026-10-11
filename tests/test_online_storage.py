# source/networking/online_storage.c

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
COUNT, TASK = 8, 0x14
TASKS = 0x4C
BITS = TASKS + COUNT * TASK


def tasks_table(random_: random.Random) -> tuple:
    """A table of online tasks, some of them there, and the number of one of them."""
    data = bytearray(random_.randbytes(BITS + 4))
    data[0:0x20] = b"online tasks".ljust(0x20, b"\0")
    there = [random_.random() < 0.7 for index in range(COUNT)]
    there[random_.randrange(COUNT)] = True
    numbers = []
    bits = 0
    for index in range(COUNT):
        identifier = random_.getrandbits(15) | 0x8000 if there[index] else 0
        struct.pack_into("<HH", data, TASKS + index * TASK, identifier, random_.choice((0, 1, 3, 0x21)))
        struct.pack_into("<I", data, TASKS + index * TASK + 0xC, random_.choice((0x1234, 0x77, random_.getrandbits(32))))
        if there[index]:
            numbers.append(identifier << 16 | index)
        bits |= there[index] << index
    struct.pack_into("<I", data, BITS, bits)
    struct.pack_into(
        "<iiBBHIIiiihHII",
        data,
        0x20,
        COUNT,
        TASK,
        0,
        1,
        4,
        0x64407440,
        0,
        next((index for index in range(COUNT) if not there[index]), COUNT),
        max(index + 1 for index in range(COUNT) if there[index]),
        sum(there),
        random_.getrandbits(15) - 0x8000,
        0,
        TASKS,
        BITS,
    )
    return Buffer(bytes(data), pointers=(0x44, 0x48)), random_.choice(numbers)


def run(t) -> None:
    random_ = random.Random(514)

    def read(harness, address: int, size: int) -> bytes:
        return bytes(harness.mu.mem_read(address, size))

    RESULTS = (
        0,
        0,
        0,
        5,
        0x8007000E,
        0x80150001,
        0x80150002,
        0x80150003,
        0x80150004,
        0x80150005,
        0x80150006,
        0x80150007,
        0x80150008,
        0x80150009,
        0x8015C001,
        0x8015C002,
        0x8015C003,
        0x8015C005,
        0x8015C006,
        0x8015C007,
        0x8015C008,
        0x8015C009,
        0x8015C00A,
        0x80004005,
        0x80000000,
        NONE,
    )

    for n in range(3000):
        task = Buffer(random_.randbytes(0x14))
        name = random_.randbytes(0x20)

        def named(harness, arguments, name=name, could=random_.choice((1, 1, 1, 1, 0))):
            length = read(harness, arguments[6], 4)
            harness.mu.mem_write(arguments[5], name)
            return could, (*arguments[:5], length)

        def started(harness, arguments, hr=random_.choice(RESULTS), handle=random_.getrandbits(32)):
            harness.mu.mem_write(arguments[9], struct.pack("<I", handle))
            return hr, (
                arguments[0],
                arguments[1],
                read(harness, arguments[2], 0x20),
                arguments[3],
                arguments[4],
                harness.normalize(arguments[5]),
                arguments[6],
                arguments[7],
                arguments[8],
                harness.normalize(arguments[9]),
            )

        t.check(
            "online_storage_write_start",
            random_.choice((0, 1, 2, 5, NONE)),
            Buffer(random_.randbytes(4)),
            random_.randrange(4),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            Buffer(random_.randbytes(0x20)),
            random_.getrandbits(32),
            extra=[task],
            externals={
                "unknown_000b4120": named,
                "online_task_new_signed_in": lambda harness, arguments, index=random_.choice((0x80010003, 5, NONE)): index,
                "online_task_get": lambda harness, arguments, task=task, there=random_.random() < 0.9: (
                    task.address if there else 0,
                    (arguments[0],),
                ),
                "unknown_003ad176": started,
                "online_task_close": lambda harness, arguments: (0, (arguments[0],)),
            },
        )

        tasks, number = tasks_table(random_)

        def results(harness, arguments, hr=random_.choice((0, 0, 0x80004005, 5, NONE))):
            return hr, (arguments[0], read(harness, arguments[1], 4), harness.normalize(arguments[2]), harness.normalize(arguments[3]))

        t.check(
            "online_storage_read_get_results",
            number,
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            globals={"online_tasks": tasks},
            externals={"unknown_003ab6f9": results},
        )

        tasks, number = tasks_table(random_)

        def progress(harness, arguments, hr=random_.choice((0, 0, 0, 0x80004005)), percent=random_.choice((0, 1, 50, 99, 100, 100, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, random_.getrandbits(32), random_.randrange(101)))):
            harness.mu.mem_write(arguments[1], struct.pack("<I", percent))
            return hr, (arguments[0], arguments[2], arguments[3])

        t.check(
            "online_storage_get_progress",
            random_.choice((number, number, number, number ^ 0x10000, NONE, number | 0xFFF8)),
            Buffer(random_.randbytes(4)),
            globals={"online_tasks": tasks},
            externals={"XOnlineStorageGetProgress": progress},
        )
