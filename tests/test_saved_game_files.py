# source/saved games/saved_game_files.c
#
# The console's library is stood in for: what it says of the hard disk is
# the test's

import random
import struct

from difftest import Buffer, GlobalData
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

    def index(unit: int, type_: int, file: int, built_in: int = 0, salt: int = 0, top: int = 0) -> int:
        return unit | type_ << 4 | file << 8 | built_in << 21 | salt << 22 | top << 31

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

    for name, callee in (("saved_game_auxiliary_read", "unknown_002ada70"), ("saved_game_auxiliary_write", "unknown_002adec0")):
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
