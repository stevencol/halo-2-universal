# the copy of a map of the disc to the hard disk (source/cache/cache_files.c),
# which the game makes a step at a time, and the tasks of files that it uses
# (source/cseries/async.c)
#
# The tasks are not run: the test stands in for the functions that start
# them, and gives each step what a task would have left

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, GlobalData
from test_async import PLACES, WAITS, async_globals, calls_so_far, task_offset
from test_cache_files import header as map_header

NONE = 0xFFFFFFFF

# cached_map_file_globals
MAP_FILE_SIZE = 0x804
MAP_FILE_COUNT = 6
OPEN_MAP_INDEX = 0x3018
FAILURES = 0x301C
FAILURE_SIZE = 0x108
FAILURE_COUNT = 8
MAP_GLOBALS_SIZE = 0x385C

# a map's header
FILE_LENGTH = 0x8
PATH = 0x20
SCENARIO_TYPE = 0x140
COPY_TIME = 0x14C
USES_SHARED_MAP = 0x174
TIMES = 0x178
CHECKSUM = 0x2D0

# cached_map_file_copy_globals
BYTES_TRANSFERRED = 0x800
BYTES_REQUESTED = 0x804
OFFSET = 0x808
FILE_SIZE = 0x80C
TASK_ID = 0x810
STATE = 0x814
RESULT = 0x818
DONE = 0x81C
DESTINATION_INDEX = 0x820
SOURCE_FILE = 0x824
BUFFER = 0x828
BUFFER_SIZE = 0x82C
RELEASE_REQUESTED = 0x830
SOURCE_PATH = 0x831
CURRENT = 0x934
PENDING = 0xA38
REQUEST_PRIORITY = 0x100
COPY_CHECKSUM = 0xB3C
COPY_GLOBALS_SIZE = 0xB40
STATE_COUNT = 20

NOW = 0x01C4A504_93122C35
COPY_BUFFER = PHYSICAL_ADDRESS + 0x10000
COPY_BUFFER_CONTENT_SIZE = 0x1000

LOCKOUT = b"d:\\maps\\lockout.map"
NAMES = [b"lockout", b"LOCKOUT", b"scenarios\\multi\\lockout\\lockout", LOCKOUT, b"ascension", b"d:\\maps\\zanzibar.map"]
NAMES += [b"t:\\test", b"shared", b""]
PATHS = [LOCKOUT, b"d:\\maps\\ascension.map", b"d:\\maps\\zanzibar.map", b"t:\\test.map", b"d:\\maps\\shared.map", b""]


def string(text: bytes) -> Buffer:
    return Buffer(text + b"\0")


def xor_checksum(data: bytes) -> int:
    count = len(data) >> 3
    low, high = (0x08040201, 0x80402010) if count & 1 else (0, 0)
    for n in range(count):
        a, b = struct.unpack_from("<2I", data, n * 8)
        low ^= a
        high ^= b
    return low ^ high


def map_globals(
    random_: random.Random,
    paths: dict[int, bytes] | None = None,
    open_map_index: int = NONE,
    failures: tuple = (),
) -> GlobalData:
    """The table of the map files, with the path of some, and the table of the copies that failed."""
    data = bytearray(random_.randbytes(MAP_GLOBALS_SIZE))
    for index in range(MAP_FILE_COUNT):
        at = index * MAP_FILE_SIZE
        struct.pack_into("<I", data, at, 0x1000 + index)
        path = (paths or {}).get(index, b"")
        data[at + 4 + PATH : at + 4 + PATH + 256] = path.ljust(256, b"\0")
        # the time of the copy: before or after now
        struct.pack_into("<Q", data, at + 4 + COPY_TIME, NOW + random_.choice((-3, -2, -1, 0, 1)) * 0x10000)
        struct.pack_into("<Q", data, at + 4 + TIMES, NOW - 0x1000 * index)
    struct.pack_into("<I", data, OPEN_MAP_INDEX, open_map_index)
    data[FAILURES:] = bytes(FAILURE_SIZE * FAILURE_COUNT)
    for n, (path, time, count) in enumerate(failures):
        at = FAILURES + n * FAILURE_SIZE
        data[at : at + len(path)] = path
        struct.pack_into("<2i", data, at + 0x100, time, count)
    return GlobalData(bytes(data), clean=clean_failures)


def clean_failures(data: bytes) -> bytes:
    """The table of the map files without what is after the end of the path of
    each copy that failed: the game copies the 256 bytes of a path that it
    wrote in its stack, and what is after the end of the string is whatever
    the stack had."""
    data = bytearray(data)
    for n in range(FAILURE_COUNT):
        at = FAILURES + n * FAILURE_SIZE
        end = data.index(0, at, at + 256) if 0 in data[at : at + 256] else at + 256
        data[end : at + 256] = bytes(at + 256 - end)
    return bytes(data)


def random_failures(random_: random.Random) -> tuple:
    return tuple(
        (random_.choice(PATHS), random_.randrange(1000), random_.randrange(5)) for _ in range(random_.choice((0, 2, 8)))
    )


def copy_header(random_: random.Random, good: bool = True) -> bytearray:
    data = bytearray(map_header().data if good else random_.randbytes(0x800))
    struct.pack_into("<h", data, SCENARIO_TYPE, random_.randrange(5))
    struct.pack_into("<H", data, SCENARIO_TYPE + 2, random_.choice((0, 0xABCD)))
    for shared_index in range(3):
        data[USES_SHARED_MAP + shared_index] = random_.choice((0, 0, 1))
        # of the shared maps that the map was made with: those of the table, mostly
        index = (2, 0, 1)[shared_index]
        time = NOW - 0x1000 * index + random_.choice((0, 0, 0, 1))
        struct.pack_into("<Q", data, TIMES + 8 + shared_index * 8, time)
    return data


def copy_globals(random_: random.Random, **fields: object) -> GlobalData:
    """The globals of the copy. Without a value given, a field gets one of those that the steps tell apart."""
    data = bytearray(COPY_GLOBALS_SIZE)
    header = fields.get("header")
    data[:0x800] = header if header is not None else copy_header(random_, random_.random() < 0.8)

    transferred = fields.get("bytes_transferred", random_.choice((0x800, 0x800, 0x7FF, 0x400, 0)))
    requested = fields.get("bytes_requested", random_.choice((transferred, transferred, 0x1000)))
    offset = fields.get("offset", random_.choice((0x800, 0x40800)))
    file_size = fields.get(
        "file_size", random_.choice((NONE, 0x800, 0x801, offset + transferred, offset + transferred + 0x1000, 0x500000))
    )
    buffer = fields.get("buffer", random_.choice((0, COPY_BUFFER)))
    buffer_size = fields.get("buffer_size", random_.choice((0x40000, 0x100000)) if buffer else 0)
    struct.pack_into("<IiII", data, BYTES_TRANSFERRED, transferred, requested, offset, file_size)
    struct.pack_into("<I", data, TASK_ID, random_.getrandbits(32))
    struct.pack_into("<2i", data, STATE, fields.get("state", 0), fields.get("result", random_.randrange(4)))
    data[DONE] = fields.get("done", 1)
    struct.pack_into("<i", data, DESTINATION_INDEX, fields.get("destination_index", random_.randrange(MAP_FILE_COUNT)))
    struct.pack_into("<I", data, SOURCE_FILE, fields.get("source_file", random_.choice((NONE, 0x77))))
    struct.pack_into("<Ii", data, BUFFER, buffer, buffer_size)
    data[RELEASE_REQUESTED] = fields.get("release_requested", random_.randrange(2))

    source_path = fields.get("source_path", random_.choice(PATHS))
    data[SOURCE_PATH : SOURCE_PATH + len(source_path)] = source_path
    for at, name in ((CURRENT, "current"), (PENDING, "pending")):
        path = fields.get(name, source_path if name == "current" else random_.choice(PATHS))
        data[at : at + len(path)] = path
        priority = fields.get(name + "_priority", random_.randrange(7))
        struct.pack_into("<i", data, at + REQUEST_PRIORITY, priority)

    checksum = fields.get("checksum", random_.getrandbits(32))
    struct.pack_into("<I", data, COPY_CHECKSUM, checksum)
    return GlobalData(bytes(data))


def get_time(harness, arguments):
    harness.mu.mem_write(arguments[0], struct.pack("<Q", NOW))
    return 0, ()


def compare_file_time(harness, arguments):
    a, b = (struct.unpack("<Q", harness.mu.mem_read(address, 8))[0] for address in arguments)
    return (a > b) - (a < b), (a, b)


TIMES_EXTERNALS = {"GetSystemTimeAsFileTime": get_time, "CompareFileTime": compare_file_time}


def run(t) -> None:
    random_ = random.Random(20)
    content = random_.randbytes(COPY_BUFFER_CONTENT_SIZE)
    t.harness.mu.mem_write(COPY_BUFFER, content)

    # ---------- small functions

    for size in (0, 7, 8, 9, 16, 24, 100, 0x800, 0x7FF):
        t.check("cache_file_xor_checksum", Buffer(random_.randbytes(0x800)), size)

    for name1 in NAMES:
        for name2 in NAMES:
            t.check("cache_file_map_paths_equal", string(name1), string(name2))

    for shared_index in range(-1, 5):
        t.check("cached_map_file_shared_index", shared_index)

    t.check("file_time_get_current", Buffer(random_.randbytes(8)), externals={"GetSystemTimeAsFileTime": get_time})

    # ---------- the place of a copy

    for _ in range(40):
        globals_ = {"cached_map_file_globals": map_globals(random_)}
        for best_index in (NONE, *range(MAP_FILE_COUNT)):
            for index in (NONE, *range(MAP_FILE_COUNT)):
                if _ and random_.random() < 0.8:
                    continue
                t.check("cached_map_file_is_better_place", best_index, index, globals=globals_, externals=TIMES_EXTERNALS)

    for _ in range(30):
        globals_ = {"cached_map_file_globals": map_globals(random_, open_map_index=random_.choice((NONE, 3, 4, 5, 0)))}
        for scenario_type in range(5):
            for file_length in (0x800, 0x05000000, 0x05000001, 0x11800000, 0x20800000, 0x80000000):
                # the function takes the type as 16 bits
                t.check(
                    "cached_map_file_choose_place",
                    scenario_type | random_.choice((0, 0xABCD0000)),
                    file_length,
                    globals=globals_,
                    externals=TIMES_EXTERNALS,
                )

    # ---------- the copies that failed

    for _ in range(25):
        failures = random_failures(random_)
        for name in NAMES:
            globals_ = {"cached_map_file_globals": map_globals(random_, failures=failures)}
            t.check("cached_map_file_failure_find", string(name), globals=globals_)
            t.check(
                "cached_map_file_failure_add",
                string(name),
                globals=globals_,
                externals={"GetTickCount": lambda harness, arguments: 500},
            )
            t.check("cached_map_file_failure_remove", string(name), globals=globals_)
            t.check("cached_map_file_failure_is_final", string(name), globals=globals_)

    # ---------- what there is of a copy, and asking for one

    for _ in range(60):
        paths = {random_.randrange(MAP_FILE_COUNT): random_.choice(PATHS) for _ in range(random_.randrange(3))}
        failures = random_failures(random_) if random_.random() < 0.3 else ()
        state = random_.randrange(STATE_COUNT)
        for name in NAMES:
            globals_ = {
                "cached_map_file_globals": map_globals(random_, paths, failures=failures),
                "cached_map_file_copy_globals": copy_globals(random_, state=state),
            }
            for function in ("current_priority", "pending_priority", "priority"):
                t.check("cached_map_file_copy_" + function, string(name), globals=globals_)
            t.check("cached_map_file_status", string(name), globals=globals_)
            t.check("cached_map_file_request", string(name), random_.randrange(7), globals=globals_)
            t.check("cached_map_file_get_progress", string(name), globals=globals_)

    for state in range(STATE_COUNT):
        globals_ = {"cached_map_file_copy_globals": copy_globals(random_, state=state)}
        t.check("cached_map_file_copy_cancel", globals=globals_)
        for offset, file_size in ((0x800, 0x100000), (0x40800, 0x40800), (0x80000, 0x40000), (0, 0), (0x800, 0x90000000)):
            globals_ = {"cached_map_file_copy_globals": copy_globals(random_, state=state, offset=offset, file_size=file_size)}
            t.check("cached_map_file_copy_get_progress", Buffer(random_.randbytes(4), format="<f"), globals=globals_)
            t.check("cached_map_file_copy_get_progress", 0, globals=globals_)

    # ---------- the buffer

    def free(harness, arguments):
        return 0

    for buffer, buffer_size in ((0, 0), (COPY_BUFFER, 0x40000), (COPY_BUFFER, 0x100000)):
        for given in (COPY_BUFFER + 0x1000, 0):
            globals_ = {"cached_map_file_copy_globals": copy_globals(random_, buffer=buffer, buffer_size=buffer_size)}
            externals = {
                "texture_cache_steal_memory": lambda harness, arguments, given=given: given,
                "texture_cache_return_memory": free,
            }
            for size in (0x40000, 0x100000, 0):
                t.check("cached_map_file_copy_buffer_acquire", size, globals=globals_, externals=externals)
            t.check("cached_map_file_copy_buffer_release", globals=globals_, externals=externals)

    # the game wants the memory back: the copy goes on until it gives it up
    for buffer in (0, COPY_BUFFER):
        for updates in (1, 3):

            def update(harness, arguments, updates=updates):
                if calls_so_far(harness, "cached_map_file_copy_update") == updates - 1:
                    address = harness.global_addresses["cached_map_file_copy_globals"]
                    harness.mu.mem_write(address + BUFFER, bytes(4))
                return 1

            t.check(
                "cached_map_file_copy_buffer_reclaim",
                random_.getrandbits(32),
                random_.getrandbits(32),
                globals={"cached_map_file_copy_globals": copy_globals(random_, buffer=buffer, done=1)},
                externals={"cached_map_file_copy_update": update},
            )

    # ---------- the steps of the copy

    def task(task_id: int):
        return lambda harness, arguments: task_id

    def choose_place(harness, arguments, place):
        return place, (arguments[0] & 0xFFFF, arguments[1])

    for state in range(STATE_COUNT):
        for case in range(60):
            source_path = random_.choice(PATHS[:4])
            paths = {index: random_.choice((b"d:\\maps\\shared.map", b"")) for index in range(3)}
            if random_.random() < 0.2:
                paths[random_.randrange(3, MAP_FILE_COUNT)] = random_.choice(PATHS)
            failures = random_failures(random_) if random_.random() < 0.5 else ()
            transferred = random_.choice((0x800, 0x800, 0x7FF, 0x400, 0))
            checksum = random_.getrandbits(32)
            header = copy_header(random_, random_.random() < 0.85)
            if random_.random() < 0.7:
                # what the copy will have when this part is counted
                struct.pack_into("<I", header, CHECKSUM, checksum ^ xor_checksum(content[:transferred]))
            place = random_.choice((NONE, 3, 4, 5))
            given = random_.choice((0, COPY_BUFFER))

            t.check(
                "cached_map_file_copy_update",
                globals={
                    "cached_map_file_globals": map_globals(random_, paths, failures=failures),
                    "cached_map_file_copy_globals": copy_globals(
                        random_,
                        state=state,
                        header=header,
                        source_path=source_path,
                        bytes_transferred=transferred,
                        checksum=checksum,
                        # the step that counts what was copied always has the buffer
                        **({"buffer": COPY_BUFFER} if state == 12 else {}),
                    ),
                },
                externals={
                    **TIMES_EXTERNALS,
                    "GetTickCount": lambda harness, arguments: 500,
                    "async_create_file": task(0x101),
                    "async_read_position": task(0x102),
                    "async_get_file_size": task(0x103),
                    "async_write_position": task(0x104),
                    "async_flush_file": task(0x105),
                    "async_copy_position": task(0x106),
                    "async_close_file": task(0x107),
                    "cached_map_file_choose_place": lambda harness, arguments, place=place: choose_place(
                        harness, arguments, place
                    ),
                    "texture_cache_steal_memory": lambda harness, arguments, given=given: given,
                    "texture_cache_return_memory": free,
                },
            )

    # the size of a part: what is left of the map, around the size of the buffer
    for buffer_size in (0x40000, 0x100000):
        for left in (buffer_size - 1, buffer_size, buffer_size + 1, 1, 0):
            for priority in (0, 3):
                t.check(
                    "cached_map_file_copy_update",
                    globals={
                        "cached_map_file_globals": map_globals(random_),
                        "cached_map_file_copy_globals": copy_globals(
                            random_,
                            state=11,
                            offset=0x40800,
                            file_size=0x40800 + left,
                            buffer=COPY_BUFFER,
                            buffer_size=buffer_size,
                            current_priority=priority,
                        ),
                    },
                    externals={
                        "async_copy_position": task(0x106),
                        "texture_cache_steal_memory": lambda harness, arguments: COPY_BUFFER,
                        "texture_cache_return_memory": free,
                    },
                )

    # ---------- the tasks of files

    def allocate(harness, arguments):
        return harness.global_addresses["async_globals"] + task_offset(11), ()

    def queue(harness, arguments):
        data = bytearray(harness.mu.mem_read(arguments[0], 0x40))
        (function,) = struct.unpack_from("<I", data, 0x34)
        struct.pack_into("<I", data, 0x34, 0)
        return 0x00ABCD0B, (harness.normalize(arguments[0]), bytes(data), harness.normalize(function))

    tasks = {"async_task_allocate": allocate, "async_task_queue": queue}
    for _ in range(4):
        globals_ = {"async_globals": async_globals(random_)}
        done = Buffer(b"\x55")
        category = random_.randrange(8)
        priority = random_.randrange(8)
        for mode in range(6):
            for access_flags in (0, 1, 2, 3, 0x101):
                for file_flags in (0, 1, 2, 4, 6, 7):
                    t.check(
                        "async_create_file",
                        mode,
                        0x60800000,
                        access_flags,
                        file_flags,
                        category,
                        priority,
                        0x60800100,
                        done,
                        globals=globals_,
                        externals=tasks,
                    )
        for count in (Buffer(random_.randbytes(4)), 0):
            t.check(
                "async_copy_position",
                0x10,
                0x14,
                COPY_BUFFER,
                random_.getrandbits(20),
                random_.getrandbits(30),
                random_.getrandbits(30),
                category,
                priority,
                count,
                done,
                globals=globals_,
                externals=tasks,
            )
        t.check("async_flush_file", random_.getrandbits(16), category, priority, done, globals=globals_, externals=tasks)

    t.check(
        "async_flush_file_work",
        Buffer(random_.randbytes(0x28)),
        externals={"FlushFileBuffers": lambda harness, arguments: 1},
    )

    def set_file_pointer(harness, arguments, seek):
        file, distance, high, method = arguments
        record = (file, distance, struct.unpack("<i", harness.mu.mem_read(high, 4))[0], method)
        return (distance if seek else NONE), record

    def transfer(harness, arguments, taken):
        file, buffer, size, bytes_address, overlapped = arguments
        count = {"all": size, "less": size // 2, "none": 0, "error": size}[taken]
        harness.mu.mem_write(bytes_address, struct.pack("<I", count))
        return (0 if taken == "error" else 1), (file, buffer, size, overlapped)

    for read_maximum, write_maximum in ((0x40000, 0x40000), (0x100, 0x40000), (0x40000, 0x80)):
        for size, bytes_done in ((0x800, 0), (0x100, 0), (0x80, 0x40), (0x1000, 0xF00), (0, 0), (0x100000, 0)):
            for writing in (0, 1, 0x100):
                for seek in (True, False):
                    for taken in ("all", "less", "none", "error"):
                        for with_count in (True, False):
                            parameters = struct.pack(
                                "<10I",
                                0x10,
                                0x14,
                                0x00123800,
                                0x00456800,
                                COPY_BUFFER,
                                size,
                                0x24 if with_count else 0,
                                bytes_done,
                                writing,
                                0x55555555,
                            )
                            t.check(
                                "async_copy_work",
                                Buffer(parameters, pointers=(0x18,) if with_count else ()),
                                globals={
                                    "async_read_maximum_size": read_maximum,
                                    "async_write_maximum_size": write_maximum,
                                },
                                externals={
                                    "SetFilePointer": lambda harness, arguments, seek=seek: set_file_pointer(
                                        harness, arguments, seek
                                    ),
                                    "ReadFile": lambda harness, arguments, taken=taken: transfer(harness, arguments, taken),
                                    "WriteFile": lambda harness, arguments, taken=taken: transfer(harness, arguments, taken),
                                },
                            )
