# source/tag_files/files.c, files_xbox.c
#
# The console's files are the test's: each function of its library is stood
# in for, and what is compared is what the game asks of it and what it leaves
# in the reference of the file

import random
import struct

from difftest import Buffer, Pointer
from test_async import calls_so_far

NONE = 0xFFFFFFFF
REFERENCE_SIZE = 0x110
PATH = 0x8
HANDLE = 0x108
POSITION = 0x10C

PATHS = (
    b"maps\\shared.map",
    b"d:\\core\\core.bin",
    b"Z:\\cache000.map",
    b"z:\\low",
    b"a:\\low",
    b"A:\\up",
    # a letter, and no colon after it
    b"ab\\x",
    b"u:\\saved\\game",
    b"",
    b"c",
    b"1:\\x",
    b"c:x",
    b"e:/x",
    b"[:\\x",
    b"`:\\x",
    b"{:\\x",
    b"@:\\x",
    b"a:\\" + b"y" * 300,
    b"z" * 300,
)


def text(harness, address: int) -> bytes:
    return bytes(harness.mu.mem_read(address, 0x200)).split(b"\0", 1)[0]


def run(t) -> None:
    random_ = random.Random(40)

    def reference(path: bytes | None = None, has_filename: bool = True, position: int | None = None) -> Buffer:
        data = bytearray(random_.randbytes(REFERENCE_SIZE))
        struct.pack_into("<IHh", data, 0, 0x66696C6F, (random_.getrandbits(16) & ~1) | has_filename, 0)
        path = random_.choice(PATHS) if path is None else path
        path = path[:0xFF] if random_.random() < 0.9 else path[:0x100]
        data[PATH : PATH + 0x100] = (path + bytes(0x100))[:0x100]
        struct.pack_into("<2I", data, HANDLE, 0x1234, random_.choice((0, 0x100, 0x7FFFFFF0)) if position is None else position)
        return Buffer(bytes(data))

    def errors(*values: int) -> dict:
        """The library's error, one value for each time that it is asked."""
        return {
            "GetLastError": lambda harness, arguments: values[min(calls_so_far(harness, "GetLastError"), len(values) - 1)],
            "SetLastError": lambda harness, arguments: 0,
        }

    def by_path(result: int):
        return lambda harness, arguments: (result, (text(harness, arguments[0]), *arguments[1:]))

    # ---------- paths

    for path in PATHS:
        t.check("file_location_get_full_path", Buffer(random_.randbytes(0x100)), Buffer(path + b"\0"))

    names = (b"", b"name", b"\\name", b"n" * 0x40, b"n" * 0x120)
    for path in (b"", b"d:\\", b"d:\\core", b"maps\\", b"p" * 0xF0, b"p" * 0xFE, b"p" * 0xFF):
        for name in names:
            buffer = (path + b"\0" + random_.randbytes(0x100))[:0x100]
            t.check("file_path_add_name", Buffer(buffer), Buffer(name + b"\0"))
            t.check("file_path_add_extension", Buffer(buffer), Buffer(name + b"\0"))

    for n in range(12):
        directory = reference()
        if n % 2:
            # a path that takes all of the reference, with no end
            data = bytearray(directory.data)
            data[PATH : PATH + 0x100] = b"w" * 0x100
            directory = Buffer(bytes(data))
        t.check(
            "find_files_start",
            Buffer(random_.randbytes(0x248 + 0x140)),
            random_.getrandbits(32),
            directory,
            Buffer(random_.choice((b"*.map", b"", b"q" * 0x120)) + b"\0"),
        )

    for depth in (-1, 0, 1, 5, 15):
        for _ in range(4):
            data = bytearray(random_.randbytes(0x248))
            struct.pack_into("<h", data, 4, depth)
            for index in range(16):
                if random_.random() < 0.5:
                    struct.pack_into("<I", data, 0x208 + index * 4, NONE)
            t.check("find_files_end", Buffer(bytes(data)), externals={"CloseHandle": lambda harness, arguments: 1})

    # ---------- a file

    for _ in range(60):
        has_filename = random_.random() < 0.7
        t.check(
            "file_delete",
            reference(has_filename=has_filename),
            externals={
                "SetFileAttributesA": by_path(random_.random() < 0.8),
                "DeleteFileA": by_path(random_.random() < 0.8),
                "RemoveDirectoryA": by_path(random_.random() < 0.7),
                **errors(5),
            },
        )

    for attributes in (0x80, 0x10, NONE):
        for last_error in ((2,), (3,), (5,), (5, 3), (5, 2), (3, 2)):
            t.check(
                "file_exists",
                reference(),
                externals={"GetFileAttributesA": by_path(attributes), **errors(*last_error)},
            )

    flags = (0, 1, 2, 3, 6, 7, 0xA, 0xB, 0x10, 0x11, 0x13, 0x21, 0x41, 0x81, 0x101, 0x1E1, 0x17, NONE)
    for flag in flags:
        for handle, last_error, end in (
            (0x4321, 0, 0x5000),
            (0x4321, 0, NONE),
            (NONE, 2, 0),
            (NONE, 3, 0),
            (NONE, 4, 0),
            (NONE, 5, 0),
            (NONE, 15, 0),
            (NONE, 32, 0),
            (NONE, 123, 0),
            (NONE, 0, 0),
            (NONE, 1, 0),
            (NONE, 124, 0),
            (NONE, 0x80000002, 0),
        ):
            t.check(
                "file_open",
                reference(),
                flag,
                Buffer(random_.randbytes(4)),
                externals={
                    "CreateFileA": by_path(handle),
                    "SetFilePointer": lambda harness, arguments, end=end: end,
                    "CloseHandle": lambda harness, arguments: 1,
                    **errors(last_error),
                },
            )

    for result in (1, 0):
        t.check("file_close", reference(), externals={"CloseHandle": lambda harness, arguments, result=result: result, **errors(6)})

    for _ in range(40):
        file = reference(position=0x100)
        position = random_.choice((0x100, 0x100, 0, 0x2000))
        moved = random_.choice((position, position, NONE))
        pointer = {"SetFilePointer": lambda harness, arguments, moved=moved: moved, **errors(6)}
        t.check("file_set_position", file, position, random_.choice((0, 1, 0x100)), externals=pointer)
        t.check(
            "file_set_eof",
            file,
            position,
            externals={**pointer, "SetEndOfFile": lambda harness, arguments, result=random_.random() < 0.7: result},
        )

    def transfer(name: str, result: int, count: int | None):
        """A read or a write: how many bytes the library says, if it says."""

        def handler(harness, arguments):
            file, buffer, size, bytes_done, overlapped = arguments
            if count is not None:
                harness.mu.mem_write(bytes_done, struct.pack("<I", count))
            return result, (file, buffer, size, overlapped)

        return {name: handler}

    for _ in range(80):
        file = reference(position=0x100)
        buffer = Buffer(random_.randbytes(0x40))
        count = random_.choice((0, 0x10, 0x40))
        done = random_.choice((count, count, count - 1 if count else 1, None))
        result = random_.random() < 0.8
        position = random_.choice((0x100, 0x100, 0, 0x2000))
        moved = random_.choice((position, position, position, NONE))
        pointer = {"SetFilePointer": lambda harness, arguments, moved=moved: moved, **errors(6)}
        silent = random_.choice((0, 0, 1))
        t.check("file_read", file, count, buffer, silent, externals={**transfer("ReadFile", result, done), **errors(6)})
        t.check("file_write", file, count, buffer, externals={**transfer("WriteFile", result, done), **errors(6)})
        t.check(
            "file_read_from_position",
            file,
            position,
            count,
            silent,
            buffer,
            externals={**transfer("ReadFile", result, done), **pointer},
        )
        t.check(
            "file_write_to_position",
            file,
            position,
            count,
            buffer,
            externals={**transfer("WriteFile", result, done), **pointer},
        )

    def attributes_ex(result: int, data: bytes):
        def handler(harness, arguments):
            path, level, address = arguments
            if result:
                harness.mu.mem_write(address, data)
            return result, (text(harness, path), level)

        return {"GetFileAttributesExA": handler, **errors(6)}

    for result in (1, 1, 0):
        for _ in range(4):
            data = random_.randbytes(0x24)
            t.check("file_get_last_modification_date", Buffer(random_.randbytes(8)), reference(), externals=attributes_ex(result, data))
            t.check("file_get_size", reference(), Buffer(random_.randbytes(4)), externals=attributes_ex(result, data))

    # ---------- the parts of a path

    split_paths = (
        b"",
        b"a",
        b".",
        b"\\",
        b"name",
        b"name.ext",
        b"name.",
        b".ext",
        b"name.one.two",
        b"d:\\name.ext",
        b"d:\\maps\\name.ext",
        b"d:\\maps\\fonts\\name.ext",
        b"d:\\maps\\fonts\\name",
        b"d:\\maps\\fonts\\",
        b"d:\\maps\\fonts\\\\",
        b"d:\\maps.old\\fonts\\name",
        b"d:\\maps.old\\fonts.new\\name.ext",
        b"d:\\maps\\fonts.new\\",
        b"\\name.ext",
        b"\\maps\\name.ext",
        b".\\name",
        b"..\\name.ext",
        b"maps\\.",
        b"maps\\.\\",
        b"maps\\name.ext.",
        b"\\\\",
        b"\\.\\",
        b"a\\b",
        b"a.b\\c.d\\e.f",
        b"p" * 0xFF,
    )
    for path in split_paths:
        for has_filename in (0, 1):
            t.check(
                "file_path_split",
                Buffer(path + b"\0" + random_.randbytes(8)),
                Buffer(random_.randbytes(4)),
                Buffer(random_.randbytes(4)),
                Buffer(random_.randbytes(4)),
                Buffer(random_.randbytes(4)),
                has_filename,
            )

    # ---------- going through the files of a directory

    # what the console finds, in the order it is asked: a file, a directory
    # or nothing more in the directory where the search is
    STATE_SIZE, DATA, DATA_SIZE, HANDLES = 0x248, 0x248, 0x140, 0x208
    DIRECTORY = 0x10

    def find_data(kind: str) -> bytes:
        data = bytearray(random_.randbytes(DATA_SIZE))
        attributes = random_.getrandbits(32) & ~DIRECTORY
        name = random_.choice((b"file.map", b"x", b"", b".x", b"..", b".", b"..x", b"long" * 60))[:0x103]
        if kind in ("directory", "dot", "dots"):
            attributes |= DIRECTORY
            name = {"directory": random_.choice((b"sub", b"fonts", b".a", b"..b", b"", b"d" * 0x103)), "dot": b".", "dots": b".."}[kind]
        struct.pack_into("<I", data, 0, attributes)
        data[0x2C : 0x2C + len(name) + 1] = name + b"\0"
        return bytes(data)

    def search(found: list) -> dict:
        def step(harness, data: int):
            index = calls_so_far(harness, "FindFirstFileA") + calls_so_far(harness, "FindNextFileA")
            kind = found[index] if index < len(found) else "end"
            if kind != "end":
                harness.mu.mem_write(data, datas[index])
            return kind

        datas = [find_data(kind) for kind in found]

        def first(harness, arguments):
            path, data = arguments
            kind = step(harness, data)
            handle = NONE if kind == "end" else 0x5000 + calls_so_far(harness, "FindFirstFileA")
            return handle, (text(harness, path), data)

        def next_(harness, arguments):
            return step(harness, arguments[1]) != "end"

        return {"FindFirstFileA": first, "FindNextFileA": next_, "CloseHandle": lambda harness, arguments: 1}

    kinds = ("file", "file", "directory", "directory", "dot", "dots", "end", "end")
    for n in range(400):
        data = bytearray(random_.randbytes(STATE_SIZE + DATA_SIZE))
        depth = random_.choice((0, 0, 0, 1, 2, 5, -1, -3, 14))
        struct.pack_into("<Ihh", data, 0, random_.choice((0, 1, 2, 3, 4, 5, 6, 7, random_.getrandbits(32))), depth, random_.getrandbits(15))
        path = random_.choice((b"", b"d:\\maps", b"d:\\maps\\", b"maps\\fonts\\deep", b"x", b"\\", b"p" * 0xF8, b"p" * 0xFF))
        if depth > 0:
            path += b"".join(b"\\d%d" % index for index in range(depth))
        path = path[:0xFF]
        data[8 : 8 + 0x100] = (path + bytes(0x100))[:0x100]
        pattern = random_.choice((b"*.map", b"*", b"", b"q" * 0xFF))
        data[0x108 : 0x108 + 0x100] = (pattern + bytes(0x100))[:0x100]
        for index in range(16):
            # some directories above have a search open, and this one may
            struct.pack_into("<I", data, HANDLES + index * 4, random_.choice((NONE, NONE, 0x6000 + index)))
        found = [random_.choice(kinds) for _ in range(random_.randint(0, 8))]
        t.check(
            "find_files_next",
            Buffer(bytes(data)),
            Buffer(random_.randbytes(REFERENCE_SIZE)),
            random_.choice((0, Buffer(random_.randbytes(8)), Buffer(random_.randbytes(8)))),
            externals=search(found),
        )

    # ---------- making a reference, a file and a directory

    for path in PATHS + (b"p" * 0xFF, b"q" * 0x100):
        for directory in (0, 1, 0x80):
            t.check(
                "file_reference_create_from_path",
                Buffer(random_.randbytes(REFERENCE_SIZE)),
                Buffer(path + b"\0"),
                directory | random_.getrandbits(24) << 8,
            )

    for has_filename in (True, False):
        for handle, made in ((0x4321, 1), (NONE, 0), (0, 0x100)):
            for _ in range(4):
                t.check(
                    "file_create",
                    reference(has_filename=has_filename),
                    externals={
                        "CreateFileA": lambda harness, arguments, handle=handle: (handle, (text(harness, arguments[0]), *arguments[1:])),
                        "CloseHandle": lambda harness, arguments: 1,
                        "CreateDirectoryA": lambda harness, arguments, made=made: (made, (text(harness, arguments[0]), arguments[1])),
                        **errors(5),
                    },
                )

    # the files of a directory: the search is stood in for
    for maximum in (0, 1, 3, 4, 5, NONE, 0x80000000):
        for found in (0, 1, 3, 4, 10):
            count = 5

            def next_(harness, arguments, found=found):
                state, file, date = arguments
                index = calls_so_far(harness, "find_files_next")
                if index < found:
                    harness.mu.mem_write(file, bytes([index + 1]) * REFERENCE_SIZE)
                # the state is in the stack, which is not at the same place in the game and in source/
                return index < found, (file, date)

            def start(harness, arguments):
                state, flags, directory, pattern = arguments
                return 0, (flags, directory, text(harness, pattern))

            t.check(
                "find_files",
                random_.getrandbits(32),
                reference(),
                maximum,
                Buffer(random_.randbytes(count * REFERENCE_SIZE)),
                externals={"find_files_start": start, "find_files_next": next_, "find_files_end": lambda harness, arguments: (0, ())},
            )
