# source/networking/network_channel.c

import random
import struct

from difftest import Buffer, GlobalData, Mock, mock_address

NONE = 0xFFFFFFFF
CHANNEL = 0xF8
OWN, OTHER = 0x97C, 0x2850


def run(t) -> None:
    random_ = random.Random(406)

    def channel() -> bytearray:
        data = bytearray(random_.randbytes(CHANNEL))
        struct.pack_into("<II", data, 0x10, random_.choice((NONE, 0, 1, 2)), random_.choice((NONE, 0, 1, 2)))
        data[0x48] = random_.choice((0, 8, 0x10, 0x18, 0xFF, 0xE7))
        struct.pack_into("<i", data, 0x54, random_.choice((0, 1, 2, 3, 4, 5, 5, 5)))
        return data

    def things() -> dict:
        # what the channels have of their own, of two kinds
        return {
            "unknown_004d87d8": Buffer(random_.randbytes(3 * OWN)),
            "unknown_004d87dc": Buffer(random_.randbytes(3 * OTHER)),
        }

    def at_time() -> dict:
        now = random_.getrandbits(32)
        return {
            "globals": {
                "unknown_00510548": bytes((random_.choice((0, 0, 1)),)),
                "unknown_0051054c": struct.pack("<I", now),
                "unknown_004e6398": random_.randbytes(8),
            },
            "externals": {"GetTickCount": lambda harness, arguments, now=now: now},
        }

    # ---------- the bits that a console asks to be talked with

    for flags in (*range(0x100), 0x100, 0x101, 0x80000000, 0xFFFFFF00, 0x10004, NONE):
        t.check("network_channel_flags_valid", flags)

    # ---------- numbers of what a channel has of its own

    for name in (
        "network_channel_unknown_00088fd0",
        "network_channel_unknown_00089000",
        "network_channel_unknown_00089030",
        "network_channel_unknown_00089070",
    ):
        for n in range(200):
            data = channel()
            # one that says so has what it says
            if struct.unpack_from("<I", data, 0x10)[0] == NONE:
                data[0x48] &= ~8 & 0xFF
            if struct.unpack_from("<I", data, 0x14)[0] == NONE:
                data[0x48] &= ~0x10 & 0xFF
            t.check(name, Buffer(bytes(data)), globals=things())

    # ---------- the times of a channel

    for n in range(100):
        setup = at_time()
        t.check("network_channel_reset_times", Buffer(bytes(channel())), globals=setup["globals"], externals=setup["externals"])
        t.check(
            "network_channel_set_time",
            random_.randrange(6),
            Buffer(bytes(channel())),
            globals=setup["globals"],
            externals=setup["externals"],
        )

    # ---------- a channel stops talking, and is let go of

    def with_thing(data: bytearray) -> tuple:
        # what the channel has at 0x3C goes after it, if it has it: its third number is a function
        pointers = []
        if random_.random() < 0.6:
            struct.pack_into("<I", data, 0x3C, len(data))
            pointers.append(0x3C)
            data += struct.pack("<3I", random_.getrandbits(32), random_.getrandbits(32), mock_address(0)) + random_.randbytes(0x28)
        else:
            struct.pack_into("<I", data, 0x3C, 0)
        return bytes(data), tuple(pointers)

    def sent(harness, arguments):
        # the message is in the stack: what it says is what counts
        return 0, (arguments[0], harness.normalize(arguments[1]), arguments[2], arguments[3], bytes(harness.mu.mem_read(arguments[4], 12)))

    for n in range(600):
        data, pointers = with_thing(channel())
        t.check(
            "network_channel_close",
            Buffer(data, pointers=pointers),
            random_.choice((0, 3, 6, 6, 0xD, 0x11, random_.getrandbits(32))),
            externals={
                "network_message_gateway_send": sent,
                "unknown_00092e00": lambda harness, arguments: (1, (arguments[0], arguments[1])),
            },
            mocks=[Mock(lambda call: 0, stack_arguments=1)],
        )

    for n in range(600):
        data = channel()

        def closed(harness, arguments, state=random_.choice((2, 2, 5))):
            # a channel that stops talking is in its second state, mostly
            harness.mu.mem_write(arguments[0] + 0x54, struct.pack("<I", state))
            return 0, (harness.normalize(arguments[0]), arguments[1])

        t.check(
            "network_channel_release",
            Buffer(bytes(data)),
            globals=things(),
            externals={
                "network_channel_close": closed,
                "unknown_00094bf0": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
        )

    # ---------- a channel says that it talks, is given what it talks with, and tries to talk

    def message(harness, arguments):
        # the message is in the stack: what it says is what counts
        return 0, (harness.normalize(arguments[0]), arguments[1], arguments[2], bytes(harness.mu.mem_read(arguments[3], arguments[2])))

    def to_address(harness, arguments):
        return 0, (arguments[0], harness.normalize(arguments[1]), arguments[2], arguments[3], bytes(harness.mu.mem_read(arguments[4], arguments[3])))

    for n in range(400):
        data = channel()
        struct.pack_into("<I", data, 0x14, random_.randrange(3))
        t.check(
            "network_channel_send_established",
            Buffer(bytes(data)),
            random_.choice((0, 1, 1, 0x80)),
            globals=things(),
            externals={"unknown_00095580": message, "network_message_gateway_send": to_address},
        )

    for n in range(400):
        setup = at_time()
        data = channel()
        struct.pack_into("<i", data, 0x54, random_.choice((2, 3, 3, 3, 4, 5)))
        t.check(
            "network_channel_accept",
            Buffer(bytes(data)),
            random_.getrandbits(32),
            globals=setup["globals"],
            externals=dict(
                setup["externals"],
                network_channel_reset_times=lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                network_channel_send_established=lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1] & 0xFF)),
            ),
        )

    for n in range(800):
        setup = at_time()
        now = struct.unpack("<I", setup["globals"]["unknown_0051054c"])[0]
        # what the channel has at 0xC goes after it
        data = channel() + bytearray(struct.pack("<3i", random_.choice((500, 2000)), random_.choice((0, 3, 3, 10)), random_.choice((5000, 5000, 0))))
        struct.pack_into("<I", data, 0xC, CHANNEL)
        struct.pack_into("<I", data, 0x88, (now - random_.choice((0, 4999, 5000, 5001, 0x80000000))) & 0xFFFFFFFF)
        struct.pack_into("<I", data, 0x8C, (now - random_.choice((0, 0, 1, 500, -1, -500))) & 0xFFFFFFFF)
        struct.pack_into("<i", data, 0x90, random_.choice((0, 2, 3, 4, 10, -1)))
        t.check(
            "network_channel_update_connecting",
            Buffer(bytes(data), pointers=(0xC,)),
            globals=setup["globals"],
            externals=dict(
                setup["externals"],
                network_channel_close=lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                network_message_gateway_send=to_address,
            ),
        )

    # ---------- a channel starts to talk, and a channel that was not in use is taken

    for n in range(800):
        setup = at_time()
        # what the channel is of goes after it: its second number is the next one that a channel gives
        data = channel() + bytearray(struct.pack("<II", random_.getrandbits(32), random_.choice((0, 7, 0xFFFFFFFE, NONE, random_.getrandbits(32)))))
        struct.pack_into("<I", data, 0x0, CHANNEL)
        struct.pack_into("<I", data, 0x48, random_.choice((0, 0x38, 0xF8, 0xC0, 0xFFFFFFFF)))
        address = bytearray(random_.randbytes(0x14))
        struct.pack_into("<h", address, 0x12, random_.choice((4, 4, 16)))
        if n % 5 == 0:
            # this console's own address
            address[0:4] = bytes((1, 0, 0, 0x7F))
        t.check(
            "network_channel_connect",
            Buffer(bytes(address)),
            Buffer(bytes(data), pointers=(0x0,)),
            random_.choice((0, 1, 1, 0x80)),
            globals=dict(setup["globals"], **things()),
            externals=dict(
                setup["externals"],
                unknown_00092d10=lambda harness, arguments, could=random_.choice((1, 1, 1, 0)): (
                    could,
                    (harness.normalize(arguments[0]), arguments[1], arguments[2], harness.normalize(arguments[3])),
                ),
                network_channel_close=lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                network_channel_update_connecting=lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                network_channel_reset_times=lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                unknown_00095cf0=lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                unknown_00094bf0=lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            ),
        )

    def takes(harness, arguments, which=None):
        return which, (arguments[0],)

    for n in range(600):
        t.check(
            "network_channel_new",
            random_.choice((0, 8, 0x10, 0x20, 0x38, 0x38, 0x18, 0x28, 0x30, 0xFF, random_.getrandbits(32))),
            random_.randrange(6),
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(12)),
            Buffer(bytes(channel())),
            globals=things(),
            externals={
                "network_channel_take_unknown10": lambda harness, arguments, which=random_.choice((0, 1, 2, 2, NONE)): takes(harness, arguments, which),
                "network_channel_take_unknown14": lambda harness, arguments, which=random_.choice((0, 1, 2, 2, NONE)): takes(harness, arguments, which),
                "network_channel_release": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
        )

    for n in range(400):
        count = random_.choice((0, 1, 6, 6, 6, -1))
        channels = bytearray()
        for other in range(6):
            data = channel()
            struct.pack_into("<i", data, 0x54, random_.choice((0, 0, 2, 5, 5, 5)) if n % 6 else 5)
            channels += data
        t.check(
            "network_channel_allocate",
            Buffer(b"10.0.0.5\0"),
            random_.choice((0x38, 0x38, 0, random_.getrandbits(32))),
            globals={
                "unknown_004d8ba0": bytes((random_.choice((1, 1, 1, 0, 0x80)),)),
                "unknown_004d87d0": struct.pack("<i", count),
                "unknown_004d87d4": Buffer(bytes(channels)),
                # the four things that every channel is given
                "unknown_00528000": GlobalData(bytes(4)),
                "unknown_00528b28": GlobalData(bytes(4)),
                "unknown_00529188": GlobalData(bytes(4)),
                "unknown_004cf6d4": GlobalData(bytes(12)),
            },
            externals={
                "network_channel_new": lambda harness, arguments, could=random_.choice((1, 1, 0)): (
                    could,
                    tuple(harness.normalize(argument) for argument in arguments),
                )
            },
        )

    # ---------- one of the things that a channel has of its own is taken

    for name, size, starts in (
        ("network_channel_take_unknown10", OWN, "unknown_00095cf0"),
        ("network_channel_take_unknown14", OTHER, "unknown_00094bf0"),
    ):
        for n in range(300):
            count = random_.choice((0, 1, 3, 3, 3, -1))
            table = bytearray(random_.randbytes(3 * size))
            for other in range(3):
                table[other * size + 4] = random_.choice((0, 1, 1, 0x80)) if n % 5 else 1
            globals_ = {
                "unknown_004d8ba0": bytes((random_.choice((1, 1, 1, 0, 0x80)),)),
                "unknown_004d87d0": struct.pack("<i", count),
                "unknown_00528000": GlobalData(bytes(0xB28)),
                "unknown_004d87d8": Buffer(random_.randbytes(3 * OWN)),
                "unknown_004d87dc": Buffer(random_.randbytes(3 * OTHER)),
            }
            # one of the second kind is left pointing at a place of what every channel is of
            globals_["unknown_004d87d8" if size == OWN else "unknown_004d87dc"] = Buffer(
                bytes(table), places=() if size == OWN else tuple(other * size + 0xC for other in range(3))
            )
            t.check(
                name,
                random_.getrandbits(32),
                globals=globals_,
                externals={starts: lambda harness, arguments: (0, (harness.normalize(arguments[0]),))},
            )