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
