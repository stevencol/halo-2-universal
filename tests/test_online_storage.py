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
                "online_storage_make_path": named,
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

    # ---------- a block starts to be read

    for n in range(3000):
        task = Buffer(random_.randbytes(0x14))
        name = random_.randbytes(0x20)

        def named(harness, arguments, name=name, could=random_.choice((1, 1, 1, 1, 0))):
            length = read(harness, arguments[6], 4)
            harness.mu.mem_write(arguments[5], name)
            return could, (*arguments[:5], length)

        def started(harness, arguments, hr=random_.choice(RESULTS + (0x8015C004, 0x8015C004)), handle=random_.getrandbits(32)):
            harness.mu.mem_write(arguments[7], struct.pack("<I", handle))
            return hr, (
                arguments[0],
                arguments[1],
                read(harness, arguments[2], 0x20),
                harness.normalize(arguments[3]),
                arguments[4],
                arguments[5],
                arguments[6],
                harness.normalize(arguments[7]),
            )

        t.check(
            "online_storage_read_start",
            random_.choice((0, 1, 2, 5, NONE)),
            random_.randrange(4),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            Buffer(random_.randbytes(0x20)),
            random_.getrandbits(32),
            Buffer(random_.randbytes(4)),
            extra=[task],
            externals={
                "online_storage_make_path": named,
                "online_task_new_signed_in": lambda harness, arguments, index=random_.choice((0x80010003, 5, NONE)): index,
                "online_task_get": lambda harness, arguments, task=task, there=random_.random() < 0.9: (
                    task.address if there else 0,
                    (arguments[0],),
                ),
                "unknown_003ab6ea": started,
                "online_task_close": lambda harness, arguments: (0, (arguments[0],)),
            },
        )

    # ---------- a block of the third kind starts to be written, and to be read

    for n in range(3000):
        for function, library, count, last in (
            ("online_storage_write_file_start", "unknown_003ad149", 9, True),
            ("online_storage_read_file_start", "XOnlineStorageDownload", 7, False),
        ):
            task = Buffer(random_.randbytes(0x14))
            name = random_.randbytes(0x20)

            def named(harness, arguments, name=name, could=random_.choice((1, 1, 1, 1, 0))):
                length = read(harness, arguments[6], 4)
                harness.mu.mem_write(arguments[5], name)
                return could, (*arguments[:5], length)

            def started(harness, arguments, count=count, hr=random_.choice(RESULTS + (0x8015C004, 0x8015C004)), handle=random_.getrandbits(32)):
                harness.mu.mem_write(arguments[count - 1], struct.pack("<I", handle))
                return hr, (
                    arguments[0],
                    arguments[1],
                    read(harness, arguments[2], 0x20),
                    *arguments[3:count - 1],
                    harness.normalize(arguments[count - 1]),
                )

            out = Buffer(random_.randbytes(4))
            numbers = [random_.getrandbits(32) for index in range(5)]
            owner = random_.randrange(4)
            unknown0 = random_.choice((0, 1, 2, 5, NONE))
            t.check(
                function,
                *((unknown0, owner, *numbers, out) if last else (unknown0, out, owner, *numbers)),
                extra=[task],
                externals={
                    "online_storage_make_path": named,
                    "online_task_new_signed_in": lambda harness, arguments, index=random_.choice((0x80010003, 5, NONE)): index,
                    "online_task_get": lambda harness, arguments, task=task, there=random_.random() < 0.9: (
                        task.address if there else 0,
                        (arguments[0],),
                    ),
                    library: started,
                    "online_task_close": lambda harness, arguments: (0, (arguments[0],)),
                },
            )

    # ---------- the name of something in the storage

    def wide(text: str) -> bytes:
        return text.encode("utf-16-le") + b"\0\0"

    def wide_at(harness, address: int) -> bytes:
        # a text, up to where it ends: what is after it is not set
        raw = read(harness, address, 0x200)
        for index in range(0, 0x200, 2):
            if raw[index:index + 2] == b"\0\0":
                return raw[:index]
        return raw

    def printed(name: str):
        """usnzprintf, for the two formats that the function has: the C library of the tests does not take wide texts
        for %s. The second text of "%s/%s" is the name, which is not one of the arguments that the test is given."""

        def stand_in(harness, arguments, name=name):
            buffer, count, format_, first = arguments
            asked = wide_at(harness, format_).decode("utf-16-le")
            if asked == "%d":
                text = str(first)
            else:
                assert asked == "%s/%s", asked
                text = wide_at(harness, first).decode("utf-16-le") + "/" + name
            harness.mu.mem_write(buffer, wide(text[: count - 1]))
            return 0, (count, asked, text[: count - 1])

        return stand_in

    for n in range(1500):
        name = "".join(random_.choice("abcXYZ019_/. ") for index in range(random_.choice((0, 1, 5, 20, 20, 0xF8, 0xFF, 0x100, 0x130))))
        t.check(
            "online_storage_make_path",
            random_.choice((0, 0, 1, 2, 5, NONE)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            Buffer(wide(name) + random_.randbytes(8)),
            Buffer(random_.randbytes(0x200)),
            Buffer(struct.pack("<I", 0x100)),
            globals={
                "unknown_0050944e": bytes((random_.choice((0, 0, 1)),)),
                "unknown_004672dc": Buffer(wide(random_.choice(("test", "test", "", "x" * 0x120)))),
            },
            externals={
                "usnzprintf": printed(name),
                "unknown_003ab6b1": lambda harness, arguments, hr=random_.choice((0, 0, 5, 0x80004005, NONE)): (
                    hr,
                    (*arguments[:5], wide_at(harness, arguments[5]), harness.normalize(arguments[6]), harness.normalize(arguments[7])),
                ),
            },
        )
