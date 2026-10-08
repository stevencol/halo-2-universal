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
                    "game_voice_talkers": struct.pack("<H", random_.getrandbits(16) | 1),
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
                            "game_voice_talkers": struct.pack("<H", random_.getrandbits(16) | 1),
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
                                "game_voice_controller_can_talk": lambda harness, arguments, with_headset=with_headset: with_headset,
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
                        "game_voice_player_has_muted",
                        player,
                        other,
                        globals={**state(enabled, initialized), "game_voice_player_mutes": struct.pack("<16H", *words)},
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
                        "game_voice_library_begin": lambda harness, arguments: 0,
                        "game_voice_library_end": lambda harness, arguments: 0,
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

    # ---------- the players and the session

    SESSION_SIZE = 0x7420
    MEMBERSHIP = 0x4C
    MEMBERSHIP_PLAYERS = 0x10D0

    def flags_of(*values: int):
        """What game_voice_player_flags says, call after call; the last thing for the calls after."""

        def handler(harness, arguments):
            calls = sum(1 for call in harness._calls if call[0] == "game_voice_player_flags")
            return values[min(calls, len(values) - 1)]

        return handler

    for enabled, initialized in active():
        for unknown in (1, 0, 0x100):
            t.check(
                "game_voice_build_packet",
                random_.getrandbits(32),
                random_.getrandbits(32),
                random_.getrandbits(32),
                globals={
                    **state(enabled, initialized),
                    "unknown_004c9888": unknown,
                    "unknown_00525a00": GlobalRange(0x1704),
                },
                externals={
                    "voice_queues_build_packet": lambda harness, arguments, result=random_.getrandbits(32): (
                        result,
                        (arguments[0], harness.normalize(arguments[1]), arguments[2], arguments[3]),
                    )
                },
            )

        for player in (0, 5, 15):
            for values in ((1,), (3,), (0,), (2,), (0xFFFD,), (0x10001, 0x10000), (1, 2), (5, 0xFFFFFFFD), (0xFFFE, 0)):
                t.check(
                    "game_voice_player_can_be_heard",
                    player,
                    globals=state(enabled, initialized),
                    externals={"game_voice_player_flags": flags_of(*values)},
                )
            for value in (0, 1, 2, 4, 0xFFFD, 0xFFFB, 0x10006):
                for name in ("game_voice_player_voice_off", "game_voice_player_not_allowed"):
                    t.check(
                        name,
                        player,
                        globals=state(enabled, initialized),
                        externals={"game_voice_player_flags": flags_of(value)},
                    )

            for has_membership in (True, False):
                for players in (1 << player, 0xFFFFFFFF ^ (1 << player), 0xFFFFFFFF, 0):
                    membership = bytearray(random_.randbytes(MEMBERSHIP_PLAYERS + 4))
                    struct.pack_into("<I", membership, MEMBERSHIP_PLAYERS, players)
                    membership = Buffer(bytes(membership))
                    t.check(
                        "game_voice_player_flags",
                        player,
                        globals={
                            **state(enabled, initialized),
                            "game_voice_player_flag_words": struct.pack("<16H", *(random_.getrandbits(16) | 1 for _ in range(16))),
                        },
                        externals={
                            "game_voice_get_membership": lambda harness, arguments, membership=membership, has_membership=has_membership: (
                                membership.address if has_membership else 0
                            )
                        },
                        extra=[membership],
                    )

        for unknown in (1, 0):
            for source in (1, 2, 0, 3, 0xFFFFFFFF):
                for found in (1, 0, 0x100):
                    place = Buffer(random_.randbytes(4))
                    t.check(
                        "game_voice_get_session",
                        place,
                        globals={
                            **state(enabled, initialized),
                            "unknown_004c9888": unknown,
                            "game_voice_session_source": source,
                        },
                        externals={
                            "network_life_cycle_get_session": lambda harness, arguments, found=found: found,
                            "network_life_cycle_get_other_session": lambda harness, arguments, found=found: found ^ 1,
                        },
                    )

        for found in (1, 0):
            for unknown741c in (1, 0, 0x100):
                for first in (0, NONE, 5):
                    session = bytearray(random_.randbytes(SESSION_SIZE))
                    struct.pack_into("<I", session, 0x741C, unknown741c)
                    struct.pack_into("<I", session, MEMBERSHIP, first)
                    session = Buffer(bytes(session))

                    def get_session(harness, arguments, session=session, found=found):
                        # the place for the session is in the stack
                        if found:
                            harness.mu.mem_write(arguments[0], struct.pack("<I", session.address))
                        return found, ()

                    t.check(
                        "game_voice_get_membership",
                        globals=state(enabled, initialized),
                        externals={"game_voice_get_session": get_session},
                        extra=[session],
                    )

    for state_ in (7, 6, 8, 0, 0x107):
        thing = bytearray(random_.randbytes(0x48))
        struct.pack_into("<I", thing, 0x44, state_)
        thing = Buffer(bytes(thing))
        for there in (True, False):
            t.check(
                "game_voice_unknown_00053d90",
                externals={
                    "unknown_000649c0": lambda harness, arguments, thing=thing, there=there: thing.address if there else 0
                },
                extra=[thing],
            )

    def sound_file(file: bytes = b"RIFF", form: bytes = b"WAVE", format_: bytes = b"fmt ") -> Buffer:
        return Buffer(file + random_.randbytes(4) + form + format_ + random_.randbytes(0x10))

    for message in (
        sound_file(),
        sound_file(file=b"RIFX"),
        sound_file(file=b"\0IFF"),
        sound_file(form=b"WAVF"),
        sound_file(format_=b"fmt\0"),
        sound_file(format_=b"fmu "),
    ):
        for size in (0x20, 0x11, 0x10, 0, 0x80000020, 0x7FFFFFFF):
            t.check("game_voice_message_is_sound_file", message, size)

    # how the processor is set is the emulator's, and is not compared: only
    # what the C library is asked
    t.check("game_voice_library_begin", externals={"_control87": lambda harness, arguments: 0})
    t.check(
        "game_voice_library_end",
        externals={"_control87": lambda harness, arguments: 0, "_clearfp": lambda harness, arguments: 0},
    )

    # ---------- the consoles and the players of the session

    PLAYERS = 0x10D4
    PLAYER_SIZE = 0x13C
    MEMBERSHIP_SIZE = PLAYERS + 16 * PLAYER_SIZE

    def session_at(found: int, session: Buffer):
        def handler(harness, arguments):
            # the place for the session is in the stack
            if found:
                harness.mu.mem_write(arguments[0], struct.pack("<I", session.address))
            return found, ()

        return handler

    def a_session(unknown741c: int, first: int) -> Buffer:
        data = bytearray(random_.randbytes(SESSION_SIZE))
        struct.pack_into("<I", data, 0x741C, unknown741c)
        struct.pack_into("<I", data, MEMBERSHIP, first)
        return Buffer(bytes(data))

    def gives(value: object):
        return lambda harness, arguments: value

    def gives_buffer(buffer: Buffer, there: bool = True, offset: int = 0):
        return lambda harness, arguments: buffer.address + offset if there else 0

    for source in (2, 1, 0, 0x102):
        for found in (1, 0):
            thing = bytearray(random_.randbytes(0x18))
            struct.pack_into("<I", thing, 0x14, source)
            thing = Buffer(bytes(thing))

            def session_source(harness, arguments, thing=thing, found=found):
                if found:
                    harness.mu.mem_write(arguments[0], struct.pack("<I", thing.address))
                return found, ()

            t.check(
                "game_voice_update_session_source",
                globals={"game_voice_session_source": random_.getrandbits(32) | 4},
                externals={"network_life_cycle_get_current_session": session_source},
                extra=[thing],
            )

    def with_525a00(result: int, position: int):
        def handler(harness, arguments):
            return result, tuple(
                harness.normalize(argument) if n == position else argument for n, argument in enumerate(arguments)
            )

        return handler

    for enabled, initialized in active():
        for unknown in (1, 0, 3):
            both = {**state(enabled, initialized), "unknown_004c9888": unknown, "unknown_00525a00": GlobalRange(0x1704)}
            t.check(
                "game_voice_read_packet",
                random_.getrandbits(32),
                random_.getrandbits(32),
                globals=both,
                externals={"voice_queues_read_packet": with_525a00(0, 1)},
            )
            for result in (1, 0, 0x100, 0xFFFFFF01):
                t.check(
                    "game_voice_has_packet_for_peer",
                    random_.getrandbits(32),
                    globals=both,
                    externals={"voice_queues_go_to_peer": with_525a00(result, 1)},
                )
            t.check(
                "game_voice_packet_size_for_peer",
                random_.getrandbits(32),
                globals=both,
                externals={"voice_queues_size_for_peer": with_525a00(random_.getrandbits(32), 0)},
            )

            for value in (0x12345678, 0):
                t.check(
                    "game_voice_unknown_00054cc0",
                    globals={
                        **state(enabled, initialized),
                        "unknown_004c9888": random_.choice((0, 1, 2, 3, 4, 0xFFFFFFFF, 0x80000001)) if unknown else unknown,
                        "unknown_004c9880": value ^ 0x1111,
                        "unknown_004c9884": value ^ 0x2222,
                    },
                )
        for n in range(8):
            t.check(
                "game_voice_unknown_00054cc0",
                globals={
                    **state(enabled, initialized),
                    "unknown_004c9888": (0, 1, 2, 3, 4, 0xFFFFFFFF, 0x80000001, 0x7FFFFFFF)[n],
                    "unknown_004c9880": 0x1111,
                    "unknown_004c9884": 0x2222,
                },
            )

        t.check(
            "game_voice_get_talkers",
            globals={**state(enabled, initialized), "game_voice_talkers": struct.pack("<H", random_.getrandbits(16) | 0x8001)},
        )

        for found in (1, 0):
            for unknown741c in (1, 0):
                for first in (0, NONE):
                    for name in ("game_voice_get_local_peer", "game_voice_session_unknown40"):
                        session = a_session(unknown741c, first)
                        t.check(
                            name,
                            globals=state(enabled, initialized),
                            externals={"game_voice_get_session": session_at(found, session)},
                            extra=[session],
                        )

        for peer in (0, 3, NONE, 15):
            for other in (peer, 0, NONE, 7):
                t.check(
                    "game_voice_unknown_000549d0",
                    globals={**state(enabled, initialized), "unknown_004c9878": other},
                    externals={"game_voice_get_local_peer": gives(peer)},
                )
            for is_it in (1, 0, 0x100):
                for other in (NONE, 0, 5):
                    t.check(
                        "game_voice_unknown_00054c70",
                        globals={**state(enabled, initialized), "unknown_004c9878": other},
                        externals={"game_voice_unknown_000549d0": gives(is_it)},
                    )

        for there in (True, False):
            membership = Buffer(random_.randbytes(MEMBERSHIP_SIZE))
            for peer in (0, 1, 15, NONE):
                t.check(
                    "game_voice_local_peer_unknownA8",
                    globals=state(enabled, initialized),
                    externals={
                        "game_voice_get_membership": gives_buffer(membership, there),
                        "game_voice_get_local_peer": gives(peer),
                    },
                    extra=[membership],
                )
            for name in ("game_voice_get_players", "game_voice_get_player_array"):
                t.check(
                    name,
                    globals=state(enabled, initialized),
                    externals={"game_voice_get_membership": gives_buffer(membership, there)},
                    extra=[membership],
                )

            for player in (0, 1, 9, 15):
                for local in (1 << player, 0xFFFFFFFF ^ (1 << player), 0, 0xFFFFFFFF):
                    t.check(
                        "game_voice_player_controller",
                        player,
                        globals=state(enabled, initialized),
                        externals={
                            "game_voice_get_local_players": gives(local),
                            "game_voice_get_player_array": gives_buffer(membership, there, PLAYERS),
                        },
                        extra=[membership],
                    )

        for n in range(40):
            # sixteen players: some of this console, some with the number looked for
            identifier = random_.choice((0, 5, 0x12345678, NONE))
            peer = random_.choice((0, 2, NONE))
            data = bytearray(random_.randbytes(MEMBERSHIP_SIZE))
            for player in range(16):
                at = PLAYERS + player * PLAYER_SIZE
                struct.pack_into("<I", data, at + 0xC, random_.choice((peer, peer, 1, 3)) & 0xFFFFFFFF)
                struct.pack_into("<I", data, at + 0x14, random_.choice((identifier, identifier ^ 1, 9)) & 0xFFFFFFFF)
            struct.pack_into("<I", data, MEMBERSHIP_PLAYERS, random_.choice((0xFFFF, 0, random_.getrandbits(16), 0x8000, 0xFFFF0000)))
            membership = Buffer(bytes(data))
            t.check(
                "game_voice_find_local_player",
                identifier,
                globals=state(enabled, initialized),
                externals={
                    "game_voice_get_membership": gives_buffer(membership, random_.getrandbits(3) != 0),
                    "game_voice_get_local_peer": gives(peer),
                },
                extra=[membership],
            )

    # ---------- who can talk and who is talking

    TALKED = 0x8C
    PROFILE_SIZE = 0xC70

    for enabled, initialized in active():
        for controller in range(4):
            for setting in (0, 3, 1):
                for headset in (1, 0, 0x80):
                    for flags in (0x20, 0, 0xFFFFFFDF, 0xFFFFFFFF):
                        for says in (1, 0, 0x100):
                            profiles = bytearray(random_.randbytes(4 * PROFILE_SIZE))
                            struct.pack_into("<I", profiles, controller * PROFILE_SIZE, flags)
                            t.check(
                                "game_voice_controller_can_talk",
                                controller,
                                globals={
                                    **state(enabled, initialized, headsets=tuple(headset if n == controller else random_.getrandbits(1) for n in range(4))),
                                    "game_voice_controller_settings": struct.pack(
                                        "<4I", *(setting if n == controller else random_.randint(0, 3) for n in range(4))
                                    ),
                                    "unknown_0054e8e0": bytes(profiles),
                                },
                                externals={"unknown_001906da": gives(says)},
                            )

            t.check(
                "game_voice_get_controller_setting",
                controller,
                globals={
                    **state(enabled, initialized),
                    "game_voice_controller_settings": struct.pack("<4I", *(random_.getrandbits(32) | 1 for _ in range(4))),
                },
            )
            t.check(
                "game_voice_set_controller_setting",
                controller,
                random_.getrandbits(32),
                globals={
                    **state(enabled, initialized),
                    "game_voice_controller_settings": struct.pack("<4I", *(random_.getrandbits(32) for _ in range(4))),
                },
            )
            for player in (NONE, 0, 7, 15):
                t.check(
                    "game_voice_controller_player_words",
                    controller,
                    globals={
                        **state(enabled, initialized),
                        "game_voice_player_mutes": struct.pack("<16H", *(random_.getrandbits(16) | 1 for _ in range(16))),
                        "game_voice_player_flag_words": struct.pack("<16H", *(random_.getrandbits(16) | 0x8000 for _ in range(16))),
                    },
                    externals={"game_voice_find_local_player": gives(player)},
                )

        for player in (0, 6, 15):
            for local in (1 << player, 0xFFFFFFFF ^ (1 << player)):
                for controller in (0, 3, NONE):
                    for talked in (1, 0, 0x80):
                        data = bytearray(random_.randbytes(VOICE_SIZE))
                        data[VOICE_INITIALIZED] = initialized
                        if controller != NONE:
                            data[TALKED + controller] = talked
                        t.check(
                            "game_voice_player_is_talking",
                            player,
                            globals={"game_voice_enabled": bytes((enabled,)), "voice_globals": GlobalData(bytes(data))},
                            externals={
                                "game_voice_get_local_players": gives(local),
                                "game_voice_player_controller": gives(controller),
                                "voice_remote_talker_is_talking": voice_call(talked),
                            },
                        )

        for unknown in (0, 1, 2, 0x101):
            for name in ("game_voice_unknown_00054ec0", "game_voice_unknown_00054ef0"):
                t.check(name, globals={**state(enabled, initialized), "unknown_004c9888": unknown})
        t.check(
            "game_voice_get_session_source",
            globals={**state(enabled, initialized), "game_voice_session_source": random_.getrandbits(32) | 1},
        )

        for there in (True, True, False):
            for n in range(6):
                peer = random_.choice((0, 2, NONE))
                data = bytearray(random_.randbytes(MEMBERSHIP_SIZE))
                for player in range(16):
                    struct.pack_into("<I", data, PLAYERS + player * PLAYER_SIZE + 0xC, random_.choice((peer, peer, 1, 3)) & 0xFFFFFFFF)
                membership = Buffer(bytes(data))
                t.check(
                    "game_voice_get_local_players",
                    globals=state(enabled, initialized),
                    externals={
                        "game_voice_get_membership": gives_buffer(membership, there),
                        "game_voice_get_players": gives(random_.choice((0xFFFF, 0, random_.getrandbits(16), 0xFFFFFFFF, 0x8001))),
                        "game_voice_get_local_peer": gives(peer),
                    },
                    extra=[membership],
                )

    for start in (b"", b"x" * 0x90):
        for format_, first, second in (
            (b"player identifier change", 0, 0),
            (b"%d and %d", 0xFFFFFFFF, 12),
            (b"%0127d|", 5, 0),
            (b"%0126d|", 5, 0),
            (b"%0200x", 0xABC, 0),
            (b"", 0, 0),
        ):
            t.check(
                "game_voice_text_printf",
                Buffer((start + b"\0" + random_.randbytes(0x90))[:0x88]),
                Buffer(format_ + b"\0"),
                first,
                second,
            )

    # ---------- each frame

    for enabled, initialized in active():
        data = bytearray(random_.randbytes(VOICE_SIZE))
        data[VOICE_INITIALIZED] = initialized
        struct.pack_into("<I", data, 0x10, ENGINE)
        struct.pack_into("<I", data, TALKED, 0x01800101)
        t.check(
            "game_voice_library_update",
            globals={"game_voice_enabled": bytes((enabled,)), "voice_globals": GlobalData(bytes(data))},
            externals={
                "_control87": gives(0),
                "_clearfp": gives(0),
                # the players that talked are forgotten before the engine works
                "unknown_00332f22": lambda harness, arguments: (
                    0,
                    (arguments[0], bytes(harness.mu.mem_read(harness.global_addresses["voice_globals"] + TALKED, 4))),
                ),
            },
        )

    t.check(
        "game_voice_reset_players",
        globals={
            "game_voice_talkers": struct.pack("<H", 0xFFFF),
            "game_voice_session_players": 0x12345678,
            "game_voice_session_player_identifiers": random_.randbytes(0xC0),
            "game_voice_player_mutes": random_.randbytes(0x20),
            "game_voice_player_flag_words": random_.randbytes(0x20),
            "game_voice_pool": 0x80100000,
        },
    )

    for network in (1, 0, 0x80):
        for network_state in range(-1, 12):
            for in_progress in (1, 0):
                for other in (1, 0):
                    for flags in (1, 0, 0xFE):
                        thing = bytearray(random_.randbytes(0x4C))
                        thing[0x48] = flags
                        thing = Buffer(bytes(thing))
                        t.check(
                            "game_voice_update_network_state",
                            globals={
                                "unknown_00527330": bytes((network,)),
                                "unknown_00527334": network_state & 0xFFFFFFFF,
                                "unknown_004c9888": 0x55,
                            },
                            externals={
                                "game_in_progress": gives(in_progress),
                                "unknown_00068250": gives(other),
                                "unknown_000649c0": gives_buffer(thing, flags != 0xFE or in_progress),
                            },
                            extra=[thing],
                        )

    for index in (0, 3, 15):
        for has in (0x12345678, 0):
            for flags in (1, 0, 0xFE, 0xFF):
                for there, state7 in ((True, 7), (True, 6), (False, 7)):
                    other = bytearray(random_.randbytes(0xC18))
                    struct.pack_into("<I", other, 0xC14, index)
                    game_data = bytearray(random_.randbytes(GAME_SIZE))
                    game_data[0x184] = flags
                    thing = bytearray(random_.randbytes(0x4C))
                    struct.pack_into("<I", thing, 0x44, state7)
                    thing = Buffer(bytes(thing))
                    table = [random_.getrandbits(32) | 1 for _ in range(16)]
                    table[index] = has
                    t.check(
                        "game_voice_unknown_00053d40",
                        globals={
                            "unknown_004e9ae8": Buffer(bytes(other)),
                            "unknown_0055e4d0": struct.pack("<16I", *table),
                            "main_game_globals": Buffer(bytes(game_data)),
                        },
                        externals={"unknown_000649c0": gives_buffer(thing, there)},
                        extra=[thing],
                    )

    # ---------- a player of the session

    for enabled, initialized in active():
        for player in (0, 5, 15):
            for wanted in (1, 0):
                for has in (1, 0):
                    for set_ in (1, 0):
                        if not (enabled and initialized):
                            # with no voice the game reads the players of no session
                            continue
                        membership = Buffer(random_.randbytes(MEMBERSHIP_SIZE))
                        t.check(
                            "game_voice_update_player",
                            player,
                            globals={
                                **state(enabled, initialized),
                                "game_voice_player_mutes": random_.randbytes(0x20),
                                "game_voice_player_flag_words": random_.randbytes(0x20),
                                "game_voice_talkers": struct.pack("<H", random_.choice((0, 0xFFFF, random_.getrandbits(16)))),
                            },
                            externals={
                                # the game does not see whether there is a session: there always is one here
                                "game_voice_get_membership": gives_buffer(membership),
                                "game_voice_player_can_be_heard": gives(wanted),
                                "voice_has_remote_talker": voice_call(has),
                                "voice_remote_talker_set": lambda harness, arguments, set_=set_: (
                                    set_,
                                    (harness.normalize(arguments[0]), arguments[1], arguments[2] & 0xFF),
                                ),
                            },
                            extra=[membership],
                        )

        for player in (0, 7, 15, 16, 0x20003, 0xFFFF0000, NONE, 0x1FFFF):
            for game_there, game_active, table_there in ((1, 1, 1), (0, 1, 1), (1, 0, 1), (1, 1, 0), (1, 0x80, 1)):
                for first in (0, NONE, 5):
                    for says in (1, 0, 0x100):
                        game_data = bytearray(random_.randbytes(GAME_SIZE))
                        game_data[0x1120] = game_active
                        table = bytearray(random_.randbytes(0x7C + 16 * 0x88))
                        if (player & 0xFFFF) < 16:
                            struct.pack_into("<I", table, 0x7C + (player & 0xFFFF) * 0x88, first)
                        table = Buffer(bytes(table))
                        other = bytearray(random_.randbytes(0x10))
                        struct.pack_into("<I", other, 0xC, 0)
                        t.check(
                            "game_voice_unknown_00054df0",
                            player,
                            globals={
                                **state(enabled, initialized),
                                "main_game_globals": Buffer(bytes(game_data)) if game_there else 0,
                                "unknown_004cf770+0xC": table if table_there else 0,
                            },
                            externals={"unknown_000696d0": gives(says)},
                        )

        for player in (0, 3, 15, NONE):
            for says in (1, 0):
                # the players: sixteen of 0x21c bytes, each with its salt first
                elements = Buffer(random_.randbytes(16 * 0x21C))
                array = bytearray(random_.randbytes(0x4C))
                struct.pack_into("<I", array, 0x24, 0x21C)

                array_buffer = Buffer(bytes(array) + elements.data, pointers=())
                data = bytearray(array_buffer.data)
                struct.pack_into("<I", data, 0x44, 0x4C)
                t.check(
                    "game_voice_get_player_datum",
                    player,
                    globals={**state(enabled, initialized), "player_data": Buffer(bytes(data), pointers=(0x44,))},
                    externals={"game_voice_unknown_00054df0": gives(says)},
                )

    for controller in range(4):
        for profile in (0, 3, NONE):
            for there in (1, 0, 0x80):
                profiles = bytearray(random_.randbytes(4 * PROFILE_SIZE))
                struct.pack_into("<I", profiles, controller * PROFILE_SIZE + 4, profile)
                kept = bytearray(random_.randbytes(4 * 0xD0))
                if profile != NONE:
                    kept[profile * 0xD0] = there
                t.check(
                    "game_voice_update_profile",
                    controller,
                    globals={"unknown_0054e8e0": bytes(profiles), "unknown_004cd904": bytes(kept)},
                    externals={"game_voice_controller_player_words": gives(random_.getrandbits(32))},
                )

    # ---------- the frame, and what a player said

    def named(*names: str, **results: object) -> dict:
        """Functions that do nothing here: the calls to them are compared, with
        their pointers to globals as the places they point to."""

        def handler(result):
            return lambda harness, arguments: (result, tuple(harness.normalize(argument) for argument in arguments))

        return {**{name: handler(0) for name in names}, **{name: handler(result) for name, result in results.items()}}

    for enabled, initialized in active():
        for kind in (2, 1, 0):
            for source in (1, 0, 2):
                for network in (1, 0):
                    for first_on in (1, 0, 0x80):
                        first = bytearray(random_.randbytes(0x48))
                        first[0] = first_on
                        t.check(
                            "game_voice_update",
                            globals={
                                **state(enabled, initialized),
                                "game_voice_kind": kind,
                                "game_voice_session_source": source,
                                "unknown_004c9888": 0x55,
                                "unknown_005259b8": GlobalData(bytes(first)),
                                "unknown_00527108": GlobalRange(0xC4),
                            },
                            externals=named(
                                "game_voice_library_update",
                                "game_voice_update_session_source",
                                "game_voice_update_network_state",
                                "game_voice_update_session_players",
                                "voice_remove_remote_talkers",
                                "game_voice_reset_players",
                                "game_voice_update_players",
                                "game_voice_update_chosen_peer",
                                "voice_channels_update",
                                "voice_targets_update",
                                game_voice_unknown_00054ec0=network,
                            ),
                        )

    def nth(name: str, *values: int):
        def handler(harness, arguments):
            calls = sum(1 for call in harness._calls if call[0] == name)
            return values[min(calls, len(values) - 1)]

        return handler

    def data_cases():
        # (the player of the controller, then again; its flags; the third flag; the number; the first word; the third thing on; its number of the player)
        yield (3, 3), 0, 0, 0x55, 7, 1, 1
        yield (3, 5), 0, 0, 0x55, 7, 1, 0
        yield (3, 3), 0, 0, 0x55, 7, 0, 1
        yield (0, 0), 0xFFFD, 0, 1, 0xFFFF, 0x80, 0x100
        yield (NONE, 3), 0, 0, 0x55, 7, 1, 1
        yield (3, NONE), 0, 0, 0x55, 7, 1, 1
        yield (3, 3), 2, 0, 0x55, 7, 1, 1
        yield (3, 3), 0, 1, 0x55, 7, 1, 1
        yield (3, 3), 0, 0, 0, 7, 1, 1
        yield (3, 3), 0, 0, 0x55, 0, 1, 1
        yield (15, 15), 1, 0, 0x80000000, 0x10000, 1, 1
        yield (15, 15), 1, 0, 0x80000000, 0x10001, 1, 1

    for enabled, initialized in active():
        for players, flags, third, number, first_word, on, of_player in data_cases():
            thing = bytearray(random_.randbytes(0x84))
            thing[0] = on
            for player in range(16):
                struct.pack_into("<I", thing, 0x44 + player * 4, of_player if player in players else 0)

            def words(harness, arguments, first_word=first_word):
                # the two numbers are in the stack, and so is the flag
                harness.mu.mem_write(arguments[1], struct.pack("<2H", first_word & 0xFFFF, 0x1234))
                return 0, (arguments[0], harness.normalize(arguments[2]))

            def on_its_way(harness, arguments):
                player, unknown, two, data, size, flag = arguments
                return 0, (
                    player,
                    harness.normalize(unknown),
                    bytes(harness.mu.mem_read(two, 4)),
                    data,
                    size,
                    bytes(harness.mu.mem_read(flag, 1)),
                )

            t.check(
                "game_voice_local_data_ready",
                random_.randint(0, 3),
                0x80345000,
                random_.getrandbits(16),
                globals={
                    **state(enabled, initialized),
                    "unknown_005259b8": GlobalRange(0x48),
                    "unknown_00527104": GlobalRange(4),
                    "unknown_00527108": GlobalData(bytes(thing)),
                    "unknown_00525a00": GlobalRange(0x1704),
                },
                externals={
                    "game_voice_find_local_player": nth("game_voice_find_local_player", *players),
                    "game_voice_player_flags": gives(flags),
                    "game_voice_player_not_allowed": gives(third),
                    "voice_targets_get": lambda harness, arguments, number=number: (
                        number,
                        (harness.normalize(arguments[0]), arguments[1]),
                    ),
                    "voice_unknown_00527104_get_words": words,
                    "voice_queues_add": on_its_way,
                },
            )

    def place(found: int, session: Buffer):
        def handler(harness, arguments):
            if found:
                harness.mu.mem_write(arguments[0], struct.pack("<I", session.address))
            return found, ()

        return handler

    for enabled, initialized in active():
        for source in (2, 1, 0):
            for found, other_found in ((1, 1), (0, 1), (1, 0)):
                for count in (0, 1, 5, 16, 0xFFFFFFFF):
                    data = bytearray(random_.randbytes(SESSION_SIZE))
                    struct.pack_into("<I", data, 0x741C, 1)
                    struct.pack_into("<I", data, MEMBERSHIP, 0)
                    struct.pack_into("<I", data, MEMBERSHIP + 8, count)
                    session = Buffer(bytes(data))
                    other = Buffer(random_.randbytes(0x10))
                    in_other = [random_.choice((NONE, 0, 3)) for _ in range(16)]

                    def which(harness, arguments, session=session, in_other=in_other):
                        peer = (arguments[1] - session.address - MEMBERSHIP - 0xC) // 0x10C
                        return in_other[peer], (arguments[0], arguments[1])

                    t.check(
                        "game_voice_unknown_00053c70",
                        globals={**state(enabled, initialized), "game_voice_session_source": source},
                        externals={
                            "network_life_cycle_get_other_session": place(found, session),
                            "network_life_cycle_get_session": place(other_found, other),
                            "network_session_find_peer": which,
                        },
                        extra=[session, other],
                    )

    # ---------- the console chosen for the voice

    PEER_SIZE = 0x10C

    def peers(count: int, local: int, least9c: int, leasta0: int) -> bytes:
        """What the session says of who is in it: consoles that can be chosen
        and that cannot, each for one reason or another."""
        data = bytearray(random_.randbytes(MEMBERSHIP_SIZE))
        struct.pack_into("<I", data, 8, count & 0xFFFFFFFF)
        everyone = ((1 << count) - 1) & 0xFFFFFFFF if 0 <= count < 32 else 0
        for peer in range(16):
            at = peer * PEER_SIZE
            good = random_.getrandbits(2) != 0
            values = [
                least9c + random_.randint(0, 5),
                leasta0 + random_.randint(0, 9),
                random_.choice((0, 1, 2)),
                everyone,
            ]
            if not good:
                which = random_.randrange(4)
                values[which] = (least9c - 1, leasta0 - 1, 3, everyone ^ 1)[which]
            struct.pack_into("<4i", data, at + 0x9C, values[0], values[1], values[2], struct.unpack("<i", struct.pack("<I", values[3]))[0])
        return bytes(data)

    for enabled, initialized in active():
        for n in range(60):
            count = random_.choice((0, 1, 2, 4, 8, 16, -1))
            local = random_.choice((0, 0, 1, 3, NONE))
            host = random_.choice((local, local, local, 0, 2, NONE))
            least9c, leasta0 = random_.randint(1, 50), random_.randint(1, 50)
            membership = Buffer(peers(count, local, least9c, leasta0))
            session = Buffer(random_.randbytes(0x10))
            has_session = random_.getrandbits(3) != 0
            has_membership = random_.getrandbits(3) != 0
            now = random_.choice((NONE, 0, 1, 2, 3))

            def get_session(harness, arguments, session=session, has_session=has_session):
                # the place for the session is in the stack
                if has_session:
                    harness.mu.mem_write(arguments[0], struct.pack("<I", session.address))
                return has_session, ()

            t.check(
                "game_voice_update_chosen_peer",
                globals={
                    **state(enabled, initialized),
                    "unknown_004c9878": 0x55,
                    "unknown_004ce120": leasta0,
                    "unknown_004ce124": least9c,
                },
                externals={
                    "game_voice_get_session": get_session,
                    "game_voice_get_membership": gives_buffer(membership, has_membership),
                    "game_voice_get_local_peer": gives(local),
                    "game_voice_session_unknown40": gives(host),
                    "game_voice_unknown_00054ef0": gives(random_.getrandbits(1)),
                    "network_session_get_chosen_peer": gives(now),
                    "unknown_0005c6d0": lambda harness, arguments: 0,
                },
                extra=[membership, session],
            )

        for n in range(40):
            peer = random_.choice((0, 1, 5, 15))
            local = random_.choice((0, 0xFFFF, 1 << random_.randrange(16), random_.getrandbits(16)))
            # of each player: its flags, the third flag, its number and its first word
            flags = [random_.choice((0, 0, 0, 2, 0xFFFD)) for _ in range(16)]
            third = [random_.choice((0, 0, 0, 1)) for _ in range(16)]
            numbers = [random_.choice((0, 0x55, 0x80000000)) for _ in range(16)]
            first_words = [random_.choice((0, 1 << peer, 0xFFFF ^ (1 << peer), 0xFFFF)) for _ in range(16)]
            membership = Buffer(random_.randbytes(0x10))

            def words(harness, arguments, first_words=first_words):
                # the two numbers are in the stack; the number of the player says which player
                player = sum(1 for call in harness._calls if call[0] == "voice_unknown_00527104_get_words")
                harness.mu.mem_write(arguments[1], struct.pack("<2H", first_words[player % 16], 0x1234))
                return 0, (arguments[0], harness.normalize(arguments[2]))

            t.check(
                "game_voice_unknown_00054890",
                peer,
                globals={
                    **state(enabled, initialized),
                    "unknown_005259b8": GlobalRange(0x48),
                    "unknown_00527104": GlobalRange(4),
                },
                externals={
                    "game_voice_get_membership": gives_buffer(membership, random_.getrandbits(3) != 0),
                    "game_voice_unknown_000549d0": gives(random_.choice((0, 0, 0, 1))),
                    "game_voice_get_local_players": gives(local),
                    "game_voice_player_flags": lambda harness, arguments, flags=flags: flags[arguments[0] & 15],
                    "game_voice_player_not_allowed": lambda harness, arguments, third=third: third[arguments[0] & 15],
                    "voice_targets_get": lambda harness, arguments, numbers=numbers: (
                        numbers[arguments[1] & 15],
                        (harness.normalize(arguments[0]), arguments[1]),
                    ),
                    "voice_unknown_00527104_get_words": words,
                },
                extra=[membership],
            )

    # ---------- the players, each frame

    for enabled, initialized in active():
        for n in range(12):
            membership = bytearray(random_.randbytes(MEMBERSHIP_SIZE))
            struct.pack_into("<I", membership, MEMBERSHIP_PLAYERS, random_.choice((0xFFFF, 0, random_.getrandbits(16), 0xFFFF00FF)))
            membership = Buffer(bytes(membership))
            have_voices = random_.getrandbits(16)
            data = bytearray(random_.randbytes(VOICE_SIZE))
            data[VOICE_INITIALIZED] = initialized
            # with the mode of the players of other consoles, or without it
            struct.pack_into("<2I", data, 0xC, random_.choice((0xF, 0xF, 0xB)), ENGINE)
            t.check(
                "game_voice_update_players",
                globals={
                    "game_voice_enabled": bytes((enabled,)),
                    "voice_globals": GlobalData(bytes(data)),
                    "game_voice_talkers": struct.pack("<H", random_.choice((0xFFFF, random_.getrandbits(16)))),
                },
                externals={
                    "game_voice_get_membership": gives_buffer(membership, n != 3),
                    "game_voice_get_local_players": gives(random_.choice((0, 1, 0x8001, random_.getrandbits(16)))),
                    "voice_has_remote_talker": lambda harness, arguments, have_voices=have_voices: (
                        (have_voices >> (arguments[1] & 15)) & 1,
                        (harness.normalize(arguments[0]), arguments[1]),
                    ),
                    "unknown_00332dd3": lambda harness, arguments: 0,
                    "game_voice_update_local_player": lambda harness, arguments: 0,
                    "game_voice_update_player": lambda harness, arguments: 0,
                },
                extra=[membership],
            )

    def set_mode(harness, arguments):
        # the mode is the test's to set: it is what the function reads next of the voice
        return 0, tuple(arguments)

    for n in range(240):
        player = random_.choice((0, 3, 15))
        controller = random_.choice((0, 1, 3, 3, 3, NONE))
        setting = random_.choice((0, 1, 3))
        mode = random_.choice((0, 2, 3, 0, 2))
        initialized = random_.choice((1, 1, 1, 0))
        flag5 = random_.choice((0x20, 0x20, 0, 0xFFFFFFDF))
        count = random_.choice((0, 1, 3, 250, NONE, 2))
        if not initialized:
            # with no voice the game has no players to look for in the list, and does not see it
            count = random_.choice((0, NONE))
        membership = bytearray(random_.randbytes(MEMBERSHIP_SIZE))
        lists = bytearray(random_.randbytes(4 * 250 * 0x10))
        if controller != NONE and count not in (0, NONE):
            # some of the players are in the list, at any place of it; and one nearly
            for other in random_.sample(range(16), 6):
                at = controller * 4000 + random_.randrange(count) * 0x10
                lists[at : at + 0xC] = membership[PLAYERS + other * PLAYER_SIZE : PLAYERS + other * PLAYER_SIZE + 0xC]
            other = random_.randrange(16)
            at = controller * 4000 + random_.randrange(count) * 0x10
            lists[at : at + 0xC] = membership[PLAYERS + other * PLAYER_SIZE : PLAYERS + other * PLAYER_SIZE + 0xC]
            lists[at + 0xB] ^= 0x10
        membership = Buffer(bytes(membership))
        profiles = bytearray(random_.randbytes(4 * PROFILE_SIZE))
        counts = [random_.choice((0, 5, NONE)) for _ in range(4)]
        modes = [random_.randint(0, 3) for _ in range(4)]
        settings = [random_.randint(0, 3) for _ in range(4)]
        if controller != NONE:
            struct.pack_into("<I", profiles, controller * PROFILE_SIZE, flag5 | (random_.getrandbits(32) & ~0x20 if flag5 else 0) if flag5 != 0xFFFFFFDF else flag5)
            counts[controller] = count
            modes[controller] = mode
            settings[controller] = setting
        data = bytearray(random_.randbytes(VOICE_SIZE))
        data[VOICE_INITIALIZED] = initialized
        struct.pack_into("<4I", data, VOICE_MODES_OFFSET, *modes)
        t.check(
            "game_voice_update_local_player",
            player,
            globals={
                "game_voice_enabled": bytes((1,)),
                "voice_globals": GlobalData(bytes(data)),
                "game_voice_controller_settings": struct.pack("<4I", *settings),
                "unknown_0054e8e0": bytes(profiles),
                "game_voice_muted_players": bytes(lists),
                "game_voice_muted_player_counts": struct.pack("<4I", *counts),
                "game_voice_player_flag_words": random_.randbytes(0x20),
                "game_voice_player_mutes": random_.randbytes(0x20),
                "game_voice_talkers": struct.pack("<H", random_.choice((0xFFFF, 0, random_.getrandbits(16)))),
            },
            externals={
                "game_voice_player_controller": gives(controller),
                # the game does not see whether there is a session: there always is one here
                "game_voice_get_membership": gives_buffer(membership),
                "unknown_001906da": gives(random_.choice((1, 1, 0))),
                "game_voice_controller_can_talk": gives(random_.choice((1, 1, 0))),
                "game_voice_set_controller_mode": set_mode,
                "game_voice_get_players": gives(random_.choice((0xFFFF, random_.getrandbits(16), 0))),
                "game_voice_get_local_players": gives(random_.choice((0, 1 << player, random_.getrandbits(16)))),
                "game_voice_update_profile": lambda harness, arguments: 0,
            },
            extra=[membership],
        )

    # ---------- whether the players of the session are the same

    def session_players_case(kind: str):
        """What the session says, and what the game had: the same, or not the same in one way or another."""
        mask = random_.choice((0xFFFF, 0, random_.getrandbits(16), 0x8001))
        data = bytearray(random_.randbytes(MEMBERSHIP_SIZE))
        struct.pack_into("<I", data, MEMBERSHIP_PLAYERS, mask)
        had = bytearray()
        for player in range(16):
            at = PLAYERS + player * PLAYER_SIZE
            had += data[at : at + 0xC] if mask >> player & 1 else bytes(0xC)
        had_mask = mask
        if kind == "mask":
            had_mask = mask ^ (1 << random_.randrange(32))
        elif kind == "identifier":
            had[random_.randrange(0xC0)] ^= 1 << random_.randrange(8)
        elif kind == "last identifier":
            had[0xBF] ^= 0x80
        return bytes(data), had_mask, bytes(had)

    for enabled, initialized in active():
        for kind in ("same", "mask", "identifier", "last identifier", "same", "identifier"):
            for there in (True, True, False):
                data, had_mask, had = session_players_case(kind)
                membership = Buffer(data)
                if not there and kind == "same":
                    # with no session there are no players
                    had_mask, had = 0, bytes(0xC0)
                t.check(
                    "game_voice_update_session_players",
                    globals={
                        **state(enabled, initialized),
                        "game_voice_session_players": had_mask,
                        "game_voice_session_player_identifiers": had,
                        "unknown_00525a00": GlobalRange(0x1704),
                        "game_voice_pool": 0x80100000,
                    },
                    externals={
                        "game_voice_get_membership": gives_buffer(membership, there),
                        "voice_queues_reset": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                    },
                    extra=[membership],
                )

    # ---------- the console chosen for the voice: one console alone that falls short

    # the mutations found that no case had a console that could be chosen but
    # for one thing, with no other to choose: each thing is tried alone here
    for short in ("nothing", "first number", "second number", "cannot be chosen", "not joined to all", "is this one"):
        for says in (0, 1):
            count = 2
            local = 0
            least9c, leasta0 = 20, 30
            data = bytearray(random_.randbytes(MEMBERSHIP_SIZE))
            struct.pack_into("<I", data, 8, count)
            for peer in range(16):
                # every console but the second falls short in everything
                struct.pack_into("<4i", data, peer * PEER_SIZE + 0x9C, 0, 0, 3, 0)
            values = [least9c, leasta0, 0, (1 << count) - 1]
            if short == "first number":
                values[0] = least9c - 1
            elif short == "second number":
                values[1] = leasta0 - 1
            elif short == "cannot be chosen":
                values[2] = 3
            elif short == "not joined to all":
                values[3] = 1
            struct.pack_into("<4i", data, (0 if short == "is this one" else 1) * PEER_SIZE + 0x9C, *values)
            membership = Buffer(bytes(data))
            session = Buffer(random_.randbytes(0x10))

            def get_session(harness, arguments, session=session):
                harness.mu.mem_write(arguments[0], struct.pack("<I", session.address))
                return 1, ()

            t.check(
                "game_voice_update_chosen_peer",
                globals={
                    **state(1, 1),
                    "unknown_004c9878": 0x55,
                    "unknown_004ce120": leasta0,
                    "unknown_004ce124": least9c,
                },
                externals={
                    "game_voice_get_session": get_session,
                    "game_voice_get_membership": gives_buffer(membership),
                    "game_voice_get_local_peer": gives(local),
                    "game_voice_session_unknown40": gives(local),
                    "game_voice_unknown_00054ef0": gives(says),
                    "network_session_get_chosen_peer": gives(NONE),
                    "unknown_0005c6d0": lambda harness, arguments: 0,
                },
                extra=[membership, session],
            )
