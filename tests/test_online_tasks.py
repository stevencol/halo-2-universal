# source/networking/online_tasks.c

import random
import struct

from difftest import Buffer, Mock, Pointer, mock_address

NONE = 0xFFFFFFFF
COUNT, TASK = 8, 0x14
TASKS = 0x4C
BITS = TASKS + COUNT * TASK
# after the table: what gives its memory, with its two functions; and something that goes over it
ALLOCATION = BITS + 4
ITERATOR = ALLOCATION + 12
SIZE = ITERATOR + 0x10


def run(t) -> None:
    random_ = random.Random(508)

    def table(allocation: bool = False) -> tuple:
        """A table of tasks, some of them there, and the number of one of them or of one that there is not."""
        data = bytearray(random_.randbytes(SIZE))
        data[0:0x20] = b"online tasks".ljust(0x20, b"\0")
        there = [random_.random() < 0.6 for index in range(COUNT)]
        numbers = []
        bits = 0
        for index in range(COUNT):
            identifier = random_.getrandbits(15) | 0x8000 if there[index] else 0
            struct.pack_into(
                "<HHIII",
                data,
                TASKS + index * TASK,
                identifier,
                random_.choice((0, 1, 2, 3, 4, 6, 0x24, 0x26, 0x20, random_.getrandbits(16))),
                random_.choice((2, 2, 3, 3, 0x21, 0x21, 0, 5)),
                random_.getrandbits(32),
                random_.choice((0, NONE, 0x1234, 0x1234, 0x77)),
            )
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
            ALLOCATION if allocation else 0,
            next((index for index in range(COUNT) if not there[index]), COUNT),
            max((index + 1 for index in range(COUNT) if there[index]), default=0),
            sum(there),
            random_.getrandbits(15) - 0x8000,
            0,
            TASKS,
            BITS,
        )
        struct.pack_into("<3I", data, ALLOCATION, ALLOCATION + 4, mock_address(0), mock_address(1))
        struct.pack_into("<4I", data, ITERATOR, random_.getrandbits(32), 0, NONE, random_.choice((NONE, NONE, 0, 3, 7)))
        number = random_.choice(numbers)
        number = random_.choice((number, number, number, number ^ 0x10000, NONE, number | 0xFFF8, 0x80000000 + COUNT))
        pointers = [0x44, 0x48, ALLOCATION, ITERATOR + 4] + ([0x30] if allocation else [])
        return Buffer(bytes(data), pointers=tuple(pointers), places=(ITERATOR,)), number

    for n in range(2000):
        tasks, number = table()
        t.check("online_task_iterator_next", Pointer(tasks, ITERATOR), extra=[tasks])

        tasks, number = table()
        t.check("online_task_get_status", number, globals={"online_tasks": tasks})

        tasks, number = table()
        t.check("online_task_new", globals={"online_tasks": tasks})

        # how a task goes on each time that it is asked to, while it is done with
        steps = random_.choice(((0x1500F2,), (0, 0x1500F2), (0, 0, 5, 0x1500F2), (0x80004005,), (0, 0x80070057)))

        def goes_on(harness, arguments, steps=steps):
            from test_async import calls_so_far

            return steps[min(calls_so_far(harness, "unknown_0006c670"), len(steps) - 1)], (harness.normalize(arguments[0]),)

        closes = {
            "unknown_0006c670": goes_on,
            "XOnlineFriendsEnumerateFinish": lambda harness, arguments, said=random_.choice((0, 0, 0x80004005)): (said, tuple(arguments)),
            "unknown_0008c550": lambda harness, arguments: (0, tuple(arguments)),
            "XOnlineTaskClose": lambda harness, arguments: (0, tuple(arguments)),
        }
        tasks, number = table()
        t.check("online_task_close", number, globals={"online_tasks": tasks}, externals=closes)

        tasks, number = table(allocation=random_.random() < 0.8)
        t.check(
            "online_tasks_dispose",
            globals={"online_tasks": tasks, "unknown_00479748": random_.choice((NONE, NONE, number))},
            externals=dict(closes, unknown_0006b950=lambda harness, arguments: (0, ())),
            mocks=[Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)) for index in range(2)],
        )

    for n in range(300):
        made = Buffer(random_.randbytes(0x230), places=(0x44, 0x48))
        allocator = Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,))
        t.check(
            "online_tasks_initialize",
            globals={
                "heap_allocation": allocator,
                "online_tasks": random_.getrandbits(32),
                "unknown_004771c8": random_.randbytes(0x2580),
                "unknown_00479748": random_.getrandbits(32),
            },
            extra=[made],
            externals={"unknown_0006bfa0": lambda harness, arguments: (0, ())},
            mocks=[
                Mock(lambda call, made=made: made.address, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
        )

    # ---------- more of the tasks

    for n in range(2000):
        session = lambda harness, arguments, said=random_.choice((1, 1, 1, 0, 2)): (said, tuple(arguments))
        tasks, number = table()
        t.check(
            "online_task_new_signed_in",
            globals={"online_tasks": tasks, "unknown_00467214": random_.choice((0, 0, 0, 2, NONE))},
            externals={"unknown_0006cd50": session},
        )

        tasks, number = table()
        t.check("online_task_get", number, globals={"online_tasks": tasks})

        # of the tasks that there are: the kind and the owner of one, or others
        data = tasks.data
        index = random_.randrange(COUNT)
        kind, owner = struct.unpack_from("<II", data, TASKS + index * TASK + 4)
        kind = random_.choice((kind, kind, 2, 3, 0x21, 7))
        owner = random_.choice((owner, owner, NONE, 0xFF, 0, random_.getrandbits(32)))
        t.check("online_task_find", owner, kind, globals={"online_tasks": tasks})
        tasks, number = table()
        t.check("online_task_exists", owner, kind, globals={"online_tasks": tasks})

        # a task that there is, for what does not look
        tasks, number = table()
        bits = struct.unpack_from("<I", tasks.data, BITS)[0]
        there = [index for index in range(COUNT) if bits >> index & 1]
        if there:
            index = random_.choice(there)
            number = struct.unpack_from("<H", tasks.data, TASKS + index * TASK)[0] << 16 | index
            t.check("online_task_get_type", number, globals={"online_tasks": tasks})
            tasks = Buffer(tasks.data, pointers=tasks.pointers, places=tasks.places)
            t.check(
                "online_task_restart",
                number,
                globals={"online_tasks": tasks},
                externals={
                    "unknown_0006c670": lambda harness, arguments: (0x1500F2, (harness.normalize(arguments[0]),)),
                    "XOnlineFriendsEnumerateFinish": lambda harness, arguments: (0, tuple(arguments)),
                    "unknown_0008c550": lambda harness, arguments: (0, tuple(arguments)),
                    "XOnlineTaskClose": lambda harness, arguments: (0, tuple(arguments)),
                },
            )
