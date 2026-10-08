# source/networking/game_voice.c

import random
import struct

from difftest import Buffer, GlobalData, Memory, Mock, mock_address


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
                    "voice_reset_controller_levels": with_voice(),
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
