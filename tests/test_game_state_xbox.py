# the files of the state of the game (source/saved games/game_state_xbox.c)
#
# The tasks of files are not run: the test stands in for the functions that
# start them, and says what each would have done

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer

NONE = 0xFFFFFFFF

# xbox_game_state_globals
BUFFER = 0x4
BUFFER_SIZE = 0x8
WRITE_BUFFER = 0xC
WRITE_DONE = 0x10
BYTES_WRITTEN = 0x14
WRITE_FILE_INDEX = 0x18
FILE_VALID_FOR_READ = 0x1A
FILE_OPEN = 0x1C
HANDLES = 0x20
GLOBALS_SIZE = 0x28

STATE = PHYSICAL_ADDRESS + 0x20000
COPY = PHYSICAL_ADDRESS + 0x30000
STATE_SIZE = 0x123
FILE_SIZE = 0x40B000

# a task
PARAMETERS = 0x8
CATEGORY = 0x30
FUNCTION = 0x34
DONE = 0x38


def game_state_globals(random_: random.Random, **fields: object) -> bytes:
    data = bytearray(random_.randbytes(GLOBALS_SIZE))
    struct.pack_into("<Ii", data, BUFFER, STATE, fields.get("buffer_size", STATE_SIZE))
    struct.pack_into("<I", data, WRITE_BUFFER, fields.get("write_buffer", 0))
    data[WRITE_DONE] = fields.get("write_done", 0)
    struct.pack_into("<I", data, BYTES_WRITTEN, fields.get("bytes_written", STATE_SIZE))
    struct.pack_into("<h", data, WRITE_FILE_INDEX, fields.get("write_file_index", random_.randrange(2)))
    data[FILE_VALID_FOR_READ : FILE_VALID_FOR_READ + 2] = bytes(fields.get("valid", (random_.randrange(2), random_.randrange(2))))
    struct.pack_into("<2I", data, HANDLES, 0x50, 0x54)
    return bytes(data)


def read_string(harness, address: int) -> bytes:
    return bytes(harness.mu.mem_read(address, 0x200)).split(b"\0", 1)[0]


def read_integer(harness, address: int) -> int:
    return struct.unpack("<I", harness.mu.mem_read(address, 4))[0]


def globals_address(harness) -> int:
    return harness.global_addresses["xbox_game_state_globals"]


def calls_so_far(harness, name: str) -> int:
    return sum(1 for call in harness._calls if call[0] == name)


def run(t) -> None:
    random_ = random.Random(21)
    t.harness.mu.mem_write(STATE, random_.randbytes(0x1000))

    # ---------- opening and closing the files

    for plan in (("new", "new"), ("no file", "new"), ("new", "no file"), ("no size", "no size"), ("no file", "no file")):
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
            file_index = (file_address - globals_address(harness) - HANDLES) // 4
            harness.mu.mem_write(file_address, struct.pack("<I", NONE if plan[file_index] == "no file" else 0x60 + file_index))
            harness.mu.mem_write(read_integer(harness, address + DONE), b"\x01")
            record = (
                path,
                access,
                share_mode,
                disposition,
                flags,
                file_index,
                create_directories & 0xFF,
                read_integer(harness, address),
                read_integer(harness, address + CATEGORY),
                harness.normalize(read_integer(harness, address + FUNCTION)),
            )
            # the task has addresses of the stack: what it has is compared here
            harness.mu.mem_write(address, bytes(0x40))
            return 0x00ABCD00, record

        def set_file_size(harness, arguments, plan=plan):
            file, size, category, priority, success, done = arguments
            harness.mu.mem_write(success, b"\x00" if plan[file - 0x60] == "no size" else b"\x01")
            harness.mu.mem_write(done, b"\x01")
            return 0, (file, size, category, priority)

        t.check(
            "game_state_create_or_open_file",
            globals={"xbox_game_state_globals": game_state_globals(random_)},
            extra=[task],
            externals={
                "async_task_allocate": allocate,
                "async_task_queue": queue,
                "async_set_file_size": set_file_size,
                "SwitchToThread": lambda harness, arguments: 0,
            },
        )

    def close_file(harness, arguments):
        file, category, priority, done = arguments
        harness.mu.mem_write(done, b"\x01")
        return 0, (file, category, priority)

    for _ in range(3):
        t.check(
            "game_state_close_file",
            globals={"xbox_game_state_globals": game_state_globals(random_)},
            externals={"async_close_file": close_file, "SwitchToThread": lambda harness, arguments: 0},
        )

    # ---------- the end of a write

    free = {"physical_memory_buffer_free": lambda harness, arguments: 0}
    for write_done in (0, 1, 0x80):
        for bytes_written in (STATE_SIZE, STATE_SIZE - 1, 0):
            for file_index in (0, 1):
                for write_buffer in (COPY, 0):
                    globals_ = game_state_globals(
                        random_,
                        write_done=write_done,
                        bytes_written=bytes_written,
                        write_file_index=file_index,
                        write_buffer=write_buffer,
                    )
                    if write_buffer:
                        t.check(
                            "game_state_write_finish",
                            write_buffer,
                            0,
                            globals={"xbox_game_state_globals": globals_},
                            externals=free,
                        )
                    if write_done or not write_buffer:
                        t.check(
                            "game_state_write_reclaim",
                            write_buffer,
                            0,
                            globals={"xbox_game_state_globals": globals_},
                            externals=free,
                        )

    # the write is not done: the function waits for it, or for another thread to end it
    for switches in (1, 3):
        for what in ("done", "ended"):

            def switch_to_thread(harness, arguments, switches=switches, what=what):
                if calls_so_far(harness, "SwitchToThread") == switches - 1:
                    address = globals_address(harness)
                    if what == "done":
                        harness.mu.mem_write(address + WRITE_DONE, b"\x01")
                    else:
                        harness.mu.mem_write(address + WRITE_BUFFER, bytes(4))
                return 0

            t.check(
                "game_state_write_reclaim",
                COPY,
                0,
                globals={"xbox_game_state_globals": game_state_globals(random_, write_buffer=COPY, write_done=0)},
                externals={**free, "SwitchToThread": switch_to_thread},
            )

    # ---------- writing and reading

    def reclaim(harness, arguments):
        # the write before this one ends. The function does nothing with its
        # arguments, and the compiler of source/ knows it and does not set
        # them: they are not compared
        harness.mu.mem_write(globals_address(harness) + WRITE_BUFFER, bytes(4))
        return 0, ()

    for file_index in (0, 1, 0xABCD0001, 0x00010000):
        for pending in (0, COPY):
            for given in (COPY, 0):
                for written in (STATE_SIZE, STATE_SIZE - 1):
                    for switches in (0, 2):

                        def write_position(harness, arguments, written=written, switches=switches):
                            file, buffer, size, offset, flags, category, priority, bytes_written, done = arguments
                            harness.mu.mem_write(bytes_written, struct.pack("<I", written))
                            if not switches:
                                harness.mu.mem_write(done, b"\x01")
                            address = globals_address(harness)
                            record = (
                                file,
                                buffer,
                                bytes(harness.mu.mem_read(buffer, size)),
                                size,
                                offset,
                                flags,
                                category,
                                priority,
                                bytes_written - address,
                                done - address,
                            )
                            return 0x00ABCD01, record

                        def switch_to_thread(harness, arguments, switches=switches):
                            if calls_so_far(harness, "SwitchToThread") == switches - 1:
                                harness.mu.mem_write(globals_address(harness) + WRITE_DONE, b"\x01")
                            return 0

                        t.check(
                            "game_state_write_to_file",
                            file_index,
                            globals={
                                "xbox_game_state_globals": game_state_globals(random_, write_buffer=pending, write_done=0)
                            },
                            externals={
                                "game_state_write_reclaim": reclaim,
                                "physical_memory_buffer_allocate": lambda harness, arguments, given=given: given,
                                "async_write_position": write_position,
                                "SwitchToThread": switch_to_thread,
                                "main_loading_idle": lambda harness, arguments: 0,
                            },
                        )

    for file_index in (0, 1, 0xABCD0001):
        for pending in (0, COPY):
            for valid in ((0, 0), (1, 1), (1, 0), (0, 1)):
                for read in (STATE_SIZE, STATE_SIZE - 1, STATE_SIZE + 1, 0):

                    def read_position(harness, arguments, read=read):
                        file, size, offset, category, priority, bytes_read, buffer, done = arguments
                        harness.mu.mem_write(bytes_read, struct.pack("<I", read))
                        harness.mu.mem_write(done, b"\x01")
                        return 0x00ABCD02, (file, size, offset, category, priority, buffer)

                    t.check(
                        "game_state_read_from_file",
                        file_index,
                        globals={
                            "xbox_game_state_globals": game_state_globals(random_, write_buffer=pending, valid=valid)
                        },
                        externals={
                            "game_state_write_reclaim": reclaim,
                            "async_read_position": read_position,
                            "SwitchToThread": lambda harness, arguments: 0,
                        },
                    )
