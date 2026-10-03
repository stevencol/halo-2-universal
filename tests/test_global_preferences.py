# the preferences that the game keeps on the hard disk
# (source/main/global_preferences.c)
#
# The tasks of the file and the console's signature are not run: the test
# stands in for them, and says what each would have done

import random
import struct

from difftest import Buffer, GlobalData

NONE = 0xFFFFFFFF

# global_preferences_globals
INITIALIZED = 0x0
DIRTY = 0x1
FILE = 0x4
CURRENT = 0x8
WRITTEN = 0x204
TASK_ID = 0x400
DONE = 0x404
GLOBALS_SIZE = 0x408

# the file: its signature and its data
FILE_SIZE = 0x1FC
DATA = 0x14
DATA_SIZE = 0x1E8
VERSION = DATA + 0x0
LANGUAGE = DATA + 0x4
UNKNOWN8 = DATA + 0x8
NAMES = DATA + 0x18
NAME = DATA + 0x118
UNKNOWN15C = DATA + 0x15C
UNKNOWN160 = DATA + 0x160
UNKNOWN164 = DATA + 0x164
UNKNOWN168 = DATA + 0x168
CACHE_BUILD_NUMBER = DATA + 0x1D4

SIGNATURE = bytes(range(0xA0, 0xB4))
SIGNATURE_HANDLE = 0x5150

# a task
PARAMETERS = 0x8
CATEGORY = 0x30
FUNCTION = 0x34
TASK_DONE = 0x38


def preferences_file(random_: random.Random, **fields: int) -> bytearray:
    data = bytearray(random_.randbytes(FILE_SIZE))
    data[:DATA] = fields.get("signature", SIGNATURE)
    struct.pack_into("<i", data, VERSION, fields.get("version", 8))
    for name, offset in (
        ("language", LANGUAGE),
        ("unknown8", UNKNOWN8),
        ("unknown15C", UNKNOWN15C),
        ("unknown160", UNKNOWN160),
        ("unknown164", UNKNOWN164),
        ("unknown168", UNKNOWN168),
        ("unknown18C", UNKNOWN168 + 0x24),
    ):
        if name in fields:
            struct.pack_into("<i", data, offset, fields[name])
    return data


def preferences_globals(random_: random.Random, **fields: object) -> GlobalData:
    data = bytearray(random_.randbytes(GLOBALS_SIZE))
    data[INITIALIZED] = fields.get("initialized", 1)
    data[DIRTY] = fields.get("dirty", 0)
    struct.pack_into("<I", data, FILE, fields.get("file", 0x55))
    current = fields.get("current")
    if current is not None:
        data[CURRENT : CURRENT + FILE_SIZE] = current
    written = fields.get("written")
    if written is not None:
        data[WRITTEN : WRITTEN + FILE_SIZE] = written
    data[DONE] = fields.get("done", 1)
    return GlobalData(bytes(data))


def wide(text: str, size: int | None = None) -> Buffer:
    data = text.encode("utf-16-le") + b"\0\0"
    return Buffer(data if size is None else data.ljust(size, b"\x11"))


def nothing(harness, arguments):
    return 0


def run(t) -> None:
    random_ = random.Random(29)

    # ---------- the values

    for _ in range(3):
        t.check("global_preferences_reset", globals={"global_preferences_globals": preferences_globals(random_)})

    languages = (-1, 0, 8, 9, 100, -200)
    for _ in range(250):
        file = preferences_file(
            random_,
            language=random_.choice(languages),
            unknown8=random_.choice(languages),
            unknown15C=random_.choice((-1, 0, 7, 15, 16, 1000)),
            unknown160=random_.choice((-1, 0, 2, 3, 4)),
            unknown164=random_.choice((-1, 0, 2, 3, 4)),
            unknown168=random_.choice((-1, 0, 5, 8, 9, 0x7FFFFFFF)),
            unknown18C=random_.choice((-1, 0, 5, 8, 9)),
        )
        t.check(
            "global_preferences_validate",
            Buffer(bytes(file)),
            globals={"global_preferences_globals": preferences_globals(random_)},
        )

    names = ("", "Chief", "x" * 30, "y" * 31, "z" * 32, "w" * 45)
    for name in names:
        for index in range(4):
            t.check(
                "global_preferences_set_name",
                index,
                wide(name),
                globals={"global_preferences_globals": preferences_globals(random_)},
            )
        t.check(
            "global_preferences_set_other_name",
            wide(name),
            globals={"global_preferences_globals": preferences_globals(random_)},
        )

    for value in (-5, -1, 0, 1, 2, 3, 4, 15, 16, 0x7FFFFFFF, 0x80000000, NONE):
        for function in ("unknown15C", "unknown160", "unknown164"):
            t.check(
                "global_preferences_set_" + function,
                value,
                globals={"global_preferences_globals": preferences_globals(random_)},
            )
        for function, offset in (("unknown160", UNKNOWN160), ("unknown164", UNKNOWN164)):
            current = preferences_file(random_)
            struct.pack_into("<I", current, offset, value & NONE)
            t.check(
                "global_preferences_get_" + function,
                Buffer(random_.randbytes(4)),
                globals={"global_preferences_globals": preferences_globals(random_, current=bytes(current))},
            )

    # ---------- the signature

    def signature_end(harness, arguments, result=0):
        handle, signature = arguments
        harness.mu.mem_write(signature, SIGNATURE)
        return result, (handle,)

    def signature_update(harness, arguments, result=0):
        handle, data, size = arguments
        return result, (handle, bytes(harness.mu.mem_read(data, 16)), size)

    for result in (0, 1, 0x57):
        t.check(
            "xbox_signature_end",
            Buffer(random_.randbytes(20)),
            globals={"xbox_signature_handle": 0x77},
            externals={"XCalculateSignatureEnd": lambda harness, arguments, result=result: signature_end(harness, arguments, result)},
        )

    def signing(begin: int = SIGNATURE_HANDLE, update: int = 0, end: int = 0) -> dict:
        return {
            "XCalculateSignatureBegin": lambda harness, arguments: begin,
            "XCalculateSignatureUpdate": lambda harness, arguments: signature_update(harness, arguments, update),
            "XCalculateSignatureEnd": lambda harness, arguments: signature_end(harness, arguments, end),
        }

    # ---------- writing

    for dirty in (1, 0):
        for done in (1, 0):
            for changed in (True, False):
                for file in (0x55, NONE):
                    for begin, update, end in ((SIGNATURE_HANDLE, 0, 0), (NONE, 0, 0), (SIGNATURE_HANDLE, 5, 0), (SIGNATURE_HANDLE, 0, 1)):
                        current = preferences_file(random_)
                        written = bytearray(current)
                        written[:DATA] = random_.randbytes(DATA)
                        if changed:
                            written[DATA + random_.randrange(DATA_SIZE)] ^= 1
                        t.check(
                            "global_preferences_flush",
                            globals={
                                "global_preferences_globals": preferences_globals(
                                    random_, dirty=dirty, done=done, file=file, current=bytes(current), written=bytes(written)
                                ),
                                "xbox_signature_handle": 0x1234,
                            },
                            externals={**signing(begin, update, end), "async_write_position": lambda harness, arguments: 0x00ABCD05},
                        )

    for file in (0x55, NONE):
        for dirty in (0, 1):
            for switches in (0, 2):

                def switch_to_thread(harness, arguments, switches=switches):
                    calls = sum(1 for call in harness._calls if call[0] == "SwitchToThread")
                    wrote = any(call[0] == "global_preferences_flush" for call in harness._calls)
                    if calls >= switches - 1 or wrote:
                        harness.mu.mem_write(harness.global_addresses["global_preferences_globals"] + DONE, b"\x01")
                    return 0

                def start_write(harness, arguments):
                    # the task that writes is not done at once: the function has to wait for it too
                    harness.mu.mem_write(harness.global_addresses["global_preferences_globals"] + DONE, b"\0")
                    return 0

                t.check(
                    "global_preferences_flush_blocking",
                    globals={
                        "global_preferences_globals": preferences_globals(
                            random_, file=file, dirty=dirty, done=0 if switches else 1
                        )
                    },
                    externals={
                        "SwitchToThread": switch_to_thread,
                        "global_preferences_flush": start_write,
                        "async_flush_file_blocking": nothing,
                    },
                )

    def flush_file(harness, arguments):
        file, category, priority, done = arguments
        harness.mu.mem_write(done, b"\x01")
        return 0, (file, category, priority)

    for category in (8, 2):
        t.check(
            "async_flush_file_blocking",
            category,
            random_.getrandbits(16),
            externals={"async_flush_file": flush_file, "SwitchToThread": nothing},
        )

    # ---------- starting

    plans = (
        {},
        {"initialized": 1},
        {"file": NONE},
        {"read": FILE_SIZE - 1},
        {"version": 7},
        {"signature": bytes(20)},
        {"begin": NONE},
        {"update": 5},
        {"end": 1},
        {"language": 50, "unknown160": 9},
    )
    for plan in plans:
        stored = preferences_file(
            random_,
            version=plan.get("version", 8),
            signature=plan.get("signature", SIGNATURE),
            language=plan.get("language", 4),
            unknown160=plan.get("unknown160", 2),
        )
        task = Buffer(bytes(0x40))

        def allocate(harness, arguments, task=task):
            return task.address, ()

        def queue(harness, arguments, plan=plan):
            # the task that opens the file, done at once
            address = arguments[0]
            path_address, access, share_mode, disposition, flags, file_address, create_directories = struct.unpack(
                "<7I", harness.mu.mem_read(address + PARAMETERS, 28)
            )
            path = bytes(harness.mu.mem_read(path_address, 64)).split(b"\0", 1)[0]
            harness.mu.mem_write(file_address, struct.pack("<I", plan.get("file", 0x55)))
            (done,) = struct.unpack("<I", harness.mu.mem_read(address + TASK_DONE, 4))
            harness.mu.mem_write(done, b"\x01")
            record = (
                path,
                access,
                share_mode,
                disposition,
                flags,
                harness.normalize(file_address),
                create_directories & 0xFF,
                struct.unpack("<I", harness.mu.mem_read(address, 4))[0],
                struct.unpack("<I", harness.mu.mem_read(address + CATEGORY, 4))[0],
                harness.normalize(struct.unpack("<I", harness.mu.mem_read(address + FUNCTION, 4))[0]),
                harness.normalize(done),
            )
            harness.mu.mem_write(address, bytes(0x40))
            return 0x00ABCD00, record

        def read_position(harness, arguments, plan=plan, stored=stored):
            file, size, offset, category, priority, bytes_read, buffer, done = arguments
            harness.mu.mem_write(buffer, bytes(stored))
            harness.mu.mem_write(bytes_read, struct.pack("<I", plan.get("read", FILE_SIZE)))
            harness.mu.mem_write(done, b"\x01")
            return 0, (file, size, offset, category, priority)

        t.check(
            "global_preferences_initialize",
            globals={
                "global_preferences_globals": preferences_globals(
                    random_, initialized=plan.get("initialized", 0), done=0
                ),
                "xbox_signature_handle": 0x1234,
            },
            extra=[task],
            externals={
                **signing(plan.get("begin", SIGNATURE_HANDLE), plan.get("update", 0), plan.get("end", 0)),
                "async_task_allocate": allocate,
                "async_task_queue": queue,
                "async_read_position": read_position,
                "global_preferences_flush_blocking": nothing,
                "async_set_file_size": lambda harness, arguments: 0x00ABCD06,
                "SwitchToThread": nothing,
            },
        )

    # ---------- the profiles and the game variant of the preferences

    for index in range(4):
        for found in (0, 1, 0x100):
            t.check(
                "global_preferences_find_player_profile",
                index,
                Buffer(random_.randbytes(4)),
                globals={"global_preferences_globals": preferences_globals(random_)},
                externals={
                    "player_profile_find": lambda harness, arguments, found=found: (
                        found,
                        (arguments[0], harness.normalize(arguments[1]), arguments[2]),
                    ),
                },
            )

    VARIANT_SIZE, TAG_INSTANCE_SIZE = 0x130, 0x10
    for n in range(200):
        found = n % 4 == 3
        file = random_.choice((0, 3, 0x7FFFFFFF))

        def find(harness, arguments, found=found, file=file):
            name, variant, file_ = arguments
            if found:
                harness.mu.mem_write(variant, bytes(range(256)) + bytes(VARIANT_SIZE - 256))
            if found or file == 3:
                # the file is told at times though there is no variant
                harness.mu.mem_write(file_, struct.pack("<I", file))
            return found, (harness.normalize(name), variant)

        # the instances of the tags, then the tag that the game globals name
        # (it starts with a block) and the first element of its block
        tag_index = random_.choice((0, 1, 2, 0x101))
        count = tag_index + 1 if tag_index > 2 else 3
        string_list = random_.choice((0x00ABCDEF, NONE))
        BLOCK, ELEMENT = TAG_INSTANCE_SIZE * count, TAG_INSTANCE_SIZE * count + 8
        tags = bytearray(random_.randbytes(ELEMENT + 0x28))
        struct.pack_into("<I", tags, tag_index * TAG_INSTANCE_SIZE + 8, BLOCK)
        struct.pack_into("<iI", tags, BLOCK, 1, ELEMENT)
        struct.pack_into("<I", tags, ELEMENT + 0x1C, string_list)
        game_globals = bytearray(random_.randbytes(0x170))
        struct.pack_into("<I", game_globals, 0x16C, random_.choice((tag_index | 0xE0010000, tag_index, NONE)))
        text = random_.choice(("Slayer", "", "a name of a variant that is longer than thirty-one", "x" * 31))

        def get_string(harness, arguments, text=text):
            string_list_index, string_id, string = arguments
            harness.mu.mem_write(string, text.encode("utf-16-le") + bytes(2))
            return 0, (string_list_index, string_id)

        t.check(
            "global_preferences_get_game_variant",
            Buffer(random_.randbytes(VARIANT_SIZE)),
            globals={
                "global_preferences_globals": preferences_globals(random_),
                "global_game_globals": Buffer(bytes(game_globals)) if n % 7 else 0,
                "global_tag_instances": Buffer(bytes(tags), pointers=(tag_index * TAG_INSTANCE_SIZE + 8, BLOCK + 4)),
            },
            externals={
                "unknown_00212c20": find,
                "unicode_string_list_get_string": get_string,
            },
        )
