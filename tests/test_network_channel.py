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
        # not a number that is the address of a function of source/, which would be taken for one
        while 0x20000000 <= now < 0x21000000:
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
                "network_message_queue_reset": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
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
            externals={"network_message_queue_send": message, "network_message_gateway_send": to_address},
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
                network_connection_reset=lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                network_message_queue_reset=lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
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
        ("network_channel_take_unknown10", OWN, "network_connection_reset"),
        ("network_channel_take_unknown14", OTHER, "network_message_queue_reset"),
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

    # ---------- the things that a channel has, one after another

    KINDS = (0, 1, 8, 0x19, 0x31, 0x39, 0xFF)

    for n in range(2500):
        data = channel()
        own = random_.choice((0, 1, 2, 3))
        struct.pack_into("<i", data, 0x20, own)
        for place in range(3):
            struct.pack_into("<II", data, 0x24 + place * 8, random_.choice(KINDS), random_.getrandbits(32))
        pointers = []
        if random_.random() < 0.7:
            # what the channel has at 0x3C goes after it, with things of its own
            struct.pack_into("<I", data, 0x3C, len(data))
            pointers.append(0x3C)
            thing = bytearray(random_.randbytes(0x34))
            struct.pack_into("<i", thing, 0xC, random_.choice((0, 1, 2, 4, 4)))
            for place in range(4):
                struct.pack_into("<II", thing, 0x10 + place * 8, random_.choice(KINDS), random_.getrandbits(32))
            data += thing
        else:
            struct.pack_into("<I", data, 0x3C, 0)
        index = random_.choice((NONE, NONE, 0, 1, 2, 3, 0x7FFFFFFF, 0x80000000, 0x80000001, 0x80000003, 0x80000004, 0xFFFFFFFE))
        t.check(
            "network_channel_parts_next",
            Buffer(bytes(data), pointers=tuple(pointers)),
            Buffer(struct.pack("<IIi", random_.choice((0, 0, 1, 8, 0x11, 0x30, 0x100)), index, random_.choice((0, 1, 5, -1))) + random_.randbytes(8)),
        )

    for n in range(200):
        count = random_.choice((0, 1, 2, 4))
        t.check(
            "network_channel_unknown3c_new",
            Buffer(random_.randbytes(0x34)),
            random_.choice((0, 1, 0x80)),
            Buffer(random_.randbytes(16)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            count if n % 10 else -1,
            Buffer(random_.randbytes(16)),
        )

    # ---------- what happens to a channel's thing of the first kind is seen to

    for n in range(1200):
        setup = at_time()
        data = channel()
        struct.pack_into("<I", data, 0x10, random_.randrange(3))
        data[0x48] = random_.choice((8, 8, 8, 0x18, 0x38, 0, 0x10))
        struct.pack_into("<i", data, 0x54, random_.choice((3, 4, 4, 5)))
        pointers = []
        # after the channel: its observer, a thing with functions, and their tables
        owner, part, owner_table, part_table = (len(data) + offset for offset in (0, 4, 8, 0x18))
        tail = struct.pack("<II", owner_table, part_table)
        tail += struct.pack("<4I", 0x11, 0x22, mock_address(0), mock_address(1))
        tail += struct.pack("<8I", 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, mock_address(2), mock_address(3))
        pointers += [owner, part]
        if random_.random() < 0.8:
            struct.pack_into("<I", data, 0x40, owner)
            pointers.append(0x40)
        else:
            struct.pack_into("<I", data, 0x40, 0)
        # the things of the channel: of some kinds, and all of them the same thing
        count = random_.choice((0, 1, 2, 3))
        struct.pack_into("<i", data, 0x20, count)
        for place in range(3):
            struct.pack_into("<II", data, 0x24 + place * 8, random_.choice((1, 4, 8, 0xC, 0x19, 0x31, 0x3D)), part)
            pointers.append(0x24 + place * 8 + 4)
        struct.pack_into("<I", data, 0x3C, 0)
        data += tail
        # the things of the first kind: each keeps how many times it does what it does, and what happened to it
        own = bytearray(random_.randbytes(3 * OWN))
        for other in range(3):
            struct.pack_into("<i", own, other * OWN + 0x100, random_.choice((0, 0, 1, 3)))
            struct.pack_into("<i", own, other * OWN + 0x104, 0)
            events = [random_.choice((1, 1, 2, 3, 4, 4, 5)) for event in range(random_.choice((0, 1, 2, 4)))] + [0]
            struct.pack_into(f"<{len(events)}i", own, other * OWN + 0x108, *events)
            struct.pack_into("<i", own, other * OWN + 0x968, random_.choice((0, 100, 101, 400)))

        def does(harness, arguments):
            # as many times as the thing says, and one less each time
            left = struct.unpack("<i", harness.mu.mem_read(arguments[0] + 0x100, 4))[0]
            if left > 0:
                harness.mu.mem_write(arguments[0] + 0x100, struct.pack("<i", left - 1))
            return left > 0, (harness.normalize(arguments[0]), arguments[1])

        def happened(harness, arguments):
            # the next of what the thing has, with three numbers that go where the last things say, in the stack
            at = struct.unpack("<i", harness.mu.mem_read(arguments[0] + 0x104, 4))[0]
            harness.mu.mem_write(arguments[0] + 0x104, struct.pack("<i", at + 1))
            kind = struct.unpack("<i", harness.mu.mem_read(arguments[0] + 0x108 + at * 4, 4))[0]
            for place, value in zip(arguments[1:4], (0x1000 + at, 0x2000 + at, (99, 100, 150, 151, 500)[at % 5])):
                harness.mu.mem_write(place, struct.pack("<I", value))
            return kind, (harness.normalize(arguments[0]),)

        t.check(
            "network_channel_update_unknown10",
            Buffer(bytes(data), pointers=tuple(pointers)),
            globals=dict(
                setup["globals"],
                unknown_004d87d8=Buffer(bytes(own)),
                unknown_004d87dc=Buffer(random_.randbytes(3 * OTHER)),
                unknown_004cf728=struct.pack("<f", random_.choice((1.0, 1.5, 2.0))),
                unknown_004cf72c=struct.pack("<i", random_.choice((0, 50, 100))),
            ),
            externals=dict(setup["externals"], network_connection_drop_oldest_sent=does, network_connection_next_event=happened),
            mocks=[
                Mock(lambda call: 0, stack_arguments=3, registers=("ecx",)),
                # the two bytes that it is told are the low ones of what is in the stack
                Mock(lambda call: 0, stack_arguments=3, registers=("ecx",), record=lambda call: (call.registers["ecx"], call.stack[0], call.stack[1] & 0xFF, call.stack[2] & 0xFF)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=2, registers=("ecx",), record=lambda call: (call.registers["ecx"], call.stack[0], call.stack[1] & 0xFF)),
            ],
        )

    # ---------- what a channel has itself writes itself in a packet, and reads itself

    def a_stream(size: int, position: int, blank: bool = False) -> Buffer:
        data = bytearray(random_.randbytes(size))
        if blank:
            for bit in range(max(position, 0), size * 8):
                data[bit >> 3] &= ~(1 << (bit & 7))
        header = struct.pack("<I4iB3x7i", 0x34, size, 1, 1, position, 0, *(random_.getrandbits(8) for n in range(7)))
        return Buffer(header + bytes(data) + random_.randbytes(8), pointers=(0,))

    for n in range(300):
        size = random_.choice((1, 4, 8))
        position = random_.choice((0, 1, 7, 8, size * 8 - 1, size * 8, size * 8 + 3))
        part = bytearray(random_.randbytes(8))
        part[4] = random_.choice((0, 1, 1, 0x80))
        part[5] = random_.choice((0, 1, 1))
        t.check(
            "network_channel_unknown18_write",
            Buffer(bytes(part)),
            random_.getrandbits(32),
            a_stream(size, position, blank=True),
            random_.getrandbits(32),
            random_.getrandbits(32),
        )
        t.check("network_channel_unknown18_read", Buffer(bytes(part)), random_.getrandbits(32), a_stream(size, position))
    # ---------- a packet that came by a channel is read

    def put_bits(data: bytearray, position: int, value: int, count: int) -> None:
        for bit in range(count):
            at = position + bit
            if at >> 3 < len(data):
                data[at >> 3] = data[at >> 3] & ~(1 << (at & 7)) | ((value >> bit) & 1) << (at & 7)

    for n in range(1600):
        setup = at_time()
        look_only = random_.choice((0, 0, 0, 1))
        data = channel()
        struct.pack_into("<II", data, 0x10, random_.randrange(3), random_.randrange(3))
        data[0x48] = random_.choice((0, 8, 0x10, 0x18, 0x18, 0xFF, 0xE7))
        pointers = []
        # after the channel: its observer and its table, the table of the things, and four things. Each thing has, after
        # its functions, how it says that its reading went, the number that it gives, and how many bits it reads
        crafted = random_.random() < 0.6
        owner, owner_table, part_table, parts = (len(data) + offset for offset in (0, 4, 0x14, 0x34))
        tail = struct.pack("<I", owner_table)
        tail += struct.pack("<4I", 0x11, mock_address(0), 0x33, 0x44)
        tail += struct.pack("<8I", 0x11, 0x22, 0x33, 0x44, 0x55, mock_address(1), 0x77, 0x88)
        pointers.append(owner)
        went = random_.choice(((0,), (0,), (0,), (0, 0, 0, 1), (0, 0, 0, 2), (0, 0, 0, 3), (0, 1, 2, 3)))
        for place in range(4):
            pointers.append(parts + place * 0x10)
            tail += struct.pack(
                "<4I",
                part_table,
                random_.choice(went),
                random_.choice((NONE, 0, 7, random_.getrandbits(32))),
                0 if crafted else random_.choice((0, 0, 1, 5, 8)),
            )
        if random_.random() < 0.85:
            struct.pack_into("<I", data, 0x40, owner)
            pointers.append(0x40)
        else:
            struct.pack_into("<I", data, 0x40, 0)
        # the things of the channel, and what it has at 0x3C, with its own
        struct.pack_into("<i", data, 0x20, random_.choice((0, 1, 2, 3)))
        for place in range(3):
            struct.pack_into("<II", data, 0x24 + place * 8, random_.choice((1, 1, 0, 4, 0x19, 0x31, 0x30)), parts + random_.randrange(4) * 0x10)
            pointers.append(0x24 + place * 8 + 4)
        if random_.random() < 0.6:
            second = len(data) + len(tail)
            struct.pack_into("<I", data, 0x3C, second)
            pointers.append(0x3C)
            tail += struct.pack("<B3x3I", 1, random_.getrandbits(32), 0x99, random_.choice((0, 1, 2, 4)))
            for place in range(4):
                tail += struct.pack("<II", random_.choice((1, 1, 0, 4, 0x19, 0x30)), parts + random_.randrange(4) * 0x10)
                pointers.append(second + 0x10 + place * 8 + 4)
            tail += bytes((random_.choice((0, 1)), random_.choice((0, 0, 1, 2)), 0, 0))
        else:
            struct.pack_into("<I", data, 0x3C, 0)
        data += tail
        # the packet. One that is made on purpose has, where it is read from, the bit that says that there is more and
        # how many bits, and ends just there or a byte before or after; the things read nothing of it
        if crafted:
            more = random_.choice((0, 1, 1, 1, 1))
            bit_count = random_.choice((0, 1, 7, 8, 0x100, 0x2FFF, 0x3000, 0x3001, 0x3FFF, 0x2000))
            start = 32 if not look_only else random_.choice((0, 3, 32))
            start += -(start + 15 + bit_count) % 8
            size = (start + 15 + bit_count) // 8 + random_.choice((0, 0, -1, 1, -2))
            packet = bytearray(random_.randbytes(max(size, 1)))
            size = len(packet)
            put_bits(packet, start, more | bit_count << 1, 15)
            if not look_only:
                # read from the start, after the word that only the game for testing does not have
                put_bits(packet, 0, random_.getrandbits(32) | 1, 32)
                put_bits(packet, 32, random_.getrandbits(start - 32), start - 32)
                if start > 32:
                    # the things read nothing, so what is read is from the start: the bit is there
                    start = 32
                    put_bits(packet, start, more | bit_count << 1, 15)
        else:
            size = random_.choice((1, 3, 4, 5, 8, 0x40, 0x700))
            packet = bytearray(random_.randbytes(size))
            start = random_.choice((0, 0, 1, 31, 32, size * 8 - 1, size * 8, size * 8 + 1))
            if random_.random() < 0.15:
                packet[:4] = (b"gbed" + bytes(4))[: max(size, 4)][:4] if size >= 4 else packet[:4]
            elif random_.random() < 0.1:
                packet[:4] = b"debg"[: len(packet[:4])]
        header = struct.pack(
            "<I4iB3x7i", 0x34, size, 1, random_.choice((1, 3, 5)), start, random_.choice((0, 0, 0, 1)) if look_only else random_.choice((0, 1)), *(random_.getrandbits(8) for bit in range(7))
        )
        stream = Buffer(header + bytes(packet) + random_.randbytes(8), pointers=(0,))
        # the things of the second kind: each keeps how many messages it has whole
        other = bytearray(random_.randbytes(3 * OTHER))
        for thing in range(3):
            struct.pack_into("<i", other, thing * OTHER + 0x100, random_.choice((0, 0, 1, 3)))

        def reads(call):
            # one of the channel's things reads its own: it gives its number and goes on in the packet
            mu = t.harness.mu
            went_, number, bits = struct.unpack("<3I", mu.mem_read(call.registers["ecx"] + 4, 12))
            if number != NONE:
                mu.mem_write(call.stack[0], struct.pack("<I", number))
            position = struct.unpack("<i", mu.mem_read(call.stack[1] + 0x10, 4))[0]
            mu.mem_write(call.stack[1] + 0x10, struct.pack("<i", position + bits))
            return went_

        def read_record(call):
            # the number that the things before gave is in the stack
            return call.registers["ecx"], bytes(t.harness.mu.mem_read(call.stack[0], 4)), call.stack[1]

        def whole(harness, arguments):
            # the next message of the thing: its kind, its size and what it says
            left = struct.unpack("<i", harness.mu.mem_read(arguments[0] + 0x100, 4))[0]
            if left > 0:
                harness.mu.mem_write(arguments[0] + 0x100, struct.pack("<i", left - 1))
                harness.mu.mem_write(arguments[1], struct.pack("<I", 0x10 + left))
                harness.mu.mem_write(arguments[2], struct.pack("<I", 0x200 + left))
                harness.mu.mem_write(arguments[3], bytes((left,)) * 0x20)
            return left > 0, (harness.normalize(arguments[0]),)

        t.check(
            "network_channel_read_packet",
            stream,
            Buffer(bytes(data), pointers=tuple(pointers)),
            random_.getrandbits(32),
            look_only,
            globals=dict(
                setup["globals"],
                unknown_004d87d8=Buffer(random_.randbytes(3 * OWN)),
                unknown_004d87dc=Buffer(bytes(other)),
            ),
            externals=dict(
                setup["externals"],
                network_connection_drop_oldest_sent=lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                network_connection_next_event=lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                network_connection_received=lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1]))),
                network_message_queue_receive=whole,
                unknown_00093aa0=lambda harness, arguments: (
                    0,
                    (bytes(harness.mu.mem_read(arguments[0], 0x20)), arguments[1], arguments[2], arguments[3], arguments[4]),
                ),
            ),
            mocks=[
                Mock(lambda call: 0, stack_arguments=2, registers=("ecx",)),
                Mock(reads, stack_arguments=2, registers=("ecx",), record=read_record),
            ],
        )

    # ---------- what is done each time with a channel

    for n in range(2000):
        setup = at_time()
        now = setup["externals"]["GetTickCount"](None, None)
        data = channel()
        struct.pack_into("<II", data, 0x10, random_.randrange(3), random_.randrange(3))
        data[0x48] = random_.choice((0, 8, 8, 0x18, 0x10, 0xFF, 0xE7))
        struct.pack_into("<i", data, 0x54, random_.choice((0, 2, 3, 3, 4, 4, 4, 5, 5, 5, 5, 6)))
        data[0x84] = random_.choice((0, 1, 1, 0x80))
        # how long it has been in its fourth state, by what it has at 0xC: just enough, or not
        timeout = random_.choice((0, 100, 5000))
        struct.pack_into("<I", data, 0x94, (now - timeout - random_.choice((-1, 0, 0, 1, 1, 1000))) & 0xFFFFFFFF)
        struct.pack_into("<I", data, 0x88, (now - random_.choice((0, 50, 5000))) & 0xFFFFFFFF)
        struct.pack_into("<I", data, 0x8C, (now - random_.choice((-50, 0, 50))) & 0xFFFFFFFF)
        struct.pack_into("<i", data, 0x90, random_.choice((0, 1, 5)))
        pointers = [0xC]
        # after the channel: what it has at 0xC, its observer and its table, the table of the things, and four things.
        # Each thing has, after its functions: whether the channel is to stop and why, and whether it has something to
        # send and whether that is urgent
        tries, owner, owner_table, part_table, parts = (len(data) + offset for offset in (0, 0x10, 0x14, 0x2C, 0x4C))
        struct.pack_into("<I", data, 0xC, tries)
        tail = struct.pack("<4i", random_.choice((10, 100)), random_.choice((1, 3)), random_.choice((1000, 10000)), timeout)
        tail += struct.pack("<I", owner_table)
        tail += struct.pack("<6I", 0x11, 0x22, 0x33, 0x44, 0x55, mock_address(0))
        tail += struct.pack("<8I", 0x11, mock_address(1), mock_address(2), 0x44, 0x55, 0x66, 0x77, 0x88)
        pointers.append(owner)
        stops = random_.choice((0, 0, 0, 1))
        for place in range(4):
            pointers.append(parts + place * 0x14)
            tail += struct.pack(
                "<5I",
                part_table,
                random_.choice((0, 0, 0, 1, 0x100)) if stops else 0,
                random_.choice((6, 9, 0x11, NONE, random_.getrandbits(32))),
                random_.choice((0, 0, 1, 0x100)),
                random_.choice((0, 0, 1, 0x100)),
            )
        if random_.random() < 0.7:
            struct.pack_into("<I", data, 0x40, owner)
            pointers.append(0x40)
        else:
            struct.pack_into("<I", data, 0x40, 0)
        struct.pack_into("<i", data, 0x20, random_.choice((0, 1, 2, 3)))
        kinds = (1, 1, 0, 0x10, 0x11, 0x21, 0x20, 0x31, 0x19)
        for place in range(3):
            struct.pack_into("<II", data, 0x24 + place * 8, random_.choice(kinds), parts + random_.randrange(4) * 0x14)
            pointers.append(0x24 + place * 8 + 4)
        if random_.random() < 0.6:
            second = len(data) + len(tail)
            struct.pack_into("<I", data, 0x3C, second)
            pointers.append(0x3C)
            tail += struct.pack("<B3x3I", 1, random_.getrandbits(32), mock_address(3), random_.choice((0, 1, 2, 4)))
            for place in range(4):
                tail += struct.pack("<II", random_.choice(kinds), parts + random_.randrange(4) * 0x14)
                pointers.append(second + 0x10 + place * 8 + 4)
            tail += bytes((random_.choice((0, 1)), random_.choice((0, 0, 1, 2)), 0, 0))
        else:
            struct.pack_into("<I", data, 0x3C, 0)
        data += tail

        def stops_(call):
            # one of the channel's things says whether the channel is to stop, and why
            mu = t.harness.mu
            stop, reason = struct.unpack("<2I", mu.mem_read(call.registers["ecx"] + 4, 8))
            # with NONE it gives no reason: the one that there was is the one
            if stop and reason != NONE:
                mu.mem_write(call.stack[0], struct.pack("<I", reason))
            return stop

        def has(call):
            # and whether it has something to send, and whether that is urgent: a byte, written only when it is
            mu = t.harness.mu
            something, urgent = struct.unpack("<2I", mu.mem_read(call.registers["ecx"] + 0xC, 8))
            if urgent:
                mu.mem_write(call.stack[0], b"\x01")
            return something

        def can_send(
            call,
            can=random_.choice((0, 1, 1, 1, 0x100)),
            unknown10=random_.choice((0, 0, 1)),
            wanted=random_.choice((0, 1)),
            size=random_.choice((0x30, 0x100, 0x516)),
            voice_size=random_.choice((0, 0, 1, 0x80, NONE)),
            voice=random_.randbytes(0x200),
        ):
            # the observer says whether the channel can send, and gives the rest
            mu = t.harness.mu
            mu.mem_write(call.stack[4], bytes((unknown10,)))
            mu.mem_write(call.stack[5], bytes((wanted,)))
            mu.mem_write(call.stack[6], struct.pack("<I", size))
            mu.mem_write(call.stack[7], struct.pack("<I", voice_size))
            mu.mem_write(call.stack[9], voice)
            return can

        def can_send_record(call):
            # the three bytes are the low ones of what is in the stack, and the rest are places in it
            return call.registers["ecx"], call.stack[0], call.stack[1] & 0xFF, call.stack[2] & 0xFF, call.stack[3] & 0xFF, call.stack[8]

        def writes(harness, arguments):
            # the stream that the packet is written in is in the stack, and so are the three numbers: a byte of each
            mu = harness.mu
            stream_ = bytes(mu.mem_read(arguments[2], 0x1C))
            return 0, (
                arguments[0] & 0xFF,
                harness.normalize(arguments[1]),
                stream_[4:0x15],
                stream_[0x18:0x1C],
                arguments[3] & 0xFF,
                arguments[4],
                bytes(mu.mem_read(arguments[5], 0x200)) if arguments[5] else None,
                *(bytes(mu.mem_read(arguments[place], 1)) for place in (6, 7, 8)),
            )

        t.check(
            "network_channel_update",
            Buffer(bytes(data), pointers=tuple(pointers)),
            globals=dict(
                setup["globals"],
                unknown_004d87d8=Buffer(random_.randbytes(3 * OWN)),
                unknown_004d87dc=Buffer(random_.randbytes(3 * OTHER)),
            ),
            externals=dict(
                setup["externals"],
                network_message_gateway_send=lambda harness, arguments: (
                    0,
                    (arguments[0], harness.normalize(arguments[1]), arguments[2], arguments[3], bytes(harness.mu.mem_read(arguments[4], arguments[3]))),
                ),
                unknown_00092e00=lambda harness, arguments: (1, (arguments[0], arguments[1])),
                network_channel_write_packet=writes,
                network_connection_drop_oldest_sent=lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                network_connection_next_event=lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            ),
            mocks=[
                Mock(can_send, stack_arguments=10, registers=("ecx",), record=can_send_record),
                Mock(stops_, stack_arguments=1, registers=("ecx",), record=lambda call: (call.registers["ecx"],)),
                Mock(has, stack_arguments=1, registers=("ecx",), record=lambda call: (call.registers["ecx"],)),
                Mock(lambda call: 0, stack_arguments=1),
            ],
        )

    # ---------- a channel writes a packet, and sends it

    for n in range(2000):
        setup = at_time()
        data = channel()
        struct.pack_into("<II", data, 0x10, random_.randrange(3), random_.randrange(3))
        pointers = []
        size = random_.choice((1, 5, 8, 0x10, 0x30, 0x33, 0x100, 0x516, 0x600))
        bits = size * 8
        crafted = random_.random() < 0.5
        # after the channel: its observer and its table, the table of the things, and four things. Each thing has,
        # after its functions: the bits that it says that it needs, how many more than it is given it writes, and what
        # it says when it has written
        owner, owner_table, part_table, parts = (len(data) + offset for offset in (0, 4, 0x14, 0x34))
        tail = struct.pack("<I", owner_table)
        tail += struct.pack("<4I", mock_address(0), 0x22, 0x33, 0x44)
        tail += struct.pack("<8I", 0x11, 0x22, 0x33, mock_address(1), mock_address(2), 0x66, 0x77, 0x88)
        pointers.append(owner)
        for place in range(4):
            pointers.append(parts + place * 0x10)
            if crafted:
                # one thing, that leaves so many bits
                needs = max(bits - random_.choice((0, 1, 2, 15, 16, 17, 18, 100)), 0)
                more = random_.choice((0, 0, 0, 0, 1, 2, -1 & 0xFFFFFFFF))
            else:
                needs = random_.choice((0, 0, 1, 8, 13, 64, bits // 4, bits // 2, bits - 1, bits, NONE))
                more = random_.choice((0, 0, 0, 0, 1, 8, 64, -1 & 0xFFFFFFFF))
            tail += struct.pack("<4I", part_table, needs, more, random_.choice((0, 0, 1, 0x100)))
        if random_.random() < 0.85:
            struct.pack_into("<I", data, 0x40, owner)
            pointers.append(0x40)
        else:
            struct.pack_into("<I", data, 0x40, 0)
        kinds = (1, 1, 1, 0, 4, 0x19, 0x31, 0x30)
        struct.pack_into("<i", data, 0x20, 1 if crafted else random_.choice((0, 1, 2, 3)))
        for place in range(3):
            struct.pack_into("<II", data, 0x24 + place * 8, 1 if crafted else random_.choice(kinds), parts + random_.randrange(4) * 0x10)
            pointers.append(0x24 + place * 8 + 4)
        if not crafted and random_.random() < 0.6:
            second = len(data) + len(tail)
            struct.pack_into("<I", data, 0x3C, second)
            pointers.append(0x3C)
            tail += struct.pack("<B3x3I", 1, random_.getrandbits(32), 0x99, random_.choice((0, 1, 2, 4)))
            for place in range(4):
                tail += struct.pack("<II", random_.choice(kinds), parts + random_.randrange(4) * 0x10)
                pointers.append(second + 0x10 + place * 8 + 4)
            tail += bytes((random_.choice((0, 1)), random_.choice((0, 0, 1, 2)), 0, 0))
        else:
            struct.pack_into("<I", data, 0x3C, 0)
        data += tail
        header = struct.pack("<I4iB3x7i", 0x34, size, random_.choice((1, 8, 3)), random_.choice((0, 1, 3)), random_.getrandbits(8), random_.choice((0, 1)), *(random_.getrandbits(8) for bit in range(7)))
        stream = Buffer(header + random_.randbytes(size) + random_.randbytes(8), pointers=(0,))

        def needs_(call):
            return struct.unpack("<I", t.harness.mu.mem_read(call.registers["ecx"] + 4, 4))[0]

        def writes_(call):
            # the thing writes its bits, and some more or one less: the stream goes on
            mu = t.harness.mu
            more_, said = struct.unpack("<2I", mu.mem_read(call.registers["ecx"] + 8, 8))
            position = struct.unpack("<I", mu.mem_read(call.stack[1] + 0x10, 4))[0]
            mu.mem_write(call.stack[1] + 0x10, struct.pack("<I", (position + call.stack[2] + more_) & 0xFFFFFFFF))
            return said

        def number(harness, arguments, number_=random_.choice((NONE, 0, 1, 7, random_.getrandbits(31)))):
            return number_, (harness.normalize(arguments[0]), arguments[1])

        def sends(harness, arguments, sent_=random_.choice((0, 0, 1, 0x40, NONE))):
            # the packet is what the stream has, of the size that it has now
            mu = harness.mu
            stream_ = bytes(mu.mem_read(arguments[1], 0x34))
            size_ = struct.unpack_from("<i", stream_, 4)[0]
            mu.mem_write(arguments[5], struct.pack("<I", sent_))
            return 0, (
                arguments[0],
                harness.normalize(arguments[1]),
                stream_[4:0x15],
                bytes(mu.mem_read(arguments[1] + 0x34, max(min(size_, size + 8), 0))),
                arguments[2],
                arguments[3],
                arguments[4],
            )

        t.check(
            "network_channel_write_packet",
            random_.choice((0, 1, 1)),
            Buffer(bytes(data), pointers=tuple(pointers)),
            stream,
            random_.choice((0, 1, 1, 0x100)),
            random_.choice((0, 1, 0x80, NONE)),
            random_.getrandbits(32),
            *(Buffer(random_.randbytes(4)) if random_.random() < 0.8 else 0 for place in range(3)),
            globals=dict(
                setup["globals"],
                unknown_004d87d8=Buffer(random_.randbytes(3 * OWN)),
                unknown_004d87dc=Buffer(random_.randbytes(3 * OTHER)),
            ),
            externals=dict(
                setup["externals"],
                network_connection_new_sequence=number,
                network_connection_set_sent_size=lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1]), arguments[2])),
                unknown_000931a0=sends,
            ),
            mocks=[
                Mock(lambda call: 0, stack_arguments=3, registers=("ecx",), record=lambda call: (call.registers["ecx"], call.stack[0], call.stack[1], call.stack[2] & 0xFF)),
                Mock(needs_, stack_arguments=2, registers=("ecx",)),
                Mock(writes_, stack_arguments=4, registers=("ecx",)),
            ],
        )

    # ---------- the messages that say that the other console talks with a channel, or does not any more

    for n in range(1200):
        channels = bytearray()
        for index in range(3):
            channels += channel()
        index = random_.randrange(3)
        mine, theirs = struct.unpack_from("<2I", channels, index * CHANNEL + 0x4C)
        first = random_.choice((mine, mine, mine, mine ^ 1, random_.getrandbits(32)))
        second = random_.choice((theirs, theirs, theirs ^ 1, random_.getrandbits(32)))
        stubs = {
            "network_channel_close": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
            # the address is in the stack: what it says is what counts
            "network_channel_connect": lambda harness, arguments: (
                0,
                (bytes(harness.mu.mem_read(arguments[0], 0x14)), harness.normalize(arguments[1]), arguments[2] & 0xFF),
            ),
            "network_channel_accept": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
        }
        t.check(
            "network_channel_handle_established",
            index,
            Buffer(struct.pack("<2I", first, second)),
            globals={"unknown_004d87d4": Buffer(bytes(channels))},
            externals=stubs,
        )
        t.check(
            "network_channel_handle_closed",
            index,
            Buffer(struct.pack("<3I", first, second, random_.getrandbits(32))),
            globals={"unknown_004d87d4": Buffer(bytes(channels))},
            externals=stubs,
        )
