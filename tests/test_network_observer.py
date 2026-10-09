# source/networking/network_observer.c

import random
import struct

from difftest import Buffer, Mock, mock_address

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
                "unknown_000783d0": finds,
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
                "unknown_000784a0": noted,
                "unknown_000886e0": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "unknown_0007b650": lambda harness, arguments: (0, (arguments[0],)),
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
                "unknown_000784a0": noted,
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
                "unknown_00076ff0": with_channel_and_observer,
                "unknown_000776a0": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
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
                "unknown_00076ff0": with_channel_and_observer,
                "unknown_000776a0": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                "network_observer_channel_unknown_000773a0": with_channel_and_observer,
                "network_observer_channel_update_probe": with_channel_and_observer,
                "unknown_000779f0": with_observer,
                "unknown_00078ac0": with_observer,
                "unknown_0007a4a0": with_observer,
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
                "unknown_0007b4c0": asks,
                "unknown_0007b6c0": lambda harness, arguments, done=random_.choice((1, 1, 0, 0x100)): (done, (arguments[0],)),
                "unknown_0007b7b0": found,
                "unknown_0007b650": lambda harness, arguments: (0, (arguments[0],)),
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
        setup["externals"]["unknown_00079660"] = lambda harness, arguments: (
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
            externals={"unknown_00079850": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1]))},
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
