# a frame of the game's main loop and its time (source/main/main.c)
#
# The parts of the game that a frame calls are not run: the test stands in
# for each, and the calls, in their order and with their arguments, are what
# is compared

import random
import struct

from difftest import PHYSICAL_ADDRESS, Memory
from test_game import GAME_SIZE, main_globals
from test_game_state import GAME, TIME, game_time

NONE = 0xFFFFFFFF
UNKNOWN_0051EA14 = PHYSICAL_ADDRESS + 0x60000

# main_time_globals
TIME_GLOBALS_SIZE = 0x98

# main_globals
UNKNOWN2 = "x02"
LOAD_CORE = "x05"
CORE_NAME = 0x6

# what a frame calls, with no arguments or with plain ones
PLAIN = (
    "unknown_002237d4 main_game_goto_main_menu main_game_change_update main_revert_map_if_possible_private "
    "main_revert_map_private main_reset_map_private main_switch_structure_bsp_private main_save_map_private "
    "unknown_0012bbd0 main_unknown4E_private input_update input_abstraction_update saved_game_files_find_unit_files global_preferences_flush "
    "font_cache_frame cached_map_file_copy_update main_map_precache_update unknown_001479c3 unknown_00155f80 "
    "network_link_receive network_update simulation_idle network_life_cycle_update unknown_001870a0 network_send "
    "game_frame unknown_001554b0 unknown_0016f280 unknown_00222a70 unknown_00223240 sound_update "
    "simulation_update unknown_00222d80"
).split()


def nothing(harness, arguments):
    return 0


def time_globals(random_: random.Random, **fields: int) -> bytes:
    data = bytearray(random_.randbytes(TIME_GLOBALS_SIZE))
    data[0] = fields.get("unknown0", 0)
    data[1] = fields.get("unknown1", 0)
    struct.pack_into("<2I", data, 0x8, fields.get("tick_count", 5000), fields.get("tick_count_high", 0))
    struct.pack_into("<q", data, 0x10, fields.get("last_vblank", 1000))
    struct.pack_into("<q", data, 0x28, fields.get("target_vblank", 1000))
    return bytes(data)


def game(random_: random.Random, active: int = 1, game_mode: int = 1) -> Memory:
    data = bytearray(random_.randbytes(GAME_SIZE))
    struct.pack_into("<I", data, 8, game_mode)
    data[0x1120] = active
    return Memory(bytes(data))


def tick_counts(first: int, second: int, pause: bool = False):
    """GetTickCount: what it gives the first time and after. The second time,
    the game can be paused from another place."""

    def handler(harness, arguments):
        calls = sum(1 for call in harness._calls if call[0] == "GetTickCount")
        if calls == 1 and pause:
            harness.mu.mem_write(harness.global_addresses["application_paused"], b"\x01")
        return first if calls == 0 else second + calls

    return handler


def run(t) -> None:
    random_ = random.Random(25)

    # ---------- the time of a frame

    for case in range(700):
        last_vblank = random_.choice((1000, 1000 + (1 << 32), -5, (1 << 32) - 3))
        target_vblank = last_vblank + random_.choice((-10, 0, 0, 3, 1 << 32, -(1 << 32)))
        vblank_count = last_vblank + random_.choice((1, 2, 30, 599, 600, 601, 700, -100, 0, 1 << 33))
        if random_.random() < 0.3:
            vblank_count = target_vblank + random_.choice((-1, 0, 1))
        tick_count = random_.choice((5000, 0xFFFFFF00))
        game_globals = random_.choice((GAME, GAME, GAME, 0))
        t.check(
            "main_time_update",
            globals={
                "main_time_globals": time_globals(
                    random_,
                    unknown0=random_.randrange(2),
                    unknown1=random_.randrange(2),
                    tick_count=tick_count,
                    tick_count_high=random_.choice((0, 3)),
                    last_vblank=last_vblank,
                    target_vblank=target_vblank,
                ),
                "main_vblank_count": struct.pack("<q", vblank_count),
                "display_settings+2": struct.pack("<h", random_.choice((0, 0, 1, 2, 3, 4, -3))),
                "xbox_video_settings": struct.pack("<h6B", random_.choice((60, 60, 50, 0, -1)), 0, 0, 0, 0, 0, 0),
                "main_game_globals": game_globals,
                "game_time_globals": TIME,
            },
            memory={
                GAME: game(random_, random_.choice((1, 1, 0)), random_.choice((1, 2, 3, 3))),
                TIME: game_time(random_.getrandbits(31)),
            },
            externals={
                "GetTickCount": tick_counts(123, random_.choice((tick_count - 100, tick_count - 1, tick_count - 2, tick_count + 16)) & NONE),
                "user_interface_unknown_0014a224": lambda harness, arguments, result=random_.randrange(2): result,
            },
        )

    # ---------- a frame

    def unknown_00146660(harness, arguments, values):
        seconds, real_address, long_address = arguments
        record = (seconds, bytes(harness.mu.mem_read(real_address, 4)), bytes(harness.mu.mem_read(long_address, 4)))
        harness.mu.mem_write(real_address, values[0])
        harness.mu.mem_write(long_address, values[1])
        return 0, record

    def ran_ticks(harness, arguments):
        return 0, (arguments[0], bytes(harness.mu.mem_read(arguments[1], 4)))

    def network_link_receive(harness, arguments):
        # what the game sets around this call
        flag = harness.mu.mem_read(harness.global_addresses["unknown_00510548"], 1)[0]
        tick = bytes(harness.mu.mem_read(harness.global_addresses["unknown_0051054c"], 4))
        return 0, (arguments[0], flag, tick)

    def load_core(harness, arguments):
        return 0, (bytes(harness.mu.mem_read(arguments[0], 64)).split(b"\0", 1)[0],)

    actions = ("x4A", "x47", "x46", "x4B", "x48", "x49", "x4E", UNKNOWN2, LOAD_CORE)
    for case in range(500):
        if case < len(actions):
            fields = {actions[case]: 1}
        elif case == len(actions):
            fields = {}
        else:
            fields = {action: 1 for action in actions if random_.random() < 0.25}
        globals_bytes = bytearray(main_globals(random_, **fields))
        globals_bytes[CORE_NAME : CORE_NAME + 64] = b"core_name".ljust(64, b"\0")
        paused = random_.random() < 0.15
        paused_later = not paused and random_.random() < 0.15
        vblank_count = 1000 + random_.choice((1, 2, 30, 700))
        copy_globals = bytearray(random_.randbytes(0xB40))
        copy_globals[0x81C] = random_.randrange(2)
        values = (random_.randbytes(4), random_.randbytes(4))

        t.check(
            "main_loop_body",
            globals={
                "main_globals": bytes(globals_bytes),
                "main_game_load_globals+0x4": random_.choice((0, 2, 5, 7)),
                "main_game_load_globals+0x10d": bytes([random_.randrange(2)]),
                "main_game_globals": random_.choice((GAME, GAME, 0)),
                "game_time_globals": TIME,
                "unknown_0055c14d": bytes([random_.randrange(2)]),
                "unknown_0051ea14": random_.choice((UNKNOWN_0051EA14, 0)),
                "unknown_0055c150": random_.choice((0, 1, 2, 3, NONE)),
                "global_preferences_globals+0x1": bytes([random_.randrange(2)]),
                "async_globals": random_.randbytes(0x25A0),
                "cached_map_file_copy_globals": bytes(copy_globals),
                "application_paused": b"\x01" if paused else b"\0",
                "unknown_004d8ba0": bytes([random_.randrange(2)]),
                "unknown_00510548": random_.randbytes(1),
                "unknown_0051054c": random_.getrandbits(32),
                "unknown_00510560": random_.getrandbits(32),
                "main_time_globals": time_globals(random_),
                "main_vblank_count": struct.pack("<q", vblank_count),
                "display_settings+2": bytes(2),
                "xbox_video_settings": struct.pack("<h6B", 60, 0, 0, 0, 0, 0, 0),
            },
            memory={GAME: game(random_, random_.choice((1, 1, 0))), TIME: game_time(77)},
            externals={
                **{name: nothing for name in PLAIN},
                "async_task_count": lambda harness, arguments: 7,
                "game_state_load_core": load_core,
                "GetTickCount": tick_counts(123, 6000, pause=paused_later),
                "simulation_in_progress": lambda harness, arguments, result=random_.choice((1, 1, 0)): result,
                "network_link_receive": network_link_receive,
                "unknown_00146660": lambda harness, arguments, values=values: unknown_00146660(harness, arguments, values),
                "game_run_ticks": ran_ticks,
            },
        )
