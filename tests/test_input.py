# source/input/input_xbox.c
#
# The gamepads are the test's: the functions of the console's library are
# stood in for, and say what is plugged in and what each gamepad has pressed

import random
import struct

from difftest import Buffer, GlobalData, GlobalRange
from test_async import calls_so_far

NONE = 0xFFFFFFFF
GLOBALS_SIZE = 0x1B0
SUPPRESSED = 0x1
RUMBLE_DISABLED = 0x2
UPDATE_TIME = 0x4
UNIT_MASK = 0x8
HANDLES = 0x14
STATES = 0x24
STATE_SIZE = 0x48
TYPES = 0x144
RUMBLES = 0x19C
FEEDBACK_SIZE = 0x46
XSTATE_SIZE = 22
PROFILE_SIZE = 0xC70
GAME_SIZE = 0x1200

DEVICE_TYPES = {
    "xdevice_type_gamepad": GlobalRange(4),
    "xdevice_type_voice_microphone": GlobalRange(4),
    "xdevice_type_voice_headphone": GlobalRange(4),
    "xdevice_type_memory_unit": GlobalRange(4),
}

THUMBSTICKS = (0, 1, -1, 8999, 9000, 9001, 9002, -8999, -9000, -9001, -9002, 20000, -20000, 32767, -32767, -32768)


def run(t) -> None:
    random_ = random.Random(42)

    def input_globals(handles: tuple[int, ...] | None = None, **fields: int) -> GlobalData:
        data = bytearray(random_.randbytes(GLOBALS_SIZE))
        data[0] = fields.get("initialized", 1)
        data[SUPPRESSED] = fields.get("suppressed", 0)
        data[RUMBLE_DISABLED] = fields.get("rumble_disabled", 0)
        struct.pack_into("<I", data, UPDATE_TIME, fields.get("update_time", 5000))
        if handles is None:
            handles = tuple(random_.choice((0, 0x100 + n)) for n in range(4))
        struct.pack_into("<4I", data, HANDLES, *handles)
        return GlobalData(bytes(data))

    def some_handles() -> tuple[int, ...]:
        return tuple(random_.choice((0, 0x100 + n, 0x100 + n)) for n in range(4))

    # ---------- the start and the end

    def init_devices(harness, arguments):
        count, address = arguments
        pairs = struct.unpack("<6I", harness.mu.mem_read(address, 24))
        return 0, (count, *(harness.normalize(pairs[n]) if n % 2 == 0 else pairs[n] for n in range(6)))

    for _ in range(3):
        t.check(
            "input_initialize",
            globals={"input_globals": input_globals(), **DEVICE_TYPES},
            externals={"XInitDevices": init_devices, "input_update": lambda harness, arguments: 0},
        )

    for _ in range(8):
        t.check(
            "input_dispose",
            globals={"input_globals": input_globals()},
            externals={"XInputClose": lambda harness, arguments: 0},
        )

    for initialized in (1, 1, 1, 0):
        for now in (5000, 5001, 5016, 5099, 5100, 5101, 9000, 4999, 0, 0x80000000, NONE):
            t.check(
                "input_update",
                globals={"input_globals": input_globals(initialized=initialized, suppressed=random_.getrandbits(1))},
                externals={
                    "GetTickCount": lambda harness, arguments, now=now: now,
                    "input_update_devices": lambda harness, arguments: 0,
                    "input_update_gamepads": lambda harness, arguments: 0,
                    "input_flush_rumble": lambda harness, arguments: 0,
                },
            )

    # ---------- what the game asks

    for suppressed in (0, 1):
        for _ in range(3):
            globals_ = {"input_globals": input_globals(suppressed=suppressed)}
            for index in range(4):
                t.check("input_get_gamepad_state", index, globals=globals_)
                t.check("input_get_gamepad_type", index, globals=globals_)

    for mask in (0, 1, 3, 0x5A5, 0xFFFF, 0x8000):
        data = bytearray(input_globals().data)
        struct.pack_into("<H", data, UNIT_MASK, mask)
        for index in (0, 1, 2, 5, 9, 15):
            # the letters of the high drives are where the mask is: they start
            # right after its first byte
            if index + 1 < 12:
                t.check(
                    "input_get_unit_drive_letter",
                    index,
                    Buffer(random_.randbytes(1)),
                    globals={"input_globals": GlobalData(bytes(data))},
                )

    def profiles() -> bytes:
        data = bytearray(random_.randbytes(4 * PROFILE_SIZE))
        for index in range(4):
            struct.pack_into("<I", data, index * PROFILE_SIZE, random_.choice((0, 0x10, 0x10, 0xFFFFFFEF, NONE)))
            struct.pack_into("<I", data, index * PROFILE_SIZE + 0x114, random_.choice((0, 2, 2, 0xFFFFFFFD, NONE)))
        return bytes(data)

    for _ in range(40):
        t.check(
            "input_set_gamepad_rumbler_state",
            random_.randrange(4),
            random_.choice((0, 0x1234, 0xFFFF, 0x12345678)),
            random_.choice((0, 0x4321, 0xFFFF, 0x87654321)),
            globals={"input_globals": input_globals(), "unknown_0054e8e0": profiles()},
        )

    # ---------- what is plugged in

    for _ in range(200):
        changed = random_.random() < 0.85
        insertions = random_.getrandbits(4) | (random_.getrandbits(32) & ~0xF if random_.random() < 0.2 else 0)
        removals = random_.getrandbits(4) | (random_.getrandbits(32) & ~0xF if random_.random() < 0.2 else 0)
        memory_units = random_.random() < 0.5

        def device_changes(harness, arguments, changed=changed, insertions=insertions, removals=removals, memory_units=memory_units):
            kind, insertions_address, removals_address = arguments
            first = not calls_so_far(harness, "XGetDeviceChanges")
            if first and changed:
                harness.mu.mem_write(insertions_address, struct.pack("<I", insertions))
                harness.mu.mem_write(removals_address, struct.pack("<I", removals))
            return (changed if first else memory_units), (harness.normalize(kind),)

        def capabilities(harness, arguments, results=tuple(random_.choice((0, 0, 0, 5)) for _ in range(4)), kinds=tuple(random_.choice((0, 1, 2, 3)) for _ in range(4))):
            device, address = arguments
            n = calls_so_far(harness, "XInputGetCapabilities")
            harness.mu.mem_write(address, bytes([kinds[n]]))
            return results[n], (device,)

        t.check(
            "input_update_devices",
            globals={"input_globals": input_globals(), **DEVICE_TYPES},
            externals={
                "XGetDeviceChanges": device_changes,
                "XInputClose": lambda harness, arguments: 0,
                "XInputOpen": lambda harness, arguments, opened=tuple(random_.choice((0, 0x200, 0x300)) for _ in range(4)): (
                    opened[arguments[1] & 3] and opened[arguments[1] & 3] + arguments[1],
                    (harness.normalize(arguments[0]), *arguments[1:]),
                ),
                "XInputGetCapabilities": capabilities,
                "GetTickCount": lambda harness, arguments: 0x7777,
                "unknown_002184a0": lambda harness, arguments: 0,
            },
        )

    # ---------- what the gamepads say

    for _ in range(400):
        handles = some_handles()
        states = []
        for _n in range(4):
            analog = bytes(random_.choice((0, 1, 0x1F, 0x20, 0x21, 0x3F, 0x40, 0x80, 0xBF, 0xC0, 0xFF, random_.getrandbits(8))) for _b in range(8))
            states.append(
                struct.pack("<IH8s4h", random_.getrandbits(32), random_.getrandbits(16), analog, *(random_.choice(THUMBSTICKS) for _a in range(4)))
            )
        data = bytearray(input_globals(handles).data)
        for index in range(4):
            state = STATES + index * STATE_SIZE
            # how long the buttons have been down: about to reach the most
            for at in range(0x10, 0x20):
                data[state + at] = random_.choice((0, 1, 0xFE, 0xFF))
            for at in range(0x20, 0x40, 2):
                struct.pack_into("<H", data, state + at, random_.choice((0, 5, 0xFFF0, 0xFFFF)))
            for at in range(8):
                data[state + at] = random_.choice((0, 0x30, 0x80, 0xFF))
                data[state + 8 + at] = random_.choice((0, 0x30, 0x7F, 0x80, 0xFF))

        def get_state(harness, arguments, states=states, results=tuple(random_.choice((0, 0, 0, 0x48F, 0x8007048F)) for _ in range(4))):
            device, address = arguments
            harness.mu.mem_write(address, states[device & 3])
            return results[device & 3], (device,)

        t.check(
            "input_update_gamepads",
            random_.choice((0, 16, 17, 100, NONE - 4, 70000)),
            globals={"input_globals": GlobalData(bytes(data))},
            externals={"XInputGetState": get_state},
        )

    # ---------- the rumble

    def set_state(harness, arguments):
        device, feedback = arguments
        speeds = struct.unpack("<2H", harness.mu.mem_read(feedback + 0x42, 4))
        return 0, (device, harness.normalize(feedback), *speeds)

    for _ in range(200):
        feedbacks = bytearray(random_.randbytes(4 * FEEDBACK_SIZE))
        for index in range(4):
            struct.pack_into("<I", feedbacks, index * FEEDBACK_SIZE, random_.choice((0, 0, 0x3E5, 5)))
        game = bytearray(random_.randbytes(GAME_SIZE))
        game[0x1120] = random_.random() < 0.8
        time = bytearray(random_.randbytes(12))
        time[0] = random_.random() < 0.7
        time[1] = random_.random() < 0.4
        t.check(
            "input_flush_rumble",
            globals={
                "input_globals": input_globals(suppressed=random_.random() < 0.2, rumble_disabled=random_.random() < 0.2),
                "input_feedbacks": GlobalData(bytes(feedbacks)),
                "main_game_globals": Buffer(bytes(game)) if random_.random() < 0.9 else 0,
                "game_time_globals": Buffer(bytes(time)),
            },
            externals={"XInputSetState": set_state},
        )
