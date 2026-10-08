# source/main/main_game.c
#
# The functions that load a map and let go of it call those of the caches and
# of the game: the tests stand in for them, and what is compared is the order
# of the calls and what is left in the globals

import random
import struct

from difftest import Buffer, GlobalData, Memory, mock_address
from test_async import calls_so_far
from test_scenario_load import physical_memory

NONE = 0xFFFFFFFF
LOAD_GLOBALS_SIZE = 0x110
STATE = 0x4
MAP_NAME = 0x8
OPTIONS_SIZE = 0x1118
GAME_SIZE = 0x1200

# the globals tag: the texts of each language
LANGUAGE_TEXTS = 0x188
LANGUAGE_TEXTS_SIZE = 0x1C
LANGUAGES = 9

MEMORY_USERS = (
    ("unknown_00081f80", "unknown_00081780"),
    ("user_interface_memory_pool_delete", "user_interface_memory_pool_new"),
    ("saved_game_files_free_memory", "saved_game_files_allocate_memory"),
    ("game_voice_pool_delete", "game_voice_pool_new"),
    ("unknown_000b3db0", "unknown_000b3d30"),
    ("unknown_0021ebe0", "unknown_0021ebc0"),
    ("animation_cache_free_memory", "animation_cache_allocate_memory"),
    ("geometry_cache_free_memory", "geometry_cache_allocate_memory"),
)


def nothing(harness, arguments):
    return 0


def stand_ins(*names: str, **results: object) -> dict:
    return {**{name: nothing for name in names}, **{name: (lambda harness, arguments, value=value: value) for name, value in results.items()}}


def run(t) -> None:
    random_ = random.Random(36)

    def load_globals(state: int | None = None, configuration: int | None = None, unknown10c: int | None = None) -> bytes:
        data = bytearray(random_.randbytes(LOAD_GLOBALS_SIZE))
        struct.pack_into("<I", data, 0, random_.randint(0, 5) if configuration is None else configuration)
        struct.pack_into("<I", data, STATE, random_.choice((0, 1, 2, 3, 5)) if state is None else state)
        data[0x10C] = random_.getrandbits(1) if unknown10c is None else unknown10c
        return bytes(data)

    def memory() -> bytes:
        stage = random_.randint(0, 3)
        return physical_memory(stage, random_.getrandbits(24), random_.getrandbits(24))

    def game_globals_tag() -> Buffer:
        data = bytearray(random_.randbytes(LANGUAGE_TEXTS + LANGUAGES * LANGUAGE_TEXTS_SIZE))
        for language in range(LANGUAGES):
            data[LANGUAGE_TEXTS + language * LANGUAGE_TEXTS_SIZE + 0x18] = random_.choice((0, 0, 1, 0x80))
        return Buffer(bytes(data))

    def language() -> dict:
        return {"global_language": random_.choice((NONE, 0, 1, 5, 8)), "global_game_globals": game_globals_tag()}

    def language_externals() -> dict:
        return {"XGetLanguage": lambda harness, arguments, value=random_.randint(0, 9): value}

    # ---------- the stages of loading a map

    for keep in (0, 1, 0x100, 0xFF):
        for _ in range(5):
            t.check(
                "main_game_load_begin",
                keep,
                globals={"main_game_load_globals": load_globals(), "physical_memory_globals": memory()},
            )

    names = (b"scenarios\\solo\\03a_oldmombasa\\earthcity_1", b"", b"x" * 0x103, b"y" * 0x104, b"z" * 0x140)
    for keep in (0, 0, 1):
        for name in names:
            t.check(
                "main_game_load_end",
                keep,
                Buffer(name + b"\0"),
                globals={
                    **language(),
                    "main_game_load_globals": load_globals(),
                    "texture_cache_globals": random_.randbytes(0x38),
                    # a data array: the game marks it valid and empties it
                    "unknown_00502104": Buffer(random_.randbytes(0x4C)),
                },
                externals={
                    **language_externals(),
                    **stand_ins(
                        "animation_cache_open",
                        "language_texts_load",
                        "geometry_cache_open",
                        "texture_cache_load_preloaded_pixels",
                        "texture_cache_open",
                        "data_delete_all",
                    ),
                },
            )

    def scenario_globals() -> dict:
        return {
            "global_scenario_index": random_.getrandbits(16),
            "global_scenario": 0x60F00000,
            "global_structure_bsp_index": struct.pack("<h", random_.randint(0, 5)),
            "global_structure_bsp": 0x60F00100,
            "global_structure_lightmap": 0x60F00200,
            "global_collision_bsp": 0x60F00300,
            "global_bsp3d": 0x60F00400,
        }

    for loaded in (0, 1, 1, 1, 0x100):
        for _ in range(8):
            t.check(
                "main_game_load_fail",
                loaded,
                globals={
                    **language(),
                    **scenario_globals(),
                    "main_game_load_globals": load_globals(),
                    "physical_memory_globals": memory(),
                },
                externals={
                    **language_externals(),
                    **stand_ins(
                        "sound_cache_close",
                        "texture_cache_close",
                        "geometry_cache_close",
                        "animation_cache_close",
                        "scenario_tags_unload",
                    ),
                },
            )

    # the game has what closes the animation cache in the function itself:
    # the cache is given, with no blocks to throw out
    for _ in range(60):
        arrays = [Buffer(random_.randbytes(0x4C)) if random_.random() < 0.8 else 0 for _ in range(2)]
        t.check(
            "main_game_close_caches",
            globals={
                **language(),
                "main_game_load_globals": load_globals(),
                "animation_cache_initialized": bytes([random_.random() < 0.8]),
                "animation_cache_opened": bytes([random_.random() < 0.8]),
                "animation_cache_globals": arrays[0],
                "animation_cache_globals+0x4": arrays[1],
                "animation_cache_globals+0xc": 0x60F00500,
            },
            externals={
                **language_externals(),
                **stand_ins("sound_cache_close", "texture_cache_close", "geometry_cache_close", "lruv_flush"),
            },
        )

    for _ in range(10):
        t.check(
            "main_game_open_texture_cache",
            globals={"main_game_load_globals": load_globals(), "physical_memory_globals": memory()},
            externals=stand_ins("texture_cache_open"),
        )

    for state in (*range(-1, 8), 0x102, NONE - 1):
        t.check("main_game_memory_configuration_is_session", state & 0xFFFFFFFF)

    # ---------- how the memory is shared out

    def options(game_mode: int, simulation: int, coop: int) -> Buffer:
        data = bytearray(random_.randbytes(OPTIONS_SIZE))
        struct.pack_into("<I", data, 0, game_mode)
        data[4] = simulation & 0xFF
        data[0x12C] = coop
        return Buffer(bytes(data))

    def some_options() -> Buffer:
        return options(random_.choice((0, 1, 1, 1, 2, 3)), random_.choice((0, 1, 2, 3, 4, 5, 6, -1)), random_.choice((0, 1, 0x80)))

    for game_mode in (0, 1, 2, 3):
        for simulation in (0, 1, 3, 4, 5, 6, -1):
            for coop in (0, 1):
                for session, state in ((NONE, 1), (3, 1), (3, 0), (3, 2)):
                    t.check(
                        "main_game_memory_configuration",
                        options(game_mode, simulation, coop),
                        globals={"unknown_00467214": session},
                        externals=stand_ins(unknown_0006cd50=state),
                    )
    t.check("main_game_memory_configuration", 0, globals={"unknown_00467214": 3}, externals=stand_ins(unknown_0006cd50=1))

    users = stand_ins(*(name for pair in MEMORY_USERS for name in pair))
    for _ in range(150):
        t.check(
            "main_game_configure_memory",
            some_options() if random_.random() < 0.95 else 0,
            globals={
                "unknown_00467214": random_.choice((NONE, 3)),
                "main_game_load_globals": load_globals(configuration=random_.randint(0, 5)),
                "physical_memory_globals": memory(),
            },
            externals={**users, **stand_ins(unknown_0006cd50=random_.choice((0, 1)))},
        )

    # ---------- a game starts and ends

    def game(active: int = 1) -> Buffer:
        data = bytearray(random_.randbytes(GAME_SIZE))
        struct.pack_into("<I", data, 8, random_.choice((0, 1, 2, 3)))
        data[0xC] = random_.choice((0, 1, 4, 5))
        data[0x1120] = active
        return Buffer(bytes(data))

    def network(active: int) -> dict:
        other = bytearray(random_.randbytes(0x40))
        other[0x2F] = active
        data = bytearray(random_.randbytes(0x10))
        data[0] = random_.random() < 0.7
        data[0xA] = random_.random() < 0.2
        return {"unknown_004cf770": bytes(data), "unknown_004cf770+0xc": Buffer(bytes(other))}

    for _ in range(200):
        t.check(
            "main_game_start",
            some_options(),
            globals={"main_game_globals": game(0), **network(random_.choice((0, 1, 0x80)))},
            externals=stand_ins(
                "game_initialize_for_new_map",
                "unknown_000d5560",
                "unknown_000c09c0",
                "game_create_players",
                "unknown_00158530",
                "unknown_00138f10",
                "unknown_00200d80",
                "game_dispose_from_old_map",
                scenario_switch_structure_bsp=random_.random() < 0.8,
            ),
        )

    for _ in range(300):
        t.check(
            "main_game_unload",
            globals={
                "main_game_globals": game(random_.random() < 0.7) if random_.random() < 0.9 else 0,
                **network(random_.random() < 0.7),
                # the game has nothing for the stages 1 and 3
                "main_game_load_globals": load_globals(state=random_.choice((0, 2, 4, 5)), unknown10c=random_.random() < 0.2),
                "physical_memory_globals": memory(),
            },
            externals=stand_ins(
                "unknown_000693a0",
                "game_dispose_from_old_map",
                "main_game_close_caches",
                "scenario_unload",
                "texture_cache_close",
                scenario_switch_structure_bsp=1,
            ),
        )

    def unknown_00163890(result):
        # what it is called with, and how far the loading is then
        def handler(harness, arguments):
            address = harness.global_addresses["main_game_load_globals"] + STATE
            return result, (*arguments, struct.unpack("<I", harness.mu.mem_read(address, 4))[0])

        return handler

    for result in (0, 1):
        for _ in range(5):
            t.check(
                "main_game_unknown_0018f100",
                random_.getrandbits(32) | 0x40000000,
                globals={"main_game_load_globals": load_globals(), "physical_memory_globals": memory()},
                externals={**stand_ins("texture_cache_open", "texture_cache_close"), "unknown_00163890": unknown_00163890(result)},
            )

    for _ in range(10):
        t.check(
            "main_game_launch_main_menu",
            Buffer(random_.randbytes(OPTIONS_SIZE)),
            random_.getrandbits(32),
            externals=stand_ins("game_options_new_default", "game_options_setup_local_players"),
        )

    # ---------- changing the game

    CHANGE_GLOBALS_SIZE = 0x1230
    CHANGE_PENDING, CHANGE_TO_NOTHING, CHANGE_OPTIONS = 0x10D, 0x10E, 0x118

    def change_globals(pending: int, to_nothing: int, state: int | None = None) -> GlobalData:
        data = bytearray(random_.randbytes(CHANGE_GLOBALS_SIZE))
        struct.pack_into("<I", data, STATE, random_.choice((0, 2, 5)) if state is None else state)
        data[CHANGE_PENDING] = pending
        data[CHANGE_TO_NOTHING] = to_nothing
        return GlobalData(bytes(data))

    def network_game() -> dict:
        return {
            "unknown_00527330": bytes([random_.random() < 0.7]),
            "unknown_00527334": random_.choice((3, 8, 3, 8, 5, 0)),
        }

    def options_start(harness, address: int) -> bytes:
        """What a function is given as the options of a game, when they are in
        the stack: the stack is not at the same place in the game and in source/."""
        return bytes(harness.mu.mem_read(address, 0x130))

    def flag(harness, name: str) -> int:
        return harness.mu.mem_read(harness.global_addresses[name], 1)[0]

    # the game on the map starts again
    for _ in range(60):
        game_buffer = game()
        data = bytearray(game_buffer.data)
        bsp_index = random_.randint(0, 3)
        struct.pack_into("<h", data, 8 + 0x120, random_.choice((bsp_index, bsp_index, bsp_index + 1)))
        started = random_.random() < 0.7
        t.check(
            "main_game_restart",
            globals={
                "main_game_globals": Buffer(bytes(data)),
                "main_game_load_globals": load_globals(),
                "global_structure_bsp_index": struct.pack("<h", bsp_index),
                "input_globals+0x24": random_.randbytes(0x120),
            },
            externals={
                **stand_ins(
                    "unknown_0014bac0",
                    "game_dispose_from_old_structure_bsp",
                    "game_dispose_from_old_map",
                    scenario_switch_structure_bsp=1,
                ),
                "main_game_start": lambda harness, arguments, started=started: (
                    started,
                    # with the options, whether the game knows that it is starting again
                    (
                        options_start(harness, arguments[0]),
                        harness.mu.mem_read(harness.global_addresses["main_game_load_globals"] + 0x10C, 1)[0],
                    ),
                ),
                "main_game_load_failed": lambda harness, arguments: (
                    0,
                    (harness.mu.mem_read(harness.global_addresses["main_game_load_globals"] + 0x10C, 1)[0],),
                ),
            },
        )

    # another game is asked for
    for _ in range(200):
        game_options = Buffer(random_.randbytes(OPTIONS_SIZE))
        data = bytearray(game_options.data)
        struct.pack_into("<I", data, 0, random_.choice((1, 3, 3)))
        data[5] = random_.random() < 0.2
        struct.pack_into("<i", data, 0x264, random_.choice((0, 1, 2, 3, 6, 7, -1)))
        t.check(
            "main_game_change",
            Buffer(bytes(data)) if random_.random() < 0.85 else 0,
            globals={"main_game_load_globals": change_globals(0, 0), **network_game()},
            externals={"GetTickCount": lambda harness, arguments: 0x4321, **stand_ins("network_life_cycle_end")},
        )

    for _ in range(200):
        t.check(
            "main_game_change_update",
            globals={
                "main_game_load_globals": change_globals(random_.random() < 0.8, random_.random() < 0.4),
                "unknown_0051ea10": random_.choice((0, 0, 1, NONE)),
                "main_globals": random_.randbytes(0x54),
            },
            externals=stand_ins(
                "main_game_unload",
                "main_game_configure_memory",
                "main_game_open_texture_cache",
                "main_game_change_immediate",
                unknown_00149318=random_.random() < 0.2,
            ),
        )

    for _ in range(200):
        game_options = some_options()
        data = bytearray(game_options.data)
        struct.pack_into("<I", data, 0x264, random_.getrandbits(32))
        t.check(
            "main_game_change_immediate",
            Buffer(bytes(data)) if random_.random() < 0.9 else 0,
            globals={"main_globals": random_.randbytes(0x54), "input_globals+0x24": random_.randbytes(0x120)},
            externals=stand_ins(
                "main_game_unload",
                "main_game_configure_memory",
                "main_game_open_texture_cache",
                "main_game_campaign_started",
                "unknown_001483c3",
                "main_game_load_failed",
                main_game_load_map=random_.random() < 0.8,
                main_game_start=random_.random() < 0.8,
                unknown_0012bf90=random_.random() < 0.5,
            ),
        )

    # the map of a game: on the hard disk, to be copied, or not there at all
    for status in range(5):
        for _ in range(12):
            t.check(
                "main_game_load_map",
                some_options(),
                globals={
                    "main_game_loading_map": bytes([random_.getrandbits(1)]),
                    "main_game_loading_map_time": 7,
                    "unknown_0051ea00": b"\0",
                    "main_globals": random_.randbytes(0x54),
                    "main_game_load_globals": change_globals(0, 0),
                    **network_game(),
                },
                externals={
                    "GetTickCount": lambda harness, arguments: 0x4321,
                    # the flag is set while the tags are loaded
                    "scenario_load": lambda harness, arguments, loaded=random_.random() < 0.7: (
                        loaded,
                        (arguments[0], flag(harness, "main_game_loading_map")),
                    ),
                    # a flag in a register: only its low byte is the argument
                    "main_game_load_begin": lambda harness, arguments: (0, (arguments[0] & 0xFF,)),
                    "main_game_load_end": lambda harness, arguments: (0, (arguments[0] & 0xFF, arguments[1])),
                    "main_game_load_fail": lambda harness, arguments: (0, (arguments[0] & 0xFF,)),
                    **stand_ins(
                        "unknown_0011c840",
                        "network_life_cycle_end",
                        cached_map_file_status=status,
                        main_game_unknown_0018f100=random_.random() < 0.6,
                    ),
                },
            )

    for failing in (0, 1):
        for _ in range(3):
            t.check(
                "main_game_load_failed",
                globals={"main_game_load_failing": bytes([failing])},
                externals={
                    "main_game_launch_main_menu": lambda harness, arguments: (0, (arguments[1],)),
                    "main_game_change_immediate": lambda harness, arguments: (
                        random_.getrandbits(1) * 0,
                        (flag(harness, "main_game_load_failing"),),
                    ),
                    **stand_ins("main_game_unload", "main_game_open_texture_cache", "main_files_damaged"),
                },
            )

    # ---------- what is left

    PROFILE_SIZE = 0xC70
    PROFILE_SAVED_GAME = 0x1F8

    def profiles() -> bytes:
        data = bytearray(random_.randbytes(4 * PROFILE_SIZE))
        for index in range(4):
            data[index * PROFILE_SIZE] = random_.getrandbits(8) & 0xEF | random_.choice((0, 0x10, 0x10))
        return bytes(data)

    for _ in range(200):
        data = bytearray(random_.randbytes(OPTIONS_SIZE))
        struct.pack_into("<I", data, 0, random_.choice((1, 1, 2, 3)))
        struct.pack_into("<I", data, 0x18, random_.choice((NONE, NONE, random_.getrandbits(32))))
        data[0x12C] = random_.choice((0, 0, 0, 1, 0x80))
        t.check(
            "main_game_campaign_started",
            Buffer(bytes(data)),
            globals={
                "unknown_0054e7cc": bytes([random_.choice((0, 1, 1, 1, 0x80))]),
                "unknown_0054e8e0": profiles(),
            },
            externals=stand_ins(
                "unknown_0019040d",
                "game_state_load_saved_game",
                unknown_0018fada=random_.choice((NONE, 0, 1, 2, 3)),
            ),
        )

    filler = random_.randbytes(OPTIONS_SIZE)

    def new_default(harness, arguments):
        # the options are in the stack: all of them are written, so that what
        # the function leaves in them can be compared
        harness.mu.mem_write(arguments[0], filler)
        return 0, ()

    def options_given(*before):
        return lambda harness, arguments: (0, (*before, bytes(harness.mu.mem_read(arguments[-1], OPTIONS_SIZE))))

    for _ in range(60):
        data = bytearray(random_.randbytes(GAME_SIZE))
        data[0x1120] = random_.choice((0, 1, 1, 1, 0x80))
        struct.pack_into("<I", data, 8, random_.choice((1, 2, 3, 3, 3)))
        t.check(
            "main_game_goto_main_menu",
            random_.getrandbits(32) | 0x40000000,
            globals={"main_game_globals": Buffer(bytes(data)) if random_.random() < 0.85 else 0},
            externals={
                "game_options_new_default": new_default,
                "game_options_setup_local_players": lambda harness, arguments: (0, (arguments[0], bytes(harness.mu.mem_read(arguments[1], OPTIONS_SIZE)))),
                "main_game_change": options_given(),
                **stand_ins("unknown_001483c3"),
            },
        )

    # the allocation whose memory is the texture cache's. How long it waits
    # when there is never any is a matter of the processor's clock: here the
    # memory comes at once or after the cache is given some time
    CACHE_SIZE = 0x40
    REGISTERS = 0x80000000
    for _ in range(80):
        # 31: after 30 times the console is waited for, once
        failures = random_.choice((0, 0, 1, 5, 29, 30, 31, 31))
        address = random_.getrandbits(24) | 0x80000000

        def steal_now(harness, arguments, failures=failures, address=address):
            return (address if calls_so_far(harness, "texture_cache_steal_memory_now") >= failures else 0), tuple(arguments)

        cache = bytearray(random_.randbytes(CACHE_SIZE))
        struct.pack_into("<i", cache, 0x30, random_.choice((0, 1, 0x200, 0x200, 0x200, -1)))
        t.check(
            "main_game_texture_allocation_allocate",
            Buffer(struct.pack("<3I", random_.getrandbits(32), mock_address(0), mock_address(1))),
            random_.choice((0, NONE, 0x80000000, 1, 0x1000, 0x12345, 0x7FFFFFFF)),
            globals={
                "texture_cache_globals+0x10": Buffer(bytes(cache)),
                "d3d_device": Buffer(bytes(0x934) + struct.pack("<I", REGISTERS) + bytes(0x10)),
            },
            # the registers of the console: it has things to do
            memory={REGISTERS + 0x3000: Memory(bytes(0x240) + struct.pack("<2I", 1, 0) + bytes(0x100))},
            externals={
                "texture_cache_steal_memory_now": steal_now,
                **stand_ins("texture_cache_idle", "D3DDevice_KickOff", "SwitchToThread"),
            },
        )

    for _ in range(10):
        t.check(
            "main_game_texture_allocation_deallocate",
            Buffer(random_.randbytes(12)),
            random_.getrandbits(32),
            externals=stand_ins("texture_cache_return_memory"),
        )

    # ---------- the next map of the campaign

    REPORT_SIZE = 0x33B
    MAPS = 0x400
    report = random_.randbytes(REPORT_SIZE)

    def options_of(harness, address: int) -> bytes:
        return bytes(harness.mu.mem_read(address, OPTIONS_SIZE))

    def string(harness, address: int) -> bytes:
        return bytes(harness.mu.mem_read(address, 0x200)).split(b"\0", 1)[0]

    def write_report(harness, arguments):
        harness.mu.mem_write(arguments[1], report)
        return 0, (options_of(harness, arguments[0]),)

    def send_report(harness, arguments):
        data, size, content_type = arguments
        return 0, (bytes(harness.mu.mem_read(data, REPORT_SIZE)), size, string(harness, content_type))

    def change_options(harness, arguments):
        # something of the options is changed, to see that they are read after
        address = arguments[0]
        harness.mu.mem_write(address + 0x14, bytes(b ^ 0x55 for b in harness.mu.mem_read(address + 0x14, 4)))
        return 0, (options_of(harness, address),)

    for _ in range(150):
        data = bytearray(random_.randbytes(GAME_SIZE))
        struct.pack_into("<I", data, 8 + 0x14, random_.choice((NONE, 0, 1, random_.getrandbits(32))))
        struct.pack_into("<I", data, 8 + 0x18, random_.choice((NONE, 0, 3, random_.getrandbits(32))))
        # the maps of the campaign are in a global of the test: what is known
        # of one is anywhere in it, or nowhere
        maps = bytearray(random_.randbytes(MAPS))
        name = random_.choice((b"", b"scenarios\\solo\\03a_oldmombasa\\03a_oldmombasa", b"m" * 0x103, b"n" * 0x104, b"o" * 0x130))
        offset = random_.randrange(0, 0x100)
        maps[offset + 8 : offset + 8 + len(name) + 1] = name + b"\0"
        found = random_.random() < 0.85

        def map_data(harness, arguments, offset=offset, found=found):
            address = harness.global_addresses["unknown_0054e8e0"] + offset if found else 0
            return address, tuple(arguments)

        t.check(
            "main_game_next_map",
            globals={
                "main_game_globals": Buffer(bytes(data)),
                "unknown_00467214": random_.choice((NONE, 0, 5)),
                "unknown_0054e8e0": bytes(maps),
            },
            externals={
                "unknown_0006cd50": lambda harness, arguments, state=random_.choice((0, 1, 1, 2)): state,
                "unknown_001c5ca0": write_report,
                "unknown_000b3ed0": send_report,
                "unknown_0019c440": lambda harness, arguments, next_=random_.choice((NONE, 0, 1, 7, 7, 7)): next_,
                "unknown_0019c270": map_data,
                "time": lambda harness, arguments, value=random_.getrandbits(32): value,
                "rand": lambda harness, arguments, value=random_.getrandbits(15): value,
                "GetTickCount": lambda harness, arguments, value=random_.getrandbits(32): value,
                "unknown_00152450": change_options,
                "unknown_00152660": lambda harness, arguments: (0, (options_of(harness, arguments[0]),)),
                "unknown_00063f50": lambda harness, arguments, result=random_.choice((0, 1)): (
                    result,
                    (arguments[0], arguments[1], string(harness, arguments[2])),
                ),
                "main_game_change": lambda harness, arguments: (0, (options_of(harness, arguments[0]),)),
                **stand_ins("main_game_goto_main_menu"),
            },
        )
