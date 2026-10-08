# source/networking/game_voice.c

import random
import struct

from difftest import Buffer, GlobalData, GlobalRange, Memory, Mock, mock_address


def run(t) -> None:
    random_ = random.Random(109)

    POOL, OTHER_POOL = 0x80100000, 0x80200000
    NONE = 0xFFFFFFFF

    def pools() -> dict:
        return {
            "game_voice_pool": POOL,
            "game_voice_menu_pool": OTHER_POOL,
            "game_voice_use_menu_pool": bytes((random_.choice((0, 1, 0x80)),)),
        }

    for n in range(120):
        given = random_.choice((0x80345000, 0))
        could = random_.choice((1, 0, 0x100))

        def allocate(harness, arguments, given=given, could=could):
            pool, reference, size, unknown4, unknown8 = arguments
            if could & 0xFF:
                harness.mu.mem_write(reference, struct.pack("<I", given))
            return could, (pool, size, unknown4, unknown8)

        t.check(
            "game_voice_allocate",
            random_.choice((0, 1, 0x200, 0x1000, 0xFFFFFFFF)),
            random_.getrandbits(32),
            globals=pools(),
            externals={"memory_pool_block_allocate": allocate},
        )

    for n in range(60):
        t.check(
            "game_voice_free",
            random_.choice((0, 0x80345000, 0x80345010)),
            random_.getrandbits(32),
            globals=pools(),
            externals={
                "memory_pool_block_free": lambda harness, arguments: (
                    0,
                    (arguments[0], struct.unpack("<I", harness.mu.mem_read(arguments[1], 4))[0]),
                ),
            },
        )

    # ---------- the pool of the voice

    VOICE_SIZE = 0x90
    VOICE_INITIALIZED = 4
    POOL_SIZE = 0x40
    FAILED = 0x80004005
    # in the memory of the console, which the emulator has
    FUNCTIONS = 0x80001000

    def voice(initialized: int) -> GlobalData:
        data = bytearray(random_.randbytes(VOICE_SIZE))
        data[VOICE_INITIALIZED] = initialized
        return GlobalData(bytes(data))

    def with_voice(result: int = 0):
        # the voice is not at the same place in the game and in source/: what
        # it has when it is called is compared
        def handler(harness, arguments):
            (address,) = arguments
            return result, (harness.normalize(address), bytes(harness.mu.mem_read(address, VOICE_SIZE)))

        return handler

    for configuration in (*range(-1, 8), NONE - 3, 0x100, 0x80000000):
        for initialized in (0, 1, 0x80):
            # the memory of the pool, with room for what is before where its blocks start
            memory = Buffer(random_.randbytes(POOL_SIZE + 0x20), places=(0x24,))

            t.check(
                "game_voice_pool_new",
                configuration & 0xFFFFFFFF,
                globals={
                    "game_voice_pool": 0x1234,
                    "game_voice_amount": random_.getrandbits(32),
                    "game_voice_kind": random_.getrandbits(32),
                    # its functions are the test's: the game's are those of
                    # the files of the players, which take the size alone
                    "game_voice_allocation": GlobalData(struct.pack("<2I", FUNCTIONS, 2)),
                    "voice_globals": voice(initialized),
                },
                memory={FUNCTIONS: Memory(struct.pack("<2I", mock_address(0), mock_address(1)))},
                mocks=[
                    Mock(lambda call, memory=memory: memory.address, stack_arguments=1, registers=("ecx",)),
                    Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                ],
                externals={
                    "voice_reset_controller_masks": with_voice(),
                    "voice_reset_controllers": with_voice(),
                    "voice_start": with_voice(random_.choice((1, 0, 0x100, 0xFFFFFF01))),
                },
                extra=[memory],
            )

    for has_pool in (True, False, True, True):
        # the pool, and after it what its memory was asked of: its functions
        # are after the pointer to them
        data = bytearray(random_.randbytes(POOL_SIZE))
        data[0x24:0x28] = struct.pack("<I", POOL_SIZE)
        pool = Buffer(
            bytes(data) + struct.pack("<3I", POOL_SIZE + 4, mock_address(0), mock_address(1)),
            pointers=(0x24, POOL_SIZE),
        )

        def stop(harness, arguments, pool=pool):
            # the pool is whole when the voice is stopped
            (address,) = arguments
            return 0, (harness.normalize(address), bytes(harness.mu.mem_read(pool.address, POOL_SIZE)))

        t.check(
            "game_voice_pool_delete",
            globals={"game_voice_pool": pool if has_pool else 0, "voice_globals": voice(random_.choice((0, 1)))},
            externals={"voice_dispose": stop},
            mocks=[
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
            extra=[] if has_pool else [pool],
        )

    # ---------- the pool of the voice and of the menu, and the voice

    GAME_SIZE = 0x1200
    GAME_MODE = 8

    def game(mode: int) -> Buffer:
        data = bytearray(random_.randbytes(GAME_SIZE))
        struct.pack_into("<I", data, GAME_MODE, mode)
        return Buffer(bytes(data))

    def voice_call(result: int = 0):
        def handler(harness, arguments):
            return result, (harness.normalize(arguments[0]), *arguments[1:])

        return handler

    for enabled in (1, 0x80, 0):
        for kind in (2, 1, 0, 3):
            for mode in (1, 2, 0, 0x101):
                for initialized in (0, 1):
                    t.check(
                        "game_voice_start",
                        globals={
                            "game_voice_enabled": bytes((enabled,)),
                            "game_voice_kind": kind,
                            "main_game_globals": game(mode),
                            "voice_globals": voice(initialized),
                        },
                        externals={"voice_initialize": voice_call()},
                    )

    for enabled in (1, 0x80, 0):
        for initialized in (1, 0x80, 0):
            t.check(
                "game_voice_stop",
                globals={
                    "game_voice_enabled": bytes((enabled,)),
                    "voice_globals": voice(initialized),
                    "unknown_004c9966": struct.pack("<H", random_.getrandbits(16) | 1),
                },
                externals={"voice_dispose": voice_call()},
            )

    def allocation() -> Buffer:
        # an allocation: its functions are after the pointer to them
        return Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,))

    for amount in (1, 1, 1, 0, 2, 0xFFFFFFFF):
        for own in (True, False):
            for give in (True, True, False):
                # the memory of the pool, with room for what is before where its blocks start
                memory = Buffer(random_.randbytes(POOL_SIZE + 0x20))
                t.check(
                    "game_voice_menu_pool_new",
                    globals={
                        "game_voice_amount": amount,
                        "game_voice_menu_pool": 0x1234,
                        "game_voice_use_menu_pool": bytes((random_.choice((0, 0x80)),)),
                        "unknown_0046dd54": allocation() if own else 0,
                        "heap_allocation": allocation(),
                    },
                    mocks=[
                        Mock(lambda call, memory=memory, give=give: memory.address if give else 0, stack_arguments=1, registers=("ecx",)),
                        Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                    ],
                    externals={"game_voice_start": lambda harness, arguments: 0},
                    extra=[memory],
                )

    for amount in (1, 1, 0, 2):
        for has_pool in (True, False):
            for enabled in (1, 0):
                for initialized in (1, 0):
                    data = bytearray(random_.randbytes(POOL_SIZE))
                    data[0x24:0x28] = struct.pack("<I", POOL_SIZE)
                    pool = Buffer(
                        bytes(data) + struct.pack("<3I", POOL_SIZE + 4, mock_address(0), mock_address(1)),
                        pointers=(0x24, POOL_SIZE),
                    )

                    def stop(harness, arguments, pool=pool):
                        # the pool is whole when the voice is stopped
                        (address,) = arguments
                        return 0, (harness.normalize(address), bytes(harness.mu.mem_read(pool.address, POOL_SIZE)))

                    t.check(
                        "game_voice_menu_pool_delete",
                        globals={
                            "game_voice_amount": amount,
                            "game_voice_menu_pool": pool if has_pool else 0,
                            "game_voice_use_menu_pool": bytes((random_.choice((1, 0x80)),)),
                            "game_voice_enabled": bytes((enabled,)),
                            "voice_globals": voice(initialized),
                            "unknown_004c9966": struct.pack("<H", random_.getrandbits(16) | 1),
                        },
                        externals={"voice_dispose": stop},
                        mocks=[
                            Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                            Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                        ],
                        extra=[] if has_pool else [pool],
                    )

    # ---------- small things of the voice

    ENGINE = 0x80400000
    VOICE_MODES_OFFSET = 0x54
    HEADSET = 0x64
    MESSAGE_BUSY = 0x88

    def active():
        # the game has to do with the voice and the voice is started; then, what lacks one or the other
        yield 1, 1
        yield 0x80, 0x80
        yield 0, 1
        yield 1, 0
        yield 1, 1

    def a_voice(initialized: int, modes: tuple = (), headsets: tuple = ()) -> GlobalData:
        data = bytearray(random_.randbytes(VOICE_SIZE))
        data[VOICE_INITIALIZED] = initialized
        struct.pack_into("<I", data, 0x10, ENGINE)
        for controller, mode in enumerate(modes):
            struct.pack_into("<I", data, VOICE_MODES_OFFSET + controller * 4, mode)
        for controller, headset in enumerate(headsets):
            data[HEADSET + controller] = headset
        return GlobalData(bytes(data))

    def state(enabled: int, initialized: int, **voice_options) -> dict:
        return {"game_voice_enabled": bytes((enabled,)), "voice_globals": a_voice(initialized, **voice_options)}

    identifiers = (0, 1, 0x80000000, 0xFFFFFFFF, 0x12345678)

    for enabled, initialized in active():
        for found in (1, 0, 0x100, 0xFFFFFF01):
            t.check(
                "game_voice_has_remote_talker",
                random_.choice(identifiers),
                globals=state(enabled, initialized),
                externals={"voice_has_remote_talker": voice_call(found)},
            )

        for controller in range(4):
            for mode in range(4):
                t.check(
                    "game_voice_set_controller_mode",
                    controller,
                    mode,
                    globals={
                        **state(enabled, initialized),
                        "voice_modes": struct.pack("<4I", *(0x00340000 + random_.getrandbits(16) for _ in range(4))),
                    },
                    externals={
                        "unknown_00332da8": lambda harness, arguments, result=random_.choice((0, 1, FAILED)): result
                    },
                )

            for mask in (0, 1):
                t.check(
                    "game_voice_set_controller_mask",
                    mask,
                    controller,
                    globals={**state(enabled, initialized), "game_voice_masks": GlobalRange(0x20)},
                    externals={
                        "voice_set_controller_mask": lambda harness, arguments: (
                            0,
                            (
                                harness.normalize(arguments[0]),
                                arguments[1],
                                harness.normalize(arguments[2]),
                                bytes(harness.mu.mem_read(arguments[2], 0x10)),
                            ),
                        )
                    },
                )

            t.check("game_voice_message_busy", controller, globals=state(enabled, initialized))

            for mode in (3, 2, 0x103):
                for headset in (1, 0, 0x80):
                    modes = tuple(mode if n == controller else random_.randint(0, 3) for n in range(4))
                    headsets = tuple(headset if n == controller else random_.getrandbits(1) for n in range(4))
                    for name in ("game_voice_stop_message", "game_voice_stop_message_with_headset"):
                        t.check(
                            name,
                            controller,
                            globals=state(enabled, initialized, modes=modes, headsets=headsets),
                            externals={"unknown_00332e93": lambda harness, arguments: FAILED},
                        )

            for setting in (0, 1, 2, 3):
                for with_headset in (1, 0):
                    for message, size in ((0x80345000, 0x20), (0, 0x20), (0x80345000, 0), (0x80345000, 0x80000000), (0x80345000, 1)):
                        t.check(
                            "game_voice_play_message",
                            controller,
                            message,
                            size,
                            globals={
                                **state(enabled, initialized),
                                "game_voice_controller_settings": struct.pack(
                                    "<4I", *(setting if n == controller else random_.randint(0, 3) for n in range(4))
                                ),
                            },
                            externals={
                                "unknown_000536b0": lambda harness, arguments, with_headset=with_headset: with_headset,
                                "voice_play_message": lambda harness, arguments: (
                                    0,
                                    (arguments[0], arguments[1] & 0xFF, arguments[2], arguments[3], harness.normalize(arguments[4])),
                                ),
                            },
                        )

            t.check(
                "game_voice_record_message",
                controller,
                0x80345000,
                0x80346000,
                random_.getrandbits(32),
                random_.getrandbits(16),
                0x80347000,
                globals=state(enabled, initialized),
                externals={
                    "voice_record_message": lambda harness, arguments: (
                        0,
                        (arguments[0], arguments[1], harness.normalize(arguments[2]), *arguments[3:]),
                    )
                },
            )

        for player in (0, 1, 7, 15):
            for other in (0, 3, 15):
                for bits in (0, 0xFFFF, 1 << other, 0xFFFF ^ (1 << other)):
                    words = [random_.getrandbits(16) for _ in range(16)]
                    words[player] = bits
                    t.check(
                        "game_voice_unknown_00053850",
                        player,
                        other,
                        globals={**state(enabled, initialized), "unknown_004c9968": struct.pack("<16H", *words)},
                    )

        for unknown in (1, 0, 2, 0x100):
            for found in (1, 0):
                t.check(
                    "game_voice_remote_talker_data",
                    random_.choice(identifiers),
                    0x80345000,
                    random_.getrandbits(8),
                    globals={**state(enabled, initialized), "unknown_004c9888": unknown},
                    externals={
                        "game_voice_has_remote_talker": lambda harness, arguments, found=found: found,
                        "unknown_00053f50": lambda harness, arguments: 0,
                        "unknown_00053f80": lambda harness, arguments: 0,
                        "voice_remote_talker_data": lambda harness, arguments: (
                            0,
                            (arguments[0], arguments[1], harness.normalize(arguments[2]), arguments[3]),
                        ),
                    },
                )

    for initialized in (1, 0x80, 0):
        for controller in range(4):
            t.check(
                "game_voice_get_controller_mode",
                controller,
                globals={"voice_globals": a_voice(initialized, modes=tuple(random_.getrandbits(32) for _ in range(4)))},
            )
