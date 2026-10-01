# the tasks that another thread does (source/cseries/async.c)
#
# The functions of the console that wait, make threads and read files are not
# decompiled: the test stands in for them

import random
import struct

from difftest import Buffer, FunctionPointer, GlobalData, GlobalPointer, Mock, StopCall, mock_address

NONE = 0xFFFFFFFF
INFINITE = 0xFFFFFFFF

# async_globals
FREE_LIST_MUTEX = 0x4
TASK_LIST_MUTEX = 0x8
SEMAPHORE = 0xC
GENERATION = 0x10
TASKS = 0x14
TASK_COUNT = 150
TASK_SIZE = 0x40
FREE_LIST = 0x2594
TASK_LIST = 0x2598
TASKS_ADDED = 0x259C
GLOBALS_SIZE = 0x25A0

# a task
PARAMETERS = 0x8
CATEGORY = 0x30
FUNCTION = 0x34
DONE = 0x38
NEXT = 0x3C

FREE_LIST_MUTEX_HANDLE = 0x1F
TASK_LIST_MUTEX_HANDLE = 0x2F
SEMAPHORE_HANDLE = 0x3F

READ = "async_read_work"


def task_offset(index: int) -> int:
    return TASKS + index * TASK_SIZE


def task_pointer(index: int) -> GlobalPointer:
    return GlobalPointer("async_globals", task_offset(index))


# where a call can leave a pointer to a task or to a function
PLACES = (
    *(task_offset(index) + field for index in range(TASK_COUNT) for field in (FUNCTION, DONE, NEXT)),
    FREE_LIST,
    TASK_LIST,
)


def async_globals(
    random_: random.Random,
    tasks: dict[int, dict] | None = None,
    task_list: tuple[int, ...] = (),
    free_list: tuple[int, ...] = (),
    generation: int | None = None,
) -> GlobalData:
    """The globals, with the tasks of two lists. A task is its priority, its
    id, its parameters, its category, its function (a name, or a number), and
    the offset in the globals of its flag."""
    data = bytearray(random_.randbytes(GLOBALS_SIZE))
    struct.pack_into("<3I", data, FREE_LIST_MUTEX, FREE_LIST_MUTEX_HANDLE, TASK_LIST_MUTEX_HANDLE, SEMAPHORE_HANDLE)
    struct.pack_into("<I", data, GENERATION, random_.getrandbits(20) if generation is None else generation)
    pointers: list[int] = []
    functions: dict[int, str] = {}
    for offset in PLACES:
        struct.pack_into("<I", data, offset, 0)

    for index, task in (tasks or {}).items():
        at = task_offset(index)
        struct.pack_into("<i", data, at, task.get("priority", 0))
        struct.pack_into("<I", data, at + 4, task.get("id", NONE))
        parameters = task.get("parameters")
        if parameters is not None:
            data[at + PARAMETERS : at + PARAMETERS + len(parameters)] = parameters
        struct.pack_into("<I", data, at + CATEGORY, task.get("category", 0))
        function = task.get("function", 0)
        if isinstance(function, str):
            functions[at + FUNCTION] = function
        else:
            struct.pack_into("<I", data, at + FUNCTION, function)
        if task.get("done") is not None:
            struct.pack_into("<I", data, at + DONE, task["done"])
            pointers.append(at + DONE)

    for head, order in ((TASK_LIST, task_list), (FREE_LIST, free_list)):
        place = head
        for index in order:
            struct.pack_into("<I", data, place, task_offset(index))
            pointers.append(place)
            place = task_offset(index) + NEXT

    return GlobalData(bytes(data), pointers=tuple(pointers), functions=functions, places=PLACES)


def read_parameters(file: int, offset: int, size: int = 0x800) -> bytes:
    return struct.pack("<10I", file, 0x60800000, size, offset, 0, 0, 0, 0, 0, 0)


def random_task(random_: random.Random, index: int) -> dict:
    """A task for a list: a read, mostly, of one of a few files and places."""
    return {
        "priority": random_.choice((0, 1, 1, 2, 6, -3)),
        "id": random_.getrandbits(24) << 8 | index,
        "parameters": read_parameters(random_.choice((0x10, 0x14, 0x18)), random_.choice((0, 0x800, 0x1000, 0xFFFFF800))),
        "category": random_.randrange(8),
        "function": READ if random_.random() < 0.75 else 0x00123450,
    }


def release_semaphore(harness, arguments):
    semaphore, count, previous_count = arguments
    return 1, (semaphore, count, struct.unpack("<i", harness.mu.mem_read(previous_count, 4))[0])


WAITS = {
    "WaitForSingleObject": lambda harness, arguments: 0,
    "ReleaseMutex": lambda harness, arguments: 1,
    "ReleaseSemaphore": release_semaphore,
}


def calls_so_far(harness, name: object) -> int:
    return sum(1 for call in harness._calls if call[0] == name)


def run(t) -> None:
    random_ = random.Random(18)

    # ---------- the order of two tasks

    for next_priority, task_priority in ((1, 1), (1, 2), (2, 1), (-1, 0), (0, -1)):
        for task_function in (FunctionPointer(READ), 0x00123450):
            for next_function in (FunctionPointer(READ), 0x00123450):
                for valid in (0, 1, 0x80):
                    for _ in range(12):
                        state_file = random_.choice((0x10, 0x14))
                        state_offset = random_.choice((0, 0x1000, 0xFFFFF000))
                        t.check(
                            "async_read_goes_before",
                            next_priority,
                            task_priority,
                            Buffer(struct.pack("<2IB3x", state_file, state_offset, valid)),
                            task_function,
                            next_function,
                            Buffer(read_parameters(random_.choice((0x10, 0x14, 0xC)), random_.choice((0, 0x800, 0x1800)))),
                            Buffer(read_parameters(random_.choice((0x10, 0x14, 0xC)), random_.choice((0, 0x800, 0x1800)))),
                        )

    # ---------- the list

    def random_list(maximum: int) -> tuple[dict[int, dict], tuple[int, ...]]:
        indices = random_.sample(range(TASK_COUNT), random_.randint(0, maximum) + 1)
        tasks = {index: random_task(random_, index) for index in indices}
        # the one that the case is about, and the list
        return tasks, tuple(indices)

    for _ in range(400):
        tasks, indices = random_list(7)
        globals_ = async_globals(random_, tasks, task_list=indices[1:])
        t.check("async_task_list_insert", task_pointer(indices[0]), globals={"async_globals": globals_})

    for _ in range(150):
        tasks, indices = random_list(5)
        globals_ = async_globals(random_, tasks, task_list=indices[1:])
        t.check("async_task_queue", task_pointer(indices[0]), globals={"async_globals": globals_}, externals=WAITS)

    for _ in range(150):
        tasks, indices = random_list(5)
        globals_ = async_globals(random_, tasks, task_list=indices)
        t.check(
            "async_task_list_remove",
            task_pointer(random_.choice(indices)),
            globals={"async_globals": globals_},
            externals=WAITS,
        )

    for count in (0, 1, 2, 5, 40):
        indices = tuple(random_.sample(range(TASK_COUNT), count))
        tasks = {index: random_task(random_, index) for index in indices}
        globals_ = async_globals(random_, tasks, task_list=indices)
        t.check("async_task_count", globals={"async_globals": globals_}, externals=WAITS)

    for _ in range(150):
        tasks, indices = random_list(6)
        globals_ = async_globals(random_, tasks, task_list=indices)
        index = random_.choice(indices)
        id_ = tasks[index]["id"]
        if random_.random() < 0.25:
            # of a task that was there before
            id_ ^= 0x100
        t.check(
            "async_task_set_priority",
            id_,
            random_.choice((0, 1, 2, 6, -3)),
            globals={"async_globals": globals_},
            externals=WAITS,
        )

    # ---------- the tasks that are not in use

    for count in (1, 2, 5):
        indices = tuple(random_.sample(range(TASK_COUNT), count))
        globals_ = async_globals(random_, free_list=indices)
        t.check("async_task_allocate", globals={"async_globals": globals_}, externals=WAITS)

    # none: the function waits until another thread leaves one
    for switches in (1, 3):

        def switch_to_thread(harness, arguments, switches=switches):
            if calls_so_far(harness, "SwitchToThread") == switches - 1:
                address = harness.global_addresses["async_globals"]
                harness.mu.mem_write(address + FREE_LIST, struct.pack("<I", address + task_offset(7)))
            return 0

        t.check(
            "async_task_allocate",
            globals={"async_globals": async_globals(random_)},
            externals={**WAITS, "SwitchToThread": switch_to_thread},
        )

    def create_mutex(harness, arguments):
        return 0x100 + calls_so_far(harness, "CreateMutexA")

    for _ in range(3):
        t.check(
            "async_initialize",
            globals={
                "async_globals": GlobalData(random_.randbytes(GLOBALS_SIZE), places=PLACES),
                "async_buffer": GlobalData(bytes(0x8000)),
                "async_buffer_globals": 0x1234,
                "async_buffer_globals+4": 0x1234,
                "async_buffer_globals+8": 0x1234,
            },
            externals={
                "CreateMutexA": create_mutex,
                "CreateSemaphoreA": lambda harness, arguments: 0x200,
                "CreateThread": lambda harness, arguments: 0x300,
                "SetThreadPriority": lambda harness, arguments: 1,
            },
        )

    # ---------- a new task

    for _ in range(60):
        tasks, indices = random_list(4)
        free = tuple(index for index in random_.sample(range(TASK_COUNT), 3) if index not in indices)
        if not free:
            continue
        globals_ = async_globals(random_, tasks, task_list=indices, free_list=free)
        t.check(
            "async_task_add",
            random_.choice((0, 1, 2, 6)),
            Buffer(read_parameters(random_.choice((0x10, 0x14)), random_.choice((0, 0x800, 0x1000)))),
            random_.randrange(8),
            random_.choice((FunctionPointer(READ), 0x00123450)),
            Buffer(b"\x55"),
            globals={"async_globals": globals_},
            externals=WAITS,
        )

    # ---------- waiting for a task

    for start in (0, 1, 0xFF):
        for switches in (1, 2, 5):
            for idle in (0, 1, 0x100, 0x1FF):
                done = Buffer(bytes([start, 0x77]))

                def switch_to_thread(harness, arguments, done=done, switches=switches):
                    if calls_so_far(harness, "SwitchToThread") == switches - 1:
                        harness.mu.mem_write(done.address, b"\x01")
                    return 0

                t.check(
                    "async_wait",
                    done,
                    idle,
                    externals={"SwitchToThread": switch_to_thread, "main_loading_idle": lambda harness, arguments: 0},
                )

    # ---------- the thread

    def stop(harness, arguments):
        raise StopCall

    t.check("async_thread_procedure", externals={"async_thread_loop": stop})

    # what the function of the tasks returns each time, and after how many
    # times the test ends the call
    flag = task_offset(149) + PARAMETERS
    for results in ((1,), (0, 0, 1), (2,), (1, 1, 1), (0, 2, 0, 0, 1, 5), (0xFFFFFFFF, 1), (1, 0)):
        for with_flag in (True, False):
            tasks = {
                index: {**random_task(random_, index), "function": mock_address(0), "done": flag + index if with_flag else None}
                for index in (3, 4, 5)
            }
            globals_ = async_globals(random_, tasks, task_list=(4, 3, 5), free_list=(9,))

            def wait(harness, arguments, results=results):
                if arguments[0] == SEMAPHORE_HANDLE and calls_so_far(harness, 0) == len(results):
                    raise StopCall
                return 0

            def function(call, results=results, counter=[0]):
                # the calls of both runs: the game's, then that of source/
                result = results[counter[0] % len(results)]
                counter[0] += 1
                return result

            t.check(
                "async_thread_loop",
                globals={"async_globals": globals_, "async_current_task": 0},
                mocks=[Mock(function, stack_arguments=1)],
                externals={**WAITS, "WaitForSingleObject": wait},
            )

    # ---------- a read

    for maximum in (0x40000, 0x100):
        for size, bytes_done in ((0x800, 0), (0x100, 0), (0x80, 0x40), (0x1000, 0xF00), (0x1000, 0xE00), (0, 0), (0x100000, 0)):
            for seek in (True, False):
                for given in ("all", "less", "none"):
                    for with_count in (True, False):
                        parameters = bytearray(
                            struct.pack("<10I", 0x10, 0x60800000, size, 0x12345800, 0x20, bytes_done, 1, 2, 0x55555555, 4)
                        )
                        if not with_count:
                            struct.pack_into("<I", parameters, 0x10, 0)

                        def set_file_pointer(harness, arguments, seek=seek):
                            file, distance, high, method = arguments
                            record = (file, distance, struct.unpack("<i", harness.mu.mem_read(high, 4))[0], method)
                            return (distance if seek else 0xFFFFFFFF), record

                        def read_file(harness, arguments, given=given):
                            file, buffer, size, bytes_read, overlapped = arguments
                            count = {"all": size, "less": size // 2, "none": 0}[given]
                            harness.mu.mem_write(bytes_read, struct.pack("<I", count))
                            return 1, (file, buffer, size, overlapped)

                        t.check(
                            "async_read_work",
                            Buffer(bytes(parameters), pointers=(0x10,) if with_count else ()),
                            globals={"async_read_maximum_size": maximum},
                            externals={"SetFilePointer": set_file_pointer, "ReadFile": read_file},
                        )

    def allocate(harness, arguments):
        return harness.global_addresses["async_globals"] + task_offset(11), ()

    def queue(harness, arguments):
        task = bytearray(harness.mu.mem_read(arguments[0], TASK_SIZE))
        (function,) = struct.unpack_from("<I", task, FUNCTION)
        struct.pack_into("<I", task, FUNCTION, 0)
        return 0x00ABCD0B, (harness.normalize(arguments[0]), bytes(task), harness.normalize(function))

    for _ in range(20):
        t.check(
            "async_read_position",
            random_.getrandbits(16),
            random_.getrandbits(20),
            random_.getrandbits(30),
            random_.randrange(8),
            random_.randrange(8),
            random_.choice((0, 0x60800100)),
            0x80010000,
            Buffer(b"\x55"),
            globals={"async_globals": async_globals(random_)},
            externals={"async_task_allocate": allocate, "async_task_queue": queue},
        )
