# the copies of the maps on the hard disk (source/cache/cache_files.c): the
# game opens them when it starts
#
# The tasks of files are not run: the test stands in for the functions that
# start them, and says what each would have done

import random
import struct

from difftest import Buffer, GlobalData
from test_cache_files import header as map_header

NONE = 0xFFFFFFFF
MAP_FILE_SIZE = 0x804
MAP_FILE_COUNT = 6
GLOBALS_SIZE = 0x385C
COPY_GLOBALS_SIZE = 0xB40
MAXIMUM_SIZES = (0x0B400000, 0x20800000, 0x05000000, 0x05000000, 0x11800000, 0x11800000)
HEADER_STRINGS = (0x20, 0x120, 0x198, 0x1BC)

# a task
PRIORITY = 0x0
PARAMETERS = 0x8
CATEGORY = 0x30
FUNCTION = 0x34
DONE = 0x38


def read_string(harness, address: int) -> bytes:
    return bytes(harness.mu.mem_read(address, 0x200)).split(b"\0", 1)[0]


def read_integer(harness, address: int) -> int:
    return struct.unpack("<I", harness.mu.mem_read(address, 4))[0]


def set_done(harness, address: int) -> None:
    harness.mu.mem_write(address, b"\x01")


def header_strings(harness, address: int) -> tuple[int, ...]:
    """The first character of each string of a header: the game empties them."""
    return tuple(harness.mu.mem_read(address + offset, 1)[0] for offset in HEADER_STRINGS)


def run(t) -> None:
    random_ = random.Random(19)

    # ---------- the places of the shared maps

    for shared_index in (-1, 0, 1, 2, 3, 0x7FFFFFFF):
        t.check(
            "cached_map_file_shared_get",
            shared_index,
            globals={"cached_map_file_globals": GlobalData(bytes(GLOBALS_SIZE))},
        )

    # ---------- the header of a copy

    good = map_header().data
    for index in range(MAP_FILE_COUNT):
        for data in (good, map_header(version=7).data, random_.randbytes(0x800)):
            for bytes_read in (0x800, 0x7FF, 0):

                def read_position(harness, arguments, data=data, bytes_read=bytes_read):
                    file, size, offset, category, priority, bytes_read_address, buffer, done = arguments
                    harness.mu.mem_write(buffer, data)
                    harness.mu.mem_write(bytes_read_address, struct.pack("<I", bytes_read))
                    set_done(harness, done)
                    return 0, (file, size, offset, category, priority, harness.normalize(buffer))

                t.check(
                    "cached_map_file_read_header",
                    index,
                    globals={"cached_map_file_globals": GlobalData(random_.randbytes(GLOBALS_SIZE))},
                    externals={"async_read_position": read_position, "SwitchToThread": lambda harness, arguments: 0},
                )

    # ---------- opening the copies

    plans = ("no file", "set size fails", "write fails", "new", "bad header", "good header")
    for case in range(120):
        if case < len(plans):
            plan = [plans[case]] * MAP_FILE_COUNT
        else:
            plan = [random_.choice(plans + ("good header",) * 4) for _ in range(MAP_FILE_COUNT)]
        # of the single player shared map, against the map of the disc
        disc = random_.choice(("unreadable", "another", "same", "same"))
        task = Buffer(bytes(0x40))

        def allocate(harness, arguments, task=task):
            return task.address, ()

        def queue(harness, arguments, plan=plan):
            # the task that opens a file, done at once
            address = arguments[0]
            path_address, access, share_mode, disposition, flags, file_address, create_directories = struct.unpack(
                "<7I", harness.mu.mem_read(address + PARAMETERS, 28)
            )
            path = read_string(harness, path_address)
            index = int(path[-7:-4])
            harness.mu.mem_write(file_address, struct.pack("<I", NONE if plan[index] == "no file" else 0x1000 + index))
            set_done(harness, read_integer(harness, address + DONE))
            priority = read_integer(harness, address + PRIORITY)
            category = read_integer(harness, address + CATEGORY)
            function = harness.normalize(read_integer(harness, address + FUNCTION))
            # the task has addresses of the stack, which are not the same in
            # the game and in source/: what it has is compared here
            harness.mu.mem_write(address, bytes(0x40))
            return 0x00ABCD00, (
                path,
                access,
                share_mode,
                disposition,
                flags,
                harness.normalize(file_address),
                create_directories & 0xFF,
                priority,
                category,
                function,
            )

        def get_file_size(harness, arguments, plan=plan):
            file, category, priority, size, done = arguments
            index = file - 0x1000
            wrong = plan[index] in ("set size fails", "write fails", "new")
            harness.mu.mem_write(size, struct.pack("<I", MAXIMUM_SIZES[index] - (0x800 if wrong else 0)))
            set_done(harness, done)
            return 0, (file, category, priority)

        def set_file_size(harness, arguments, plan=plan):
            file, size, category, priority, success, done = arguments
            harness.mu.mem_write(success, b"\x00" if plan[file - 0x1000] == "set size fails" else b"\x01")
            set_done(harness, done)
            return 0, (file, size, category, priority)

        def write_position(harness, arguments, plan=plan):
            file, buffer, size, offset, flags, category, priority, bytes_written, done = arguments
            written = 0x7FF if plan[file - 0x1000] == "write fails" else size
            harness.mu.mem_write(bytes_written, struct.pack("<I", written))
            set_done(harness, done)
            return 0, (file, header_strings(harness, buffer), size, offset, flags, category, priority)

        def close_file(harness, arguments):
            file, category, priority, done = arguments
            set_done(harness, done)
            return 0, (file, category, priority)

        def read_copy_header(harness, arguments, plan=plan):
            return plan[arguments[0]] == "good header"

        def read_disc_header(harness, arguments, disc=disc):
            header, path = arguments
            record = (header_strings(harness, header), read_string(harness, path))
            copy = harness.global_addresses["cached_map_file_globals"] + MAP_FILE_SIZE + 4
            unknown144 = read_integer(harness, copy + 0x144)
            data = bytearray(good)
            struct.pack_into("<I", data, 0x144, unknown144 if disc == "same" else unknown144 ^ 0x10)
            harness.mu.mem_write(header, bytes(data))
            return disc != "unreadable", record

        t.check(
            "cached_map_files_open",
            globals={"cached_map_file_globals": GlobalData(random_.randbytes(GLOBALS_SIZE))},
            extra=[task],
            externals={
                "cached_map_files_delete": lambda harness, arguments: 0,
                "async_task_allocate": allocate,
                "async_task_queue": queue,
                "async_get_file_size": get_file_size,
                "async_set_file_size": set_file_size,
                "async_write_position": write_position,
                "async_close_file": close_file,
                "cached_map_file_read_header": read_copy_header,
                "cache_file_read_header": read_disc_header,
                "CompareFileTime": lambda harness, arguments: 0,
                "SwitchToThread": lambda harness, arguments: 0,
            },
        )

    # ---------- starting

    for language in (NONE, 0, 4, 7):
        for console_language in (1, 5, 9):
            for copies_language in (0, 4, 7):
                for build_number in (9809, 0, 9808):
                    for changed in (b"\0", b"\x01"):
                        t.check(
                            "cache_files_initialize",
                            globals={
                                "cached_map_file_globals": random_.randbytes(GLOBALS_SIZE),
                                "cached_map_file_copy_globals": random_.randbytes(COPY_GLOBALS_SIZE),
                                "global_language": language,
                                "cached_map_files_language": copies_language,
                                "cached_map_files_build_number": build_number,
                                "cached_map_files_state_changed": changed,
                            },
                            externals={
                                "XGetLanguage": lambda harness, arguments, console_language=console_language: console_language,
                                "cached_map_files_delete": lambda harness, arguments: 0,
                                "cached_map_files_open": lambda harness, arguments: 0,
                            },
                        )
