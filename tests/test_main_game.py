# source/main/main_game.c
#
# The functions that load a map and let go of it call those of the caches and
# of the game: the tests stand in for them, and what is compared is the order
# of the calls and what is left in the globals

import random
import struct

from difftest import Buffer, GlobalData
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
    ("unknown_001a479a", "unknown_001a474c"),
    ("unknown_00215810", "unknown_002157e0"),
    ("unknown_000533e0", "unknown_00053310"),
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
                        "unknown_001a01e0",
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
                        "unknown_002186f0",
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
                **stand_ins("unknown_002186f0", "texture_cache_close", "geometry_cache_close", "lruv_flush"),
            },
        )

    for _ in range(10):
        t.check(
            "main_game_open_texture_cache",
            globals={"main_game_load_globals": load_globals(), "physical_memory_globals": memory()},
            externals=stand_ins("texture_cache_open"),
        )

    for state in (*range(-1, 8), 0x102, NONE - 1):
        t.check("main_game_state_has_map", state & 0xFFFFFFFF)

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
                "unknown_00137e40",
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
                "unknown_0011be50",
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
            externals=stand_ins("game_options_new_default", "unknown_00138640"),
        )
