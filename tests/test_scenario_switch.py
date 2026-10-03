# changing the structure bsp of the scenario that is loaded
# (source/scenario/scenario.c), forgetting one (source/cache/cache_files.c)
# and what the main loop does when the game asks for a checkpoint or for
# another structure bsp (source/main/main.c)
#
# The scenario, its structure bsps and the tags are made up by the test, at
# addresses of the memory that the console maps

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, Memory, Mock
from test_game import GAME_SIZE, main_globals, systems
from test_game_state import GAME

NONE = 0xFFFFFFFF

SCENARIO = PHYSICAL_ADDRESS + 0x70000
REFERENCES = PHYSICAL_ADDRESS + 0x71000
INSTANCES = PHYSICAL_ADDRESS + 0x72000
TAGS = PHYSICAL_ADDRESS + 0x73000
PLAYERS = PHYSICAL_ADDRESS + 0x74000
TEXT_SOURCE = PHYSICAL_ADDRESS + 0x75000

REFERENCE_SIZE = 0x44
REFERENCE_COUNT = 3
INSTANCE_COUNT = 16
TAG_SIZE = 0x100


def scenario() -> Memory:
    data = bytearray(0x300)
    struct.pack_into("<iI", data, 0x210, REFERENCE_COUNT, REFERENCES)
    return Memory(bytes(data))


def references(random_: random.Random) -> Memory:
    """The structure bsps of the scenario: the second has no lightmap."""
    data = bytearray(random_.randbytes(REFERENCE_SIZE * REFERENCE_COUNT))
    for index in range(REFERENCE_COUNT):
        lightmap = NONE if index == 1 else 0xE2740008 + index
        struct.pack_into("<I", data, index * REFERENCE_SIZE + 0x14, 0xE1740002 + index)
        struct.pack_into("<I", data, index * REFERENCE_SIZE + 0x1C, lightmap)
    return Memory(bytes(data))


def instances(random_: random.Random) -> Memory:
    data = bytearray(random_.randbytes(INSTANCE_COUNT * 16))
    for index in range(INSTANCE_COUNT):
        struct.pack_into("<I", data, index * 16 + 8, TAGS + index * TAG_SIZE)
    return Memory(bytes(data))


def tags(random_: random.Random) -> Memory:
    data = bytearray(random_.randbytes(INSTANCE_COUNT * TAG_SIZE))
    for index in range(INSTANCE_COUNT):
        struct.pack_into("<I", data, index * TAG_SIZE + 0x18, 0x80500000 + index)
    return Memory(bytes(data))


def game(random_: random.Random, structure_bsp_index: int = NONE, active: int = 1, game_mode: int = 1) -> Memory:
    data = bytearray(random_.randbytes(GAME_SIZE))
    struct.pack_into("<H", data, 2, structure_bsp_index & 0xFFFF)
    struct.pack_into("<I", data, 8, game_mode)
    data[0x1120] = active
    return Memory(bytes(data))


def nothing(harness, arguments):
    return 0


def wide(harness, address: int) -> tuple[int, ...]:
    """A wide string of the emulator's memory, up to its end."""
    characters = []
    while len(characters) < 64:
        (character,) = struct.unpack("<H", harness.mu.mem_read(address + 2 * len(characters), 2))
        if not character:
            break
        characters.append(character)
    return tuple(characters)


def run(t) -> None:
    random_ = random.Random(26)

    # ---------- forgetting a structure bsp

    for lightmap in (NONE, 0xE2740009):
        reference = bytearray(random_.randbytes(REFERENCE_SIZE))
        struct.pack_into("<I", reference, 0x14, 0xE1740003)
        struct.pack_into("<I", reference, 0x1C, lightmap)
        t.check(
            "cache_file_structure_bsp_unload",
            Buffer(bytes(reference)),
            globals={"global_tag_instances": INSTANCES, "cache_file_globals": random_.randbytes(0x810)},
            memory={INSTANCES: instances(random_)},
            externals={"unknown_0012e150": nothing},
        )

    # ---------- changing the structure bsp

    def proc(call):
        return 0

    def reference_index(harness, arguments):
        return (arguments[0] - REFERENCES) // REFERENCE_SIZE

    def dispose(harness, arguments):
        # the parts of the game have no structure bsp after this
        harness.mu.mem_write(GAME + 2, b"\xff\xff")
        return 0

    for index in (NONE, 0xFFFF, 0, 1, 2, 3, 0xFFFE, 0xABCD0001):
        for loaded in (NONE, 0, 1):
            for game_index in (NONE, 0, 1):
                for unload_old in (1, 0, 0x100):
                    for load_result in (1, 0):
                        t.check(
                            "scenario_switch_structure_bsp",
                            index,
                            unload_old,
                            globals={
                                "game_systems": systems(random_, 6, some_missing=True),
                                "main_game_globals": GAME,
                                "global_scenario": SCENARIO,
                                "global_structure_bsp_index": struct.pack("<H", loaded & 0xFFFF),
                                "global_tag_instances": INSTANCES,
                                "global_structure_bsp": random_.getrandbits(32),
                                "global_structure_lightmap": random_.getrandbits(32),
                                "global_collision_bsp": random_.getrandbits(32),
                                "global_bsp3d": random_.getrandbits(32),
                                "main_time_globals": random_.randbytes(0x98),
                                "main_vblank_count": random_.randbytes(8),
                            },
                            memory={
                                GAME: game(random_, game_index),
                                SCENARIO: scenario(),
                                REFERENCES: references(random_),
                                INSTANCES: instances(random_),
                                TAGS: tags(random_),
                            },
                            mocks=[Mock(proc, stack_arguments=1)] * 68,
                            externals={
                                "cache_file_structure_bsp_load": lambda harness, arguments, load_result=load_result: (
                                    load_result,
                                    (reference_index(harness, arguments),),
                                ),
                                "cache_file_structure_bsp_unload": lambda harness, arguments: (
                                    0,
                                    (reference_index(harness, arguments),),
                                ),
                                "unknown_0016e2c0": nothing,
                                "game_dispose_from_old_structure_bsp": dispose,
                                "game_initialize_for_new_structure_bsp": nothing,
                            },
                        )

    # ---------- what the loop does

    def get_text(harness, arguments):
        # the tag that has the text, which the game gives in a register
        string_list_index, string_id, text = arguments
        record = (string_list_index, string_id, bytes(harness.mu.mem_read(text, 2)))
        harness.mu.mem_write(text, "saved".encode("utf-16-le") + b"\0\0")
        return 0, record

    def print_message(harness, arguments):
        return 0, (arguments[0], wide(harness, arguments[1]))

    messages = {
        "hud_messages_unknown_0024cdaf": nothing,
        "unicode_string_list_get_string": get_text,
        "hud_print_message": print_message,
    }

    def players(local: tuple[int, ...]) -> Memory:
        return Memory(random_.randbytes(0xC) + struct.pack("<4I", *local))

    def sound_source(value: int) -> Memory:
        # the sound to play, and the texts
        return Memory(random_.randbytes(0x394) + struct.pack("<I", random_.choice((NONE, 7))) + random_.randbytes(0x64) + struct.pack("<I", value))

    def text_source(value: int) -> Memory:
        return Memory(random_.randbytes(0x3FC) + struct.pack("<I", value))

    locals_ = ((0, NONE, NONE, NONE), (NONE, NONE, NONE, NONE), (NONE, 7, NONE, 9), (1, 2, 3, 4), (NONE, NONE, NONE, 5))
    for local in locals_:
        for source, value in ((TEXT_SOURCE, 5), (TEXT_SOURCE, NONE), (0, 5)):
            for active in (1, 0):
                for game_mode in (1, 2):
                    common = {
                        "main_game_globals": GAME,
                        "players_globals": PLAYERS,
                        "unknown_00510c94": source,
                    }
                    memory = {
                        GAME: game(random_, active=active, game_mode=game_mode),
                        PLAYERS: players(local),
                        TEXT_SOURCE: text_source(value),
                    }
                    t.check(
                        "main_save_map_private",
                        globals={"main_globals": main_globals(random_, x48=1), **common},
                        memory=memory,
                        externals={"game_state_save": nothing, **messages},
                    )
                    for switched in (1, 0):
                        data = bytearray(main_globals(random_, x4B=1))
                        struct.pack_into("<H", data, 0x50, random_.choice((0, 2, 0xFFFF)))
                        t.check(
                            "main_switch_structure_bsp_private",
                            globals={"main_globals": bytes(data), **common},
                            memory=memory,
                            externals={
                                "scenario_switch_structure_bsp": lambda harness, arguments, switched=switched: switched,
                                "unknown_0018efa0": nothing,
                                **messages,
                            },
                        )
                    for game_globals in (GAME, 0):
                        t.check(
                            "main_unknown4E_private",
                            globals={"main_globals": main_globals(random_, x4E=1), **common, "main_game_globals": game_globals},
                            memory=memory,
                            externals={"unknown_00190e37": nothing, **messages},
                        )
                        if source:
                            # the game does not look at whether there is a source of the sound
                            t.check(
                                "main_save_map",
                                globals={"main_globals": main_globals(random_), **common, "main_game_globals": GAME},
                                memory={**memory, TEXT_SOURCE: sound_source(value)},
                                externals={"unknown_001896c0": lambda harness, arguments: 0, **messages},
                            )
                        for index, loaded, asked in ((1, 1, 1), (1, 1, 0), (1, 2, 0), (2, 2, 1), (0, NONE, 0), (-1, 1, 0), (3, 1, 0), (5, 1, 1), (0x10001, 1, 0)):
                            t.check(
                                "main_switch_structure_bsp",
                                index & 0xFFFFFFFF,
                                globals={
                                    "main_globals": main_globals(random_, x4B=asked),
                                    **common,
                                    "main_game_globals": GAME,
                                    "global_scenario": SCENARIO,
                                    "global_structure_bsp_index": struct.pack("<h", loaded if loaded != NONE else -1),
                                },
                                memory={**memory, SCENARIO: scenario()},
                                externals=messages,
                            )
                        for shown in (0, 1):
                            t.check(
                                "main_print_saving_message",
                                globals={"main_globals": main_globals(random_, x4E=shown), **common, "main_game_globals": game_globals},
                                memory=memory,
                                externals=messages,
                            )
