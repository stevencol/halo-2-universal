# source/saved games/saved_game_files.c
#
# The console's library is stood in for: what it says of the hard disk is
# the test's

import random
import struct

from difftest import Buffer, GlobalData, GlobalRange, Memory, Mock, mock_address
from test_async import calls_so_far

NONE = 0xFFFFFFFF
GLOBALS_SIZE = 0x1B0
UNIT_MASK = 8
VARIANT_SIZE = 0x130


def text(harness, address: int) -> bytes:
    return bytes(harness.mu.mem_read(address, 0x200)).split(b"\0", 1)[0]


def input_globals(random_: random.Random, mask: int) -> bytes:
    data = bytearray(random_.randbytes(GLOBALS_SIZE))
    struct.pack_into("<H", data, UNIT_MASK, mask)
    # the letters of the memory units
    data[UNIT_MASK + 2 : UNIT_MASK + 10] = b"fghijklm"
    return bytes(data)


def run(t) -> None:
    random_ = random.Random(49)

    # ---------- the types of the files

    for engine in (1, 2, 3, 4, 7, 8, 9):
        data = bytearray(random_.randbytes(VARIANT_SIZE))
        struct.pack_into("<I", data, 0x44, engine)
        t.check("saved_game_file_type_from_variant", Buffer(bytes(data)))
    for game in range(7):
        t.check("saved_game_file_type_from_listed_game", game)
    for type_ in (1, 2, 4, 5, 7, 8, 9):
        t.check("saved_game_file_type_to_listed_game", type_)

    types = list(range(-2, 14)) + [NONE, 0x7FFFFFFF, 0x80000000, 0x100, 0x10000]
    for type_ in types:
        t.check("saved_game_file_type_get_string_id", type_)
        t.check("saved_game_file_type_get_size", type_)
        # a name is in the game's data or in that of source/: its text is compared
        t.check("saved_game_file_type_get_name", type_)

    # ---------- the room of the hard disk

    def cluster_size(size: int):
        return lambda harness, arguments, size=size: (size, (text(harness, arguments[0]),))

    for mask in (0, 1, 2, 3, 0xFFFE, 0xFFFF):
        for cluster in (0x4000, 0x1000, 0, 0x8000, 1, 0x10000):
            for size in (0, 1, 0x130, 0x1E0, 0x15CB8, 0x4000, 0x3FFF, 0x4001, NONE, 0x80000000, 0x7FFFFFFF & ~0xFFFFF, 0xFFFF0000):
                t.check(
                    "saved_game_file_size_to_blocks",
                    size,
                    globals={"input_globals": input_globals(random_, mask)},
                    externals={"XGetDiskClusterSize": cluster_size(cluster)},
                )
            for type_ in (0, 1, 5, 9, 10, 11, NONE, 0x80000000):
                t.check(
                    "saved_game_file_type_get_blocks",
                    type_,
                    globals={"input_globals": input_globals(random_, mask)},
                    externals={"XGetDiskClusterSize": cluster_size(cluster)},
                )

    def free_space(result: int, free: int):
        def handler(harness, arguments):
            root, available, total, total_free = arguments
            harness.mu.mem_write(available, struct.pack("<Q", free))
            harness.mu.mem_write(total, struct.pack("<Q", free ^ 0x5555555555555555))
            harness.mu.mem_write(total_free, struct.pack("<Q", free ^ 0xAAAAAAAAAAAAAAAA))
            # the three numbers are in the stack, which is not at the same place in the game and in source/
            return result, (text(harness, root),)

        return handler

    for blocks in (0, 1, 2, 0x100, 0x1FFFF, 0x20000, 0x3FFFF, 0x40000, 0x7FFFF, NONE, 0x80000000):
        needed = (blocks << 14) & 0xFFFFFFFF
        needed -= (1 << 32) if needed & 0x80000000 else 0
        needed &= 0xFFFFFFFFFFFFFFFF
        frees = {0, 1, needed, (needed - 1) & 0xFFFFFFFFFFFFFFFF, (needed + 1) & 0xFFFFFFFFFFFFFFFF}
        frees |= {needed ^ 0x100000000, (needed & 0xFFFFFFFF) | 0x100000000, 0xFFFFFFFFFFFFFFFF, 0x1_0000_0000, 0xFFFFFFFF}
        for free in sorted(frees):
            for result in (1, 0, 0x100):
                t.check(
                    "saved_game_files_hard_disk_has_blocks",
                    blocks,
                    globals={"input_globals": input_globals(random_, random_.choice((1, 1, 0, 3)))},
                    externals={"GetDiskFreeSpaceExA": free_space(result, free)},
                )

    # ---------- paths

    for unit in (0, 1, 2, 8, 15, 16, 31, 32, 33):
        for mask in (0, 1, 2, 4, 0x100, 0x8000, 0xFFFF):
            for size in (8, 4, 3, 1, 0x100):
                t.check(
                    "saved_game_files_get_unit_root",
                    unit,
                    Buffer(random_.randbytes(0x10)),
                    size | random_.getrandbits(16) << 16,
                    globals={"input_globals": input_globals(random_, mask)},
                )

    directories = (b"", b"u:\\saved\\", b"u:\\saved", b"d" * 0xF0, b"d" * 0xFC + b"\\", b"d" * 0xFF, b"d" * 0x120)
    for directory in directories:
        for type_ in (0, 1, 5, 9, 10, 11, NONE):
            t.check(
                "saved_game_file_get_type_path",
                Buffer(random_.randbytes(0x100)),
                Buffer(directory + b"\0"),
                type_,
            )

    # ---------- the indices of the files

    BUILT_IN = 1 << 21

    def index(type_: int, unit: int, file: int, built_in: int = 0, salt: int = 0, top: int = 0) -> int:
        """The index of a file: its type, its drive, its place among those of the drive."""
        return type_ | unit << 4 | file << 8 | built_in << 21 | salt << 22 | top << 31

    indices = [0, 1, NONE, BUILT_IN, BUILT_IN - 1, BUILT_IN + 1, 0x7FFFFFFF, 0x80000000, 0x80000000 | BUILT_IN]
    indices += [index(random_.randrange(16), random_.randrange(16), random_.randrange(0x2000), random_.getrandbits(1), random_.randrange(0x200), random_.getrandbits(1)) for _ in range(12)]
    for file in indices:
        for other in indices:
            t.check("saved_game_file_index_after", file, other, random_.getrandbits(32))

    for n in range(300):
        type_ = random_.randrange(16)
        file_index = random_.choice((0, 1, 2, 0x1FFF, random_.randrange(0x2000)))
        deleted_index = random_.choice((0, file_index, max(file_index - 1, 0), min(file_index + 1, 0x1FFF), random_.randrange(0x2000)))
        file = index(random_.randrange(16), type_, file_index, n % 7 == 6, random_.randrange(0x200), random_.getrandbits(1))
        deleted = index(random_.randrange(16), type_ if n % 5 else random_.randrange(16), deleted_index, random_.getrandbits(1), random_.randrange(0x200), random_.getrandbits(1))
        t.check(
            "saved_game_file_index_after_delete",
            file,
            deleted,
            globals={
                "unknown_0051ea14": 0 if n % 11 == 10 else Buffer(bytes(0x4BEFC) + struct.pack("<I", random_.getrandbits(32))),
            },
        )

    # ---------- waiting for the files

    def wait_until(name: str, offset: int, waits: int):
        """SwitchToThread: the flag that the game waits for is set after so many."""

        def handler(harness, arguments):
            if calls_so_far(harness, "SwitchToThread") >= waits - 1:
                harness.mu.mem_write(harness.global_addresses[name] + offset, b"\x01")
            return 0

        return handler

    def information(found: int, directory: bytes):
        def handler(harness, arguments):
            where, file = arguments
            before = bytes(harness.mu.mem_read(where, 1)) + bytes(harness.mu.mem_read(where + 0x14, 2))
            harness.mu.mem_write(where, (directory + bytes(0x40))[:0x40])
            # what is told of the file is in the stack
            return found, (before, file)

        return handler

    directories = (b"u:\\udata\\4d530064\\0123456789AB\\", b"", b"d" * 0x3F)
    for n in range(60):
        waits = random_.choice((0, 0, 1, 3))
        file = index(random_.randrange(16), random_.randrange(16), random_.randrange(0x2000), n % 6 == 5, random_.randrange(0x200), random_.getrandbits(1))
        t.check(
            "saved_game_file_get_directory",
            Buffer(random_.randbytes(0x100)),
            file,
            globals={"unknown_0055c14c": bytes([waits == 0])},
            externals={
                "SwitchToThread": wait_until("unknown_0055c14c", 0, waits),
                "sound_idle": lambda harness, arguments: 0,
                "saved_game_file_get_information": information(n % 4 != 3, random_.choice(directories)),
            },
        )

    for n in range(30):
        waits = random_.choice((0, 0, 1, 3))
        t.check(
            "saved_game_files_take_operation",
            random_.getrandbits(32),
            Buffer(random_.randbytes(4), places=(0,)),
            # the function gives a pointer to the global, which is not at the
            # same address in the game and in source/
            globals={"saved_game_files_operation": GlobalData(bytes([waits == 0]) + random_.randbytes(11))},
            externals={
                "GetCurrentThreadId": lambda harness, arguments, thread=random_.getrandbits(32): thread,
                "SwitchToThread": wait_until("saved_game_files_operation", 0, waits),
                "sound_idle": lambda harness, arguments: 0,
            },
        )

    # ---------- the file of the state of the game

    def start(result: int):
        def handler(harness, arguments):
            path, *rest = arguments
            return result, (text(harness, path), *rest)

        return handler

    for name, callee in (("saved_game_auxiliary_read", "saved_game_state_read_task_new"), ("saved_game_auxiliary_write", "saved_game_state_write_task_new")):
        for n in range(40):
            file = index(random_.randrange(16), random_.randrange(16), random_.randrange(0x2000), n % 6 == 5, random_.randrange(0x200))
            t.check(
                name,
                random_.getrandbits(32),
                random_.getrandbits(32),
                random_.getrandbits(32),
                random_.getrandbits(32),
                random_.getrandbits(32),
                file,
                globals={"unknown_0055c14c": b"\x01"},
                externals={
                    "saved_game_file_get_information": information(n % 4 != 3, random_.choice(directories + (b"f" * 0x30,))),
                    callee: start(random_.choice((0, 1, 1))),
                    "SwitchToThread": lambda harness, arguments: 0,
                    "sound_idle": lambda harness, arguments: 0,
                },
            )

    # ---------- the saved games of a drive

    for unit in (0, 1, 3):
        for mask in (0, 1, 0xFFFF):
            for found in (None, 0, 1, 5):
                def first(harness, arguments, found=found):
                    return (NONE if found is None else 0x7700), (text(harness, arguments[0]),)

                def next_(harness, arguments, found=found):
                    return (0x100 if calls_so_far(harness, "XFindNextSaveGame") < found else 0), (arguments[0],)

                t.check(
                    "saved_game_files_count_saved_games",
                    unit,
                    globals={"input_globals": input_globals(random_, mask)},
                    externals={"XFindFirstSaveGame": first, "XFindNextSaveGame": next_, "XFindClose": lambda harness, arguments: 1},
                )

    # ---------- what the game knows of the files

    TABLES, TABLE_SIZE, INFORMATION = 0xBEF8, 0x40004, 0x40
    # two tables: the second starts where the salt of the indices is
    FILES_SIZE = TABLES + 2 * TABLE_SIZE
    CACHED = 0x44

    def files(counts: tuple) -> Buffer:
        """The tables, each with so many files; the rest of the memory is not
        looked at, and is left at zero so that the test is not slow."""
        data = bytearray(FILES_SIZE)
        for table, count in enumerate(counts):
            base = TABLES + table * TABLE_SIZE
            struct.pack_into("<i", data, base, count)
            used = min(max(count, 0), 12) + 2
            data[base + 4 : base + 4 + used * INFORMATION] = random_.randbytes(used * INFORMATION)
        return Buffer(bytes(data))

    def cache(valid: int, count: int, entries: list) -> dict:
        data = bytearray(random_.randbytes(4 * CACHED))
        for n, file in enumerate(entries):
            struct.pack_into("<I", data, n * CACHED, file)
        return {
            "saved_game_files_cache_valid": bytes([valid]),
            "cached_saved_game_file_count": count,
            "cached_saved_game_files": bytes(data),
        }

    def file_cases() -> list:
        """Indices of files, and tables for them: in the tables, past their
        ends, built in, and with no tables."""
        cases = []
        for count in (0, 1, 5, 12, -1, -3):
            # 0x1001: all of the 13 bits of the place count
            for place in (0, 1, 4, 5, 6, 11, 12, 0x1FFF, 0x1001):
                if count >= 0 or place < 3:
                    table = random_.randrange(2)
                    counts = [random_.randrange(12), random_.randrange(12)]
                    counts[table] = count
                    cases.append((index(random_.randrange(16), table, place, 0, random_.randrange(0x200), random_.getrandbits(1)), tuple(counts)))
        cases.append((index(3, 1, 2, 1, 5), (4, 4)))
        cases.append((index(0, 0, 0, 1), (4, 4)))
        return cases

    for file, counts in file_cases():
        t.check(
            "saved_game_file_get_information",
            Buffer(random_.randbytes(INFORMATION)),
            file,
            globals={"unknown_0051ea14": files(counts), **cache(1, 4, [file] * 4)},
        )
        t.check("saved_game_file_delete_information", file, globals={"unknown_0051ea14": files(counts)})
        for done, same_file, same_thread in ((1, 1, 1), (1, 0, 0), (0, 1, 0), (0, 1, 1), (0, 0, 0), (0x80, 0, 1)):
            thread = random_.getrandbits(32)
            t.check(
                "saved_game_file_set_information",
                Buffer(random_.randbytes(INFORMATION)),
                file,
                globals={
                    "unknown_0051ea14": files(counts),
                    "saved_game_files_operation": struct.pack("<B3x2I", done, file if same_file else file ^ 0x100, thread if same_thread else thread ^ 1),
                    **cache(1, 4, [file] * 4),
                },
                externals={"GetCurrentThreadId": lambda harness, arguments, thread=thread: thread},
            )

    # with no tables, what is kept of a few files
    for n in range(80):
        file = random_.getrandbits(32) if n % 9 else NONE
        entries = [random_.getrandbits(32) for _ in range(4)]
        if n % 3:
            entries[random_.randrange(4)] = file
        if n % 5 == 4:
            entries[0] = file
            entries[random_.randrange(1, 4)] = file
        globals_ = {"unknown_0051ea14": 0, **cache(random_.choice((1, 1, 1, 0, 0x80)), random_.choice((4, 4, 4, 0, 1, 2, -1)), entries)}
        t.check("saved_game_file_get_information", Buffer(random_.randbytes(INFORMATION)), file, globals=globals_)
        t.check(
            "saved_game_file_set_information",
            Buffer(random_.randbytes(INFORMATION)),
            file,
            globals={**globals_, "saved_game_files_operation": struct.pack("<B3x2I", 1, 0, 0)},
            externals={"GetCurrentThreadId": lambda harness, arguments: 7},
        )
        t.check("saved_game_file_delete_information", file, globals={"unknown_0051ea14": 0})

    for table in (0, 1):
        # a table of the first two with more files than it can have: the file
        # goes past its end, into the next
        for count in (0, 1, 5, 12, 0xFFF, 0x1000, -1) + ((0x1001,) if table == 0 else ()):
            counts = [3, 3]
            counts[table] = count
            t.check(
                "saved_game_file_add_information",
                table,
                Buffer(random_.randbytes(INFORMATION)),
                Buffer(random_.randbytes(4)),
                globals={"unknown_0051ea14": files(tuple(counts))},
            )
    t.check("saved_game_file_add_information", 0, Buffer(random_.randbytes(INFORMATION)), Buffer(random_.randbytes(4)), globals={"unknown_0051ea14": 0})

    # ---------- the name of a file

    def wide(text_: str) -> bytes:
        return text_.encode("utf-16-le") + b"\0\0"

    names = ("", "profile", "profile 2", "profil", "Profile", "p", "a much longer name than fits")
    for known in (1, 0):
        for kept in names[:6]:
            for asked in names:
                def get(harness, arguments, known=known, kept=kept):
                    where, file = arguments
                    data = bytearray(random_.randbytes(INFORMATION))
                    data[0x14 : 0x14 + len(wide(kept))] = wide(kept)
                    harness.mu.mem_write(where, bytes(data))
                    return known, (file,)

                t.check(
                    "saved_game_file_name_differs",
                    random_.getrandbits(32),
                    Buffer(wide(asked)),
                    externals={"saved_game_file_get_information": get},
                )
    t.check(
        "saved_game_file_name_differs",
        5,
        0,
        externals={"saved_game_file_get_information": lambda harness, arguments: (1, (arguments[1],))},
    )

    # ---------- the picture of a variant

    image = random_.randbytes(0x1000)

    def reference_path(harness, address: int) -> tuple:
        # of a reference that is in the stack: what it has but for the handle and the position
        return bytes(harness.mu.mem_read(address, 0x108))

    for type_ in list(range(-1, 13)) + [NONE, 0x80000000]:
        for results in ((1, 1, 1, 1), (0, 1, 1, 1), (1, 0, 1, 1), (1, 1, 0, 1), (1, 1, 1, 0)):
            directory = random_.choice((b"u:\\udata\\4d530064\\0123456789AB\\", b"", b"d" * 0xF8, b"d" * 0x110))
            t.check(
                "saved_game_file_write_image",
                type_,
                Buffer(directory + b"\0"),
                globals={"saved_game_image": image},
                externals={
                    "file_create": lambda harness, arguments, result=results[0]: (result, (reference_path(harness, arguments[0]),)),
                    "file_open": lambda harness, arguments, result=results[1]: (result, (reference_path(harness, arguments[0]), arguments[1])),
                    "file_write": lambda harness, arguments, result=results[2]: (
                        result,
                        (reference_path(harness, arguments[0]), arguments[1], bytes(harness.mu.mem_read(arguments[2], 0x1000))),
                    ),
                    "file_close": lambda harness, arguments, result=results[3]: (result, (reference_path(harness, arguments[0]),)),
                },
            )

    # ---------- the name that the console shows of a file

    TYPE_NAMES = {
        0: ("Profile", "\u30d7\u30ec\u30a4\u30e4\u30fc", "Profil", "Profil", "Perfil", "Profilo", "\ud504\ub85c\ud544", "\u8a2d\u5b9a\u6a94", "Profile"),
        1: ("Slayer", "Slayer", "Showdown", "Assassin", "Asesino", "Massacro", "Slayer", "\u6bba\u622e\u4e4b\u738b", "Slayer"),
        7: ("Capture the Flag", "Capture the Flag", "Kampf um die Flagge", "Capture du drapeau", "Capturar la bandera", "Conquista la bandiera", "Capture the Flag", "\u6436\u65d7\u4e4b\u738b", "Capture the Flag"),
        10: ("Playlist", "\u30d7\u30ec\u30a4\u30ea\u30b9\u30c8", "Spielliste", "Liste", "Lista de juego", "Elenco partite", "\uac8c\uc784 \ubaa9\ub85d", "\u904a\u6232\u6e05\u55ae", "Playlist"),
    }
    given = ("", "mine", "a name of 16 ch.", "a name that is longer than sixteen", "x" * 140, "Slayer: nested")

    for type_ in (0, 1, 5, 7, 10, 11, NONE):
        for language in (0, 1, 2, 4, 7, 8, 9, NONE):
            for list_index in (0x12340005, NONE):
                for has in (1, 0):
                    t.check(
                        "saved_game_file_get_display_name",
                        type_,
                        Buffer(random_.randbytes(0x108)),
                        Buffer(wide(random_.choice(given))),
                        language,
                        globals={"saved_game_files_string_list_index": list_index},
                        externals={"unicode_string_list_has_string": lambda harness, arguments, has=has: has},
                    )

    for type_, names in TYPE_NAMES.items():
        for first_language in (0, 1, 3, 4, 8, 9, 20, NONE):
            cases = [name + ": " + random_.choice(given[:4]) for name in names]
            # no separator, another one, the name of another type, nothing
            cases += [names[0] + ":", names[0] + " :x", names[0], names[2] + ":  two", "Oddball: x", "", ": x", names[4].lower() + ": x"]
            for display_name in cases:
                t.check(
                    "saved_game_file_parse_display_name",
                    Buffer(wide(display_name) + bytes(0x40)),
                    type_,
                    Buffer(random_.randbytes(4)),
                    Buffer(random_.randbytes(0x28)),
                    globals={"saved_game_files_language": first_language},
                )
    for type_ in (11, NONE, 5):
        for display_name in ("", ": x", "Juggernaut: x", "x"):
            t.check(
                "saved_game_file_parse_display_name",
                Buffer(wide(display_name) + bytes(0x40)),
                type_,
                Buffer(random_.randbytes(4)),
                Buffer(random_.randbytes(0x28)),
                globals={"saved_game_files_language": 0},
            )

    # ---------- whether a name is free

    def wide_text(harness, address: int) -> bytes:
        data = bytes(harness.mu.mem_read(address, 0x200))
        for offset in range(0, len(data), 2):
            if data[offset : offset + 2] == bytes(2):
                return data[:offset]
        return data

    def create(result: int):
        def handler(harness, arguments):
            root, name, disposition, flags, path, path_size = arguments
            # the path that the console would fill is in the stack: the game gives it all zeros
            return result, (text(harness, root), wide_text(harness, name), disposition, flags, bytes(harness.mu.mem_read(path, 0x100)), path_size)

        return handler

    for name in (None, "", "mine", "a name that is longer than sixteen"):
        for files, mask in ((0x80100000, 1), (0, 1), (0x80100000, 0), (0x80100000, 2), (0x80100000, 0xFFFF)):
            for result in (0, 1, 183, 0x80000000):
                for language in (0, 4, NONE):
                    t.check(
                        "saved_game_file_name_unique",
                        0 if name is None else Buffer(wide(name)),
                        random_.choice((0, 1, 7, 10)),
                        globals={
                            "unknown_0051ea14": files,
                            "input_globals": input_globals(random_, mask),
                            "saved_game_files_language": language,
                            "saved_game_files_string_list_index": random_.choice((NONE, 5)),
                        },
                        externals={
                            "XCreateSaveGame": create(result),
                            "unicode_string_list_has_string": lambda harness, arguments: 1,
                        },
                    )

    # ---------- reading and writing a file, and waiting for it

    def started(start: int, success: int, waits: int, error: int, where: int):
        """What starts the read or the write: what it leaves, at once or after
        the game waits. `where` is which of its arguments is where it leaves it."""
        pending = []

        def handler(harness, arguments):
            result = arguments[where]
            pending.clear()
            if start:
                harness.mu.mem_write(result, struct.pack("<BBBBI", waits == 0, 0x55, success, 0x66, error))
                if waits:
                    pending.append(result)
            return start, tuple(argument for index, argument in enumerate(arguments) if index != where)

        def switch(harness, arguments):
            if calls_so_far(harness, "SwitchToThread") >= waits - 1:
                for result in pending:
                    harness.mu.mem_write(result, b"\x01")
            return 0

        return handler, switch

    for n in range(60):
        start, success, waits = random_.choice((1, 1, 1, 0, 0x100)), random_.choice((1, 1, 0, 0x80)), random_.choice((0, 0, 1, 3))
        for name, callee, where, arguments in (
            ("saved_game_file_read", "saved_game_file_read_async", 3, (random_.getrandbits(32), random_.getrandbits(32), random_.getrandbits(32))),
            ("saved_game_file_write", "saved_game_file_write_async", 4, (random_.getrandbits(32), random_.getrandbits(32), random_.getrandbits(32), random_.getrandbits(32))),
        ):
            handler, switch = started(start, success, waits, random_.getrandbits(32), where)
            t.check(
                name,
                *arguments,
                globals={"saved_game_files_last_error": random_.getrandbits(32)},
                externals={callee: handler, "SwitchToThread": switch, "sound_idle": lambda harness, arguments: 0},
            )

    # ---------- starting to read a file

    BUILT_IN_SIZE = 0x1E4
    RESULT_SIZE = 0x10C

    def built_in(count: int, entries: int = 6) -> Buffer:
        return Buffer(struct.pack("<i", count) + random_.randbytes(entries * BUILT_IN_SIZE + 8))

    def give(found: int, type_: int, language: int, directory: bytes):
        # the same for the game and for source/
        data = bytearray(random_.randbytes(INFORMATION))
        data[: len(directory) + 1] = directory + b"\0"
        struct.pack_into("<I", data, 0x38, type_)
        data[0x3D] = language

        def handler(harness, arguments):
            where, file = arguments
            before = bytes(harness.mu.mem_read(where, 1)) + bytes(harness.mu.mem_read(where + 0x14, 2))
            harness.mu.mem_write(where, bytes(data))
            return found, (before, file)

        return handler

    def take_read(result: int):
        def handler(harness, arguments):
            buffer, size, unknown, where, path = arguments
            return result, (buffer, size, unknown & 0xFF, where, text(harness, path))

        return handler

    for count in (0, 1, 3, 6, -1):
        for place in (0, 1, 2, 5, 6, 0x1FFF):
            for size in (0, 1, 0x13, 0x1E0, 0x1E1, NONE):
                if place < 6 or size == 1:
                    t.check(
                        "saved_game_file_read_async",
                        size,
                        index(random_.randrange(16), random_.randrange(16), place, 1, random_.randrange(0x200), random_.getrandbits(1)),
                        Buffer(random_.randbytes(0x1E8)),
                        Buffer(random_.randbytes(RESULT_SIZE)),
                        globals={"unknown_0051ea14": built_in(count)},
                    )

    short_directories = (b"u:\\udata\\4d\\AB\\", b"", b"d" * 0x13)
    for n in range(120):
        built = n % 5 == 4
        file = index(random_.randrange(16), random_.randrange(16), random_.randrange(0x2000), built, random_.randrange(0x200))
        t.check(
            "saved_game_file_read_async",
            random_.getrandbits(32),
            file,
            random_.getrandbits(32),
            Buffer(random_.randbytes(RESULT_SIZE)),
            # a file that comes with the game, with the files not set up, is
            # read as one of a drive
            globals={"unknown_0051ea14": 0 if built else random_.choice((0, 0x80100000))},
            externals={
                "saved_game_file_get_information": give(n % 6 != 5, random_.choice((0, 1, 5, 10, 11, NONE)), random_.choice((0, 3, 8, 0xFE, 0xFF, 0xFF)), random_.choice(short_directories)),
                "saved_game_file_read_task_new": take_read(random_.choice((0, 1, 1))),
            },
        )

    # ---------- starting to write a file

    for n in range(160):
        file = random_.getrandbits(32)
        files_ = random_.choice((0x80100000, 0x80100000, 0x80200000, 0))
        differs = random_.choice((0, 1, 1))
        operation = random_.getrandbits(32) | 1

        def take(harness, arguments, operation=operation):
            harness.mu.mem_write(arguments[1], struct.pack("<I", operation))
            return 0, (arguments[0],)

        # what is told of the file is in the stack: the stand-ins tell what it has
        information_ = give(n % 7 != 6, random_.randrange(11), random_.randrange(9), random_.choice(short_directories))
        result_buffer = Buffer(random_.randbytes(RESULT_SIZE))
        t.check(
            "saved_game_file_write_async",
            Buffer(wide(random_.choice(("mine", "", "another")))),
            file,
            random_.getrandbits(32),
            random_.getrandbits(32),
            result_buffer,
            globals={"unknown_0051ea14": files_},
            externals={
                "saved_game_file_get_information": information_,
                "saved_game_file_name_differs": lambda harness, arguments, differs=differs: differs,
                "saved_game_files_take_operation": take,
                "saved_game_file_description_new_renamed": lambda harness, arguments, result=random_.choice((0, 0x1234, 0x1234)): (
                    result,
                    (arguments[0], arguments[1], arguments[2], bytes(harness.mu.mem_read(arguments[3], INFORMATION)), arguments[4], arguments[5]),
                ),
                "saved_game_file_description_new": lambda harness, arguments, result=random_.choice((0, 0x1234, 0x1234)): (
                    result,
                    (arguments[0], arguments[1], bytes(harness.mu.mem_read(arguments[2], INFORMATION))),
                ),
                "saved_game_file_write_task_new": lambda harness, arguments, result=random_.choice((0, 1, 1)): (
                    result,
                    (arguments[0], arguments[1], arguments[2] & 0xFF, arguments[3]),
                ),
            },
        )

    # ---------- a directory that has to be there

    for path in (b"u:\\udata\\4d530064", b"", b"d" * 0xFF, b"e" * 0x120, b"c:\\x\\"):
        for exists, created in ((1, 1), (0, 1), (0, 0), (0x80, 0), (0, 0x100)):
            t.check(
                "saved_game_files_make_directory",
                Buffer(path + b"\0"),
                externals={
                    "file_exists": lambda harness, arguments, exists=exists: (exists, (reference_path(harness, arguments[0]),)),
                    "file_create": lambda harness, arguments, created=created: (created, (reference_path(harness, arguments[0]),)),
                },
            )

    # ---------- the files that come with the game

    PROFILE_SIZE = 0x1E0

    for _ in range(12):
        t.check("player_profile_set_defaults", Buffer(random_.randbytes(PROFILE_SIZE + 8)))

    def built_in_files(count: int) -> Buffer:
        room = min(max(count, 0), 3) + 2
        return Buffer(struct.pack("<i", count) + random_.randbytes(room * BUILT_IN_SIZE))

    for count in (0, 1, 3, 100, 101):
        for language in (0, 1, 4, 7, 9, NONE):
            # with 100 files the new one goes far: only a few of the files are
            # in the test's memory, so 100 is tried with nothing to add to
            files_ = 0 if count == 100 else built_in_files(count)
            t.check(
                "saved_game_files_add_default_profile",
                globals={"unknown_0051ea14": files_, "saved_game_files_language": language},
            )
    for count in (0, 1, 3, 101):
        for engine in (1, 2, 3, 4, 7, 8, 9):
            data = bytearray(random_.randbytes(VARIANT_SIZE))
            struct.pack_into("<I", data, 0x44, engine)
            used = (4 + count * BUILT_IN_SIZE, 4 + count * BUILT_IN_SIZE + 4 + VARIANT_SIZE)

            def clean(data, count=count):
                # after the variant, the new file has what was in the stack
                if count == 101:
                    return data
                start = 4 + count * BUILT_IN_SIZE + 4 + VARIANT_SIZE
                end = 4 + (count + 1) * BUILT_IN_SIZE
                return data[:start] + bytes(end - start) + data[end:]

            files_ = built_in_files(count)
            files_.clean = clean
            t.check(
                "saved_game_files_add_built_in_variant",
                Buffer(bytes(data)),
                globals={"unknown_0051ea14": files_},
            )
    t.check("saved_game_files_add_built_in_variant", Buffer(random_.randbytes(VARIANT_SIZE)), globals={"unknown_0051ea14": 0})

    # ---------- forgetting a file that is kept

    for n in range(80):
        file = random_.getrandbits(32) if n % 9 else NONE
        entries = [random_.getrandbits(32) for _ in range(4)]
        if n % 4:
            entries[random_.randrange(4)] = file
        if n % 5 == 4:
            entries[0] = file
            entries[random_.randrange(1, 4)] = file
        t.check(
            "cached_saved_game_file_delete",
            file,
            globals=cache(1, random_.choice((4, 4, 3, 2, 1, 0, -1)), entries),
        )

    # ---------- the memory of the files

    for can, files_, flags in ((1, 0x80100000, 1), (1, 0x80100000, 3), (0, 0x80100000, 1), (1, 0, 1), (1, 0x80100000, 2), (0x80, 0x80100000, 0xFFFFFFFF), (1, 0x80100000, 0)):
        t.check(
            "saved_game_files_update",
            globals={"unknown_0055c14d": bytes([can]), "unknown_0051ea14": files_, "unknown_0055c150": flags},
            externals={"unknown_002164c0": lambda harness, arguments: 0},
        )

    for string_list_index in (NONE, 0, 0x12340005):
        for has_files in (1, 0):
            t.check(
                "saved_game_files_set_string_list",
                string_list_index,
                globals={
                    "saved_game_files_string_list_index": random_.getrandbits(32),
                    "unknown_0051ea14": Buffer(random_.randbytes(0x10)) if has_files else 0,
                },
                externals={"saved_game_files_add_default_profile": lambda harness, arguments: 0},
            )

    def new(harness, arguments):
        # what the memory is asked with is in the stack: its functions, which
        # are the game's or those of source/, and a number
        functions, unknown = struct.unpack("<2I", harness.mu.mem_read(arguments[0], 8))
        return 0, (harness.normalize(functions), unknown)

    for configuration in (0, 1, 2, 5, NONE):
        t.check(
            "saved_game_files_allocate_memory",
            configuration,
            globals={
                "saved_game_files_memory_allocated": bytes([random_.getrandbits(8)]),
                "saved_game_files_allocation_functions": GlobalRange(8),
            },
            externals={"saved_game_files_new": new},
        )

    for n in range(30):
        count = random_.choice((0, 1, 4, 4, 3, -1))
        t.check(
            "saved_game_files_free_memory",
            globals={
                "unknown_0051ea14": random_.choice((0x80100000, 0x80100000, 0)),
                "cached_saved_game_file_count": count,
                # the game tells of the few files that are kept by where they are kept
                "cached_saved_game_files": GlobalData(random_.randbytes(4 * 0x44)),
                "saved_game_files_cache_valid": bytes([random_.choice((0, 1))]),
                "saved_game_files_memory_allocated": bytes([random_.getrandbits(8)]),
            },
            externals={"saved_game_file_get_information": lambda harness, arguments: 1},
        )

    MEMORY = 0x80100000
    MEMORY_SIZE = 0x6BF00
    for salt in (0, 1, 5, 0x1FE, 0x1FF, 0x200, 0x3FF, NONE, 0x80000000):
        # what the memory is asked of: its functions are the test's
        allocation = Buffer(struct.pack("<4I", 8, random_.getrandbits(32), mock_address(0), mock_address(1)), pointers=(0,))
        t.check(
            "saved_game_files_new",
            allocation,
            globals={
                "saved_game_files_next_salt": salt,
                "unknown_0055c150": random_.getrandbits(32),
                "unknown_0051ea14": random_.getrandbits(32),
            },
            memory={MEMORY: Memory(random_.randbytes(MEMORY_SIZE))},
            mocks=[Mock(lambda call: MEMORY, stack_arguments=1, registers=("ecx",))],
            externals={"unknown_00020670": lambda harness, arguments: 0},
        )

    for _ in range(6):
        t.check("saved_game_file_information_new", Buffer(random_.randbytes(INFORMATION)))

    # ---------- the name of a file

    def named_files(count: int, names: list) -> Buffer:
        data = bytearray(struct.pack("<i", count) + random_.randbytes(len(names) * BUILT_IN_SIZE + 8))
        for place, (profile_name, variant_name) in enumerate(names):
            entry = 4 + place * BUILT_IN_SIZE + 4
            data[entry + 8 : entry + 8 + len(wide(profile_name))] = wide(profile_name)
            if variant_name is not None:
                data[entry + 4 : entry + 4 + len(wide(variant_name))] = wide(variant_name)
        return Buffer(bytes(data))

    file_names = [("Default", None), ("", None), ("x" * 16, None), ("unused", "Team Slayer"), ("unused", "Rockets"), ("y" * 150, None), ("unused", "z" * 150)]
    for place in range(len(file_names)):
        for type_ in (0, 1, 5, 9, 10, 11, 15):
            for count in (len(file_names), place, place + 1, 0, -1):
                t.check(
                    "saved_game_file_get_name",
                    index(type_, random_.randrange(16), place, 1, random_.randrange(0x200), random_.getrandbits(1)),
                    Buffer(random_.randbytes(0x108)),
                    globals={"unknown_0051ea14": named_files(count, file_names)},
                )
    for n in range(40):
        built = n % 4 == 3
        t.check(
            "saved_game_file_get_name",
            index(random_.randrange(16), random_.randrange(16), random_.randrange(0x2000), built, random_.randrange(0x200)),
            Buffer(random_.randbytes(0x108)),
            # a file that comes with the game, with no memory for the files, is asked for as one of a drive
            globals={"unknown_0051ea14": 0 if built else random_.choice((0, 0x80100000))},
            externals={"saved_game_file_get_information": give(n % 5 != 4, 1, 0, random_.choice(short_directories))},
        )

    # ---------- deleting a file

    for n in range(100):
        type_ = random_.choice((0, 1, 7, 10, 12))
        unit = random_.choice((0, 0, 1, 3))
        built = n % 5 == 4
        file = NONE if n % 17 == 16 else index(type_, unit, random_.randrange(0x2000), built, random_.randrange(0x200))
        waits = random_.choice((0, 0, 1, 2))
        data = bytearray(random_.randbytes(INFORMATION))
        data[0] = 0
        data[0x14:0x38] = (wide(random_.choice(("mine", "", "a name of 16 ch."))) + bytes(0x24))[:0x24]
        data[0x3D] = random_.choice((0, 2, 4, 8, 0xFF, 0x80))

        def known(harness, arguments, data=bytes(data), found=n % 6 != 5):
            where, file_ = arguments
            before = bytes(harness.mu.mem_read(where, 1)) + bytes(harness.mu.mem_read(where + 0x14, 2))
            harness.mu.mem_write(where, data)
            return found, (before, file_)

        t.check(
            "saved_game_file_delete",
            file,
            globals={
                "unknown_0055c14c": bytes([waits == 0]),
                "input_globals": input_globals(random_, random_.choice((0xFFFF, 1, 0, 2))),
                "saved_game_files_string_list_index": random_.choice((NONE, 5)),
            },
            externals={
                "SwitchToThread": wait_until("unknown_0055c14c", 0, waits),
                "sound_idle": lambda harness, arguments: 0,
                "saved_game_file_get_information": known,
                "unicode_string_list_has_string": lambda harness, arguments: 1,
                "XDeleteSaveGame": lambda harness, arguments: (0, (text(harness, arguments[0]), wide_text(harness, arguments[1]))),
                "saved_game_file_delete_information": lambda harness, arguments: 0,
            },
        )

    # ---------- a name that no file has

    def taken(count: int):
        """XCreateSaveGame: the first so many names are those of saved games that there are."""

        def handler(harness, arguments):
            root, name, disposition, flags, path, path_size = arguments
            exists = calls_so_far(harness, "XCreateSaveGame") < count
            return 0 if exists else 2, (text(harness, root), wide_text(harness, name), disposition, flags, path_size)

        return handler

    for count in (0, 1, 2, 9, 10, 99, 4095, 4096):
        for mask in (1, 0xFFFF) if count >= 4095 else (1, 0, 2, 0xFFFF):
            t.check(
                "saved_game_file_get_unused_name",
                random_.choice((0, 1, 7, 10)),
                Buffer(random_.randbytes(0x108)),
                globals={
                    "input_globals": input_globals(random_, mask),
                    "saved_game_files_language": random_.choice((0, 4, NONE)),
                    "saved_game_files_string_list_index": random_.choice((NONE, 5)),
                    "saved_game_files_last_error": random_.getrandbits(32),
                },
                externals={
                    "XCreateSaveGame": taken(count),
                    "unicode_string_list_has_string": lambda harness, arguments: 1,
                },
            )

    # ---------- the files of a type

    UNITS = 9
    SALT = 0x4BEFC

    def known_files(type_: int, counts: dict, built_in_count: int) -> Buffer:
        """What the game knows of the files: those that come with it, and a
        table for each drive, one after another. The count of the table of
        the first memory unit is where the salt is."""

        def a_type() -> int:
            return random_.choice((type_, type_, type_, type_ ^ 1, type_ ^ 0x10, type_ ^ 0x80000000, random_.randrange(11)))

        data = bytearray(TABLES + UNITS * TABLE_SIZE)
        struct.pack_into("<i", data, 0, built_in_count)
        for n in range(min(max(built_in_count, 0), 101)):
            data[4 + n * BUILT_IN_SIZE : 4 + (n + 1) * BUILT_IN_SIZE] = random_.randbytes(BUILT_IN_SIZE)
            struct.pack_into("<I", data, 4 + n * BUILT_IN_SIZE, a_type())
        for unit in range(UNITS):
            base = TABLES + unit * TABLE_SIZE
            count = counts.get(unit, 0)
            struct.pack_into("<i", data, base, count)
            for n in range(max(count, 0) + 1):
                where = base + 4 + n * INFORMATION
                data[where : where + INFORMATION] = random_.randbytes(INFORMATION)
                struct.pack_into("<I", data, where + 0x38, a_type())
        assert struct.unpack_from("<i", data, SALT)[0] == counts.get(1, 0)
        return Buffer(bytes(data))

    for n in range(170):
        type_ = random_.choice((0, 1, 3, 7, 10, 0x13, 15))
        controller = (NONE, 0xFF, 0, 1, 2, 3)[n % 6]
        counts = {unit: random_.choice((0, 1, 2, 5, 12, -1)) for unit in range(UNITS) if random_.random() < 0.8}
        # the count of the first memory unit is the salt: one that is
        # negative has all of the bits of a salt, and no files
        counts[1] = random_.choice((counts.get(1, 0), random_.randrange(0x200), -1 - random_.getrandbits(31)))
        if counts[1] > 12:
            # a salt with all of its bits is the count of a table of no drive
            # that is there
            mask_without = True
        else:
            mask_without = False
        capacity = random_.choice((0, 1, 2, 3, 8, 40, 40, 40, 0xFFFF))
        room = 60
        t.check(
            "saved_game_files_enumerate",
            controller,
            type_,
            Buffer(struct.pack("<H", capacity) + random_.randbytes(6)),
            Buffer(random_.randbytes(4 * (min(capacity, room) + 2))),
            random_.choice((0, 1, 1, 1, 0x100, 0xFF01)),
            globals={
                "unknown_0051ea14": known_files(type_, counts, random_.choice((0, 0, 1, 3, 9, 14, -1))),
                "input_globals": input_globals(
                    random_,
                    random_.choice((0xFFFF, 0xFFFF, 1, 0, 0x1FE, random_.getrandbits(9), random_.getrandbits(9))) & (~2 if mask_without else ~0) & 0xFFFF,
                ),
            },
        )
    # many files that come with the game, and no memory for the files
    for capacity in (5, 101, 300):
        for built_in_count in (100, 101):
            t.check(
                "saved_game_files_enumerate",
                NONE,
                4,
                Buffer(struct.pack("<H", capacity) + random_.randbytes(6)),
                Buffer(random_.randbytes(4 * 180)),
                1,
                globals={
                    "unknown_0051ea14": known_files(4, {0: 12, 1: 0x155}, built_in_count),
                    "input_globals": input_globals(random_, 1),
                },
            )
    for controller in (NONE, 0xFF, 2):
        t.check(
            "saved_game_files_enumerate",
            controller,
            1,
            Buffer(struct.pack("<H", 9) + random_.randbytes(6)),
            Buffer(random_.randbytes(40)),
            random_.getrandbits(1),
            globals={"unknown_0051ea14": 0, "input_globals": input_globals(random_, 0xFFFF)},
        )
