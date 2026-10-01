# finding and reading the files of the maps (source/cache/cache_files.c), the
# tasks of source/cseries/async.c, and two string functions of
# source/cseries/cseries.c
#
# The functions of the console that open and read files are not decompiled:
# the test stands in for them

import random
import struct

from difftest import Buffer
from test_cache_files import header as map_header

NONE = 0xFFFFFFFF
MAP_FILE_SIZE = 0x804
MAP_FILE_COUNT = 6
# in a map file of the table: the handle, then the header, which has the path at 0x20
PATH_OFFSET = 4 + 0x20


def string(text: bytes) -> Buffer:
    return Buffer(text + b"\0")


def read_string(harness, address: int) -> bytes:
    return bytes(harness.mu.mem_read(address, 0x200)).split(b"\0", 1)[0]


def map_files(random_: random.Random, paths: dict[int, bytes], open_map_index: int = NONE) -> bytes:
    """The table of the map files, with a path in some of them."""
    data = bytearray(random_.randbytes(MAP_FILE_SIZE * MAP_FILE_COUNT))
    for index in range(MAP_FILE_COUNT):
        path = paths.get(index, b"")
        at = index * MAP_FILE_SIZE + PATH_OFFSET
        data[at : at + 256] = path.ljust(256, b"\0")
    return bytes(data) + struct.pack("<I", open_map_index)


def run(t) -> None:
    random_ = random.Random(15)

    # ---------- strings

    words = [b"", b"a", b"A", b"lockout", b"LOCKOUT", b"Lockout", b"lockouts", b"lockin", b"d:\\maps\\lockout.map"]
    words += [b"D:\\MAPS\\LOCKOUT.MAP", b"z", b"Z", b"[", b"@", b"`", b"{", b"\xff", b"\x80abc", b"\xe9t\xe9", b"\xc9T\xc9"]
    for a in words:
        for b in words:
            t.check("csstrcasecmp", string(a), string(b))

    formats = [
        (b"%s.map", string(b"lockout")),
        (b"d:\\maps\\%s.map", string(b"ascension")),
        (b"%s", string(b"")),
        (b"no arguments", 0),
        (b"%s", string(b"x" * 254)),
        (b"%s", string(b"x" * 255)),
        (b"%s", string(b"x" * 256)),
        (b"%s", string(b"x" * 400)),
        (b"ab%sab", string(b"y" * 252)),
        (b"%scache%03d", string(b"z:\\")),
        (b"%d", 0),
        (b"%d", 9809),
        (b"%d", -1),
        (b"%i items", 0x7FFFFFFF),
        (b"%u", 0xFFFFFFFF),
        (b"%x", 0x2651),
        (b"%X", 0xDEADBEEF),
        (b"%08x", 0x1234),
        (b"%5d|", 42),
        (b"%-5d|", 42),
        (b"%05d|", -42),
        (b"%c%%", 0x41),
        (b"%10s|", string(b"abc")),
        (b"%-10s|", string(b"abc")),
        (b"%.2s|", string(b"abc")),
    ]
    for format_, argument in formats:
        t.check("csprintf256", Buffer(random_.randbytes(256)), string(format_), argument)

    # ---------- the name and the path of a map

    paths = [b"lockout", b"Lockout", b"LOCKOUT.MAP", b"scenarios\\multi\\Lockout\\Lockout", b"a.b.c", b".hidden.map"]
    paths += [b"...", b"..a.b", b".", b"", b"[AZ]@`{az}", b"\xc9T\xc9.map", b"A" * 254, b"B" * 255, b"C" * 256]
    paths += [b"D" * 300, b"E" * 250 + b".map.map", b"F" * 255 + b".map", b"t:\\maps\\Test.map", b"w:\\X"]
    for path in paths:
        t.check("cache_file_get_map_name", string(path), Buffer(random_.randbytes(256)))

    names = [*paths, b"t:\\", b"t:", b"t", b"T:\\maps\\upper", b"w:\\", b"w:/maps", b"x:\\maps\\other", b"t:\\a\\b\\C.D"]
    names += [b"scenarios\\solo\\03a_oldmombasa\\03a_oldmombasa", b"trailing\\", b"\\leading", b"a\\b\\" + b"G" * 300]
    for name in names:
        t.check("cache_file_get_map_path", string(name), Buffer(random_.randbytes(256)))

    # ---------- the table of the map files

    lockout = b"d:\\maps\\lockout.map"
    for index in range(MAP_FILE_COUNT):
        files = map_files(random_, {index: lockout, (index + 2) % MAP_FILE_COUNT: b"d:\\maps\\shared.map"})
        for name in (b"scenarios\\multi\\lockout\\lockout", b"LOCKOUT", b"shared", b"lockou", b"lockout2", b""):
            t.check("cached_map_files_find_map", string(name), globals={"cached_map_file_globals": files})
    # the same map twice, a path in another case, and a map on another drive
    files = map_files(random_, {1: lockout, 4: lockout, 3: b"D:\\Maps\\Ascension.MAP", 5: b"t:\\test.map"})
    for name in (b"lockout", b"ascension", b"t:\\test", b"t:\\TEST.map", b"test", b"w:\\test"):
        t.check("cached_map_files_find_map", string(name), globals={"cached_map_file_globals": files})

    # the header of a map, from its file
    good = map_header().data
    for file in (0x1234, NONE):
        for read_result in (1, 0):
            for bytes_read in (0x800, 0x7FF, 0):
                for data in (good, map_header(version=7).data, random_.randbytes(0x800)):

                    def create_file(harness, arguments):
                        return file, (read_string(harness, arguments[0]), *arguments[1:])

                    def read_file(harness, arguments, data=data, read_result=read_result, bytes_read=bytes_read):
                        handle, buffer, size, bytes_read_address, overlapped = arguments
                        harness.mu.mem_write(buffer, data[:size])
                        harness.mu.mem_write(bytes_read_address, struct.pack("<I", bytes_read))
                        return read_result, (handle, buffer, size, overlapped)

                    t.check(
                        "cache_file_read_header",
                        Buffer(random_.randbytes(0x800)),
                        string(b"scenarios\\multi\\Lockout\\Lockout"),
                        externals={
                            "CreateFileA": create_file,
                            "ReadFile": read_file,
                            "CloseHandle": lambda harness, arguments: 1,
                        },
                    )

    # a read: the file is that of the two high bits of the offset
    for open_map_index in range(MAP_FILE_COUNT):
        files = map_files(random_, {}, open_map_index)
        for offset in (0, 0x00123400, 0x3FFFFFFF, 0x40000000, 0x7FFFFE00, 0x80000800, 0xBFFFFFFF, 0xC0000000, 0xFFFFFFFF):
            t.check(
                "cache_file_read_async",
                offset,
                random_.getrandbits(20),
                0x80010000,
                0x60000100,
                0x60000200,
                random_.randrange(8),
                random_.randrange(8),
                globals={"cached_map_file_globals": files},
                externals={"async_read_position": lambda harness, arguments: 0},
            )

    # ---------- the tasks

    for categories in ((), (2,), (5, 3, 6), (1, 1, 4, 0xFFFFFFFF, 7, 2)):
        tasks = bytearray()
        pointers = []
        for index, category in enumerate(categories):
            task = bytearray(random_.randbytes(0x40))
            last = index == len(categories) - 1
            struct.pack_into("<I", task, 0x30, category)
            struct.pack_into("<I", task, 0x3C, 0 if last else (index + 1) * 0x40)
            if not last:
                pointers.append(index * 0x40 + 0x3C)
            tasks += task
        for category in (0, 1, 2, 3, 4, 5, 6, 7, 0xFFFFFFFF):
            t.check(
                "async_category_busy",
                category,
                globals={
                    "async_mutex": 0x4321,
                    "async_task_list": Buffer(bytes(tasks), pointers=tuple(pointers)) if categories else 0,
                },
                externals={
                    "WaitForSingleObject": lambda harness, arguments: 0,
                    "ReleaseMutex": lambda harness, arguments: 1,
                },
            )

    # waiting for the reads: each category is busy for some checks
    for plan in ({}, {2: 1}, {6: 3}, {2: 2, 3: 1, 4: 4, 5: 1, 6: 2}, {4: 5}):
        remaining: dict[int, int] = {}

        def busy(harness, arguments, plan=plan, remaining=remaining):
            (category,) = arguments
            if category == 2 and remaining.get("done", True):
                # a new call of the function
                remaining.clear()
                remaining.update(plan)
                remaining["done"] = False
            if remaining.get(category, 0) > 0:
                remaining[category] -= 1
                return 1
            if category == 6:
                remaining["done"] = True
            return 0

        t.check(
            "cache_file_block_until_not_busy",
            externals={"async_category_busy": busy, "SwitchToThread": lambda harness, arguments: 0},
        )
