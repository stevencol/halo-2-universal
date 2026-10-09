# source/networking/network_observer.c

import random
import struct

from difftest import Buffer

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
            externals={"unknown_00076ef0": lambda harness, arguments: 0},
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
