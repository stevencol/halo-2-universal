# source/networking/voice.c

import random
import struct

from difftest import Buffer
from test_async import calls_so_far

VOICE_SIZE = 0x90
VOICE_KIND = 8
PARAMETERS_SIZE = 0x24
VOICE_INITIALIZED = 4
VOICE_MODES = 0xC
VOICE_CONTROLLER_MODES = 0x54
STATUS_SIZE = 0xC
FAILED = 0x80004005


def run(t) -> None:
    random_ = random.Random(263)

    def voice(kind: int | None = None) -> Buffer:
        data = bytearray(random_.randbytes(VOICE_SIZE))
        if kind is not None:
            struct.pack_into("<I", data, VOICE_KIND, kind & 0xFFFFFFFF)
        return Buffer(bytes(data))

    for _ in range(4):
        t.check("voice_reset_controller_levels", voice())

    for kind in (0, 1, 2, 3, -1, 0x101):
        t.check("voice_reset_controllers", voice(kind))

    for kind in (0, 1, 2, 3, -1, 0x102):
        for unknown in (0x80345000, 0, 1):
            t.check(
                "voice_engine_parameters_new",
                Buffer(random_.randbytes(PARAMETERS_SIZE)),
                voice(kind),
                globals={"unknown_00510c90": Buffer(struct.pack("<I", unknown))},
            )

    # ---------- starting and stopping

    def modes() -> bytes:
        return struct.pack("<4I", *(0x00340000 + random_.getrandbits(16) for _ in range(4)))

    def library(results: dict, engine: int, statuses: list[int]) -> dict:
        """The console's library: each function says what the case gives it for
        each call, the last thing for the calls after."""

        def result(harness, name):
            given = results.get(name, (0,))
            return given[min(calls_so_far(harness, name), len(given) - 1)]

        def create(harness, arguments):
            # what it is asked with is in the stack, which is not at the same
            # place in the game and in source/
            parameters, place = arguments
            value = result(harness, "unknown_00333055")
            harness.mu.mem_write(place, struct.pack("<I", engine))
            return value, (bytes(harness.mu.mem_read(parameters, PARAMETERS_SIZE)), place)

        def status(harness, arguments):
            engine_, controller, place = arguments
            value = result(harness, "unknown_00332db2")
            first = statuses[calls_so_far(harness, "unknown_00332db2") % len(statuses)]
            harness.mu.mem_write(place, struct.pack("<I", first) + bytes(STATUS_SIZE - 4))
            return value, (engine_, controller)

        def plain(name):
            return lambda harness, arguments: result(harness, name)

        return {
            "unknown_00333055": create,
            "unknown_00332db2": status,
            **{
                name: plain(name)
                for name in (
                    "unknown_00332d8f",
                    "unknown_00332d94",
                    "unknown_00332e7c",
                    "unknown_00332da3",
                    "unknown_00332da8",
                    "unknown_00332f27",
                    "unknown_003330f8",
                    "unknown_000552a0",
                )
            },
        }

    def start_cases():
        # all goes well, and each thing that the library can say that it could not do
        yield {}
        yield {"unknown_00333055": (FAILED,)}
        yield {"unknown_00333055": (1,)}
        yield {"unknown_00333055": (FAILED,), "unknown_00332d8f": (0,)}
        for at in range(4):
            yield {"unknown_00332d8f": (*(0,) * at, FAILED, 0)}
            yield {"unknown_00332d8f": (*(0,) * at, FAILED)}
        yield {"unknown_00332d8f": (FAILED, FAILED, FAILED, 7)}
        yield {"unknown_00332d94": (FAILED,)}
        yield {"unknown_00332d94": (0x7FFFFFFF,), "unknown_00332e7c": (FAILED,)}
        yield {"unknown_00332da3": (FAILED,), "unknown_00332da8": (FAILED,), "unknown_00332db2": (FAILED,)}

    for results in start_cases():
        for kind in (2, 1, 0, 5):
            for unknown in (0x80345000, 0x80345000, 0):
                # each controller has one of the modes
                started = bytearray(voice(kind).data)
                struct.pack_into("<4I", started, VOICE_CONTROLLER_MODES, *(random_.randint(0, 3) for _ in range(4)))
                t.check(
                    "voice_start",
                    Buffer(bytes(started)),
                    globals={
                        "unknown_00510c90": Buffer(struct.pack("<I", unknown)),
                        "voice_modes": modes(),
                    },
                    externals=library(results, 0x80400000 + random_.getrandbits(16), [random_.choice((0, 0, 1, 0x100)) for _ in range(4)]),
                )

    def with_voice(result: int = 0):
        def handler(harness, arguments):
            (address,) = arguments
            return result, (address, bytes(harness.mu.mem_read(address, VOICE_SIZE)))

        return handler

    for initialized in (0, 0, 1, 0x80):
        for started in (1, 0, 0x100, 0xFFFFFF01):
            buffer = voice()
            data = bytearray(buffer.data)
            data[VOICE_INITIALIZED] = initialized
            t.check(
                "voice_initialize",
                Buffer(bytes(data)),
                random_.choice((0, 1, 2, 0x12345678)),
                externals={
                    "voice_reset_controller_levels": with_voice(),
                    "voice_reset_controllers": with_voice(),
                    "voice_start": with_voice(started),
                },
            )

    for initialized in (1, 1, 0x80, 0):
        for flags in (0xF, 0xB, 0, 4, 0xFFFFFFFB):
            for engine in (0x80400000, 0):
                data = bytearray(random_.randbytes(VOICE_SIZE))
                data[VOICE_INITIALIZED] = initialized
                struct.pack_into("<2I", data, VOICE_MODES, flags, engine)

                def before_stop(harness, arguments):
                    (address,) = arguments
                    return 0, (address, bytes(harness.mu.mem_read(address, VOICE_SIZE)))

                t.check(
                    "voice_dispose",
                    Buffer(bytes(data)),
                    externals={**library({}, 0, [0]), "unknown_000552a0": before_stop},
                )
