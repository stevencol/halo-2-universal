# the state of the game and its checkpoints (source/saved games/game_state.c),
# and the checksum that it uses (source/memory/crc.c)
#
# The functions that write and read the files, and those of other parts of
# the game, are not run: the test stands in for them

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, GlobalRange, Memory, Mock, mock_address
from test_cache_files import BUILD

NONE = 0xFFFFFFFF

# where the test puts what the globals point to
GAME = PHYSICAL_ADDRESS + 0x40000
TIME = PHYSICAL_ADDRESS + 0x42000
BASE = PHYSICAL_ADDRESS + 0x100000
HEADER = BASE
ALLOCATION = BASE + 0x1288

CPU_SIZE = 0x3BE000
GPU_SIZE = 0x40000

# game_state_globals
VALID = 0x2
PREVIOUS_VALID = 0x3
FILE_INDEX = 0x4
HEADERS = 0x8
SAVE_TIME = 0x2518
DEATHS = 0x251C
BASE_ADDRESS = 0x2520
CPU_ALLOCATION_SIZE = 0x2524
CHECKSUM = 0x252C
REVERT_TIME = 0x2530
HEADER_POINTER = 0x2534
GLOBALS_SIZE = 0x2540

# a header
HEADER_SIZE = 0x1288
MAP_NAME = 0x8
HEADER_BUILD = 0x108
CACHE_FILE_CHECKSUM = 0x128
OPTIONS = 0x130
OPTIONS_SIZE = 0x1118
REVERTED = OPTIONS + 4
STRUCTURE_BSP_INDEX = 0x1248

# main_game_globals
GAME_OPTIONS = 0x8
GAME_ACTIVE = 0x1120
GAME_SIZE = 0x1128

LOCKOUT = b"scenarios\\multi\\lockout\\lockout"
ALLOCATION_CHECKSUM = 0x1234ABCD


def header(random_: random.Random, **fields: object) -> bytes:
    data = bytearray(random_.randbytes(HEADER_SIZE))
    struct.pack_into("<2I", data, 0, fields.get("checksum", ALLOCATION_CHECKSUM), fields.get("base", BASE))
    data[MAP_NAME : MAP_NAME + 256] = fields.get("map_name", LOCKOUT).ljust(256, b"\0")
    data[HEADER_BUILD : HEADER_BUILD + 32] = fields.get("build", BUILD).ljust(32, b"\0")
    struct.pack_into("<I", data, CACHE_FILE_CHECKSUM, fields.get("cache_file_checksum", 0x600DF00D))
    data[OPTIONS : OPTIONS + OPTIONS_SIZE] = fields.get("options", bytes(range(256)) * 4 + bytes(OPTIONS_SIZE - 1024))
    # the two bytes after the index are not set by the game: zero, as in a header that it clears
    struct.pack_into("<hH", data, STRUCTURE_BSP_INDEX, fields.get("structure_bsp_index", 2), 0)
    return bytes(data)


def game_state_globals(random_: random.Random, **fields: object) -> bytes:
    data = bytearray(random_.randbytes(GLOBALS_SIZE))
    data[0] = fields.get("initialized", 1)
    data[VALID] = fields.get("valid", 1)
    data[PREVIOUS_VALID] = fields.get("previous_valid", 0)
    struct.pack_into("<hH", data, FILE_INDEX, fields.get("file_index", 0), 0)
    headers = fields.get("headers")
    if headers:
        data[HEADERS : HEADERS + 2 * HEADER_SIZE] = headers[0] + headers[1]
    struct.pack_into("<Ih", data, SAVE_TIME, fields.get("save_time", 1000), fields.get("deaths", 0))
    struct.pack_into("<Ii", data, BASE_ADDRESS, fields.get("base", BASE), fields.get("cpu_allocation_size", 0x1290))
    struct.pack_into("<I", data, CHECKSUM, fields.get("checksum", ALLOCATION_CHECKSUM))
    struct.pack_into("<I", data, HEADER_POINTER, HEADER)
    return bytes(data)


def game_time(time: int, seconds_per_tick: float = 1 / 30) -> Memory:
    return Memory(struct.pack("<BBhfI", 1, 1, 30, seconds_per_tick, time))


def game(random_: random.Random, options: bytes, active: int = 1) -> Memory:
    data = bytearray(random_.randbytes(GAME_SIZE))
    data[GAME_OPTIONS : GAME_OPTIONS + OPTIONS_SIZE] = options
    data[GAME_ACTIVE] = active
    return Memory(bytes(data))


def procs(first_mock: int, count: int) -> bytes:
    """A table of procs: each is a function that the test stands in for."""
    return struct.pack(f"<{count}I", *(mock_address(first_mock + n) for n in range(count)))


def run(t) -> None:
    random_ = random.Random(22)

    # ---------- the checksum

    t.check("crc_build_table", Buffer(random_.randbytes(0x400)))
    for size in (0, 1, 5, 100, 0x400, -1, -5, 0x80000000):
        for initialized in (b"\0", b"\x01"):
            t.check(
                "crc_checksum_buffer",
                Buffer(random_.randbytes(0x400)),
                size,
                Buffer(random_.randbytes(4)),
                globals={"crc_table_initialized": initialized, "crc_table": random_.randbytes(0x400)},
            )

    t.check("no_operation1", random_.getrandbits(32))

    # ---------- the memory of the state

    protect = {"XPhysicalProtect": lambda harness, arguments: 0}
    for base in (BASE, 0x80061000):
        t.check(
            "game_state_buffer_protect",
            random_.getrandbits(32),
            globals={"game_state_globals": game_state_globals(random_, base=base)},
            externals=protect,
        )

    for size in (0, 1, 3, 4, 5, 0x1288, 0x10001, 0x7FFFFFFD):
        for allocated in (0, 0x1288, 0x12345):
            globals_ = {"game_state_globals": game_state_globals(random_, cpu_allocation_size=allocated)}
            t.check("game_state_malloc", size, globals=globals_)
            t.check("game_state_allocation_allocate", random_.getrandbits(32), size, globals=globals_)
            for alignment_bits in (0, 2, 5, 12, 0x105):
                t.check("game_state_malloc_aligned", size, alignment_bits, globals=globals_)

    # ---------- small procs

    for index in (0, 3, 0x7FFF, 0xFFFF):
        t.check(
            "game_state_set_structure_bsp_index",
            globals={
                "game_state_globals": game_state_globals(random_),
                "global_structure_bsp_index": struct.pack("<H", index),
            },
            memory={HEADER: Memory(header(random_))},
        )

    t.check(
        "game_state_set_revert_time",
        random_.getrandbits(32),
        globals={"game_state_globals": game_state_globals(random_), "game_time_globals": TIME},
        memory={TIME: game_time(123456)},
    )

    for allocation in (ALLOCATION, 0):
        t.check(
            "game_state_allocation_restore",
            random_.getrandbits(32),
            globals={"game_state_allocation": allocation, "game_state_allocation_functions": GlobalRange(8)},
            memory={ALLOCATION: Memory(random_.randbytes(4), places=(0,))},
        )

    def proc(call):
        return 0

    for flags in (0, 0x20, 0x60, 0xDEADBEEF):
        t.check(
            "game_state_call_before_load_procs",
            flags,
            globals={"game_state_before_load_procs": procs(0, 5)},
            mocks=[Mock(proc, stack_arguments=1)] * 5,
        )
        t.check(
            "game_state_call_after_load_procs",
            flags,
            globals={"game_state_after_load_procs": procs(0, 20)},
            mocks=[Mock(proc, stack_arguments=1)] * 20,
        )

    # ---------- starting and ending

    for initialized in (0, 1):
        t.check(
            "game_state_initialize",
            globals={
                "game_state_globals": game_state_globals(random_, initialized=initialized, cpu_allocation_size=0),
                "game_state_allocation": 0x1234,
                "game_state_allocation_functions": GlobalRange(8),
            },
            memory={BASE: Memory(random_.randbytes(0x1400), places=(HEADER_SIZE,))},
            externals={
                "game_state_allocate_buffer": lambda harness, arguments: BASE,
                "game_state_create_or_open_file": lambda harness, arguments: 0,
            },
        )
        t.check(
            "game_state_dispose",
            globals={
                "game_state_globals": game_state_globals(random_, initialized=initialized),
                "xbox_game_state_globals": random_.randbytes(0x28),
            },
            externals={"game_state_close_file": lambda harness, arguments: 0},
        )

    for scenario_path in (LOCKOUT, b"x" * 300, b""):
        cache_file_globals = bytearray(random_.randbytes(0x810))
        cache_file_globals[8 + 0x1BC : 8 + 0x1BC + 256] = scenario_path.ljust(256, b"\0")[:256]
        t.check(
            "game_state_initialize_for_new_map",
            globals={
                "game_state_globals": game_state_globals(random_),
                "cache_file_globals": bytes(cache_file_globals),
                "main_game_globals": GAME,
            },
            memory={HEADER: Memory(random_.randbytes(HEADER_SIZE)), GAME: game(random_, random_.randbytes(OPTIONS_SIZE))},
            externals=protect,
        )

    # ---------- a checkpoint

    def write_to_file(harness, arguments):
        # the game gives the index with what is after it in the globals
        return 0, (arguments[0] & 0xFFFF,)

    for file_index in (0, 1):
        for valid in (0, 1):
            t.check(
                "game_state_save",
                globals={
                    "game_state_globals": game_state_globals(random_, file_index=file_index, valid=valid, deaths=3),
                    "game_state_before_save_procs": procs(0, 4),
                    "game_state_after_save_procs": procs(4, 1),
                    "game_time_globals": TIME,
                    "main_vblank_count": random_.randbytes(8),
                    "main_time_globals": random_.randbytes(0x98),
                },
                memory={TIME: game_time(random_.getrandbits(32)), HEADER: Memory(header(random_))},
                mocks=[Mock(proc, stack_arguments=1)] * 5,
                externals={"game_state_write_to_file": write_to_file},
            )

    # the game is lost: soon after the checkpoint or not
    for valid, previous_valid in ((1, 1), (1, 0), (0, 1), (0, 0)):
        for ticks in (0, 1, 239, 240, 241, 100000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF):
            for deaths in (0, 3, 4, 5, 0x7FFF):
                for file_index in (0, 1):
                    t.check(
                        "game_state_note_death",
                        globals={
                            "game_state_globals": game_state_globals(
                                random_,
                                valid=valid,
                                previous_valid=previous_valid,
                                deaths=deaths,
                                file_index=file_index,
                                save_time=5000,
                            ),
                            "game_time_globals": TIME,
                        },
                        memory={TIME: game_time((5000 + ticks) & 0xFFFFFFFF)},
                    )
    for seconds_per_tick in (1 / 60, 1.0, 0.0, 8.0, 7.9999995):
        t.check(
            "game_state_note_death",
            globals={
                "game_state_globals": game_state_globals(random_, valid=1, previous_valid=1, save_time=10),
                "game_time_globals": TIME,
            },
            memory={TIME: game_time(11, seconds_per_tick)},
        )

    # ---------- the headers

    def verify(harness, arguments, result=1):
        return result, ()

    def match(harness, arguments, result=1):
        # the options are in the stack or in the globals: what they have is compared
        return result, tuple(bytes(harness.mu.mem_read(address, 16)) for address in arguments)

    variants = [
        {},
        {"build": b"02.09.27.09808"},
        {"checksum": ALLOCATION_CHECKSUM ^ 1},
        {"base": BASE + 4},
        {"map_name": b""},
        {"map_name": b"lockout"},
        {"map_name": b"\\"},
        {"map_name": b"a\\" * 127 + b"a"},
    ]
    for fields in variants:
        for result in (1, 0, 0x100, 0x80):
            t.check(
                "game_state_header_valid",
                Buffer(header(random_, **fields)),
                globals={"game_state_globals": game_state_globals(random_)},
                externals={"game_options_verify": lambda harness, arguments, result=result: verify(harness, arguments, result)},
            )

    others = [
        {},
        {"map_name": b"scenarios\\multi\\lockout\\lockouT"},
        {"map_name": b"scenarios\\multi\\lockout\\lockout2"},
        {"cache_file_checksum": 0x600DF00E},
        {"structure_bsp_index": 3},
        {"structure_bsp_index": -2},
        {"checksum": 5, "base": 7, "build": b"another"},
    ]
    for fields in variants[:3]:
        for other in others:
            for result in (1, 0):
                t.check(
                    "game_state_headers_match",
                    Buffer(header(random_, **fields)),
                    Buffer(header(random_, **other)),
                    globals={"game_state_globals": game_state_globals(random_)},
                    externals={
                        "game_options_verify": verify,
                        "game_options_match": lambda harness, arguments, result=result: match(harness, arguments, result),
                    },
                )

    # ---------- going back to a checkpoint

    def reclaim(harness, arguments):
        address = harness.global_addresses["xbox_game_state_globals"]
        harness.mu.mem_write(address + 0xC, bytes(4))
        return 0, ()

    def switch_structure_bsp(harness, arguments, result=1):
        return result, (arguments[0] & 0xFFFF, arguments[1])

    def xbox_globals(valid: tuple[int, int], write_buffer: int = 0) -> bytes:
        data = bytearray(random_.randbytes(0x28))
        struct.pack_into("<I", data, 0xC, write_buffer)
        data[0x1A:0x1C] = bytes(valid)
        return bytes(data)

    cases = [
        {},
        {"flags": 1},
        {"flags": 2},
        {"flags": 3},
        {"flags": 0xFFFFFFFC},
        {"reads": 3},
        {"file_index": 1},
        {"write_buffer": BASE + 0x200000},
        {"valid": 0},
        {"file_valid": (0, 1)},
        {"file_valid": (1, 0), "file_index": 1},
        {"saved": {"build": b"no"}},
        {"saved": {"checksum": 1}},
        {"game": 0},
        {"active": 0},
        {"options_match": 0},
        {"switch": 0},
        {"current": {"map_name": b"scenarios\\solo\\other\\other"}},
        {"current": {"structure_bsp_index": 9}},
        {"current": {"cache_file_checksum": 1}},
    ]
    for case in cases:
        file_index = case.get("file_index", 0)
        options = random_.randbytes(OPTIONS_SIZE)
        saved = header(random_, options=options, **case.get("saved", {}))
        current = header(random_, options=options, **case.get("current", {}))
        headers = [header(random_, map_name=b"wrong\\one"), header(random_, map_name=b"wrong\\one")]
        headers[file_index] = saved
        reads = case.get("reads", 1)

        def read_from_file(harness, arguments, reads=reads, saved=saved):
            # the state that is read is that of the checkpoint only the last time
            calls = sum(1 for call in harness._calls if call[0] == "game_state_read_from_file")
            data = bytearray(saved)
            if calls < reads - 1:
                data[MAP_NAME] ^= 1
            harness.mu.mem_write(HEADER, bytes(data))
            return 1, (arguments[0] & 0xFFFF,)

        t.check(
            "game_state_revert",
            case.get("flags", 0),
            globals={
                "game_state_globals": game_state_globals(
                    random_, valid=case.get("valid", 1), file_index=file_index, headers=headers
                ),
                "xbox_game_state_globals": xbox_globals(case.get("file_valid", (1, 1)), case.get("write_buffer", 0)),
                "main_game_globals": GAME if case.get("game", 1) else 0,
                "main_globals+0x46": b"\0",
                "game_state_before_load_procs": procs(0, 5),
                "game_state_after_load_procs": procs(5, 20),
            },
            memory={HEADER: Memory(current), GAME: game(random_, options, case.get("active", 1))},
            mocks=[Mock(proc, stack_arguments=1)] * 25,
            externals={
                "game_state_write_reclaim": reclaim,
                "game_options_verify": verify,
                "game_options_match": lambda harness, arguments, result=case.get("options_match", 1): match(
                    harness, arguments, result
                ),
                "scenario_switch_structure_bsp": lambda harness, arguments, result=case.get("switch", 1): (
                    switch_structure_bsp(harness, arguments, result)
                ),
                "game_state_read_from_file": read_from_file,
            },
        )
