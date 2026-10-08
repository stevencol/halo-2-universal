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
VOICE_ENGINE = 0x10
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
                    "voice_remove_remote_talkers",
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
                    externals={**library({}, 0, [0]), "voice_remove_remote_talkers": before_stop},
                )

    # ---------- the players of other consoles

    ENGINE = 0x80400000
    TALKER_SIZE = 0xC
    LEVELS = 0x14

    def a_voice(initialized: int, flags: int) -> Buffer:
        data = bytearray(random_.randbytes(VOICE_SIZE))
        data[VOICE_INITIALIZED] = initialized
        struct.pack_into("<2I", data, VOICE_MODES, flags, ENGINE)
        return Buffer(bytes(data))

    def states():
        # started and with the mode of the players of other consoles; then, what lacks one or the other
        yield 1, 0xF
        yield 0x80, 4
        yield 1, 0xFFFFFFFB
        yield 0, 0xF
        yield 1, 0xF

    def talkers_list(talkers: list[tuple[int, int, int]], result: int = 0):
        def handler(harness, arguments):
            # the count and the talkers go to the stack, which is not at the
            # same place in the game and in source/
            engine, count, place = arguments
            harness.mu.mem_write(count, struct.pack("<I", len(talkers)))
            harness.mu.mem_write(place, b"".join(struct.pack("<3I", *talker) for talker in talkers))
            return result, (engine,)

        return handler

    def some_talkers(identifier: int) -> list[tuple[int, int, int]]:
        """Up to as many as the engine has room for; one of them can be the one looked for, or nearly."""
        low = identifier & 0xFFFFFFFF
        high = 0xFFFFFFFF if low & 0x80000000 else 0
        near = [
            (low, high, 0),
            (low, high, 0xFFFFFFFC),
            (low, high, 1),
            (low, high, 2),
            (low, high ^ 1, 0),
            (low ^ 0x100, high, 0),
            (low, high ^ 0xFFFFFFFF, 0),
        ]
        talkers = [
            (random_.getrandbits(32), random_.choice((0, 0xFFFFFFFF, 0x00090000)), random_.getrandbits(3))
            for _ in range(random_.choice((0, 1, 3, 29, 30)))
        ]
        if talkers and random_.getrandbits(2):
            talkers[random_.randrange(len(talkers))] = random_.choice(near)
        return talkers

    identifiers = (0, 1, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x12345678, 0xC0000005)

    for initialized, flags in states():
        for n in range(24):
            identifier = random_.choice(identifiers)
            t.check(
                "voice_has_remote_talker",
                a_voice(initialized, flags),
                identifier,
                externals={
                    "unknown_00332def": talkers_list(some_talkers(identifier), random_.choice((0, 0, 0, 1, FAILED)))
                },
            )

    for initialized, flags in states():
        for result in (0, 1, FAILED, 0x100):
            t.check(
                "voice_remote_talker_unknown_000551c0",
                a_voice(initialized, flags),
                random_.choice(identifiers),
                externals={"unknown_00332e18": lambda harness, arguments, result=result: result},
            )

    for initialized in (1, 0x80, 0, 1):
        for controller in range(4):
            for result in (0, 1, FAILED):
                given = random_.randbytes(0x10)

                def levels(harness, arguments, result=result, given=given):
                    harness.mu.mem_write(arguments[2], given)
                    return result

                t.check(
                    "voice_update_controller_levels",
                    a_voice(initialized, random_.getrandbits(32)),
                    controller,
                    Buffer(random_.randbytes(0x10)),
                    externals={"unknown_00332dad": levels},
                )

    for initialized in (1, 0x80, 0, 1):
        for identifier in identifiers:
            t.check(
                "voice_remote_talker_unknown_00055250",
                a_voice(initialized, random_.getrandbits(32)),
                identifier,
                random_.getrandbits(32),
                0x80345000 + random_.getrandbits(8),
                externals={"unknown_00332e34": lambda harness, arguments: 0},
            )

    for initialized, flags in states():
        for n in range(8):
            t.check(
                "voice_remove_remote_talkers",
                a_voice(initialized, flags),
                externals={
                    "unknown_00332def": talkers_list(
                        some_talkers(random_.choice(identifiers)), random_.choice((0, 0, 0, 1, FAILED))
                    ),
                    "unknown_00332dd3": lambda harness, arguments: FAILED,
                },
            )

    for initialized, flags in states():
        for has_voice in (1, 0, 0x100, 0x80):
            for result in (0, 1, FAILED):
                t.check(
                    "voice_remote_talker_set",
                    a_voice(initialized, flags),
                    random_.choice(identifiers),
                    has_voice,
                    externals={
                        "unknown_00332db7": lambda harness, arguments, result=result: result,
                        "unknown_00332dd3": lambda harness, arguments, result=result: result,
                    },
                )

    # ---------- the messages

    HEADSET = 0x64
    MESSAGE_SIZE = 0x68
    MESSAGE_LENGTH = 0x78
    MESSAGE_BUSY = 0x88
    TALKED = 0x8C

    def controller_voice(initialized: int, controller: int, mode: int, headset: int) -> bytearray:
        data = bytearray(random_.randbytes(VOICE_SIZE))
        data[VOICE_INITIALIZED] = initialized
        struct.pack_into("<I", data, VOICE_ENGINE, ENGINE)
        struct.pack_into("<I", data, VOICE_CONTROLLER_MODES + controller * 4, mode)
        data[HEADSET + controller] = headset
        return data

    def sound_file(file: bytes = b"RIFF", form: bytes = b"WAVE", format_: bytes = b"fmt ") -> Buffer:
        return Buffer(file + random_.randbytes(4) + form + format_ + random_.randbytes(0x10))

    def message_cases():
        # (started, mode, headset, size, the message, without a headset)
        yield 1, 3, 1, 0x20, sound_file(), 0
        yield 0x80, 3, 0, 0x11, sound_file(), 1
        yield 1, 3, 0x80, 0x7FFFFFFF, sound_file(), 0x80
        yield 1, 3, 0, 0x20, sound_file(), 0
        yield 0, 3, 1, 0x20, sound_file(), 1
        yield 1, 2, 1, 0x20, sound_file(), 1
        yield 1, 0x103, 1, 0x20, sound_file(), 1
        yield 1, 3, 1, 0x10, sound_file(), 1
        yield 1, 3, 1, 0, sound_file(), 1
        yield 1, 3, 1, 0x80000020, sound_file(), 1
        yield 1, 3, 1, 0x20, sound_file(file=b"RIFX"), 1
        yield 1, 3, 1, 0x20, sound_file(form=b"WAVF"), 1
        yield 1, 3, 1, 0x20, sound_file(format_=b"fmt\0"), 1
        yield 1, 3, 1, 0x20, sound_file(file=b"\0IFF"), 1

    for started, mode, headset, size, message, unknown in message_cases():
        for controller in range(4):
            for result in (0, 1, FAILED):
                t.check(
                    "voice_play_message",
                    message,
                    unknown,
                    size,
                    controller,
                    Buffer(bytes(controller_voice(started, controller, mode, headset))),
                    externals={"unknown_00332e8a": lambda harness, arguments, result=result: result},
                )

    for started, mode, headset in ((1, 3, 1), (0x80, 3, 0x80), (0, 3, 1), (1, 2, 1), (1, 3, 0), (1, 0x103, 1)):
        for controller in range(4):
            for result in (0, 1, FAILED):
                for buffer_size in (0, 1, 3, 4, 0x25, 0x40):
                    # what the library records is the test's
                    recorded = random_.randbytes(buffer_size)

                    def record(harness, arguments, result=result, recorded=recorded):
                        harness.mu.mem_write(arguments[4], recorded)
                        return result

                    # the two places are kept in the voice: all in one buffer,
                    # so that they are at the same address in both
                    voice_data = controller_voice(started, controller, mode, headset)
                    for offset in (*range(MESSAGE_SIZE, MESSAGE_BUSY, 4),):
                        struct.pack_into("<I", voice_data, offset, random_.getrandbits(32))
                    t.check(
                        "voice_record_message",
                        buffer_size,
                        controller,
                        Buffer(bytes(voice_data)),
                        random_.getrandbits(32),
                        Buffer(random_.randbytes(0x48)),
                        Buffer(random_.randbytes(4)),
                        Buffer(random_.randbytes(4)),
                        externals={"unknown_00332e81": record},
                    )

    # ---------- what the library tells the game

    for controller in range(4):
        for change in (0, 1, 2, 0xFFFFFFFF, 0x100):
            t.check("voice_callback_headset_changed", voice(), controller, change)

        for size in (0, 1, 0x80000000, 0x40):
            t.check(
                "voice_callback_data_ready",
                voice(),
                controller,
                size,
                0x80345000 + random_.getrandbits(8),
                externals={"unknown_00053a20": lambda harness, arguments: 0},
            )

        for wanted in ((True, True), (True, False), (False, True), (False, False)):
            # the voice, and after it the two places
            data = bytearray(voice().data) + random_.randbytes(8)
            struct.pack_into("<I", data, MESSAGE_SIZE + controller * 4, VOICE_SIZE if wanted[0] else 0)
            struct.pack_into("<I", data, MESSAGE_LENGTH + controller * 4, VOICE_SIZE + 4 if wanted[1] else 0)
            pointers = tuple(
                offset + controller * 4 for offset, is_wanted in zip((MESSAGE_SIZE, MESSAGE_LENGTH), wanted) if is_wanted
            )
            t.check(
                "voice_callback_message_recorded",
                Buffer(bytes(data), pointers=pointers),
                controller,
                random_.getrandbits(32),
                random_.getrandbits(32),
            )

        t.check("voice_callback_message_stopped", voice(), controller)

    for _ in range(3):
        t.check("voice_callback_unknownC", voice(), *(random_.getrandbits(32) for _ in range(4)))
        t.check("voice_callback_unknown8", voice(), *(random_.getrandbits(32) for _ in range(3)))
