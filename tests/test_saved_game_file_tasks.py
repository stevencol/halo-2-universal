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

    # ---------- a file that is copied to give it another name

    SOURCE, DESTINATION, COPY_BUFFER, SIZE, READ_POSITION, WRITE_POSITION = 0x84, 0x88, 0x8C, 0x90, 0x94, 0x98
    PIECE = 0x20000

    def copy_description(size: int, read_position: int, write_position: int, source: int = 0x4321, destination: int = 0x8765) -> Buffer:
        data = bytearray(random_.randbytes(DESCRIPTION))
        struct.pack_into("<6I", data, SOURCE, source, destination, 0x80200000, size & 0xFFFFFFFF, read_position, write_position)
        return Buffer(bytes(data))

    def transfer(result: int, count: int | None):
        def handler(harness, arguments):
            file, buffer, size, done, overlapped = arguments
            if count is not None:
                harness.mu.mem_write(done, struct.pack("<I", count))
            # how many were done is left in the stack
            return result, (file, buffer, size, overlapped)

        return handler

    for size, position in (
        (0x130, 0),
        (PIECE, 0),
        (PIECE + 1, 0),
        (PIECE + 1, 1),
        (PIECE - 1, 0),
        (5 * PIECE, 2 * PIECE),
        (5 * PIECE, 4 * PIECE + 5),
        (0x1E0, 0x1E0),
        (0x100, 0x200),
        (NONE, 0),
    ):
        left = (size - position) & 0xFFFFFFFF
        piece = min(left, PIECE)
        for seek in (position, position ^ 1, NONE):
            for result, count in ((1, piece), (0, piece), (1, max(piece - 1, 0)), (1, piece + 1)):
                externals = {
                    "SetFilePointer": lambda harness, arguments, seek=seek: seek,
                    "ReadFile": transfer(result, count),
                    "WriteFile": transfer(result, count),
                }
                t.check("saved_game_file_copy_read", copy_description(size, position, random_.getrandbits(32)), externals=externals)
                t.check("saved_game_file_copy_write", copy_description(size, random_.getrandbits(32), position), externals=externals)

    for source in (0x4321, NONE, 0):
        for destination in (0x8765, NONE, 0):
            t.check(
                "saved_game_file_copy_close",
                copy_description(1, 2, 3, source, destination),
                externals={"CloseHandle": lambda harness, arguments: 1},
            )

    # ---------- the end of giving a file another name

    INPUT_GLOBALS_SIZE = 0x1B0

    def input_globals(mask: int) -> bytes:
        data = bytearray(random_.randbytes(INPUT_GLOBALS_SIZE))
        struct.pack_into("<H", data, 8, mask)
        data[10:18] = b"fghijklm"
        return bytes(data)

    def wide_text(harness, address: int) -> bytes:
        data = bytes(harness.mu.mem_read(address, 0x200))
        for offset in range(0, len(data), 2):
            if data[offset : offset + 2] == bytes(2):
                return data[:offset]
        return data

    for n in range(120):
        renamed = random_.choice((1, 1, 0, 0x80))
        type_ = random_.choice((0, 1, 7, 10, 12))
        unit = random_.choice((0, 0, 1, 3))
        file = type_ | unit << 4 | random_.randrange(0x2000) << 8 | random_.getrandbits(9) << 22
        description = bytearray(random_.randbytes(DESCRIPTION))
        struct.pack_into("<I", description, 0, file)
        # the name that the file has: one of them fills all its room, with no end
        old_name = random_.choice(("mine", "", "the old name", "a name of 17 char"))
        description[0x18 : 0x18 + 0x24] = (wide(old_name) + bytes(0x24))[:0x24]
        description[0x41] = random_.choice((0, 2, 4, 8, 0xFF, 0x80))
        description[0x44 : 0x44 + 0x22] = (wide(random_.choice(("another", "", "a name of 16 ch.", "0123456789abcdefg"))) + bytes(0x22))[:0x22]
        # the turn is in the same buffer, after what is told of the file
        struct.pack_into("<I", description, 0x80, DESCRIPTION)
        buffer = Buffer(bytes(description) + random_.randbytes(12), pointers=(0x80,))
        asked = n % 3 == 0
        t.check(
            "saved_game_file_rename_finish",
            buffer,
            renamed,
            globals={
                "input_globals": input_globals(random_.choice((0xFFFF, 1, 0, 2))),
                "global_language": NONE if asked else random_.randrange(9),
                "saved_game_files_string_list_index": random_.choice((NONE, 5)),
            },
            externals={
                "XGetLanguage": lambda harness, arguments, value=random_.choice((0, 1, 2, 5, 8)): value,
                "unicode_string_list_has_string": lambda harness, arguments: 1,
                "XDeleteSaveGame": lambda harness, arguments: (0, (text(harness, arguments[0]), wide_text(harness, arguments[1]))),
                "saved_game_file_set_information": lambda harness, arguments: (
                    0,
                    (bytes(harness.mu.mem_read(arguments[0], INFORMATION)), arguments[1]),
                ),
            },
        )

    # ---------- giving a file another name, a step at a time

    STATE = 0x9C

    def step_externals(results: dict, copy: int) -> dict:
        def open_copy(harness, arguments):
            # whether there is a file to copy is left in the stack
            harness.mu.mem_write(arguments[1], bytes([copy]))
            return results["open"], (arguments[0],)

        return {
            "unknown_002ad020": lambda harness, arguments: results["create"],
            "saved_game_file_write_image": lambda harness, arguments: (results["image"], (arguments[0], text(harness, arguments[1]))),
            "unknown_002ad220": open_copy,
            "saved_game_file_copy_read": lambda harness, arguments: results["read"],
            "saved_game_file_copy_write": lambda harness, arguments: results["write"],
            "saved_game_file_copy_close": lambda harness, arguments: 0,
            "saved_game_file_rename_finish": lambda harness, arguments: (0, (arguments[0], arguments[1] & 0xFF)),
            "CloseHandle": lambda harness, arguments: 1,
        }

    names = ("create", "image", "open", "read", "write")
    for state in list(range(-1, 9)) + [NONE, 0x100]:
        for failing in (None, *names):
            for copy in (0, 1, 0x80):
                for size, written in ((0x1000, 0x800), (0x1000, 0x1000), (0x1000, 0x1001), (0x80000000, 1)):
                    results = {name: int(name != failing) for name in names}
                    data = bytearray(random_.randbytes(DESCRIPTION))
                    struct.pack_into("<I", data, 0, random_.getrandbits(32))
                    data[4:0x18] = (random_.choice((b"u:\\udata\\4d\\AB\\", b"")) + bytes(0x14))[:0x14]
                    struct.pack_into("<6I", data, SOURCE, random_.choice((0x4321, NONE)), random_.choice((0x8765, 0)), 0x80200000, size, random_.getrandbits(32), written)
                    struct.pack_into("<I", data, STATE, state & 0xFFFFFFFF)
                    t.check(
                        "saved_game_file_rename_step",
                        Buffer(bytes(data)),
                        Buffer(random_.randbytes(4)),
                        externals=step_externals(results, copy),
                    )

    # ---------- the path of the file that is written

    for debug in (0, 1, 0x80):
        for type_ in (0, 1, 5, 10, 11, NONE):
            for directory in (b"u:\\udata\\4d\\AB\\", b"", b"d" * 0x13):
                data = bytearray(random_.randbytes(DESCRIPTION))
                data[4 : 4 + 0x14] = (directory + bytes(0x14))[:0x14]
                # the name of the file follows its directory: it is no part of the path
                data[0x18:0x1A] = bytes(2)
                struct.pack_into("<I", data, 0x3C, type_)
                data[0x7C] = debug
                t.check("saved_game_file_description_get_path", Buffer(bytes(data)), Buffer(random_.randbytes(0x100)))
