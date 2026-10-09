# source/networking/network_observer.c

import random
import struct

from difftest import Buffer, Mock, f32, mock_address

NONE = 0xFFFFFFFF
SIZE = 0x4F40
SESSIONS = 0x14
CHANNELS, CHANNEL = 0xA8, 0x528


def run(t) -> None:
    random_ = random.Random(326)

    def observer() -> bytearray:
        data = bytearray(random_.randbytes(SIZE))
        for channel in range(15):
            at = CHANNELS + channel * CHANNEL
            struct.pack_into("<I", data, at, random_.choice((0, 0, 1, 7, 3)))
            data[at + 9] = random_.choice((0, 1, 2, 4, 8, 0xF, 0xFF, random_.getrandbits(8)))
            struct.pack_into("<I", data, at + 0xC, random_.choice((NONE, 0, 1, 2, 3, 4, 5)))
        return data

    # ---------- the observer is let go of

    for n in range(40):
        t.check(
            "network_observer_dispose",
            Buffer(bytes(observer())),
            externals={"network_observer_channel_delete": lambda harness, arguments: 0},
        )

    # ---------- what it has of a session

    for n in range(80):
        t.check(
            "network_observer_set_session",
            random_.randrange(4),
            Buffer(bytes(observer())),
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(16)),
            random_.choice((NONE, NONE, 0, 1, random_.getrandbits(32))),
            random_.getrandbits(32),
        )

    # ---------- two numbers and how much one count is of two

    for n in range(160):
        data = observer()
        struct.pack_into(
            "<II",
            data,
            0x4E08,
            random_.choice((NONE, 0, 5, random_.getrandbits(32))),
            random_.choice((NONE, 0, 5, random_.getrandbits(32))),
        )
        struct.pack_into(
            "<ii",
            data,
            0x4E18,
            random_.choice((0, 0, 1, 7, 1000, -3, 0x7FFFFFFF)),
            random_.choice((0, 0, 1, 3, 250, -3, 1)),
        )
        t.check(
            "network_observer_get_unknown4e08",
            Buffer(bytes(data)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
        )

    # ---------- which channel is one of the game's, and which has six bytes

    for n in range(300):
        t.check(
            "network_observer_find_channel",
            Buffer(bytes(observer())),
            random_.choice((0, 1, 2, 3, 3, 7, 9)),
            random_.choice((NONE, 0, 1, 2, 3, 4, 5, 6)),
        )

    for n in range(300):
        data = observer()
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL + 0x1E
        six = bytes(data[at : at + 6])
        if n % 4 == 0:
            six = six[:5] + bytes((six[5] ^ 1,))
        if n % 3 == 0:
            # another channel has the same six bytes
            other = CHANNELS + random_.randrange(15) * CHANNEL + 0x1E
            data[other : other + 6] = six
        t.check(
            "network_observer_find_channel_by_unknown1e",
            Buffer(bytes(data)),
            Buffer(six),
            random_.choice((0, 1, 2, 3, 3, 7, 9)),
        )

    # ---------- a channel of the game is told something

    for n in range(160):
        channels = bytearray(random_.randbytes(6 * 0xF8))
        for channel in range(6):
            struct.pack_into("<i", channels, channel * 0xF8 + 0x54, random_.choice((0, 1, 2, 3, 5, -1)))
        t.check(
            "network_observer_channel_unknown_00075e40",
            random_.randrange(15),
            Buffer(bytes(observer())),
            globals={"unknown_004d87d4": Buffer(bytes(channels))},
            externals={"unknown_00088650": lambda harness, arguments: 0},
        )

    # ---------- how long ago a time was

    for n in range(40):
        now = random_.getrandbits(32)
        t.check(
            "network_time_since",
            random_.choice((now, now - 1, 0, random_.getrandbits(32))) & 0xFFFFFFFF,
            globals={
                "unknown_00510548": bytes((random_.choice((0, 0, 1, 0x80)),)),
                "unknown_0051054c": random_.randbytes(4),
            },
            externals={"GetTickCount": lambda harness, arguments, now=now: now},
        )

    # ---------- a session of the three, by what it is known by

    for n in range(200):
        size = 0x7420
        data = bytearray(random_.randbytes(12 + 3 * size))
        pointers = []
        identifier = random_.randbytes(8)
        for session in range(3):
            at = 12 + session * size
            if random_.randrange(5) == 0:
                struct.pack_into("<I", data, session * 4, 0)
            else:
                struct.pack_into("<I", data, session * 4, at)
                pointers.append(session * 4)
            struct.pack_into("<I", data, at + 0x741C, random_.choice((0, 1, 5, 5, 9)))
            data[at + 0x24] = random_.choice((0, 1, 1, 0x80))
            way = random_.randrange(3)
            if way == 0:
                data[at + 0x1C : at + 0x24] = identifier
            elif way == 1:
                data[at + 0x1C : at + 0x24] = identifier[:7] + bytes((identifier[7] ^ 1,))
        t.check("network_sessions_find", Buffer(bytes(data), pointers=tuple(pointers)), Buffer(identifier))

    # ---------- things that are asked of a channel, whose channel of the game is in its fifth state

    def game_channels() -> bytearray:
        channels = bytearray(random_.randbytes(6 * 0xF8))
        for channel in range(6):
            struct.pack_into("<i", channels, channel * 0xF8 + 0x54, random_.choice((5, 5, 5, 5, 0, 2, 3, 6)))
        return channels

    TYPES = (0, 1, 5, 0x19, 0x21, 31, 32, 40, 63)

    for n in range(400):
        data = observer()
        kind = random_.choice(TYPES)
        for channel in range(15):
            # the kind has its bit, or not
            at = CHANNELS + channel * CHANNEL + 0x520
            bits = random_.getrandbits(64)
            bits = bits | 1 << kind if random_.randrange(2) else bits & ~(1 << kind)
            struct.pack_into("<Q", data, at, bits)
        t.check(
            "network_observer_channel_unknown_000768b0",
            random_.randrange(15),
            Buffer(bytes(data)),
            kind,
            globals={"unknown_004d87d4": Buffer(bytes(game_channels()))},
            externals={
                "unknown_00089070": lambda harness, arguments, waiting=random_.choice((0, 0xFFF, 0x1000, 0x2FFF, 0x3000, 0x40000000)): (
                    waiting,
                    (harness.normalize(arguments[0]),),
                )
            },
        )

    for n in range(400):
        t.check(
            "network_observer_channel_set_pending",
            random_.randrange(15),
            Buffer(bytes(observer())),
            random_.choice(TYPES),
            globals={"unknown_004d87d4": Buffer(bytes(game_channels()))},
        )

    for n in range(600):
        now = random_.getrandbits(32)
        channels = game_channels()
        for channel in range(6):
            for at in (0xA8, 0xD8):
                struct.pack_into("<I", channels, channel * 0xF8 + at, (now - random_.choice((0, 999, 1000, 1001, 29999, 30000, 30001))) & 0xFFFFFFFF)
        # what the observer has at 0x10 goes after it
        data = observer() + bytearray(random_.randbytes(0x80))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<II", data, SIZE + 0x78, random_.choice((1000, 30000, 0)), random_.choice((1000, 30000, 0)))
        t.check(
            "network_observer_channel_alive",
            random_.randrange(15),
            Buffer(bytes(data), pointers=(0x10,)),
            globals={
                "unknown_004d87d4": Buffer(bytes(channels)),
                "unknown_00510548": bytes((random_.choice((0, 0, 1)),)),
                "unknown_0051054c": struct.pack("<I", now),
            },
            externals={"GetTickCount": lambda harness, arguments, now=now: now},
        )

    for n in range(400):
        data = observer()
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL + 0x5C
        struct.pack_into("<H", data, at + 0x12, random_.choice((4, 4, 0xFFFF, 16, 16, 0, 6)))
        way = random_.randrange(4)
        if way == 0:
            data[at : at + 16] = bytes(16)
        elif way == 1:
            data[at : at + 4] = bytes(4)
        elif way == 2:
            data[at : at + 16] = bytes(14) + b"\x01\x00"
        t.check(
            "network_observer_channel_connect_status",
            Buffer(bytes(data)),
            channel,
            externals={
                "transport_address_get_connect_status": lambda harness, arguments, status=random_.choice((0, 1, 2, 3)): (
                    status,
                    (harness.normalize(arguments[0]),),
                )
            },
        )

    # ---------- two small things, and something is sent by a channel

    for n in range(80):
        # the thing, and what it has at 0x3C after it
        data = bytearray(random_.randbytes(0x40 + 0x34))
        data[0x40 + 0x30] = random_.choice((0, 1, 0x80))
        if n % 4:
            struct.pack_into("<I", data, 0x3C, 0x40)
            t.check("network_unknown_00075910", Buffer(bytes(data), pointers=(0x3C,)))
        else:
            struct.pack_into("<I", data, 0x3C, 0)
            t.check("network_unknown_00075910", Buffer(bytes(data)))

    for n in range(80):
        data = bytearray(random_.randbytes(0xF8))
        struct.pack_into("<i", data, 0x54, random_.choice((0, 1, 2, 3, 5, -1)))
        t.check("network_channel_get_address", Buffer(bytes(data)), Buffer(random_.randbytes(0x14)))

    for n in range(800):
        data = observer()
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        kind = random_.choice(TYPES)
        bits = random_.getrandbits(64)
        struct.pack_into("<Q", data, at + 0x520, bits | 1 << kind if random_.randrange(2) else bits & ~(1 << kind))
        # its own address of the game: one, or none
        struct.pack_into("<H", data, at + 0x5C + 0x12, random_.choice((4, 4, 16, 0)))
        if random_.randrange(3) == 0:
            data[at + 0x5C : at + 0x5C + 16] = bytes(16)
        channels = game_channels()
        for game_channel in range(6):
            struct.pack_into("<i", channels, game_channel * 0xF8 + 0x54, random_.choice((0, 1, 2, 3, 5, 5)))
            struct.pack_into("<I", channels, game_channel * 0xF8 + 0x14, random_.randrange(4))
        size = random_.choice((8, 0x10, 0xC8))

        def finds(harness, arguments, could=random_.choice((1, 1, 0, 0x100)), address=random_.randbytes(0x14)):
            # the address goes where the third thing says, which is in the stack, as the last three do
            harness.mu.mem_write(arguments[2], address)
            return could, (harness.normalize(arguments[0]), harness.normalize(arguments[1]), arguments[3])

        t.check(
            "network_observer_channel_send",
            random_.choice((0, 0, 1, 1, 0x80)),
            channel,
            Buffer(bytes(data)),
            random_.randrange(4),
            kind,
            size,
            Buffer(random_.randbytes(size)),
            globals={"unknown_004d87d4": Buffer(bytes(channels)), "unknown_004d87dc": Buffer(bytes(16))},
            externals={
                "network_observer_session_get_address": finds,
                "unknown_0007b140": lambda harness, arguments: (
                    0,
                    (arguments[0], bytes(harness.mu.mem_read(arguments[1], 0x14)), arguments[2], arguments[3], harness.normalize(arguments[4])),
                ),
                "unknown_00095580": lambda harness, arguments: (
                    0,
                    (harness.normalize(arguments[0]), arguments[1], arguments[2], harness.normalize(arguments[3])),
                ),
            },
        )

    # ---------- the state of a channel

    def with_owners() -> tuple:
        # what the observer has of each session starts with something that has functions: they go after the observer
        data = observer() + bytearray(16 + 20)
        pointers = []
        for session in range(4):
            struct.pack_into("<I", data, SESSIONS + session * 0x24, SIZE + session * 4)
            struct.pack_into("<I", data, SIZE + session * 4, SIZE + 16)
            pointers += [SESSIONS + session * 0x24, SIZE + session * 4]
        struct.pack_into("<5I", data, SIZE + 16, 0x11, 0x22, 0x33, 0x44, mock_address(0))
        return data, tuple(pointers)

    def at_time() -> dict:
        now = random_.getrandbits(32)
        return {
            "globals": {"unknown_00510548": bytes((random_.choice((0, 0, 1)),)), "unknown_0051054c": struct.pack("<I", now)},
            "externals": {"GetTickCount": lambda harness, arguments, now=now: now},
        }

    for n in range(400):
        data, pointers = with_owners()
        setup = at_time()
        t.check(
            "network_observer_channel_set_state",
            random_.choice((0, 1, 1, 1, 2, 3, 7)),
            Buffer(bytes(data), pointers=pointers),
            random_.randrange(15),
            globals=setup["globals"],
            externals=setup["externals"],
            mocks=[Mock(lambda call: 0, stack_arguments=1, registers=("ecx",))],
        )

    def noted(harness, arguments):
        return 0, (arguments[0], harness.normalize(arguments[1]), arguments[2], arguments[3])

    def with_channel_and_observer(harness, arguments):
        return 0, (arguments[0], harness.normalize(arguments[1]))

    for n in range(200):
        data = observer()
        channel = random_.randrange(15)
        struct.pack_into("<I", data, CHANNELS + channel * CHANNEL + 0x70, random_.choice((NONE, 5, 0)))
        t.check(
            "network_observer_channel_delete",
            channel,
            Buffer(bytes(data)),
            globals={"unknown_004d87d4": Buffer(bytes(game_channels()))},
            externals={
                "network_observer_channel_forget_address": noted,
                "unknown_000886e0": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "transport_qos_delete": lambda harness, arguments: (0, (arguments[0],)),
            },
        )

    for n in range(500):
        data = observer()
        channel = random_.randrange(15)
        struct.pack_into("<i", data, CHANNELS + channel * CHANNEL, random_.choice((0, 1, 2, 3, 3, 3, 4, 7, -1)))
        t.check(
            "network_observer_channel_update_connection",
            channel,
            Buffer(bytes(data)),
            externals={
                "network_observer_channel_connect_status": lambda harness, arguments, status=random_.choice((0, 1, 2, 2, 3)): (
                    status,
                    (harness.normalize(arguments[0]), arguments[1]),
                ),
                "network_observer_channel_set_state": lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1]), arguments[2])),
                "network_observer_channel_forget_address": noted,
            },
        )

    for n in range(300):
        data = observer()
        channel = random_.randrange(15)
        struct.pack_into("<I", data, CHANNELS + channel * CHANNEL, random_.choice((1, 1, 2, 3, 7, 0)))
        t.check(
            "network_observer_channel_connect",
            channel,
            Buffer(bytes(data)),
            externals={
                "network_observer_channel_set_state": lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1]), arguments[2])),
                "network_observer_channel_update_connection": with_channel_and_observer,
                "network_observer_channel_update_game_channel": with_channel_and_observer,
                "network_observer_channel_update_tries": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
            },
        )

    # ---------- the last numbers that are kept of something

    def a_window() -> bytearray:
        window = bytearray(random_.randbytes(0x110))
        count = random_.choice((1, 2, 16, 32))
        struct.pack_into("<II", window, 0, count, random_.randrange(count))
        return window

    for n in range(300):
        setup = at_time()
        t.check(
            "network_window_add",
            Buffer(bytes(a_window())),
            random_.choice((0, 1, 0xFF, random_.getrandbits(32))),
            globals=setup["globals"],
            externals=setup["externals"],
        )

    for n in range(400):
        data = observer()
        channel = random_.randrange(15)
        game_channel = 9
        for other in range(15):
            at = CHANNELS + other * CHANNEL
            # another channel has the same channel of the game, but is not in use
            if other < channel and random_.randrange(3) == 0:
                struct.pack_into("<II", data, at, 0, game_channel)
                struct.pack_into("<I", data, at + 0xC, game_channel)
            data[at + 0x48C] = random_.choice((0, 1, 0x80))
            data[at + 0x250 : at + 0x360] = a_window()
            data[at + 0x360 : at + 0x470] = a_window()
        struct.pack_into("<I", data, CHANNELS + channel * CHANNEL, random_.choice((1, 3, 7)))
        struct.pack_into("<I", data, CHANNELS + channel * CHANNEL + 0xC, game_channel)
        setup = at_time()
        setup["externals"]["network_observer_channel_unknown_000797b0"] = lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2]))
        t.check(
            "network_observer_unknown_00076810",
            Buffer(bytes(data)),
            game_channel,
            random_.choice((0, 1, 0x100, 0x101, 0x80)),
            random_.choice((0, 1, 0x80, 0x1FF)),
            globals=setup["globals"],
            externals=setup["externals"],
        )

    # ---------- what the observer does each time

    for n in range(300):
        data = observer()
        # which channels are let go of, the same each time that it is asked
        going = random_.sample(range(15), random_.choice((0, 3, 8, 15)))

        def goes(harness, arguments, going=going):
            return arguments[0] in going, (arguments[0], harness.normalize(arguments[1]), arguments[2])

        def gone(harness, arguments, really=random_.choice((True, True, False))):
            # the channel is not in use any more, most of the time
            if really:
                harness.mu.mem_write(arguments[1] + CHANNELS + arguments[0] * CHANNEL, bytes(4))
            return 0, (arguments[0], harness.normalize(arguments[1]))

        def with_observer(harness, arguments):
            return 0, (harness.normalize(arguments[0]),)

        t.check(
            "network_observer_update",
            Buffer(bytes(data)),
            externals={
                "network_observer_channel_unused_expired": goes,
                "network_observer_channel_delete": gone,
                "network_observer_channel_update_connection": with_channel_and_observer,
                "network_observer_channel_update_game_channel": with_channel_and_observer,
                "network_observer_channel_update_tries": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                "network_observer_channel_unknown_000773a0": with_channel_and_observer,
                "network_observer_channel_update_probe": with_channel_and_observer,
                "network_observer_update_bandwidth": with_observer,
                "network_observer_update_flow": with_observer,
                "network_observer_update_voice_mode": with_observer,
            },
        )

    # ---------- two things of a channel that go by the time

    AGES = (0, 999, 1000, 1001, 29999, 30000, 30001)

    def told(harness, arguments):
        return 0, (harness.normalize(arguments[0]), arguments[1])

    for n in range(800):
        now = random_.getrandbits(32) | 0x10000
        data = observer()
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        for place in (0x98, 0x9C):
            struct.pack_into("<I", data, at + place, random_.choice((0, (now - random_.choice(AGES)) & 0xFFFFFFFF)))
        struct.pack_into("<H", data, at + 0x5C + 0x12, random_.choice((4, 4, 16, 0)))
        if random_.randrange(3) == 0:
            data[at + 0x5C : at + 0x5C + 16] = bytes(16)
        t.check(
            "network_observer_channel_unused_expired",
            channel,
            Buffer(bytes(data)),
            random_.choice((0xE, 0x11, random_.getrandbits(32))),
            globals={
                "unknown_004d87d4": Buffer(bytes(game_channels())),
                "unknown_00510548": bytes((random_.choice((0, 0, 1)),)),
                "unknown_0051054c": struct.pack("<I", now),
                "unknown_004cf56c": struct.pack("<I", random_.choice((1000, 30000))),
                "unknown_004cf568": struct.pack("<I", random_.choice((1000, 30000))),
            },
            externals={
                "GetTickCount": lambda harness, arguments, now=now: now,
                "unknown_00089070": lambda harness, arguments, waiting=random_.choice((0, 0x3FFF, 0x4000, 0x10000)): (
                    waiting,
                    (harness.normalize(arguments[0]),),
                ),
                "unknown_00088650": told,
            },
        )

    for n in range(800):
        now = random_.getrandbits(32) | 0x10000
        channels = game_channels()
        for game_channel in range(6):
            for place in (0x98, 0xC8):
                struct.pack_into("<I", channels, game_channel * 0xF8 + place, (now - random_.choice(AGES)) & 0xFFFFFFFF)
        data = observer() + bytearray(random_.randbytes(0x88))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<I", data, SIZE + 0x74, random_.choice((1000, 30000, 30000)))
        struct.pack_into("<II", data, SIZE + 0x80, random_.choice((1000, 30000)), random_.choice((1000, 30000)))
        channel = random_.randrange(15)
        struct.pack_into("<I", data, CHANNELS + channel * CHANNEL + 0x94, (now - random_.choice(AGES)) & 0xFFFFFFFF)
        t.check(
            "network_observer_channel_unknown_000773a0",
            channel,
            Buffer(bytes(data), pointers=(0x10,)),
            globals={
                "unknown_004d87d4": Buffer(bytes(channels)),
                "unknown_00510548": bytes((random_.choice((0, 0, 1)),)),
                "unknown_0051054c": struct.pack("<I", now),
            },
            externals={"GetTickCount": lambda harness, arguments, now=now: now, "unknown_00088650": told},
        )

    # ---------- a channel asks for something of the network about its console

    for n in range(800):
        data = observer()
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        struct.pack_into("<I", data, at + 0x70, random_.choice((NONE, NONE, 5)))
        data[at + 8] = random_.choice((0, 0, 0x10, 8, 0xF7, 0xFF))
        for session in range(4):
            struct.pack_into("<I", data, SESSIONS + session * 0x24 + 4, random_.choice((NONE, NONE, 0, 3)))

        def asks(harness, arguments, number=random_.choice((NONE, 5, 6))):
            # what it asks with is in the stack
            return number, (arguments[0], arguments[1], arguments[2], bytes(harness.mu.mem_read(arguments[3], 0x3C)))

        def found(harness, arguments, any=random_.choice((1, 1, 0, 0x100))):
            return any, (arguments[0], harness.normalize(arguments[1]), arguments[2])

        t.check(
            "network_observer_channel_update_probe",
            channel,
            Buffer(bytes(data)),
            externals={
                "transport_qos_new": asks,
                "transport_qos_is_complete": lambda harness, arguments, done=random_.choice((1, 1, 0, 0x100)): (done, (arguments[0],)),
                "transport_qos_get_result": found,
                "transport_qos_delete": lambda harness, arguments: (0, (arguments[0],)),
            },
        )

    # ---------- what the observer keeps after its channels starts over

    for n in range(300):
        data = observer() + bytearray(random_.randbytes(0x13C))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<i", data, 0x4E04, random_.choice((0, 1023, 1024, 0x100000, 0x7FFFFFFF, -1, -1024, -1025, random_.getrandbits(31))))
        struct.pack_into("<ii", data, SIZE + 0x134, random_.choice((0, 10, 1024, -5)), random_.choice((0, 16, 1000, -1, 0x1FFFFF)))
        for channel in range(15):
            data[CHANNELS + channel * CHANNEL + 0x48C] = random_.choice((0, 1, 0x80))
        setup = at_time()
        t.check(
            "network_observer_reset_unknown4e08",
            Buffer(bytes(data), pointers=(0x10,)),
            globals=setup["globals"],
            externals=setup["externals"],
        )

    # ---------- the count of what comes and goes by a channel

    def a_rate(now: int) -> bytearray:
        rate = bytearray(random_.randbytes(0xD4))
        span = random_.choice((1, 100, 1000, 1000))
        ago = random_.choice((0, 1, span - 1, span, span + 1, 3 * span, 20 * span, 21 * span - 1, 21 * span, 50 * span))
        struct.pack_into("<I", rate, 0x10, random_.choice((0, (now - ago) & 0xFFFFFFFF, (now - ago) & 0xFFFFFFFF)))
        struct.pack_into("<I", rate, 0x20, span)
        struct.pack_into("<I", rate, 0x28, random_.randrange(20))
        return rate

    for n in range(600):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        t.check("network_rate_update", Buffer(bytes(a_rate(now))), globals=setup["globals"], externals=setup["externals"])

    for n in range(500):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data = observer()
        channel = random_.randrange(15)
        game_channel = random_.randrange(6)
        for other in range(15):
            at = CHANNELS + other * CHANNEL
            if struct.unpack_from("<I", data, at + 0xC)[0] == game_channel:
                struct.pack_into("<I", data, at + 0xC, NONE)
            data[at + 0x48C] = random_.choice((0, 1, 0x80))
            data[at + 0x178 : at + 0x178 + 0xD4] = a_rate(now)
        struct.pack_into("<I", data, CHANNELS + channel * CHANNEL, random_.choice((1, 3, 7)))
        struct.pack_into("<I", data, CHANNELS + channel * CHANNEL + 0xC, game_channel)
        struct.pack_into("<Q", data, CHANNELS + channel * CHANNEL + 0x178, random_.choice((0, 0xFFFFFFFF, random_.getrandbits(64))))
        channels = game_channels()
        for other in range(6):
            channels[other * 0xF8 + 0x48] = random_.choice((0, 8, 8, 0xF7, 0xFF))
            struct.pack_into("<I", channels, other * 0xF8 + 0x10, random_.randrange(3))
        things = bytearray(random_.randbytes(3 * 0x97C))
        setup["globals"]["unknown_004d87d4"] = Buffer(bytes(channels))
        setup["globals"]["unknown_004d87d8"] = Buffer(bytes(things))
        setup["externals"]["network_observer_channel_unknown_00079660"] = lambda harness, arguments: (
            0,
            (arguments[0], arguments[1], harness.normalize(arguments[2]), arguments[3], arguments[4]),
        )
        t.check(
            "network_observer_unknown_00076720",
            Buffer(bytes(data)),
            game_channel,
            random_.choice((0, 1, 100, 0x7FFFFFFF, NONE, 0x80000000)),
            random_.getrandbits(32),
            globals=setup["globals"],
            externals=setup["externals"],
        )

    # ---------- the two counts start

    for n in range(200):
        t.check(
            "network_rate_new",
            Buffer(random_.randbytes(0xD4)),
            random_.choice((1000, 2000, 20, 19, 21, 1, 39, 40, 0x7FFFFFFF, -1, -20, -39, -40, random_.getrandbits(31))),
        )

    for n in range(200):
        setup = at_time()
        window = a_window()
        if n % 10 == 0:
            struct.pack_into("<i", window, 0, random_.choice((0, -1)))
        t.check("network_window_reset", Buffer(bytes(window)), globals=setup["globals"], externals=setup["externals"])

    # ---------- how many of the last things that a channel was told of were not so

    for n in range(600):
        data = observer() + bytearray(random_.randbytes(0x17C))
        struct.pack_into("<I", data, 0x10, SIZE)
        last = random_.choice((1, 8, 16, 32))
        struct.pack_into("<I", data, SIZE + 0x174, last)
        struct.pack_into("<f", data, SIZE + 0x178, random_.choice((0.0, 0.25, 0.5, 1.0, 1.5)))
        channel = random_.randrange(15)
        bits = random_.getrandbits(last)
        struct.pack_into("<II", data, CHANNELS + channel * CHANNEL + 0x4B8, bin(bits).count("1") if n % 5 else random_.randrange(40), bits)
        t.check(
            "network_observer_channel_unknown_000797b0",
            Buffer(bytes(data), pointers=(0x10,)),
            channel,
            random_.choice((0, 0, 1, 0x100, 0x80)),
            externals={"network_observer_channel_drop_flow": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1]))},
        )

    # ---------- how many bits a second a channel does

    for name in ("network_observer_channel_unknown4a8_bits_per_second", "network_observer_channel_unknown4a4_bits_per_second"):
        for n in range(300):
            setup = at_time()
            now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
            data = observer()
            struct.pack_into("<I", data, 0x4F38, (now - random_.choice((0, 1, 7, 1000, 30001, 0x7FFFFFFF, 0x80000000, NONE))) & 0xFFFFFFFF)
            channel = random_.randrange(15)
            for at in (0x4A4, 0x4A8):
                struct.pack_into("<I", data, CHANNELS + channel * CHANNEL + at, random_.choice((0, 1, 1500, 268435, 268436, 0x10000000, random_.getrandbits(32))))
            t.check(name, channel, Buffer(bytes(data)), globals=setup["globals"], externals=setup["externals"])

    # ---------- two small things of the flow of a channel

    for n in range(200):
        setup = at_time()
        data = observer()
        channel = random_.randrange(15)
        struct.pack_into("<I", data, CHANNELS + channel * CHANNEL + 0x4DC, random_.choice((0, 0, 1, 2, 3)))
        t.check("network_observer_channel_reset_unknown4dc", channel, Buffer(bytes(data)), globals=setup["globals"], externals=setup["externals"])

    for n in range(500):
        data = observer() + bytearray(random_.randbytes(0x1EC))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<iii", data, SIZE + 0x1E0, random_.choice((1, 2, 3, 5)), random_.choice((4, 10, 100)), random_.choice((1, 16, 1000)))
        channel = random_.randrange(15)
        struct.pack_into("<i", data, CHANNELS + channel * CHANNEL + 0x4EC, random_.choice((0, 1, 2, 3, 4, 8, 9, 10, 99, 100, 1000, -1, -7)))
        t.check(
            "network_observer_channel_unknown_0007a160",
            channel,
            Buffer(bytes(data), pointers=(0x10,)),
            externals={"network_observer_channel_lower_flow": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2]))},
        )

    # ---------- the channels of the game are let go of, and something is asked about a channel

    for n in range(200):
        data = observer()
        for channel in range(15):
            # a channel that is in use has a channel of the game
            struct.pack_into("<I", data, CHANNELS + channel * CHANNEL + 0xC, random_.randrange(6))
        channels = game_channels()
        for other in range(6):
            struct.pack_into("<i", channels, other * 0xF8 + 0x54, random_.choice((0, 1, 2, 3, 5)))
        t.check(
            "network_observer_release_game_channels",
            Buffer(bytes(data)),
            globals={"unknown_004d87d4": Buffer(bytes(channels))},
            externals={
                "unknown_00088650": told,
                "network_observer_channel_update_game_channel": with_channel_and_observer,
                "unknown_000886e0": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
        )

    for n in range(600):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data, pointers = with_owners()
        # the first function of what each session has is the one that is asked
        struct.pack_into("<5I", data, SIZE + 16, mock_address(0), 0x22, 0x33, 0x44, 0x55)
        data += bytearray(random_.randbytes(0x74))
        struct.pack_into("<I", data, 0x10, SIZE + 36)
        struct.pack_into("<I", data, SIZE + 36 + 0x70, random_.choice((1000, 1000, 30000)))
        channels = game_channels()
        for other in range(6):
            struct.pack_into("<i", channels, other * 0xF8 + 0x54, random_.choice((0, 2, 3, 4, 4, 5, 5, 7)))
            struct.pack_into("<I", channels, other * 0xF8 + 0xA8, (now - random_.choice((0, 999, 1000, 1001, 30001))) & 0xFFFFFFFF)
        setup["globals"]["unknown_004d87d4"] = Buffer(bytes(channels))
        # which of them say yes: the same each time that one is asked
        yes = random_.sample(range(4), random_.choice((0, 0, 1, 2, 4)))
        t.check(
            "network_observer_channel_unknown_00077940",
            Buffer(bytes(data), pointers=pointers + (0x10,)),
            random_.randrange(15),
            globals=setup["globals"],
            externals=setup["externals"],
            mocks=[Mock(lambda call, yes=yes: (call.registers["ecx"] // 4) % 4 in yes, stack_arguments=1, registers=("ecx",))],
        )

    # ---------- the address of the game of a console, for a session and for a channel

    def an_address(harness, arguments, could=None, address=None):
        # the address goes where the second thing says
        harness.mu.mem_write(arguments[1], address)
        return could, (arguments[0], bytes(harness.mu.mem_read(arguments[2], 0x24)), arguments[3], arguments[4])

    for n in range(500):
        data = observer()
        for session in range(4):
            struct.pack_into("<II", data, SESSIONS + session * 0x24, random_.choice((0, 5, 5, 5)), random_.choice((NONE, 0, 3, 3)))
        address = random_.randbytes(0x14)
        t.check(
            "network_observer_session_get_address",
            Buffer(bytes(data)),
            Buffer(random_.randbytes(0x24)),
            Buffer(random_.randbytes(0x14)),
            random_.randrange(4),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(16)),
            externals={
                "transport_security_address_from_peer_address": lambda harness, arguments, could=random_.choice((1, 1, 1, 0, 0x100)), address=address: an_address(
                    harness, arguments, could, address
                ),
                "transport_address_connect": lambda harness, arguments, could=random_.choice((1, 1, 1, 0, 0x100)): (
                    could,
                    (bytes(harness.mu.mem_read(arguments[0], 0x14)),),
                ),
            },
        )

    for n in range(500):
        data = observer()
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        struct.pack_into("<I", data, at + 0x38, random_.choice((0, 0, 1, 5, 0xF, random_.getrandbits(32))))
        # which sessions give an address: the same each time that one is asked
        giving = random_.sample(range(4), random_.choice((0, 1, 2, 4)))

        def gives(harness, arguments, giving=giving):
            could = arguments[3] in giving
            harness.mu.mem_write(arguments[2], bytes((arguments[3] + 1,)) * 0x14 if could else bytes(0x14))
            if could:
                harness.mu.mem_write(arguments[4], struct.pack("<I", 0x40 + arguments[3]))
                harness.mu.mem_write(arguments[5], bytes((0x50 + arguments[3],)) * 8)
                harness.mu.mem_write(arguments[6], bytes((0x60 + arguments[3],)) * 16)
            return could, tuple(harness.normalize(argument) for argument in arguments)

        t.check(
            "network_observer_channel_find_address",
            channel,
            Buffer(bytes(data)),
            externals={
                "network_observer_channel_connect_status": lambda harness, arguments, status=random_.choice((0, 0, 1, 2, 3, NONE)): (
                    status,
                    (harness.normalize(arguments[0]), arguments[1]),
                ),
                "network_observer_channel_forget_address": noted,
                "network_observer_session_get_address": gives,
            },
        )

    # ---------- a channel has no address of the game any more

    for n in range(600):
        data = observer()
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        struct.pack_into("<H", data, at + 0x5C + 0x12, random_.choice((4, 4, 0xFFFF, 16, 16, 0, 6)))
        way = random_.randrange(4)
        if way == 0:
            data[at + 0x5C : at + 0x5C + 16] = bytes(16)
        elif way == 1:
            data[at + 0x5C : at + 0x5C + 4] = bytes(4)
        struct.pack_into("<i", data, at + 0x3C, random_.choice((0, 1, 3, 3, 4, -1, 31)))
        channels = game_channels()
        for other in range(6):
            struct.pack_into("<i", channels, other * 0xF8 + 0x54, random_.choice((0, 1, 2, 3, 5)))

        def secure(harness, arguments, could=random_.choice((1, 1, 0, 0x100)), value=random_.getrandbits(32)):
            # the number of the address goes where the second thing says, which is in the stack
            harness.mu.mem_write(arguments[1], struct.pack("<I", value))
            return could, (harness.normalize(arguments[0]),)

        t.check(
            "network_observer_channel_forget_address",
            channel,
            Buffer(bytes(data)),
            random_.choice((0, 1, 1, 0x80, 0x100)),
            random_.choice((0, 0xD, 0xE, random_.getrandbits(32))),
            globals={"unknown_004d87d4": Buffer(bytes(channels))},
            externals={
                "unknown_00088650": told,
                "network_message_gateway_flush_address": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                "transport_address_get_secure_in_address": secure,
                "XNetUnregisterInAddr": lambda harness, arguments: (0, (arguments[0],)),
            },
        )

    # ---------- four numbers of a channel

    for n in range(400):
        data = observer()
        channel = random_.choice((random_.randrange(15),) * 6 + (0, 14, 15, 16, NONE, 0x7FFFFFFF, 0x80000000))
        if channel < 15:
            at = CHANNELS + channel * CHANNEL
            data[at + 0x48C] = random_.choice((0, 1, 1, 1, 0x80))
            # how many numbers are kept, and the sum of them
            struct.pack_into("<I", data, at + 0x250, random_.choice((1, 2, 16, 32, 32)))
            struct.pack_into("<I", data, at + 0x358, random_.choice((0, 1, 3, 16, 31, 1000, 0x7FFFFFFF, 0x80000000, NONE)))
        else:
            # what would be one channel more is not looked at, whatever it has
            data += bytes((1,)) * CHANNEL
        t.check(
            "network_observer_channel_get_unknown4b0",
            channel,
            Buffer(bytes(data)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
        )

    # ---------- an interval of the flow is over, and how the flow of a channel is

    for n in range(300):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data = observer()
        struct.pack_into("<I", data, 0x4F38, (now - random_.choice((0, 1, 1000, 30001, 0x80000000))) & 0xFFFFFFFF)
        for channel in range(15):
            at = CHANNELS + channel * CHANNEL
            data[at + 0x48C] = random_.choice((0, 1, 1, 1, 0x80))
            for byte in (0x4A0, 0x4A1, 0x4A2):
                data[at + byte] = random_.choice((0, 0, 0, 1, 0x80))
            for byte in (0x50C, 0x50D, 0x50E):
                data[at + byte] = random_.choice((0, 0, 0, 0, 0, 1, 2))
            struct.pack_into("<I", data, at + 0x4A4, random_.choice((0, 0, 1, 1500, random_.getrandbits(32))))
            struct.pack_into("<I", data, at + 0x4B0, random_.choice((0, 5, 100, 101, NONE, random_.getrandbits(32))))
            struct.pack_into("<I", data, at + 0x4F0, random_.choice((0, 5, 100, 101, NONE, random_.getrandbits(32))))
            struct.pack_into("<II", data, at + 0x518, random_.choice((0, 3, NONE)), random_.choice((0, 3, NONE)))
        t.check("network_observer_new_interval", Buffer(bytes(data)), globals=setup["globals"], externals=setup["externals"])

    for n in range(800):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data = observer() + bytearray(random_.randbytes(0x1DC))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<i", data, SIZE + 0x1A0, random_.choice((1000, 1000, 1000, 0, -1)))
        struct.pack_into("<II", data, SIZE + 0x1D4, random_.choice((0, 5, 30)), random_.choice((0, 7, 60, NONE)))
        struct.pack_into("<I", data, 0x4F38, (now - random_.choice((0, 999, 1000, 1000, 1001, 5000, 5000))) & 0xFFFFFFFF)
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        data[at + 0x48E] = random_.choice((0, 0, 1, 0x80))
        struct.pack_into("<I", data, at + 0x4A8, random_.choice((0, 1, 100, 1500, 100000)))
        struct.pack_into("<II", data, at + 0x4B0, random_.choice((0, 10, 20, 50, 100, 130, 0x7FFFFFFF, 0x80000000)), random_.choice((0, 5, 10, 50, 0x7FFFFFFF)))
        struct.pack_into("<I", data, at + 0x4F0, random_.choice((0, 10, 20, 45, 50, 100, 0x7FFFFFFF)))
        struct.pack_into("<Ii", data, at + 0x504, random_.choice((10, 20, 50, 100, 100, 1000, 0x7FFFFFFF)), random_.choice((0, 1, 8, 160, 1600, 0x7FFFFFFF, -1)))
        t.check(
            "network_observer_channel_unknown_0007a1c0",
            Buffer(bytes(data), pointers=(0x10,)),
            channel,
            globals=setup["globals"],
            externals=dict(setup["externals"], network_observer_channel_unknown_0007a160=with_channel_and_observer),
        )

    # ---------- the observer is told three numbers

    for n in range(800):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data = observer() + bytearray(random_.randbytes(0x1B8))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<f", data, SIZE + 0x1AC, random_.choice((0.5, 1.0, 1.5, 0.0)))
        struct.pack_into("<ii", data, SIZE + 0x1B0, random_.choice((1000, 1000, 0, 5000, -1)), random_.choice((0, 1, 2, 3, 31)))
        struct.pack_into("<II", data, 0x4E08, random_.choice((0, 50, 100, NONE, 0x7FFFFFFF)), random_.choice((NONE, 0, 50, 99, 100, 0x80000000)))
        struct.pack_into("<I", data, 0x4E10, random_.choice((NONE, (now - random_.choice((0, 999, 1000, 1001, 5001))) & 0xFFFFFFFF)))
        data[0x4E14] = random_.choice((0, 0, 1, 1, 2))
        t.check(
            "network_observer_unknown_00077f90",
            random_.choice((0, NONE, 1, 10, 10, 100, 1000, 0x7FFFFFFF)),
            random_.choice((0, 49, 50, 51, 99, 100, 101, 1000, NONE, 0x7FFFFFFF, 0x80000000)),
            Buffer(bytes(data), pointers=(0x10,)),
            random_.choice((0, 4, 5, 6, 10, 11, 15, 16, 100, 150, 151, NONE, random_.getrandbits(32))),
            globals=setup["globals"],
            externals=setup["externals"],
        )

    # ---------- to how many consoles something that a player said can go

    for n in range(600):
        data = observer()
        struct.pack_into("<i", data, 0x4E04, random_.choice((0, 1, 5, 10, 17, 30, 64, 1000, -1, -1000)))
        tables = []
        for table in range(2):
            way = random_.randrange(4)
            if way == 0:
                values = [random_.randrange(-5, 70) for count in range(17)]
            elif way == 1:
                step = random_.choice((1, 2, 4, 64))
                values = [count * step for count in range(17)]
            elif way == 2:
                values = [random_.choice((2000, 2000, 2000, 10, -2000)) for count in range(17)]
            else:
                values = [2000] * 17
                values[random_.randrange(17)] = random_.choice((0, 5, 1000, 1001))
            tables.append(struct.pack("<17i", *values))
        t.check(
            "network_observer_update_voice_console_counts",
            Buffer(bytes(data)),
            globals={
                "unknown_004ce12c": tables[0],
                "unknown_004ce174": tables[1],
                "game_voice_enabled": bytes((random_.choice((0, 1, 1, 1, 0x80)),)),
                "unknown_004c9880": random_.randbytes(4),
                "unknown_004c9884": random_.randbytes(4),
            },
        )

    # ---------- a channel follows the state of its channel of the game

    def goes(harness, arguments):
        # the channel is in the state from then on, as it is in the game
        harness.mu.mem_write(arguments[1] + CHANNELS + arguments[2] * CHANNEL, struct.pack("<I", arguments[0]))
        return 0, (arguments[0], harness.normalize(arguments[1]), arguments[2])

    for n in range(1500):
        setup = at_time()
        data, pointers = with_owners()
        # the fourth function of what each session has is the one that is told
        struct.pack_into("<5I", data, SIZE + 16, 0x11, 0x22, 0x33, mock_address(0), 0x55)
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        game_channel = random_.choice((NONE, 0, 1, 2, 3, 4, 5, 5))
        state = random_.choice((0, 1, 2, 3, 4, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9))
        struct.pack_into("<I", data, at, state)
        data[at + 8] = random_.choice((0, 0, 1, 2, 2, 3, 4, 6, 0xF9, 0xFF))
        struct.pack_into("<h", data, at + 0xA, random_.choice((0, 1, 5, 0x7FFF, -1)))
        struct.pack_into("<II", data, at + 0xC, game_channel, random_.choice((7, 7, 8, NONE)))
        for window in (0x250, 0x360):
            struct.pack_into("<II", data, at + window, random_.choice((1, 2, 16, 32)), random_.randrange(32))
        channels = game_channels()
        for other in range(6):
            struct.pack_into("<I", channels, other * 0xF8 + 0x4C, random_.choice((7, 7, 8)))
            struct.pack_into("<i", channels, other * 0xF8 + 0x54, random_.choice((0, 1, 2, 2, 3, 4, 5, 5, 5, 6)))
            struct.pack_into("<I", channels, other * 0xF8 + 0x58, random_.choice((0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 9, 10, 0x100, NONE)))
        setup["globals"]["unknown_004d87d4"] = Buffer(bytes(channels))
        setup["globals"]["xbox_video_settings"] = struct.pack("<h", random_.choice((60, 50, 0, -1, 1))) + random_.randbytes(6)
        setup["globals"]["unknown_004e6398"] = struct.pack(
            "<Q", random_.choice((0, 1, 59, 60, 61, 0x100000000, 0x10000003B, 0xFFFFFFFFFFFFFFFF, random_.getrandbits(64)))
        )
        t.check(
            "network_observer_channel_update_game_channel",
            channel,
            Buffer(bytes(data), pointers=pointers),
            globals=setup["globals"],
            externals=dict(setup["externals"], network_observer_channel_forget_address=noted, network_observer_channel_set_state=goes),
            mocks=[Mock(lambda call: 0, stack_arguments=3, registers=("ecx",))],
        )

    # ---------- a channel tries again when it is time to

    for n in range(2000):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data, pointers = with_owners()
        # the third function of what each session has is the one that is asked
        struct.pack_into("<5I", data, SIZE + 16, 0x11, 0x22, mock_address(0), 0x44, 0x55)
        data += bytearray(random_.randbytes(0x70))
        parameters = SIZE + 36
        struct.pack_into("<I", data, 0x10, parameters)
        for tries in (0, 0x24, 0x48):
            struct.pack_into("<i8i", data, parameters + tries, random_.choice((0, 1, 3, 8, 8)), *(random_.choice((0, 1000, 5000)) for count in range(8)))
        struct.pack_into("<i", data, parameters + 0x6C, random_.choice((1000, 1000, 5000)))
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        struct.pack_into("<I", data, at, random_.choice((0, 1, 1, 1, 2, 2, 2, 3, 4, 4, 5, 5, 5, 6, 7, 8, 9, 9, 9)))
        struct.pack_into("<I", data, at + 4, (now - random_.choice((0, 999, 1000, 1001, 4999, 5000, 5001, 30000))) & 0xFFFFFFFF)
        data[at + 8] = random_.choice((0, 0, 1, 1, 4, 5, 0xFB, 0xFF))
        struct.pack_into("<h", data, at + 0xA, random_.choice((0, 0, 0, 1, 2, 3, 7, 8, 0x7FFF, -1)))
        struct.pack_into("<I", data, at + 0xC, random_.choice((NONE, 0, 1, 2, 3, 4, 5, 5, 5)))
        setup["globals"]["unknown_004d87d4"] = Buffer(bytes(game_channels()))
        # which of them still want the channel: the same each time that one is asked
        yes = random_.sample(range(4), random_.choice((0, 0, 1, 2, 4)))
        t.check(
            "network_observer_channel_update_tries",
            Buffer(bytes(data), pointers=pointers + (0x10,)),
            channel,
            globals=setup["globals"],
            externals=dict(
                setup["externals"],
                network_observer_channel_forget_address=noted,
                network_observer_channel_set_state=goes,
                network_observer_channel_update_connection=with_channel_and_observer,
                network_observer_channel_update_game_channel=with_channel_and_observer,
                network_observer_channel_unused_expired=lambda harness, arguments, expired=random_.choice((0, 1, 1, 0x100)): (
                    expired,
                    (arguments[0], harness.normalize(arguments[1]), arguments[2]),
                ),
                network_observer_channel_find_address=lambda harness, arguments, found=random_.choice((0, 1, 1, 0x100)): (
                    found,
                    (arguments[0], harness.normalize(arguments[1])),
                ),
                unknown_00088220=lambda harness, arguments: (
                    0,
                    (harness.normalize(arguments[0]), harness.normalize(arguments[1]), arguments[2]),
                ),
            ),
            mocks=[Mock(lambda call, yes=yes: (call.registers["ecx"] // 4) % 4 in yes, stack_arguments=2, registers=("ecx",))],
        )

    # ---------- how many packets a second, and whether they are few

    def video() -> bytes:
        # how many pictures a second the console shows, and the rest of what it is set to
        return struct.pack("<h", random_.choice((0, 50, 50, 60, -1, 49, 51))) + random_.randbytes(6)

    for n in range(800):
        data = observer() + bytearray(random_.randbytes(0x114))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<f", data, SIZE + 0x110, random_.choice((0.0, 0.5, 0.5, 1.0, 2.0)))
        t.check(
            "network_observer_packet_rate_low",
            random_.choice((0, 1)),
            f32(random_.choice((-1.0, 0.0, 12.4998, 12.4999, 12.5, 14.9998, 14.9999, 15.0, 24.9998, 24.9999, 25.0, 29.9998, 29.9999, 30.0, 59.0, 60.0))),
            Buffer(bytes(data), pointers=(0x10,)),
            random_.choice((0, 1)),
            random_.choice((0, 1)),
            globals={"xbox_video_settings": video()},
        )

    for n in range(1200):
        data = observer() + bytearray(random_.randbytes(0x114))
        struct.pack_into("<I", data, 0x10, SIZE)
        count = random_.choice((1, 2, 4, 8, 8))
        way = random_.randrange(3)
        if way == 0:
            rates = sorted((random_.choice((1.0, 0.5, 0.25, 0.1, 0.0)) for index in range(count)), reverse=True)
        elif way == 1:
            rates = [1.0 / (1 << index) for index in range(count)]
        else:
            rates = [random_.choice((1.0, 0.5, 0.25, 0.0)) for index in range(count)]
        struct.pack_into(f"<i{count}f", data, SIZE + 0x9C, count, *rates)
        struct.pack_into("<ii", data, SIZE + 0x108, random_.choice((0, 0, 45, 1000)), random_.choice((0, 90, 0x20000000)))
        t.check(
            "network_observer_packet_rate",
            Buffer(bytes(data), pointers=(0x10,)),
            random_.choice((0, 1, 360, 1350, 1349, 2700, 2699, 2250, 4500, 5400, 5399, 9000, 10800, 10799, 21600, 100000, 0x7FFFFFFF, NONE)),
            random_.choice((0, 0, 1)),
            random_.choice((0, 0, 1)),
            globals={"xbox_video_settings": video()},
        )

    # ---------- the flow of a channel

    def few(harness, arguments, low=None):
        # the first byte is the low one of a register
        return low, (arguments[0] & 0xFF, arguments[1], harness.normalize(arguments[2]), arguments[3], arguments[4])

    for n in range(300):
        data = observer()
        t.check(
            "network_observer_channel_set_flow",
            random_.randrange(15),
            Buffer(bytes(data)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            externals={"network_observer_packet_rate_low": lambda harness, arguments, low=random_.choice((0, 1)): few(harness, arguments, low)},
        )

    def flows(harness, arguments):
        return 0, (arguments[0], harness.normalize(arguments[1]), arguments[2], arguments[3], arguments[4])

    for n in range(200):
        data = observer()
        channel = random_.randrange(15)
        data[CHANNELS + channel * CHANNEL + 0x4F4] = random_.choice((0, 1, 1, 0x80))
        t.check(
            "network_observer_channel_restore_flow",
            channel,
            Buffer(bytes(data)),
            externals={"network_observer_channel_set_flow": flows},
        )

    for n in range(600):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data = observer() + bytearray(random_.randbytes(0x1D4))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<fif", data, SIZE + 0x1C8, random_.choice((0.0, 1000.0, 999.5, 999.5, 64000.0)), random_.choice((0, 1, 8, 8, 1000, 0x10000, -1)), random_.choice((0.0, 0.5, 1.0)))
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        struct.pack_into("<i", data, at + 0x494, random_.choice((0, 1, 1000, 64000, 0x7FFFFFFF, -1)))
        struct.pack_into("<I", data, at + 0x4DC, random_.choice((0, 1, 1, 1, 2, 3)))
        struct.pack_into("<I", data, at + 0x4E8, random_.choice((NONE, NONE, (now - random_.choice((0, 1, 124, 125, 126, 1000, 0x10000, 0x80000000))) & 0xFFFFFFFF)))
        t.check(
            "network_observer_channel_unknown_0007a2a0",
            channel,
            Buffer(bytes(data), pointers=(0x10,)),
            globals=setup["globals"],
            externals=setup["externals"],
        )

    # ---------- a channel is told of something of a size, and what is done with its flow each time

    for n in range(1000):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data = observer() + bytearray(random_.randbytes(0x180))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<i", data, SIZE + 0x170, random_.choice((0, 1, 2, 3)))
        struct.pack_into("<i", data, SIZE + 0x17C, random_.choice((0, 1, 2, 10)))
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        struct.pack_into("<II", data, at + 0x4B0, random_.choice((0, 50, 100, random_.getrandbits(32))), random_.choice((0, 16, 1000, random_.getrandbits(32))))
        data[at + 0x4C0] = random_.choice((0, 1, 1, 0x80))
        struct.pack_into("<I", data, at + 0x4C4, (now - random_.choice((0, 99, 100, 101, 499, 500, 501, 1000, 5000))) & 0xFFFFFFFF)
        struct.pack_into("<i", data, at + 0x4C8, random_.choice((0, 10, 50, 100, 500)))
        struct.pack_into("<i", data, at + 0x4D8, random_.choice((0, 1, 5, -1)))
        struct.pack_into("<I", data, at + 0x4E4, random_.choice((NONE, (now - random_.choice((0, 16, 1000))) & 0xFFFFFFFF)))
        t.check(
            "network_observer_channel_unknown_00079660",
            random_.choice((0, 50, 100, 100, random_.getrandbits(32))),
            random_.choice((0, 1, 1500, random_.getrandbits(32))),
            Buffer(bytes(data), pointers=(0x10,)),
            channel,
            random_.getrandbits(32),
            globals=setup["globals"],
            externals=dict(setup["externals"], network_observer_channel_set_flow=flows),
        )

    for n in range(1500):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data = observer() + bytearray(random_.randbytes(0x1F4))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<II", data, SIZE + 0x1D4, random_.choice((0, 5, 30)), random_.choice((0, 7, 60)))
        struct.pack_into("<ii", data, SIZE + 0x1EC, random_.choice((0, 3, 3)), random_.choice((1000, 1000, 0)))
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        data[at + 0x48E] = random_.choice((0, 0, 1, 0x80))
        struct.pack_into("<II", data, at + 0x4B0, random_.choice((0, 10, 50, 100, 130, 0x7FFFFFFF, 0x80000000)), random_.choice((0, 5, 10, 50)))
        data[at + 0x4C0] = random_.choice((0, 0, 0, 0, 1, 0x80))
        struct.pack_into("<i", data, at + 0x4D8, random_.choice((0, 0, 0, 0, 1, -1)))
        struct.pack_into("<I", data, at + 0x4DC, random_.choice((0, 0, 1, 1, 1, 2, 2, 3, 3)))
        struct.pack_into("<I", data, at + 0x4E0, random_.choice((NONE, (now - random_.choice((0, 999, 1000, 1001, 5000))) & 0xFFFFFFFF)))
        struct.pack_into("<I", data, at + 0x4F0, random_.choice((0, 10, 20, 45, 50, 100, 0x7FFFFFFF)))
        struct.pack_into("<ii", data, at + 0x518, random_.choice((0, 2, 3, 4)), random_.choice((0, 0, 0, 1, -1)))

        def tried(harness, arguments, was=random_.choice((0, 0, 1)), unknown=random_.choice((0, 1, 1, 0x80))):
            # a byte goes where the third thing says, which is in the stack
            harness.mu.mem_write(arguments[2], bytes((unknown,)))
            return was, (harness.normalize(arguments[0]), arguments[1])

        t.check(
            "network_observer_channel_update_flow",
            Buffer(bytes(data), pointers=(0x10,)),
            channel,
            globals=setup["globals"],
            externals=dict(
                setup["externals"],
                network_observer_channel_reset_unknown4dc=with_channel_and_observer,
                network_observer_channel_unknown_0007a1c0=lambda harness, arguments, how=random_.choice((0, 1, 2, 3)): (
                    how,
                    (harness.normalize(arguments[0]), arguments[1]),
                ),
                network_observer_channel_restore_flow=with_channel_and_observer,
                network_observer_channel_unknown_0007a160=with_channel_and_observer,
                network_observer_channel_lower_flow=lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2])),
                network_observer_channel_raise_flow=tried,
            ),
        )

    # ---------- the channel that has the most of something has its flow changed

    for n in range(800):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data = observer() + bytearray(random_.randbytes(0x1C0))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<ii", data, SIZE + 0x1B8, random_.choice((0, 5, 100)), random_.choice((1000, 1000, 0)))
        struct.pack_into("<I", data, 0x4F34, random_.choice((NONE, (now - random_.choice((0, 999, 1000, 1001, 5000, NONE))) & 0xFFFFFFFF)))
        for channel in range(15):
            at = CHANNELS + channel * CHANNEL
            data[at + 0x48C] = random_.choice((0, 1, 1, 0x80))
            data[at + 0x48E] = random_.choice((0, 0, 1, 0x80))
            struct.pack_into("<ii", data, at + 0x510, random_.choice((0, 5, 10, 50, 100, 105, 106, 1000, -1)), random_.choice((0, 5, 10, 50, 100, 105, 106)))
        t.check(
            "network_observer_channel_unknown_00079a10",
            random_.randrange(15),
            Buffer(bytes(data), pointers=(0x10,)),
            globals=setup["globals"],
            externals=dict(
                setup["externals"],
                network_observer_channel_lower_flow=lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2])),
            ),
        )

    # ---------- the flow of a channel goes down

    for n in range(600):
        data = observer() + bytearray(random_.randbytes(0x114))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<ii", data, SIZE + 0x108, random_.choice((0, 0, 45, 1000)), random_.choice((0, 90, 0x20000000)))
        t.check(
            "network_observer_packet_rate_bits_per_second",
            Buffer(bytes(data), pointers=(0x10,)),
            random_.choice((0, 0, 1)),
            f32(random_.choice((0.0, 1.0, 7.5, 12.5, 15.0, 29.97, 30.0, 0.001388, 0.004166, 0.006944, -1.0, 1e9, 1e30))),
        )

    def a_rate_of_packets(harness, arguments, rate=None):
        return f32(rate), (harness.normalize(arguments[0]), arguments[1], arguments[2], arguments[3])

    def a_flow(data: bytearray, at: int) -> None:
        # the flow of a channel, and the one that is held
        for flow in (0x494, 0x4CC):
            struct.pack_into("<ii", data, at + flow, random_.choice((1, 999, 1000, 8000, 8001, 64000, 100000, -8000)), random_.choice((0, 500, 4000, 4001, 32000)))
        struct.pack_into("<f", data, at + 0x49C, random_.choice((0.0, 5.0, 7.5, 15.5, 29.97, 30.0)))
        data[at + 0x48E] = random_.choice((0, 1))
        data[at + 0x490] = random_.choice((0, 1))
        data[at + 0x4C0] = random_.choice((0, 0, 1, 0x80))

    for n in range(1200):
        data = observer() + bytearray(random_.randbytes(0x19C))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<i", data, SIZE + 0xE0, random_.choice((0, 500, 4000)))
        struct.pack_into("<i", data, SIZE + 0x14C, random_.choice((0, 1000, 8000)))
        struct.pack_into("<if", data, SIZE + 0x194, random_.choice((1000, 100000)), random_.choice((0.1, 0.25, 0.5, 0.0)))
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        a_flow(data, at)
        data[at + 0x4F4] = random_.choice((0, 0, 1, 0x80))
        t.check(
            "network_observer_channel_lower_flow",
            Buffer(bytes(data), pointers=(0x10,)),
            channel,
            random_.choice((0, 0, 1)),
            externals={
                "network_observer_channel_unknown_00079a10": with_channel_and_observer,
                "network_observer_channel_restore_flow": with_channel_and_observer,
                "network_observer_packet_rate": lambda harness, arguments, rate=random_.choice((7.5, 15.0, 15.5, 29.97, 30.0)): a_rate_of_packets(harness, arguments, rate),
                "network_observer_channel_set_flow": flows,
                "network_observer_channel_reset_unknown4dc": with_channel_and_observer,
            },
        )

    for n in range(1200):
        setup = at_time()
        data = observer() + bytearray(random_.randbytes(0x18C))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<i", data, SIZE + 0xE0, random_.choice((0, 500, 4000)))
        struct.pack_into("<i", data, SIZE + 0x14C, random_.choice((0, 1000, 8000)))
        struct.pack_into("<fii", data, SIZE + 0x180, random_.choice((0.5, 0.75, 0.3)), random_.choice((1, 5)), random_.choice((3, 10)))
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        a_flow(data, at)
        struct.pack_into("<i", data, at + 0x4D8, random_.choice((0, 2, 5, 9, -3)))
        struct.pack_into("<I", data, at + 0x4DC, random_.choice((0, 1, 2, 3, 4)))
        struct.pack_into("<i", data, at + 0x4F0, random_.choice((0, 1, 33, 100, 1000, 30000)))
        t.check(
            "network_observer_channel_drop_flow",
            Buffer(bytes(data), pointers=(0x10,)),
            channel,
            globals=setup["globals"],
            externals=dict(
                setup["externals"],
                network_observer_channel_restore_flow=with_channel_and_observer,
                network_observer_packet_rate=lambda harness, arguments, rate=random_.choice((0.0, 7.5, 15.0, 29.97, 30.0)): a_rate_of_packets(harness, arguments, rate),
                network_observer_channel_set_flow=flows,
                network_observer_channel_lower_flow=lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2])),
            ),
        )

    # ---------- the flow of a channel goes up

    for n in range(1000):
        data = observer() + bytearray(random_.randbytes(0x114))
        struct.pack_into("<I", data, 0x10, SIZE)
        count = random_.choice((0, 1, 2, 3, 4, 5, 7, 8, 8))
        way = random_.randrange(3)
        if way == 0:
            rates = sorted((random_.choice((1.0, 0.5, 0.25, 0.1, 0.0)) for index in range(count)), reverse=True)
        elif way == 1:
            rates = [1.0 / (1 << index) for index in range(count)]
        else:
            rates = [random_.choice((1.0, 0.5, 0.25, 0.0)) for index in range(count)]
        struct.pack_into(f"<i{count}f", data, SIZE + 0x9C, count, *rates)
        t.check(
            "network_observer_next_packet_rate",
            f32(random_.choice((-1.0, 0.0, 1.0, 3.0, 3.75, 6.25, 7.5, 7.4999, 12.5, 14.9999, 15.0, 25.0, 29.9999, 30.0, 60.0))),
            Buffer(bytes(data), pointers=(0x10,)),
            random_.choice((0, 0, 1)),
            globals={"xbox_video_settings": video()},
        )

    for n in range(2500):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        # the observer goes on for two numbers more: how many channels are trying more flow, and how many can
        data = observer() + bytearray(struct.pack("<ii", random_.choice((0, 1, 2)), random_.choice((1, 2, 2)))) + bytearray(random_.randbytes(0x1A4))
        parameters = SIZE + 8
        struct.pack_into("<I", data, 0x10, parameters)
        data[parameters + 0x144] = random_.choice((0, 1, 0x80))
        struct.pack_into("<i", data, parameters + 0x150, random_.choice((8000, 64000, 100000, 100000)))
        struct.pack_into("<if", data, parameters + 0x18C, random_.choice((1000, 100000)), random_.choice((0.1, 0.25, 0.0)))
        struct.pack_into("<i", data, parameters + 0x1A0, random_.choice((1000, 1000, 1000, 0)))
        struct.pack_into("<I", data, 0x4F38, (now - random_.choice((0, 999, 1000, 1000, 1001, 5000, 5000))) & 0xFFFFFFFF)
        channel = random_.randrange(15)
        at = CHANNELS + channel * CHANNEL
        struct.pack_into("<ii", data, at + 0x494, random_.choice((1000, 1006, 8000, 8003, 64000, 64000, 100000, -8000)), random_.choice((0, 500, 4000, -4000)))
        rate = random_.choice((0.0, 7.5, 15.0, 15.0, 30.0))
        struct.pack_into("<f", data, at + 0x49C, rate)
        for byte in (0x48E, 0x490):
            data[at + byte] = random_.choice((0, 1))
        for byte in (0x4A0, 0x4A1, 0x4A2):
            data[at + byte] = random_.choice((0, 1, 1, 0x80))
        struct.pack_into("<I", data, at + 0x4A8, random_.choice((0, 1, 1500, 100000)))
        t.check(
            "network_observer_channel_raise_flow",
            Buffer(bytes(data), pointers=(0x10,)),
            channel,
            Buffer(random_.randbytes(1)),
            globals=setup["globals"],
            externals=dict(
                setup["externals"],
                network_observer_next_packet_rate=lambda harness, arguments, next=random_.choice((rate, rate, rate + 0.00005, rate + 0.0002, rate + 7.5, 30.0)): (
                    f32(next),
                    (arguments[0], harness.normalize(arguments[1]), arguments[2]),
                ),
                # the byte is the low one of a register
                network_observer_packet_rate_bits_per_second=lambda harness, arguments, bits=random_.choice((0, 1000, 20000, 200000)): (
                    bits,
                    (harness.normalize(arguments[0]), arguments[1] & 0xFF, arguments[2]),
                ),
                network_observer_channel_set_flow=flows,
            ),
        )

    # ---------- how the voice goes between the consoles, by the flow of the channels

    for n in range(1000):
        data = observer()
        chances = [random_.choice((0.0, 0.0, 0.08, 0.5)) for byte in range(5)]
        for channel in range(15):
            at = CHANNELS + channel * CHANNEL
            data[at + 0x48C] = random_.choice((0, 1, 1, 0x80))
            data[at + 0x48E] = random_.choice((0, 1, 1, 0x80))
            for chance, byte in zip(chances, (0x48F, 0x4C0, 0x50C, 0x50D, 0x50E)):
                data[at + byte] = random_.choice((1, 0x80)) if random_.random() < chance else 0
        t.check(
            "network_observer_update_voice_mode",
            Buffer(bytes(data)),
            globals={
                "unknown_004cf73c": bytes((random_.choice((0, 1, 1, 0x80)),)),
                "game_voice_enabled": bytes((random_.choice((0, 1, 1, 1, 0x80)),)),
                "unknown_004c987c": random_.randbytes(4),
            },
        )

    # ---------- each channel takes a channel of the game

    def taken(harness, arguments):
        # the name is in the stack: what it says is what counts, and which channel is taken goes by it
        name = bytes(harness.mu.mem_read(arguments[0], 0x100)).split(b"\0")[0]
        which = sum(name) % 8
        return (NONE if which > 5 else which), (name, arguments[1])

    for n in range(300):
        data = observer()
        for channel in range(15):
            # the address of the internet of its console
            data[CHANNELS + channel * CHANNEL + 0x14 : CHANNELS + channel * CHANNEL + 0x18] = bytes(
                random_.choice((0, 1, 9, 10, 127, 128, 192, 255)) for part in range(4)
            )
        t.check(
            "network_observer_take_game_channels",
            Buffer(bytes(data)),
            globals={"unknown_004d87d4": Buffer(bytes(game_channels()))},
            externals={
                "unknown_00082060": taken,
                "network_observer_channel_update_connection": with_channel_and_observer,
                "network_observer_channel_update_game_channel": with_channel_and_observer,
                "network_observer_channel_update_tries": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
            },
        )

    # ---------- the channel for a console

    for n in range(1200):
        setup = at_time()
        data, pointers = with_owners()
        # the fourth function of what each session has is the one that is told
        struct.pack_into("<5I", data, SIZE + 16, 0x11, 0x22, 0x33, mock_address(0), 0x55)
        data += bytearray(random_.randbytes(0x100))
        parameters = SIZE + 36
        struct.pack_into("<I", data, 0x10, parameters)
        struct.pack_into("<ii", data, parameters + 0xF8, random_.choice((1000, 2000, 20, 19)), random_.choice((1, 16, 32)))
        console = bytes(random_.choice((0, 1, 9, 10, 127, 128, 192, 255)) for part in range(4)) + random_.randbytes(0x20)
        way = random_.randrange(5)
        for channel in range(15):
            at = CHANNELS + channel * CHANNEL
            struct.pack_into("<I", data, at + 0x70, random_.choice((NONE, 5, 0)))
            if way >= 2:
                # all of them are in use
                struct.pack_into("<I", data, at, random_.choice((1, 2, 7)))
            if way == 3:
                # and sessions use them all
                data[at + 9] = random_.choice((1, 2, 0xF))
            if way == 2:
                data[at + 9] = random_.choice((0, 1, 1, 2, 0xF))
        if way == 0 or (way == 4 and n % 2):
            # one of them is of the console already, whether it is in use or not
            at = CHANNELS + random_.randrange(15) * CHANNEL
            data[at + 0x14 : at + 0x38] = console
            if n % 7 == 0:
                struct.pack_into("<I", data, at, 0)
        elif way == 1:
            # one of them is of a console that is the same but for its last byte
            at = CHANNELS + random_.randrange(15) * CHANNEL
            data[at + 0x14 : at + 0x38] = console[:0x23] + bytes((console[0x23] ^ 1,))
            struct.pack_into("<I", data, at, 2)
        t.check(
            "network_observer_acquire_channel",
            Buffer(bytes(data), pointers=pointers + (0x10,)),
            random_.randrange(4),
            Buffer(console),
            globals=dict(setup["globals"], unknown_004d87d4=Buffer(bytes(game_channels()))),
            externals=dict(
                setup["externals"],
                unknown_00082060=taken,
                network_observer_channel_forget_address=noted,
                unknown_000886e0=lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                transport_qos_delete=lambda harness, arguments: (0, (arguments[0],)),
            ),
            mocks=[Mock(lambda call: 0, stack_arguments=3, registers=("ecx",))],
        )

    # ---------- what the voice has to send by a channel

    # the session that the voice goes with: its sixth function says which console a channel is for
    voice_session = Buffer(struct.pack("<7I", 4, 0x11, 0x22, 0x33, 0x44, 0x55, mock_address(0)), pointers=(0,))

    def with_voice() -> dict:
        def a_session(harness, arguments, has=random_.choice((1, 1, 1, 0)), leaves=random_.choice((0, 1))):
            # the session goes where the thing says, which is in the stack; one can be left there with no session to give
            if has or leaves:
                harness.mu.mem_write(arguments[0], struct.pack("<I", voice_session.address))
            return has, ()

        voice = bytearray(random_.randbytes(0x90))
        voice[4] = random_.choice((1, 1, 1, 0, 0x80))
        return {
            "globals": {
                "game_voice_enabled": bytes((random_.choice((1, 1, 1, 0, 0x80)),)),
                "voice_globals": bytes(voice),
                "unknown_004c9888": struct.pack("<I", random_.choice((0, 1, 1, 2, 3))),
                "game_voice_session_source": struct.pack("<I", random_.choice((0, 1, 1, 2, 2, 3))),
            },
            "externals": {
                "network_life_cycle_get_session": a_session,
                "network_life_cycle_get_other_session": lambda harness, arguments: a_session(harness, arguments),
            },
            "mocks": [Mock(lambda call, peer=random_.choice((NONE, 0, 1, 5, 15)): peer, stack_arguments=1, registers=("ecx",))],
        }

    for n in range(500):
        setup = with_voice()
        t.check(
            "network_observer_voice_has_packet",
            random_.randrange(15),
            random_.choice((0, 0, 1)),
            Buffer(random_.randbytes(4)),
            globals=setup["globals"],
            externals=dict(
                setup["externals"],
                game_voice_unknown_00054890=lambda harness, arguments, so=random_.choice((0, 1)): (so, (arguments[0],)),
                game_voice_has_packet_for_peer=lambda harness, arguments, has=random_.choice((0, 1, 1)): (has, (arguments[0],)),
                game_voice_packet_size_for_peer=lambda harness, arguments, size=random_.choice((0, 37, 1200)): (size, (arguments[0],)),
            ),
            mocks=setup["mocks"],
            extra=[voice_session],
        )

    for n in range(400):
        setup = with_voice()
        t.check(
            "network_observer_voice_build_packet",
            random_.randrange(15),
            random_.choice((0, 37, 1200)),
            Buffer(random_.randbytes(16)),
            globals=setup["globals"],
            externals=dict(
                setup["externals"],
                game_voice_build_packet=lambda harness, arguments, size=random_.choice((0, 37, 1200)): (
                    size,
                    (arguments[0], harness.normalize(arguments[1]), arguments[2]),
                ),
            ),
            mocks=setup["mocks"],
            extra=[voice_session],
        )

    # ---------- what is done with the interval of the flow each time

    for n in range(1500):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data = observer() + bytearray(random_.randbytes(0x1B8))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<i", data, SIZE + 0x19C, random_.choice((1000, 1000, 0)))
        struct.pack_into("<ii", data, SIZE + 0x1A4, random_.choice((1000, 3000, 3000)), random_.choice((0, 1000, 100000)))
        struct.pack_into("<i", data, SIZE + 0x1B4, random_.choice((0, 1, 2, 3)))
        struct.pack_into("<I", data, 0x4E0C, random_.choice((NONE, NONE, 0, 5000, 100000)))
        struct.pack_into("<I", data, 0x4F38, (now - random_.choice((0, 999, 1000, 1001, 2999, 3000, 3001, 9000))) & 0xFFFFFFFF)
        data[0x4F3C] = random_.choice((0, 0, 1, 0x80))
        data[0x4F3D] = random_.choice((0, 0, 0, 1, 0x80))
        data[0x4F3E] = random_.choice((0, 0, 1, 0x80))
        held = random_.choice((0.0, 0.0, 0.0, 0.1, 0.5))
        for channel in range(15):
            at = CHANNELS + channel * CHANNEL
            # now and then, no channel counts
            data[at + 0x48C] = 0 if n % 12 == 0 else random_.choice((0, 1, 1, 0x80))
            data[at + 0x4A0] = random_.choice((0, 0, 1, 0x80))
            data[at + 0x4A1] = random_.choice((0, 0, 1, 0x80))
            struct.pack_into("<II", data, at + 0x4A4, random_.choice((0, 1, 125, 1500, 100000)), random_.choice((0, 1, 125, 126, 375, 1500, 100000)))
            struct.pack_into("<i", data, at + 0x4D8, random_.choice((1, 5)) if random_.random() < held else random_.choice((0, 0, -1)))
            struct.pack_into("<I", data, at + 0x4DC, random_.choice((0, 1, 2, 3, 4)))
        t.check(
            "network_observer_update_interval",
            Buffer(bytes(data), pointers=(0x10,)),
            globals=setup["globals"],
            externals=dict(
                setup["externals"],
                network_observer_channel_restore_flow=with_channel_and_observer,
                network_observer_unknown_00077f90=lambda harness, arguments: (
                    0,
                    (arguments[0], arguments[1], harness.normalize(arguments[2]), arguments[3]),
                ),
                network_observer_new_interval=lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            ),
        )

    # ---------- a console asks to be talked with

    for n in range(2500):
        data = observer()
        console = random_.randbytes(0x24)
        number = random_.choice((0, 5, 5, 0x1234, NONE))
        way = random_.randrange(6)
        found = random_.randrange(15)
        at = CHANNELS + found * CHANNEL
        if way != 0:
            # a channel is for the console; a session uses it, mostly, and it is in use, mostly
            data[at + 0x14 : at + 0x38] = console
            struct.pack_into("<I", data, at, 0 if way == 1 else random_.choice((2, 3, 7)))
            data[at + 9] = 0 if way == 2 else random_.choice((1, 2, 0xF))
            struct.pack_into("<I", data, at + 0xC, random_.randrange(6))
        channels = game_channels()
        for other in range(6):
            struct.pack_into("<I", channels, other * 0xF8 + 0x50, random_.choice((NONE, number, number, (number - 1) & 0xFFFFFFFF)))
            struct.pack_into("<i", channels, other * 0xF8 + 0x54, random_.choice((0, 1, 2, 3, 3, 5, 5)))
        address = bytearray(random_.randbytes(0x14))
        struct.pack_into("<h", address, 0x12, random_.choice((4, 4, 4, 16)))
        if n % 9 == 0:
            # this console's own address
            address[0:4] = bytes((1, 0, 0, 0x7F))
        # the address that the channel of the game talks with: this one, with any port, or another
        current = bytes(address)
        same = random_.choice((0, 1, 1, 2))
        if same == 0:
            current = random_.randbytes(0x12) + bytes(address[0x12:0x14])
        elif same == 2:
            current = bytes(address[0:0x10]) + random_.randbytes(2) + bytes(address[0x12:0x14])

        def known(harness, arguments, could=random_.choice((1, 1, 1, 1, 0)), console=console, values=(random_.getrandbits(32), random_.randbytes(8), random_.randbytes(16))):
            # what the console and its session are known by goes where each thing says, in the stack
            for where, value in zip(arguments[2:6], (struct.pack("<I", values[0]), values[1], values[2], console)):
                if where:
                    harness.mu.mem_write(where, value)
            return could, (bytes(harness.mu.mem_read(arguments[0], 0x14)), arguments[1], arguments[2] != 0, arguments[3] != 0, arguments[4] != 0, arguments[5] != 0)

        def talks_with(harness, arguments, current=current):
            harness.mu.mem_write(arguments[1], current)
            return 1, (harness.normalize(arguments[0]),)

        def closes(harness, arguments, does=random_.choice((1, 1, 0)), state=random_.choice((1, 2))):
            # a channel of the game that is told this is not past its second state any more, mostly
            if does:
                harness.mu.mem_write(arguments[0] + 0x54, struct.pack("<I", state))
            return 0, (harness.normalize(arguments[0]), arguments[1])

        t.check(
            "network_observer_handle_connect_request",
            Buffer(bytes(data)),
            Buffer(bytes(address)),
            Buffer(struct.pack("<II", number, random_.choice((0, 1, 2, 3, 0x15, 0x2A, 0x3F, 0x3F, 0x40, 0x80, 0x100, 0x13F, random_.getrandbits(6)))) + random_.randbytes(8)),
            globals={"unknown_004d87d4": Buffer(bytes(channels))},
            externals={
                "transport_address_get_connect_status": lambda harness, arguments, status=random_.choice((2, 2, 2, 2, 2, 0, 1, 4)): (
                    status,
                    (bytes(harness.mu.mem_read(arguments[0], 0x14)),),
                ),
                "unknown_000880b0": lambda harness, arguments, can=random_.choice((1, 1, 1, 0)): (can, (arguments[0],)),
                "transport_security_peer_address_from_address": known,
                "unknown_0007b140": lambda harness, arguments: (
                    0,
                    (arguments[0], harness.normalize(arguments[1]), arguments[2], arguments[3], bytes(harness.mu.mem_read(arguments[4], 8))),
                ),
                "network_channel_get_address": talks_with,
                "unknown_00088650": closes,
                "transport_address_connect": lambda harness, arguments: (1, (harness.normalize(arguments[0]),)),
                "unknown_00088220": lambda harness, arguments: (
                    0,
                    (harness.normalize(arguments[0]), harness.normalize(arguments[1]), arguments[2]),
                ),
                "network_observer_channel_set_state": goes,
                "network_observer_channel_update_game_channel": with_channel_and_observer,
                "network_observer_channel_update_connection": with_channel_and_observer,
                "unknown_00088360": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
            },
        )

    # ---------- two channels are compared, and the flow of a channel starts

    for n in range(200):
        values = [random_.choice((0.0, 1.0, 1.0, -1.0, 2.5, 1e9)) for channel in range(15)]
        t.check(
            "network_observer_compare_channels",
            random_.randrange(15),
            random_.randrange(15),
            Buffer(struct.pack("<15f", *values)),
        )

    for n in range(2500):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data = observer() + bytearray(random_.randbytes(0x170))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<i", data, SIZE + 0xE0, random_.choice((0, 500, 4000)))
        # the rates of packets, and the sizes of one
        struct.pack_into("<i4f", data, SIZE + 0x9C, 4, 1.0, 0.5, 0.25, 0.125)
        struct.pack_into("<iif", data, SIZE + 0x108, random_.choice((0, 45, 200)), random_.choice((0, 90)), random_.choice((0.5, 1.0)))
        struct.pack_into("<i", data, SIZE + 0x14C, random_.choice((1000, 4000)))
        struct.pack_into(
            "<5i",
            data,
            SIZE + 0x154,
            random_.choice((1000, 5000)),
            random_.choice((8000, 16000, 64000)),
            random_.choice((16000, 64000)),
            random_.choice((24000, 96000)),
            random_.choice((1000, 4000)),
        )
        struct.pack_into("<i", data, SIZE + 0x16C, random_.choice((30, 100, 1000)))
        data[0x4E01] = random_.choice((0, 0, 1, 0x80))
        struct.pack_into("<i", data, 0x4E04, random_.choice((8000, 32000, 32001, 128000, 1000000, 3, -5)))
        struct.pack_into("<I", data, 0x4F2C, (now - random_.choice((0, 999, 1000, 1001, 4999, 5000, 5001, 9000))) & 0xFFFFFFFF)
        few = random_.choice((0, 0, 1, 2))
        for channel in range(15):
            at = CHANNELS + channel * CHANNEL
            # with no other channel that has a flow, with one, or with many
            data[at + 0x48C] = random_.choice((0, 1, 1, 0x80)) if few == 0 else int(few == 2 and channel == 7)
            struct.pack_into("<ii", data, at + 0x494, random_.choice((1000, 8000, 8001, 16000, 64000)), random_.choice((500, 4000, 4001)))
            struct.pack_into("<I", data, at + 0xC, random_.randrange(6))
        channel = random_.randrange(15)
        # the channels of the game, and after them what some of them have at 0x3C
        channels = game_channels()
        things = len(channels)
        pointers = []
        for other in range(6):
            struct.pack_into("<I", channels, other * 0xF8 + 0x10, random_.randrange(3))
            thing = random_.choice((None, 0, 1))
            struct.pack_into("<I", channels, other * 0xF8 + 0x3C, 0 if thing is None else things + thing * 0x34)
            if thing is not None:
                pointers.append(other * 0xF8 + 0x3C)
            channels[other * 0xF8 + 0x48] = random_.choice((0, 8, 8, 0xFF, 0xF7))
            struct.pack_into("<i", channels, other * 0xF8 + 0x54, random_.choice((0, 2, 3, 5, 5, 5)))
        for thing in (0, 1, 0x80):
            block = bytearray(random_.randbytes(0x34))
            block[0x30] = thing
            channels += block
        own = bytearray(random_.randbytes(3 * 0x97C))
        for other in range(3):
            struct.pack_into("<i", own, other * 0x97C + 0x96C, random_.choice((0, 33, 100, 250, 5000)))
        t.check(
            "network_observer_channel_start_flow",
            channel,
            Buffer(bytes(data), pointers=(0x10,)),
            globals=dict(
                setup["globals"],
                unknown_004d87d4=Buffer(bytes(channels), pointers=tuple(pointers)),
                unknown_004d87d8=Buffer(bytes(own)),
                xbox_video_settings=video(),
            ),
            externals=setup["externals"],
        )

    # ---------- what the observer does with the flow of its channels each time

    for n in range(1500):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data, pointers = with_owners()
        # the second function of what each session has is the one that is asked
        struct.pack_into("<5I", data, SIZE + 16, 0x11, mock_address(0), 0x33, 0x44, 0x55)
        data += bytearray(random_.randbytes(0x1E0))
        parameters = SIZE + 36
        struct.pack_into("<I", data, 0x10, parameters)
        struct.pack_into("<i", data, parameters + 0x148, random_.choice((1000, 1000, 0)))
        struct.pack_into("<if", data, parameters + 0x1C0, random_.choice((1, 2, 4, 15)), random_.choice((0.0, 0.25, 0.5, 1.0)))
        struct.pack_into("<I", data, parameters + 0x1D4, random_.choice((0, 5, 30)))
        struct.pack_into("<I", data, parameters + 0x1DC, random_.choice((0, 7, 60)))
        data[0x4E00] = random_.choice((1, 1, 1, 0, 0x80))
        struct.pack_into("<I", data, 0x4F30, (now - random_.choice((0, 999, 1000, 1001, 5000))) & 0xFFFFFFFF)
        # the channels of the game, and after them what some of them have at 0x3C
        channels = game_channels()
        things = len(channels)
        game_pointers = []
        for other in range(6):
            thing = random_.choice((None, 0, 1, 2))
            struct.pack_into("<I", channels, other * 0xF8 + 0x3C, 0 if thing is None else things + thing * 0x34)
            if thing is not None:
                game_pointers.append(other * 0xF8 + 0x3C)
            struct.pack_into("<i", channels, other * 0xF8 + 0x54, random_.choice((0, 2, 3, 5, 5, 5, 5)))
        for thing in (0, 1, 0x80):
            block = bytearray(random_.randbytes(0x34))
            block[0x30] = thing
            channels += block
        for channel in range(15):
            at = CHANNELS + channel * CHANNEL
            game_channel = random_.randrange(6)
            struct.pack_into("<I", data, at + 0xC, game_channel)
            data[at + 0x48C] = random_.choice((0, 1, 1, 0x80))
            # the two bytes of the channel are as its channel of the game has it, mostly
            thing = struct.unpack_from("<I", channels, game_channel * 0xF8 + 0x3C)[0]
            has = int(thing != 0)
            byte = channels[thing + 0x30] if thing else 0
            data[at + 0x48D] = random_.choice((0, 1, 0x80))
            data[at + 0x48E] = has if random_.random() < 0.8 else random_.choice((0, 1, 0x80))
            data[at + 0x48F] = int(byte != 0) if random_.random() < 0.8 else random_.choice((0, 1, 0x80))
            struct.pack_into("<II", data, at + 0x4B0, random_.choice((0, 10, 50, 100, 130, 0x7FFFFFFF)), random_.choice((0, 5, 10, 50)))
            struct.pack_into("<I", data, at + 0x4DC, random_.choice((0, 1, 2, 3, 3)))
            data[at + 0x4F4] = random_.choice((0, 0, 1, 0x80))
            struct.pack_into("<I", data, at + 0x504, random_.choice((10, 20, 50, 100, 0x7FFFFFFF)))
        # the session that the voice goes with: its sixth function says which console a channel is for
        session = bytearray(random_.randbytes(0x7420))
        struct.pack_into("<7I", session, 0, 4, 0x11, 0x22, 0x33, 0x44, 0x55, mock_address(1))
        struct.pack_into("<I", session, 0x741C, random_.choice((0, 1, 1, 5)))
        other_session = bytearray(session)
        struct.pack_into("<I", other_session, 0x741C, random_.choice((0, 1, 1, 5)))
        voice = bytearray(random_.randbytes(0x90))
        voice[4] = random_.choice((1, 1, 1, 0, 0x80))
        values = [random_.choice((0.0, 1.0, 1.0, 2.5, -3.0, 100.0, 7.0)) for channel in range(15)]
        starts = random_.choice((1, 1, 1, 0))

        def started(harness, arguments, starts=starts):
            # a channel whose flow starts has one, mostly
            if starts:
                harness.mu.mem_write(arguments[1] + CHANNELS + arguments[0] * CHANNEL + 0x48C, b"\x01")
            return 0, (arguments[0], harness.normalize(arguments[1]))

        t.check(
            "network_observer_update_flow",
            Buffer(bytes(data), pointers=pointers + (0x10,)),
            globals=dict(
                setup["globals"],
                unknown_004d87d4=Buffer(bytes(channels), pointers=tuple(game_pointers)),
                game_voice_enabled=bytes((random_.choice((1, 1, 1, 0, 0x80)),)),
                voice_globals=bytes(voice),
                unknown_004c9888=struct.pack("<I", random_.choice((0, 1, 1, 2))),
                game_voice_session_source=struct.pack("<I", random_.choice((0, 1, 1, 2, 2, 3))),
                unknown_00527330=bytes((random_.choice((1, 1, 1, 0, 0x80)),)),
                network_life_cycle_session=Buffer(bytes(session), pointers=(0,)),
                network_life_cycle_other_session=Buffer(bytes(other_session), pointers=(0,)),
            ),
            externals=dict(
                setup["externals"],
                network_observer_channel_start_flow=started,
                game_voice_unknown_00054890=lambda harness, arguments: (arguments[0] % 2, (arguments[0],)),
                network_observer_channel_lower_flow=lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2])),
                network_observer_channel_unknown_0007a2a0=lambda harness, arguments, values=values: (
                    f32(values[arguments[0]]),
                    (arguments[0], harness.normalize(arguments[1])),
                ),
                network_observer_channel_update_flow=lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                network_observer_update_interval=lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            ),
            mocks=[
                # which of the sessions say so: the same each time that one is asked of a channel
                Mock(lambda call, yes=random_.getrandbits(15): (yes >> call.stack[0]) & 1, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call, peers=[random_.choice((NONE, 0, 1, 2, 3)) for channel in range(15)]: peers[call.stack[0]], stack_arguments=1, registers=("ecx",)),
            ],
        )

    # ---------- how much the observer lets its channels have between them

    for n in range(2500):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        data = observer() + bytearray(random_.randbytes(0x144))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<i", data, SIZE + 0xE0, random_.choice((0, 500, 4000)))
        # the rates of packets, and the sizes of one
        struct.pack_into("<i4f", data, SIZE + 0x9C, 4, 1.0, 0.5, 0.25, 0.125)
        struct.pack_into("<iif", data, SIZE + 0x108, random_.choice((0, 45, 200)), random_.choice((0, 90, 400)), random_.choice((0.5, 1.0, 2.0)))
        struct.pack_into("<if", data, SIZE + 0x100, random_.choice((1000, 5000)), random_.choice((0.05, 0.5, 1.0)))
        struct.pack_into(
            "<if5i",
            data,
            SIZE + 0x118,
            random_.choice((1000, 1000, 0)),
            random_.choice((0.25, 0.5, 1.0)),
            random_.choice((1, 3)),
            random_.choice((0, 64)),
            random_.choice((1000, 1000, 0)),
            random_.choice((1, 3)),
            random_.choice((3, 10)),
        )
        struct.pack_into("<3i", data, SIZE + 0x138, random_.choice((16, 64)), random_.choice((0, 32)), random_.choice((8, 64)))
        data[0x4E00] = random_.choice((1, 1, 1, 0, 0x80))
        struct.pack_into("<3i", data, 0x4E20, random_.choice((64, 256, 1024, 100000)), random_.choice((64, 256, 1024, 100000)), random_.choice((0, 1, 64, 256, 1024)))
        # what it has sent of late
        sent = a_rate(now)
        struct.pack_into("<I", sent, 0xD0, random_.choice((0, 1000, 16000, 64000, 1000000)))
        struct.pack_into("<f", sent, 0x24, random_.choice((1.0, 0.5, 0.05)))
        data[0x4E30 : 0x4E30 + len(sent)] = sent
        struct.pack_into("<I", data, 0x4F08, (now - random_.choice((0, 999, 1000, 1001, 5000, 5000))) & 0xFFFFFFFF)
        struct.pack_into("<I", data, 0x4F0C, (now - random_.choice((0, 999, 1000, 1001, 5000))) & 0xFFFFFFFF)
        struct.pack_into("<ii", data, 0x4F10, random_.choice((0, 1, 2, 5, 10)), random_.choice((0, 1, 2, 5, 10)))
        data[0x4F18] = random_.choice((0, 1, 0x80))
        # the channels of the game, and after them what some of them have at 0x3C
        channels = game_channels()
        things = len(channels)
        game_pointers = []
        for other in range(6):
            struct.pack_into("<I", channels, other * 0xF8 + 0x10, random_.randrange(3))
            thing = random_.choice((None, 0, 1, 2))
            struct.pack_into("<I", channels, other * 0xF8 + 0x3C, 0 if thing is None else things + thing * 0x34)
            if thing is not None:
                game_pointers.append(other * 0xF8 + 0x3C)
            channels[other * 0xF8 + 0x48] = random_.choice((0, 8, 8, 0xFF, 0xF7))
            struct.pack_into("<i", channels, other * 0xF8 + 0x54, random_.choice((0, 2, 3, 5, 5, 5)))
        for thing in (0, 1, 0x80):
            block = bytearray(random_.randbytes(0x34))
            block[0x30] = thing
            channels += block
        own = bytearray(random_.randbytes(3 * 0x97C))
        for other in range(3):
            struct.pack_into("<ii", own, other * 0x97C + 0x96C, random_.choice((0, 33, 100, 250)), random_.choice((0, 5, 40)))
        seventh = random_.choice((0.3, 0.6, 1.0))
        for channel in range(15):
            at = CHANNELS + channel * CHANNEL
            if random_.random() < seventh:
                struct.pack_into("<I", data, at, 7)
            # the second set of last numbers: how many, their sum and how old the oldest is
            struct.pack_into("<I", data, at + 0x360, random_.choice((1, 2, 16, 32, 32, 0)))
            struct.pack_into("<Ii", data, at + 0x468, random_.choice((0, 0, 1, 2, 8, 16, 32)), random_.choice((0, 999, 1000, 1001, 5000, 5001)))
        t.check(
            "network_observer_update_bandwidth",
            Buffer(bytes(data), pointers=(0x10,)),
            globals=dict(
                setup["globals"],
                unknown_004d87d4=Buffer(bytes(channels), pointers=tuple(game_pointers)),
                unknown_004d87d8=Buffer(bytes(own)),
                xbox_video_settings=video(),
            ),
            externals=setup["externals"],
        )

    # ---------- whether a channel of the game can send a packet now

    for n in range(4000):
        setup = at_time()
        data = observer() + bytearray(random_.randbytes(0xF0))
        struct.pack_into("<I", data, 0x10, SIZE)
        struct.pack_into("<3f", data, SIZE + 0x90, random_.choice((1.0, 4.0)), random_.choice((0.05, 0.1)), random_.choice((0.5, 1.0)))
        struct.pack_into("<3f", data, SIZE + 0xE4, random_.choice((0.25, 0.5)), random_.choice((0.5, 1.0)), random_.choice((0.125, 1.0)))
        frame = random_.choice((1000, 0x1_0000_0005, 0xFFFF_FFF0, random_.getrandbits(40)))
        # the channels of the game, and after them what some of them have at 0x3C
        channels = game_channels()
        things = len(channels)
        game_pointers = []
        for other in range(6):
            thing = random_.choice((None, None, 0, 1, 2))
            struct.pack_into("<I", channels, other * 0xF8 + 0x3C, 0 if thing is None else things + thing * 0x34)
            if thing is not None:
                game_pointers.append(other * 0xF8 + 0x3C)
            channels[other * 0xF8 + 0x48] = random_.choice((0, 8, 8, 0xFF, 0xF7))
            struct.pack_into("<i", channels, other * 0xF8 + 0x54, random_.choice((0, 2, 3, 5, 5, 5)))
        for thing in (0, 1, 0x80):
            block = bytearray(random_.randbytes(0x34))
            block[0x30] = thing
            channels += block
        for channel in range(15):
            at = CHANNELS + channel * CHANNEL
            # most channels are in use, each with a channel of the game of its own or with none
            struct.pack_into("<I", data, at, random_.choice((0, 1, 7, 7, 7)))
            struct.pack_into("<I", data, at + 0xC, channel if channel < 6 and random_.random() < 0.9 else NONE)
            # how long ago, in pictures, it could last send
            struct.pack_into("<Q", data, at + 0x478, (frame - random_.choice((0, 1, 2, 3, 4, 8, 59, 60, 61, 600, -5))) & 0xFFFFFFFFFFFFFFFF)
            # its part of what the observer lets have, and its flow
            struct.pack_into("<IfI", data, at + 0x480, random_.choice((NONE, 736, 744, 8000, 64000, 1000000)), random_.choice((-1.0, 0.0, 7.5, 15.0, 30.0)), random_.choice((NONE, 200, 1400, 100000)))
            data[at + 0x48C] = random_.choice((0, 1, 1, 0x80))
            data[at + 0x48D] = random_.choice((0, 1, 0x80))
            data[at + 0x48E] = random_.choice((0, 1, 0x80))
            struct.pack_into("<iif", data, at + 0x494, random_.choice((736, 744, 8000, 64000, 1000000, 0, -8)), random_.choice((200, 1400, 100000, 0)), random_.choice((0.0, 7.5, 15.0, 30.0, 60.0)))
            data[at + 0x4A1] = 0
            data[at + 0x4A2] = 0
            struct.pack_into("<I", data, at + 0x4DC, random_.choice((0, 1, 2, 2, 3)))

        def has_voice(harness, arguments, has=random_.choice((0, 0, 1, 1, 1)), size=random_.choice((0, 37, 300, 1400))):
            # how much the voice has goes where the third thing says, which is in the stack
            if has:
                harness.mu.mem_write(arguments[2], struct.pack("<I", size))
            return has, (arguments[0], arguments[1])

        t.check(
            "network_observer_game_channel_can_send",
            Buffer(bytes(data), pointers=(0x10,)),
            random_.choice((0, 1, 2, 3, 4, 5, 5, 6)),
            random_.choice((0, 0, 1)),
            random_.choice((0, 1)),
            random_.choice((0, 1)),
            Buffer(random_.randbytes(1)),
            Buffer(random_.randbytes(1)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            random_.choice((0, 100, 0x200, 2000)),
            Buffer(random_.randbytes(16)),
            globals=dict(
                setup["globals"],
                unknown_004d87d4=Buffer(bytes(channels), pointers=tuple(game_pointers)),
                xbox_video_settings=video(),
                unknown_004e6398=struct.pack("<Q", frame),
                unknown_004cf5d0=struct.pack("<f", random_.choice((0.0, 10.0, 30.0))),
            ),
            externals=dict(
                setup["externals"],
                network_observer_channel_unknown_00077940=lambda harness, arguments, urgent=random_.choice((0, 0, 0, 1)): (
                    urgent,
                    (harness.normalize(arguments[0]), arguments[1]),
                ),
                network_observer_voice_has_packet=has_voice,
                unknown_00089030=lambda harness, arguments, waiting=random_.choice((0, 16, 64, 127, 128, 512, 100000)): (
                    waiting,
                    (harness.normalize(arguments[0]),),
                ),
                unknown_00088fd0=lambda harness, arguments, sent=random_.choice((0, 100, 1000, 1400, 100000)): (sent, (harness.normalize(arguments[0]),)),
                unknown_00089000=lambda harness, arguments, count=random_.choice((0, 1, 1, 3, 20, -1)): (count, (harness.normalize(arguments[0]),)),
                network_observer_voice_build_packet=lambda harness, arguments, wrote=random_.choice((0, 0, 20, 100, 0x516, 0x517, 2000)): (
                    wrote,
                    (arguments[0], arguments[1], harness.normalize(arguments[2])),
                ),
            ),
        )
