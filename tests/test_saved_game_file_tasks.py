# source/saved games/saved_game_file_tasks.c, and the tasks with a function
# of their own of source/cseries/async.c
#
# A task is not made: what gives a place for one and what puts it in the
# list are stood in for, and what is compared is what the task has

import random
import struct

from difftest import Buffer, FunctionPointer, Mock, mock_address

NONE = 0xFFFFFFFF
TASK_SIZE = 0x40
TASK_FUNCTION = 0x34
RESULT_SIZE = 0x10C
INFORMATION = 0x40
DESCRIPTION = 0xA0
# where async_globals counts the tasks that were added
TASKS_ADDED = 0x259C


def text(harness, address: int) -> bytes:
    return bytes(harness.mu.mem_read(address, 0x200)).split(bytes(1), 1)[0]


def run(t) -> None:
    random_ = random.Random(60)

    def task_place() -> Buffer:
        # the function of the task is that of the game or that of source/,
        # and so is the function that its parameters start with
        return Buffer(random_.randbytes(TASK_SIZE), places=(8, TASK_FUNCTION))

    def task_externals(place: Buffer, identifier: int) -> dict:
        return {
            "async_task_allocate": lambda harness, arguments: place.address,
            "async_task_queue": lambda harness, arguments, identifier=identifier: identifier,
        }

    # ---------- a task with a function of its own

    for size in (0, 1, 3, 0x10, 0x22, 0x23, 0x24, 0x28, 0x100, NONE):
        for _ in range(3):
            place = task_place()
            t.check(
                "async_task_add_custom",
                size,
                random_.getrandbits(32),
                Buffer(random_.randbytes(0x30)),
                random_.getrandbits(32),
                Buffer(b"\x55"),
                globals={f"async_globals+{TASKS_ADDED:#x}": random_.getrandbits(32)},
                extra=[place],
                externals=task_externals(place, random_.choice((NONE, 0x1203, 0))),
            )

    for size in (0, 1, 0x10, 0x23, 0x7F, 0x80, 0xFF):
        data = bytearray(random_.randbytes(0x28))
        struct.pack_into("<I", data, 0, mock_address(0))
        data[0x27] = size
        t.check(
            "async_custom_task_work",
            Buffer(bytes(data)),
            mocks=[Mock(lambda call, value=random_.getrandbits(32): value, stack_arguments=3)],
        )

    # ---------- the tasks of a read and of a write

    for n in range(40):
        place = task_place()
        identifier = random_.choice((NONE, 0x1203, 0x1203, 0))
        path = random_.choice((b"u:\\udata\\4d530064\\0123456789AB\\profile", b"", b"p" * 0xFF, b"q" * 0x140))
        t.check(
            "saved_game_file_read_task_new",
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            Buffer(random_.randbytes(RESULT_SIZE)),
            Buffer(path + bytes(1)),
            globals={f"async_globals+{TASKS_ADDED:#x}": random_.getrandbits(32)},
            extra=[place],
            externals=task_externals(place, identifier),
        )
        place = task_place()
        t.check(
            "saved_game_file_write_task_new",
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            Buffer(random_.randbytes(RESULT_SIZE)),
            globals={f"async_globals+{TASKS_ADDED:#x}": random_.getrandbits(32)},
            extra=[place],
            externals=task_externals(place, identifier),
        )

    # ---------- the work of a read

    def read_file(harness, arguments):
        size, file, buffer, unknown, result = arguments
        # the reference of the file is in the stack
        return 0, (size, bytes(harness.mu.mem_read(file, 0x108)), buffer, unknown & 0xFF, result)

    for n in range(30):
        path = random_.choice((b"u:\\udata\\4d530064\\0123456789AB\\profile", b"", b"p" * 0xFF))
        result = bytearray(random_.randbytes(RESULT_SIZE))
        result[8 : 8 + len(path) + 1] = path + bytes(1)
        # the data of the task: a buffer, a size, a flag and the result, which
        # is after it in the same buffer
        data = struct.pack("<2IB3sI", random_.getrandbits(32), random_.getrandbits(32), random_.getrandbits(8), random_.randbytes(3), 0x10)
        t.check(
            "saved_game_file_read_work",
            random_.getrandbits(32),
            Buffer(data + bytes(result), pointers=(0xC,)),
            random_.getrandbits(32),
            externals={"saved_game_file_read_signed": read_file},
        )

    # ---------- what is told of a file that is written

    def wide(text_: str) -> bytes:
        return text_.encode("utf-16-le") + bytes(2)

    for n in range(40):
        t.check(
            "saved_game_file_description_new",
            Buffer(random_.randbytes(DESCRIPTION + 8)),
            random_.getrandbits(32),
            Buffer(random_.randbytes(INFORMATION)),
        )
        t.check(
            "saved_game_file_description_new_renamed",
            random_.getrandbits(32),
            Buffer(wide(random_.choice(("mine", "", "a name of 16 ch.", "a name that is longer than sixteen"))) + bytes(0x10)),
            Buffer(random_.randbytes(DESCRIPTION + 8)),
            Buffer(random_.randbytes(INFORMATION)),
            random_.getrandbits(32),
            random_.getrandbits(32),
        )

    # ---------- the tasks that read and write the file of the state of a saved game

    for name in ("saved_game_state_read_task_new", "saved_game_state_write_task_new"):
        for n in range(40):
            place = task_place()
            path = random_.choice((b"u:\\udata\\4d530064\\0123456789AB\\auxilary.bin", b"", b"p" * 0xFF, b"q" * 0x140))
            t.check(
                name,
                Buffer(path + bytes(1)),
                Buffer(random_.randbytes(RESULT_SIZE)),
                random_.choice((0, 0, random_.getrandbits(32) | 1)),
                random_.getrandbits(32),
                random_.getrandbits(32),
                random_.getrandbits(32),
                random_.getrandbits(32),
                globals={f"async_globals+{TASKS_ADDED:#x}": random_.getrandbits(32)},
                extra=[place],
                externals=task_externals(place, random_.choice((NONE, 0x1203, 0x1203, 0))),
            )

    # ---------- a file and its signature

    REFERENCE_SIZE = 0x110
    HANDLE = 0x108
    SIGNATURE = 20

    def reference() -> Buffer:
        data = bytearray(random_.randbytes(REFERENCE_SIZE))
        struct.pack_into("<2I", data, HANDLE, 0x4321, random_.choice((0, 0x100)))
        return Buffer(bytes(data))

    def signature_externals(begin: int, end: int, signature: bytes) -> dict:
        def finish(harness, arguments):
            # the signature is left in the stack
            harness.mu.mem_write(arguments[1], signature)
            return end, (arguments[0],)

        return {
            "XCalculateSignatureBegin": lambda harness, arguments: begin,
            "XCalculateSignatureUpdate": lambda harness, arguments: 0,
            "XCalculateSignatureEnd": finish,
            "CloseHandle": lambda harness, arguments, closed=random_.choice((1, 1, 0)): closed,
            "GetLastError": lambda harness, arguments: 5,
            "SetLastError": lambda harness, arguments: 0,
        }

    for n in range(150):
        size = random_.choice((0, 1, 0x130, 0x1E0))
        signature = random_.randbytes(SIGNATURE)
        stored = signature if n % 3 else bytes(SIGNATURE - 1) + bytes([signature[-1] ^ 1]) if n % 6 else bytes([signature[0] ^ 0x80]) + signature[1:]
        opened = n % 11 != 10
        read_result = random_.choice((1, 1, 1, 0))
        read_count = random_.choice((size, size, size, size + 1, max(size - 1, 0), None))
        begin = random_.choice((0x55, 0x55, 0x55, NONE))
        end = random_.choice((0, 0, 0, 5))
        position_result = random_.choice((1, 1, 1, 0))

        def open_(harness, arguments, opened=opened):
            # the error is left in the stack
            return opened, (arguments[0], arguments[1])

        def read_file(harness, arguments, read_result=read_result, read_count=read_count):
            file, buffer, count, bytes_read, overlapped = arguments
            if read_count is not None:
                harness.mu.mem_write(bytes_read, struct.pack("<I", read_count))
            return read_result, (file, buffer, count, overlapped)

        def read_from_position(harness, arguments, position_result=position_result, stored=stored):
            file, position, count, silent, buffer = arguments
            harness.mu.mem_write(buffer, stored)
            return position_result, (file, position, count, silent & 0xFF)

        file = reference()
        externals = {
            **signature_externals(begin, end, signature),
            "file_open": open_,
            "ReadFile": read_file,
            "file_read_from_position": read_from_position,
        }
        if read_count is None:
            # what is not set by the console is where the reference is, which
            # is not the same number in the game and in source/ only if the
            # buffer moves: it does not
            pass
        t.check(
            "saved_game_file_read_signed",
            size,
            file,
            random_.getrandbits(32),
            random_.choice((0, 1, 0x80, 0x100)),
            Buffer(random_.randbytes(RESULT_SIZE)),
            externals=externals,
        )

    for n in range(120):
        size = random_.choice((0, 1, 0x130, 0x1E0))
        signature = random_.randbytes(SIGNATURE)
        opened = n % 11 != 10
        written = random_.choice((1, 1, 1, 0))
        begin = random_.choice((0x55, 0x55, 0x55, NONE))
        end = random_.choice((0, 0, 0, 5))
        position_result = random_.choice((1, 1, 1, 0))

        def write_to_position(harness, arguments, position_result=position_result):
            file, position, count, buffer = arguments
            return position_result, (file, position, count, bytes(harness.mu.mem_read(buffer, SIGNATURE)))

        t.check(
            "saved_game_file_write_signed",
            reference(),
            random_.getrandbits(32),
            size,
            random_.choice((0, 1, 0x80, 0x100)),
            Buffer(random_.randbytes(RESULT_SIZE)),
            externals={
                **signature_externals(begin, end, signature),
                "file_open": lambda harness, arguments, opened=opened: (opened, (arguments[0], arguments[1])),
                "file_write": lambda harness, arguments, written=written: written,
                "file_write_to_position": write_to_position,
            },
        )
