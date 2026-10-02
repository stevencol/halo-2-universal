# the game as a whole and its main loop (source/game/game.c and
# source/main/main.c)
#
# The parts of the game are not run: the table of the parts is filled with
# functions that the test stands in for, and the order of the calls is what
# is compared

import random
import struct

from difftest import PHYSICAL_ADDRESS, Buffer, Memory, Mock, mock_address
from difftest import external_address as harness_external_address
from test_game_state import BASE, GAME, TIME, game_state_globals, game_time

NONE = 0xFFFFFFFF

SYSTEM_COUNT = 68
SYSTEM_FIELDS = 9
GAME_SIZE = 0x1200
OPTIONS_SIZE = 0x1118
DEVICE = PHYSICAL_ADDRESS + 0x50000
DEVICE_SIZE = 0x24A0
SEED = PHYSICAL_ADDRESS + 0x44000
UNKNOWN = PHYSICAL_ADDRESS + 0x46000

# main_globals
EXIT_GAME = 0x3
RESET_MAP = 0x46
REVERT_MAP = 0x47
SAVE_MAP = 0x48
REVERT_MAP_IF_POSSIBLE = 0x4A
SWITCH_STRUCTURE_BSP = 0x4B
REVERT_FLAG0 = 0x4C
REVERT_FLAG1 = 0x4D
SWITCH_STRUCTURE_BSP_INDEX = 0x50
MAIN_GLOBALS_SIZE = 0x54


def systems(random_: random.Random, field: int, some_missing: bool = False) -> bytes:
    """The table of the parts of the game: each has, in one of its fields, a
    function of its own that the test stands in for."""
    data = bytearray(SYSTEM_COUNT * SYSTEM_FIELDS * 4)
    for index in range(SYSTEM_COUNT):
        if some_missing and random_.random() < 0.3:
            continue
        struct.pack_into("<I", data, (index * SYSTEM_FIELDS + field) * 4, mock_address(index))
    return bytes(data)


def main_globals(random_: random.Random, **fields: int) -> bytes:
    data = bytearray(random_.randbytes(MAIN_GLOBALS_SIZE))
    for offset in (EXIT_GAME, RESET_MAP, REVERT_MAP, SAVE_MAP, 0x49, REVERT_MAP_IF_POSSIBLE, SWITCH_STRUCTURE_BSP):
        data[offset] = 0
    for offset in (REVERT_FLAG0, REVERT_FLAG1, 0x4E):
        data[offset] = 0
    for name, value in fields.items():
        data[int(name[1:], 16)] = value
    return bytes(data)


def nothing(harness, arguments):
    return 0


def run(t) -> None:
    random_ = random.Random(24)

    def proc(call):
        return 0

    procs = [Mock(proc)] * SYSTEM_COUNT

    t.check("no_operation")

    # ---------- the table itself: the same functions, in the same places, as the game's.
    # Before any case puts functions of its own in it
    harness = t.harness
    size = SYSTEM_COUNT * SYSTEM_FIELDS * 4
    tables = []
    for original in (True, False):
        names = {0: None}
        for n, name in enumerate(harness.externals):
            names[harness.symbols[name].address if original else harness_external_address(n)] = name
        for name, symbol in harness.symbols.items():
            if symbol.source:
                names[symbol.address if original else harness.functions[name]] = name
        address = harness.globals["game_systems"] if original else harness.variables["game_systems"]
        values = struct.unpack(f"<{size // 4}I", harness.mu.mem_read(address, size))
        tables.append([names.get(value, hex(value)) for value in values])
    t.cases["game_initialize"] = t.cases.get("game_initialize", 0) + 1
    if tables[0] != tables[1]:
        different = [n for n, (a, b) in enumerate(zip(*tables)) if a != b]
        n = different[0]
        t._fail(
            "game_initialize",
            f"game_systems: {len(different)} functions are not the game's; the first, of the part {n // SYSTEM_FIELDS}, "
            f"field {n % SYSTEM_FIELDS}: {tables[0][n]} and {tables[1][n]}",
        )

    # ---------- the parts of the game

    for allocated in (0x128C, 0x40000):
        t.check(
            "game_initialize",
            globals={
                "game_systems": systems(random_, 0),
                "game_state_globals": game_state_globals(random_, cpu_allocation_size=allocated),
                "main_game_globals": 0,
            },
            memory={BASE + allocated: Memory(random_.randbytes(GAME_SIZE))},
            mocks=procs,
            externals={"game_state_initialize": nothing, "_control87": nothing},
        )

    def options_new(harness, arguments):
        # the game is marked as being set up while its parts are called
        return 0, (bytes(harness.mu.mem_read(arguments[0], 16)), harness.mu.mem_read(GAME, 1)[0])

    for _ in range(3):
        t.check(
            "game_initialize_for_new_map",
            Buffer(random_.randbytes(OPTIONS_SIZE)),
            globals={"game_systems": systems(random_, 2, some_missing=True), "main_game_globals": GAME},
            memory={GAME: Memory(random_.randbytes(GAME_SIZE))},
            mocks=procs,
            externals={"game_options_new": options_new, "_control87": nothing},
        )

    def multiplayer_options(harness, arguments):
        return 0, (arguments[0] - GAME,)

    for game_mode in (1, 2, 3):
        for unknown178 in (0, 5):
            options = bytearray(random_.randbytes(OPTIONS_SIZE))
            struct.pack_into("<I", options, 0, game_mode)
            struct.pack_into("<I", options, 0x178, unknown178)
            t.check(
                "game_options_new",
                Buffer(bytes(options)),
                globals={"main_game_globals": GAME, "global_random_seed_reference": SEED},
                memory={GAME: Memory(random_.randbytes(GAME_SIZE)), SEED: Memory(random_.randbytes(4))},
                externals={"unknown_0019d650": multiplayer_options},
            )

    for name, field in (
        ("game_dispose_from_old_map", 3),
        ("game_initialize_for_new_structure_bsp", 4),
        ("game_dispose_from_old_structure_bsp", 5),
    ):
        for some_missing in (False, True, True):
            t.check(
                name,
                globals={
                    "game_systems": systems(random_, field, some_missing),
                    "main_game_globals": GAME,
                    "global_structure_bsp_index": struct.pack("<h", random_.randrange(8)),
                },
                memory={GAME: Memory(random_.randbytes(GAME_SIZE))},
                mocks=procs,
            )

    # ---------- the loop

    for frames in (0, 1, 3):

        def frame(harness, arguments, frames=frames):
            # the game asks to end in one of its frames
            calls = sum(1 for call in harness._calls if call[0] == "main_loop_body")
            if calls == frames - 1:
                harness.mu.mem_write(harness.global_addresses["main_globals"] + EXIT_GAME, b"\x01")
            return 0

        t.check(
            "main_loop",
            globals={
                "main_globals": main_globals(random_, x03=1 if frames == 0 else 0),
                "game_systems": systems(random_, 1),
                "main_game_globals": GAME,
            },
            mocks=procs,
            externals={
                "main_time_initialize": nothing,
                "game_initialize": nothing,
                "main_loop_body": frame,
                "unknown_0018eb80": nothing,
            },
        )

    t.check(
        "main_time_initialize",
        globals={
            "main_time_globals": random_.randbytes(0x98),
            "d3d_device": DEVICE,
        },
        memory={DEVICE: Memory(random_.randbytes(DEVICE_SIZE), places=(0x1DB8,))},
        externals={"GetTickCount": lambda harness, arguments: 0x00ABCDEF},
    )

    # ---------- what the game asks the loop for

    for flag0 in (0, 1, 7):
        for flag1 in (0, 1):
            for switch in (0, 1):
                t.check(
                    "main_revert_map_private",
                    globals={"main_globals": main_globals(random_, x4C=flag0, x4D=flag1, x4B=switch, x47=1)},
                    externals={"game_state_revert": nothing},
                )

    for game_globals in (GAME, 0):
        for active in (0, 1):
            for initialized, paused in ((0, 0), (0, 1), (1, 0), (1, 1)):
                data = bytearray(random_.randbytes(GAME_SIZE))
                data[0x1120] = active
                t.check(
                    "main_reset_map_private",
                    globals={
                        "main_globals": main_globals(random_, x46=1),
                        "main_game_globals": game_globals,
                        "game_time_globals": TIME,
                    },
                    memory={
                        GAME: Memory(bytes(data)),
                        TIME: Memory(struct.pack("<BBhfI", initialized, paused, 30, 1 / 30, 100)),
                    },
                    externals={"unknown_0018e700": nothing},
                )

    for unknown6 in (0, 1, 5):
        for unknown22 in (0, 1):
            for blocked in (0, 1):
                data = bytearray(random_.randbytes(0x24))
                data[6] = unknown6
                data[0x22] = unknown22
                t.check(
                    "main_revert_map_if_possible_private",
                    globals={"main_globals": main_globals(random_, x4A=1, x4D=1), "unknown_00510c50": UNKNOWN},
                    memory={UNKNOWN: Memory(bytes(data))},
                    externals={"unknown_00163b60": lambda harness, arguments, blocked=blocked: blocked},
                )

    fields = [{}, {"x4B": 1}, {"x46": 1}, {"x47": 1}, {"x4A": 1}, {"x48": 1}, {"x49": 1}, {"x4E": 1}, {"x4C": 1}, {"x03": 1}]
    for field in fields:
        for unknown_byte in (b"\0", b"\x01"):
            t.check(
                "main_action_pending",
                globals={
                    "main_globals": main_globals(random_, **field),
                    "unknown_004ed39d": unknown_byte,
                    "unknown_004e6470": 0,
                    "unknown_004e64a0": 0,
                },
            )
    for first, second in ((1, 0), (0, 1), (NONE, 0), (0, NONE), (NONE, NONE), (0x7FFFFFFF, 0)):
        t.check(
            "main_action_pending",
            globals={
                "main_globals": main_globals(random_),
                "unknown_004ed39d": b"\0",
                "unknown_004e6470": first,
                "unknown_004e64a0": second,
            },
        )

    for state in (0, 1, 11, 16, 17, 19):
        for switches in (0, 2):

            def switch_to_thread(harness, arguments, switches=switches):
                calls = sum(1 for call in harness._calls if call[0] == "SwitchToThread")
                if calls == switches - 1:
                    harness.mu.mem_write(harness.global_addresses["unknown_0055c14c"], b"\x01")
                return 0

            copy_globals = bytearray(random_.randbytes(0xB40))
            struct.pack_into("<2i", copy_globals, 0x814, state, 0)
            t.check(
                "main_cancel_map_copy_and_wait",
                globals={
                    "cached_map_file_copy_globals": bytes(copy_globals),
                    "unknown_0055c14c": b"\x01" if switches == 0 else b"\0",
                },
                externals={"SwitchToThread": switch_to_thread, "main_loading_idle": nothing},
            )

    # ---------- what the game is, and how it ends

    def game_memory(**fields: int) -> Memory:
        data = bytearray(random_.randbytes(GAME_SIZE))
        struct.pack_into("<I", data, 8, fields.get("game_mode", 1))
        data[0xC] = fields.get("simulation", 1) & 0xFF
        struct.pack_into("<H", data, 0x132, fields.get("difficulty", 1) & 0xFFFF)
        data[0x134] = fields.get("unknown12C", 0)
        data[0x1120] = fields.get("active", 1)
        data[0x1121] = fields.get("lost", 0)
        data[0x1128] = fields.get("finished", 0)
        struct.pack_into("<I", data, 0x112C, fields.get("finished_ticks", 0))
        return Memory(bytes(data))

    def ticks(ticks_per_second: int) -> Memory:
        return Memory(struct.pack("<BBhfI", 1, 0, ticks_per_second, 1 / 30, 100))

    for game_globals in (GAME, 0):
        for active in (0, 1, 0x80):
            t.check(
                "game_in_progress",
                globals={"main_game_globals": game_globals},
                memory={GAME: game_memory(active=active)},
            )

    for game_mode in (1, 2, 0):
        for unknown12C in (0, 1, 7):
            t.check(
                "game_is_coop",
                globals={"main_game_globals": GAME},
                memory={GAME: game_memory(game_mode=game_mode, unknown12C=unknown12C)},
            )
        for flag in (b"\0", b"\x01"):
            for difficulty in (0, 1, 2, 3, 4, 0x8003, 0xFFFF):
                t.check(
                    "game_unknown_001388a0",
                    globals={"main_game_globals": GAME, "unknown_004f55e7": flag},
                    memory={GAME: game_memory(game_mode=game_mode, difficulty=difficulty)},
                )

    for simulation in (0, 1, 2, 3, 4, 5, 6, 7, 0x80, 0xFF):
        for name in ("game_is_distributed", "game_is_authoritative", "game_is_predicted"):
            t.check(name, globals={"main_game_globals": GAME}, memory={GAME: game_memory(simulation=simulation)})

    for lost in (0, 1, 0x100, 0xFF01):
        for already in (0, 1):
            for ticks_per_second in (30, 60, 25, 1, -1, 31):
                t.check(
                    "game_lost",
                    lost,
                    globals={"main_game_globals": GAME, "game_time_globals": TIME},
                    memory={GAME: game_memory(lost=already), TIME: ticks(ticks_per_second)},
                    externals={"game_state_note_death": nothing},
                )

    for finished in (0, 1):
        for ticks_per_second in (30, 60, 25, 1, -1, 31):
            t.check(
                "game_finish",
                globals={"main_game_globals": GAME, "game_time_globals": TIME},
                memory={GAME: game_memory(finished=finished), TIME: ticks(ticks_per_second)},
            )
        for finished_ticks in (0, 1, NONE):
            t.check(
                "game_finished_wait_over",
                globals={"main_game_globals": GAME},
                memory={GAME: game_memory(finished=finished, finished_ticks=finished_ticks)},
            )

    # ---------- the options of a game

    def random_bytes(harness, arguments):
        size, buffer = arguments
        harness.mu.mem_write(buffer, bytes(range(0x41, 0x41 + size)))
        return 0

    for refresh_rate in (60, 50, 0, -1, 59, -7, 1):
        for unknown in (b"\0", b"\x01"):
            t.check(
                "game_options_new_default",
                Buffer(random_.randbytes(OPTIONS_SIZE)),
                globals={
                    "unknown_00510c49": unknown,
                    "xbox_video_settings": struct.pack("<h6B", refresh_rate, 0, 0, 0, 0, 0, 0),
                },
                externals={"unknown_0007ad80": random_bytes},
            )

    # ---------- the ticks of a frame

    for tick_count in (0, 1, 3, 8, -1):
        for stop_at in (None, 0, 1, 2, 7):
            for seconds in (0.1, 0.05, 1.0, 0.0, 0.2666):

                def pending(harness, arguments, stop_at=stop_at):
                    calls = sum(1 for call in harness._calls if call[0] == "main_action_pending")
                    return 1 if stop_at is not None and calls >= stop_at else 0

                t.check(
                    "game_run_ticks",
                    tick_count,
                    Buffer(struct.pack("<f", seconds), format="<f"),
                    globals={"game_time_globals": TIME},
                    memory={TIME: ticks(30)},
                    externals={"main_action_pending": pending, "unknown_00137fe0": nothing},
                )
