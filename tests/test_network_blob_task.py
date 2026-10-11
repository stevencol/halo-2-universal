# source/networking/network_blob_task.c

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, Memory

NONE = 0xFFFFFFFF
TASKS, TASK = 32, 0x14
# where what has a block is, for a task that has one: the tasks say where by its address
BLOB = PHYSICAL_ADDRESS + 0x60000


def run(t) -> None:
    random_ = random.Random(503)

    def tasks(free: bool = True) -> bytearray:
        data = bytearray(random_.randbytes(TASKS * TASK))
        first = random_.choice((0, 0, 1, 5, 31))
        for index in range(TASKS):
            taken = index < first or random_.random() < 0.5
            struct.pack_into("<I", data, index * TASK, random_.getrandbits(32) | 1 if taken else 0)
        if free:
            struct.pack_into("<I", data, random_.randrange(first, TASKS) * TASK, 0)
        return data

    def queues(task: int = NONE) -> bytearray:
        data = bytearray()
        for queue in range(2):
            count = random_.choice((0, 1, 5, 15))
            numbers = [random_.choice((task, random_.randrange(TASKS), random_.randrange(TASKS), NONE)) for index in range(count)]
            data += struct.pack("<16I", *(numbers + [NONE] * (16 - count)))
        return data

    for n in range(2000):
        t.check(
            "network_blob_tasks_reset",
            globals={"unknown_004d8c28": random_.randbytes(TASKS * TASK), "unknown_004d8ba8": random_.randbytes(0x80)},
        )
        t.check("network_blob_task_find_free", globals={"unknown_004d8c28": bytes(tasks(free=random_.random() < 0.9))})

        blob = bytearray(random_.randbytes(0x30))
        struct.pack_into("<h", blob, 8, random_.choice((0, 1, 1, 2, 2, 3, 4)))
        size = random_.choice((4, 8, 0x21, 0x40))
        t.check(
            "network_blob_task_start_write",
            Buffer(bytes(blob)),
            Buffer(random_.randbytes(0x40)),
            size,
            globals={"unknown_004d8c28": bytes(tasks()), "unknown_004d8ba8": bytes(queues())},
        )
        t.check(
            "network_blob_task_start_read",
            Buffer(bytes(blob)),
            Buffer(random_.randbytes(0x40)),
            random_.getrandbits(32),
            globals={"unknown_004d8c28": bytes(tasks()), "unknown_004d8ba8": bytes(queues())},
        )

        # a task ends: what has its block is in memory of its own
        task = random_.randrange(TASKS)
        table = tasks()
        has = random_.random() < 0.85
        struct.pack_into("<II", table, task * TASK, BLOB if has else 0, random_.choice((NONE, NONE, 0x10005, 7)))
        struct.pack_into("<h", blob, 8, random_.choice((4, 4, 4, 1, 0)))
        struct.pack_into("<II", blob, 0x20, random_.choice((NONE, NONE, 3)), random_.choice((0, 0x80123000)))
        t.check(
            "network_blob_task_end",
            task,
            random_.choice((1, 6, 7, 13, random_.getrandbits(32))),
            globals={"unknown_004d8c28": bytes(table), "unknown_004d8ba8": bytes(queues(task))},
            memory={BLOB: Memory(bytes(blob))},
            externals={
                "online_task_close": lambda harness, arguments: (0, (arguments[0],)),
                "network_blob_task_finished": lambda harness, arguments: (0, tuple(arguments)),
                "texture_cache_return_memory": lambda harness, arguments: (0, (arguments[0],)),
            },
        )

    # ---------- the tasks are started, and what each did is taken

    FUNCTIONS = PHYSICAL_ADDRESS + 0x61000
    BLOCK = 0x80123000

    for n in range(2000):
        # some tasks, each with what has its block; of them, some in the two lists
        table = bytearray(random_.randbytes(TASKS * TASK))
        memory = {FUNCTIONS: Memory(random_.randbytes(0x40))}
        numbers = random_.sample(range(TASKS), 12)
        for place, task in enumerate(numbers):
            blob = bytearray(random_.randbytes(0x30))
            struct.pack_into("<I", blob, 0, FUNCTIONS)
            struct.pack_into("<h", blob, 8, random_.choice((0, 1, 2, 3, 3, 4, 4)))
            memory[BLOB + place * 0x40] = Memory(bytes(blob))
            struct.pack_into(
                "<4I",
                table,
                task * TASK,
                BLOB + place * 0x40,
                random_.choice((NONE, NONE, NONE, 0x10000 + place)),
                random_.getrandbits(32),
                BLOCK + place * 0x100,
            )
        lists = bytearray()
        for queue in range(2):
            count = random_.choice((0, 1, 3, 6))
            mine = [numbers[queue * 6 + index] for index in range(count)]
            if mine and random_.random() < 0.2:
                mine[random_.randrange(len(mine))] = NONE
            lists += struct.pack("<16I", *(mine + [NONE] * (16 - count)))

        def start(pointer: int, starts: list):
            def stand_in(harness, arguments, pointer=pointer, starts=starts):
                from test_async import calls_so_far

                total = sum(calls_so_far(harness, name) for name in ("online_storage_write_file_start", "online_storage_write_start", "online_storage_read_file_start", "online_storage_read_start"))
                status, handle = starts[total % len(starts)]
                harness.mu.mem_write(arguments[pointer], struct.pack("<I", handle))
                return status, tuple(value for index, value in enumerate(arguments) if index != pointer)

            return stand_in

        starts = [(random_.choice((0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, NONE)), random_.getrandbits(32)) for index in range(8)]
        t.check(
            "network_blob_tasks_update",
            globals={
                "unknown_004d8c28": bytes(table),
                "unknown_004d8ba8": bytes(lists),
                "unknown_004402a4": struct.pack("<2i", *random_.choice(((1, 1), (1, 1), (2, 3), (0, 16), (16, 16), (-1, 2)))),
            },
            memory=memory,
            externals={
                "online_storage_write_file_start": start(7, starts),
                "online_storage_write_start": start(1, starts),
                "online_storage_read_file_start": start(1, starts),
                "online_storage_read_start": start(8, starts),
                "network_blob_take_memory": lambda harness, arguments, took=random_.choice((1, 1, 0)): (
                    took,
                    # where the task has the memory of its block and its size: the tasks are not at the same address
                    (
                        arguments[0],
                        arguments[1] - harness.global_addresses["unknown_004d8c28"],
                        arguments[2] - harness.global_addresses["unknown_004d8c28"],
                    ),
                ),
                "network_blob_task_end": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

        # what did one of the tasks ended
        task = numbers[0]
        handle = random_.choice((5, 0x10003, 0x20007))
        size = random_.choice((0x40, 0x100, 0x1000))
        struct.pack_into("<4I", table, task * TASK, BLOB, handle, size, BLOCK)
        operations = bytearray(random_.randbytes(0x4C + 8 * 0x14))
        struct.pack_into("<I", operations, 0x44, 0x4C)
        struct.pack_into("<I", operations, 0x4C + (handle & 0xFFFF) * 0x14 + 4, random_.choice((0x2A, 0x2A, 0x2A, 0x2B, 0)))
        read = random_.choice((size, size, size - 8, size + 1, 0))
        there = random_.choice((read, read, read, read + 1, 0))

        def ended(harness, arguments, said=random_.choice((0, 0, 0, 0, 1, 2, 6, 7)), read=read, there=there):
            harness.mu.mem_write(arguments[1], struct.pack("<I", read))
            harness.mu.mem_write(arguments[2], struct.pack("<I", there))
            return said, (arguments[0],)

        t.check(
            "network_blob_task_completed",
            random_.choice((0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, NONE)),
            handle,
            globals={"unknown_004d8c28": bytes(table), "online_tasks": Buffer(bytes(operations), pointers=(0x44,))},
            memory=memory,
            externals={
                "online_storage_read_get_results": ended,
                "network_blob_verify": lambda harness, arguments, said=random_.choice((1, 1, 8, 9, 11)): (said, tuple(arguments)),
                "network_blob_task_end": lambda harness, arguments: (0, tuple(arguments)),
            },
        )
