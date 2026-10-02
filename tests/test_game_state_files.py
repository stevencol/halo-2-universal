# source/saved games/game_state.c: the states that are kept in files
#
# The file of a core is read through the functions of files_xbox.c, with the
# console's library stood in for. The files of the saved games are the
# test's: what is compared is what the game asks of them

import random
import struct

from difftest import Buffer
from test_async import calls_so_far

NONE = 0xFFFFFFFF
HEADER_SIZE = 0x1288
STATE_SIZE = 0x3FE000
OPTIONS = 0x130
STRUCTURE_BSP_INDEX = 0x1248
RESULT_SIZE = 0x10C
GAME_SIZE = 0x1200
REFERENCE_SIZE = 0x110
BASE_ADDRESS = 0x80100000


def text(harness, address: int) -> bytes:
    return bytes(harness.mu.mem_read(address, 0x500)).split(b"\0", 1)[0]


def run(t) -> None:
    random_ = random.Random(41)

    # ---------- formatted text

    for format_, argument in (
        (b"%s.bin", b"core"),
        (b"d:\\core\\%s.bin", b"name"),
        (b"%s", b"x" * 1022),
        (b"%s", b"y" * 1023),
        (b"%s", b"z" * 1100),
        (b"", b""),
    ):
        t.check("csprintf", Buffer(random_.randbytes(0x400)), Buffer(format_ + b"\0"), Buffer(argument + b"\0"))

    # ---------- the file of a core

    names = (b"core", b"", b"d:\\saved\\one", b"Z:\\a", b"c:relative", b"q/slash", b"n" * 300, b"e:\\" + b"m" * 300)
    # a letter with no colon after it, and what is next to the letters
    names += (b"ab\\x", b"@:\\x", b"[:\\x", b"`:\\x", b"{:\\x", b"a:\\x", b"z:\\x", b"A:\\x")
    for name in names:
        t.check("game_state_core_file_reference", Buffer(name + b"\0"), Buffer(random_.randbytes(REFERENCE_SIZE)))

    for name in names[:5]:
        for handle, read_result, read_count, closed in (
            (0x4321, 1, None, 1),
            (0x4321, 1, 0x20, 1),
            (0x4321, 0, 0, 1),
            (0x4321, 1, None, 0),
            (NONE, 1, None, 1),
        ):
            size = 0x40

            def read_file(harness, arguments, read_result=read_result, read_count=read_count, size=size):
                file, buffer, count, bytes_read, overlapped = arguments
                harness.mu.mem_write(bytes_read, struct.pack("<I", size if read_count is None else read_count))
                return read_result, (file, buffer, count, overlapped)

            t.check(
                "game_state_core_read",
                Buffer(name + b"\0"),
                size,
                Buffer(random_.randbytes(size)),
                externals={
                    "CreateFileA": lambda harness, arguments, handle=handle: (handle, (text(harness, arguments[0]), *arguments[1:])),
                    "ReadFile": read_file,
                    "CloseHandle": lambda harness, arguments, closed=closed: closed,
                    "GetLastError": lambda harness, arguments: 2,
                    "SetLastError": lambda harness, arguments: 0,
                },
            )

    # ---------- the file of the state of a saved game

    pending: list[int] = []

    def auxiliary(started: int, success: int, waits: int):
        """The read or the write: whether it starts, and what it leaves when it
        ends, at once or after the game waits."""

        def handler(harness, arguments):
            header, header_size, data, data_size, result, saved_game = arguments
            pending.clear()
            if started:
                # what is left is in the stack, which is not at the same place
                # in the game and in source/
                harness.mu.mem_write(result, bytes([waits == 0, 0, success]))
                if waits:
                    pending.append(result)
            return started, (header, header_size, data, data_size, saved_game)

        return handler

    def switch_to_thread(waits: int):
        def handler(harness, arguments):
            if calls_so_far(harness, "SwitchToThread") >= waits - 1:
                for result in pending:
                    harness.mu.mem_write(result, b"\x01")
            return 0

        return handler

    def file_externals(name: str) -> dict:
        waits = random_.choice((0, 0, 1, 3))
        return {
            name: auxiliary(random_.random() < 0.8, random_.choice((0, 1, 1, 0x80)), waits),
            "SwitchToThread": switch_to_thread(waits),
            "main_loading_idle": lambda harness, arguments: 0,
        }

    for _ in range(40):
        saved_game = random_.getrandbits(8)
        t.check(
            "game_state_saved_game_read_header",
            HEADER_SIZE,
            random_.choice((STATE_SIZE, 0x2000)),
            saved_game,
            Buffer(random_.randbytes(0x40)),
            externals=file_externals("saved_game_auxiliary_read"),
        )
        t.check(
            "game_state_saved_game_read_data",
            STATE_SIZE,
            BASE_ADDRESS,
            saved_game,
            externals=file_externals("saved_game_auxiliary_read"),
        )
        t.check(
            "game_state_saved_game_write",
            random_.choice((0, BASE_ADDRESS)),
            HEADER_SIZE,
            STATE_SIZE,
            saved_game,
            externals=file_externals("saved_game_auxiliary_write"),
        )

    for result in (0, 1):
        t.check(
            "game_state_saved_game_create",
            random_.getrandbits(8),
            externals={"game_state_saved_game_write": lambda harness, arguments, result=result: result},
        )
        result_buffer = Buffer(random_.randbytes(RESULT_SIZE))
        t.check(
            "game_state_saved_game_read_header_async",
            Buffer(random_.randbytes(0x40)),
            result_buffer,
            random_.getrandbits(8),
            externals={"saved_game_auxiliary_read": lambda harness, arguments, result=result: result},
        )

    # ---------- loading a state

    def game(active: int) -> Buffer:
        data = bytearray(random_.randbytes(GAME_SIZE))
        data[0x1120] = active
        return Buffer(bytes(data))

    def state_globals() -> dict:
        return {
            "main_game_globals": game(random_.random() < 0.85) if random_.random() < 0.9 else 0,
            "game_state_globals+0x2520": BASE_ADDRESS,
            "game_state_globals+0x2534": Buffer(random_.randbytes(HEADER_SIZE)),
            "game_state_globals+0x2538": b"\0",
        }

    def simulation(harness, header: int) -> int:
        """The kind of game that a header says, which the game changes before it looks at it."""
        return harness.mu.mem_read(header + OPTIONS + 4, 1)[0]

    def load_externals(header: bytes) -> dict:
        def options_match(harness, arguments):
            # the options of the game, and those of the header that was read
            options, header_options = arguments
            return random_choices["match"], (options, bytes(harness.mu.mem_read(header_options, 0x10)))

        def headers_match(harness, arguments):
            read, current = arguments
            return random_choices["headers"], (simulation(harness, read), current, simulation(harness, current))

        random_choices = {
            "match": random_.random() < 0.75,
            "headers": random_.random() < 0.8,
        }
        return {
            "game_state_header_valid": lambda harness, arguments, valid=random_.random() < 0.85: (valid, ()),
            "unknown_000593e0": lambda harness, arguments: 0,
            "game_options_match": options_match,
            "scenario_switch_structure_bsp": lambda harness, arguments, result=random_.random() < 0.8: result,
            "game_state_headers_match": headers_match,
            "game_state_call_before_load_procs": lambda harness, arguments: 0,
            "game_state_call_after_load_procs": lambda harness, arguments: 0,
        }

    for _ in range(150):
        header = random_.randbytes(HEADER_SIZE)

        def read_header(harness, arguments, header=header, read=random_.random() < 0.85):
            header_size, size, saved_game, address = arguments
            harness.mu.mem_write(address, header)
            return read, (header_size, size, saved_game)

        t.check(
            "game_state_load_saved_game",
            random_.getrandbits(8),
            globals=state_globals(),
            externals={
                **load_externals(header),
                "game_state_saved_game_read_header": read_header,
                "game_state_saved_game_read_data": lambda harness, arguments: 0,
                "game_state_save": lambda harness, arguments: 0,
            },
        )

    for _ in range(200):
        header = random_.randbytes(HEADER_SIZE)

        def core_read(harness, arguments, header=header, read=random_.random() < 0.85):
            name, size, buffer = arguments
            first = not calls_so_far(harness, "game_state_core_read")
            if first:
                harness.mu.mem_write(buffer, header)
            # the header is read into the stack; the state, where the game keeps it
            return read, (text(harness, name), size, None if first else buffer)

        def change(harness, arguments, changed=random_.random() < 0.8):
            return changed, (bytes(harness.mu.mem_read(arguments[0], 0x10)),)

        t.check(
            "game_state_load_core",
            Buffer(random_.choice((b"core", b"d:\\saved\\one")) + b"\0"),
            globals=state_globals(),
            externals={
                **load_externals(header),
                "game_state_core_read": core_read,
                "main_game_change_immediate": change,
                "main_game_goto_main_menu": lambda harness, arguments: 0,
            },
        )
