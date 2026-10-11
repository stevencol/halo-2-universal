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
                "unknown_0006b640": lambda harness, arguments: (0, (arguments[0],)),
                "network_blob_task_finished": lambda harness, arguments: (0, tuple(arguments)),
                "texture_cache_return_memory": lambda harness, arguments: (0, (arguments[0],)),
            },
        )
