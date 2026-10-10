# source/networking/network_link.c

import random
import struct

from difftest import Buffer, Mock, mock_address

NONE = 0xFFFFFFFF
CHANNEL = 0xF8
CHANNELS = 6


def run(t) -> None:
    random_ = random.Random(493)

    for n in range(3000):
        # a few addresses, so that the ones of the list are often the one looked for; some of no size, or of less
        pool = []
        for size in (4, 4, 4, 16, 16, 0, 0x8000):
            address = bytearray(random_.randbytes(0x14))
            address[1:4] = bytes(3)
            struct.pack_into("<H", address, 0x12, size)
            pool.append(bytes(address))
        pool.append(pool[0][:0x10] + random_.randbytes(2) + pool[0][0x12:])
        pool.append(pool[3][:0x12] + struct.pack("<H", 4))

        def channels() -> Buffer:
            data = bytearray(random_.randbytes(CHANNELS * CHANNEL))
            for index in range(CHANNELS):
                data[index * CHANNEL + 0x48] = random_.choice((0, 0x40, 0x80, 0xC0, 0xFF, 0x10))
                struct.pack_into("<i", data, index * CHANNEL + 0x4C, random_.choice((1, 2, 3)))
                struct.pack_into("<i", data, index * CHANNEL + 0x54, random_.choice((0, 1, 2, 3, 4, 5, 5)))
            return Buffer(bytes(data))

        def link() -> Buffer:
            data = bytearray(random_.randbytes(0x224))
            struct.pack_into("<i", data, 0x20, random_.choice((0, 1, 2, 5, 15, 16, 16, -1)))
            for index in range(16):
                struct.pack_into("<I", data, 0x24 + index * 0x20, random_.randrange(CHANNELS))
                data[0x30 + index * 0x20:0x44 + index * 0x20] = random_.choice(pool)
            return Buffer(bytes(data))

        kind = random_.choice((0, 1, 2, 2, 1, 3, NONE))
        t.check(
            "network_link_find_channel",
            link(),
            kind,
            Buffer(random_.choice(pool)),
            globals={"unknown_004d87d4": channels()},
        )
        t.check(
            "network_link_channel_of_address",
            link(),
            Buffer(random_.choice(pool)),
            kind,
            globals={"unknown_004d87d4": channels()},
        )
        t.check("network_link_remove_channel", link(), random_.randrange(CHANNELS + 1))
        t.check(
            "network_link_add_channel",
            link(),
            random_.randrange(CHANNELS),
            random_.getrandbits(32),
            Buffer(random_.choice(pool)),
            globals={"unknown_004d87d4": channels()},
            externals={"network_channel_release": lambda harness, arguments: (0, (harness.normalize(arguments[0]),))},
        )
        t.check(
            "network_link_close_channels",
            link(),
            globals={"unknown_004d87d4": channels()},
            externals={
                "network_channel_close": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
            },
        )

    # ---------- its sockets are closed, and its channels go on

    for n in range(1500):
        data = bytearray(random_.randbytes(0x300))
        there = []
        for index in range(4):
            if random_.random() < 0.75:
                struct.pack_into("<I", data, 0xC + index * 4, 0x280 + index * 0x10)
                there.append(0xC + index * 4)
                struct.pack_into("<I", data, 0x280 + index * 0x10, random_.choice((NONE, 5, 0x1234, random_.getrandbits(32))))
            else:
                struct.pack_into("<I", data, 0xC + index * 4, 0)
        t.check(
            "network_link_close_sockets",
            Buffer(bytes(data), pointers=tuple(there)),
            globals={
                "unknown_004d8b18": bytes((random_.choice((1, 1, 1, 0)),)),
                "unknown_004d8b19": bytes((random_.choice((1, 1, 1, 0)),)),
            },
            externals={
                "shutdown": lambda harness, arguments, failed=random_.choice((0, 0, NONE)): (failed, tuple(arguments)),
                "closesocket": lambda harness, arguments, failed=random_.choice((0, 0, NONE)): (failed, tuple(arguments)),
                "WSAGetLastError": lambda harness, arguments: 10038,
                "VirtualFree": lambda harness, arguments, freed=random_.choice((1, 1, 0)): (
                    freed,
                    (harness.normalize(arguments[0]), arguments[1], arguments[2]),
                ),
                "GetLastError": lambda harness, arguments: 5,
            },
        )

        data = bytearray(random_.randbytes(0x224))
        struct.pack_into("<i", data, 0x20, random_.choice((0, 1, 2, 5, 16, -1)))
        for index in range(16):
            struct.pack_into("<I", data, 0x24 + index * 0x20, random_.randrange(CHANNELS))
            data[0x2C + index * 0x20] = random_.choice((0, 0, 1, 0x80))
        channels = bytearray(random_.randbytes(CHANNELS * CHANNEL))
        for index in range(CHANNELS):
            struct.pack_into("<i", channels, index * CHANNEL + 0x54, random_.choice((0, 1, 2, 3, 4, 5, 5)))
        t.check(
            "network_link_update_channels",
            Buffer(bytes(data)),
            globals={"unknown_004d87d4": Buffer(bytes(channels))},
            externals={
                "network_channel_update": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "network_channel_close": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
            },
        )

    # ---------- a packet, and the bytes that are sent for it

    for n in range(2000):
        packet = bytearray(random_.randbytes(0x824))
        struct.pack_into("<I", packet, 0, random_.choice((0, 1, 2, 3, 3, 4, NONE)))
        struct.pack_into("<I", packet, 0x1C, random_.choice((0, 1, 7, 8, 9, 0x123, 0x5FF, 0x600, random_.randrange(0x601))))
        struct.pack_into("<I", packet, 0x620, random_.choice((0, 0, 1, 0x1FF, 0x200, random_.randrange(0x201))))
        t.check("network_link_packet_pack", Buffer(bytes(packet)), Buffer(random_.randbytes(0x810)), Buffer(random_.randbytes(4)))
        # the size on the wire is asked of any number, a negative one too
        struct.pack_into("<i", packet, 0x1C, random_.choice((0, 1, 7, 8, 9, 0x600, -1, -8, -9, random_.randrange(-0x700, 0x700))))
        t.check("network_link_packet_wire_size", Buffer(bytes(packet)))

        data = bytearray(random_.randbytes(0x810))
        own = random_.choice((0, 1, 0x40, 0x5FF, 0x600, 0x600, 0x601, 0xFFFF, random_.randrange(0x601)))
        struct.pack_into("<H", data, 0, own)
        t.check(
            "network_link_packet_unpack",
            random_.choice(
                (
                    own + 2,
                    own + 2,
                    own + 2 + random_.randrange(0x201),
                    own + 2 + 0x200,
                    own + 2 + 0x201,
                    own + 1,
                    own,
                    0,
                    1,
                    2,
                    0x600,
                    0x601,
                    0xFFFFFFFF,
                    0x80000000,
                    random_.randrange(0x810),
                )
            ),
            Buffer(bytes(packet)),
            Buffer(bytes(data)),
        )

    # ---------- packets are sent

    LOOPBACK = struct.pack("<I", 0x7F000001) + bytes(12) + struct.pack("<HH", 0x1234, 4)

    def an_address() -> bytes:
        address = bytearray(random_.randbytes(0x14))
        address[1:4] = bytes(3)
        struct.pack_into("<H", address, 0x12, random_.choice((4, 4, 16, 0)))
        return random_.choice((bytes(address), bytes(address), LOOPBACK, LOOPBACK[:0x12] + struct.pack("<H", 16)))

    def read(harness, address: int, size: int) -> bytes:
        return bytes(harness.mu.mem_read(address, size)) if size else b""

    for n in range(1500):
        pool = [an_address() for index in range(3)]
        channels = bytearray(random_.randbytes(CHANNELS * CHANNEL))
        for index in range(CHANNELS):
            channels[index * CHANNEL + 0x48] = random_.choice((0, 0x40, 0x80, 0xC0, 0xFF, 0x10))
            struct.pack_into("<i", channels, index * CHANNEL + 0x54, random_.choice((0, 1, 2, 3, 4, 5, 5)))
            channels[index * CHANNEL + 0x70:index * CHANNEL + 0x84] = random_.choice(pool)

        def link() -> Buffer:
            data = bytearray(random_.randbytes(0x588))
            there = []
            for index in range(4):
                if random_.random() < 0.8:
                    struct.pack_into("<I", data, 0xC + index * 4, 0x500 + index * 0x10)
                    there.append(0xC + index * 4)
                else:
                    struct.pack_into("<I", data, 0xC + index * 4, 0)
            struct.pack_into("<i", data, 0x20, random_.choice((0, 1, 5, 16)))
            for index in range(16):
                struct.pack_into("<I", data, 0x24 + index * 0x20, random_.randrange(CHANNELS))
                data[0x30 + index * 0x20:0x44 + index * 0x20] = random_.choice(pool)
            return Buffer(bytes(data), pointers=tuple(there))

        size = random_.choice((1, 2, 0x40, 0x518, random_.randrange(1, 0x519)))
        t.check(
            "network_link_send_bytes",
            link(),
            random_.randrange(4),
            Buffer(random_.choice(pool)),
            size,
            Buffer(random_.randbytes(0x518)),
            globals={"unknown_004d87d4": Buffer(bytes(channels))},
            externals={
                "transport_endpoint_write_to": lambda harness, arguments, sent=random_.choice((size, size, 0xFFFF, 0xFFFF, 0, 5)): (
                    sent,
                    (
                        read(harness, arguments[0], 0x14),
                        harness.normalize(arguments[1]),
                        read(harness, arguments[2], arguments[3]),
                        arguments[3],
                    ),
                ),
            },
        )

        def a_packet() -> bytes:
            packet = bytearray(random_.randbytes(0x824))
            struct.pack_into("<I", packet, 0, random_.randrange(4))
            packet[8:0x1C] = random_.choice(pool)
            struct.pack_into("<I", packet, 0x1C, random_.choice((0, 0, 1, 0x123, 0x516, 0x517, 0x600, random_.randrange(0x601))))
            struct.pack_into("<I", packet, 0x620, random_.choice((0, 0, 0, 1, 0x200, random_.randrange(0x201))))
            return bytes(packet)

        send_bytes = lambda harness, arguments: (
            0,
            (
                harness.normalize(arguments[0]),
                arguments[1],
                harness.normalize(arguments[2]),
                arguments[3],
                read(harness, arguments[4], arguments[3]),
            ),
        )
        t.check(
            "network_link_send_packet",
            Buffer(a_packet()),
            link(),
            externals={
                "network_rate_update": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "network_link_send_bytes": send_bytes,
            },
        )

        send_packet = lambda harness, arguments: (0, (read(harness, arguments[0], 0x824), harness.normalize(arguments[1])))
        stream_size = random_.choice((0, 1, 0x123, 0x600, 0x600, 0x601, 0xFFFFFFFF, random_.randrange(0x601)))
        stream = Buffer(struct.pack("<II", 0x40, stream_size) + random_.randbytes(0x38 + 0x600), pointers=(0,))
        t.check(
            "network_link_send_to_address",
            Buffer(random_.choice(pool)),
            stream,
            link(),
            random_.choice((Buffer(random_.randbytes(4)), 0)),
            externals={"network_link_send_packet": send_packet},
        )
        stream = Buffer(struct.pack("<II", 0x40, stream_size) + random_.randbytes(0x38 + 0x600), pointers=(0,))
        t.check(
            "network_link_send_to_channel",
            random_.randrange(CHANNELS),
            random_.choice((stream, stream, stream, 0)),
            link(),
            random_.choice((0, 0, 1, 0x80, 0x200, 0x200, 0x201, 0xFFFFFFFF, random_.randrange(0x201))),
            Buffer(random_.randbytes(0x200)),
            random_.choice((Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4)), 0)),
            globals={"unknown_004d87d4": Buffer(bytes(channels))},
            externals={"network_link_send_packet": send_packet},
        )

    # ---------- packets are read

    from test_async import calls_so_far

    def stream_of(harness, address: int) -> tuple:
        # what a stream that reads has, up to where the function sets it
        raw = bytes(harness.mu.mem_read(address, 0x1C))
        data, size, unknown8, unknownc, position = struct.unpack_from("<5I", raw)
        return harness.normalize(data), size, unknown8, unknownc, position, raw[0x14], struct.unpack_from("<I", raw, 0x18)[0]

    for n in range(1500):
        pool = [an_address() for index in range(3)]
        channels = bytearray(random_.randbytes(CHANNELS * CHANNEL))
        for index in range(CHANNELS):
            channels[index * CHANNEL + 0x48] = random_.choice((0, 0x40, 0x80, 0xC0, 0xFF, 0x10))
            struct.pack_into("<i", channels, index * CHANNEL + 0x54, random_.choice((0, 1, 2, 3, 4, 5, 5)))

        def link(sockets: bool) -> Buffer:
            data = bytearray(random_.randbytes(0x588))
            there = []
            for index in range(4):
                if sockets and random_.random() < 0.8:
                    struct.pack_into("<I", data, 0xC + index * 4, 0x500 + index * 0x10)
                    there.append(0xC + index * 4)
                else:
                    struct.pack_into("<I", data, 0xC + index * 4, 0)
            # what takes the packets of no channel, with its functions, of which the first is asked
            if random_.random() < 0.8:
                struct.pack_into("<I", data, 0x1C, 0x560)
                struct.pack_into("<I", data, 0x560, 0x570)
                struct.pack_into("<I", data, 0x570, mock_address(0))
                there += [0x1C, 0x560]
            else:
                struct.pack_into("<I", data, 0x1C, 0)
            struct.pack_into("<i", data, 0x20, random_.choice((0, 1, 5, 16)))
            for index in range(16):
                struct.pack_into("<I", data, 0x24 + index * 0x20, random_.randrange(CHANNELS))
                data[0x30 + index * 0x20:0x44 + index * 0x20] = random_.choice(pool)
            return Buffer(bytes(data), pointers=tuple(there))

        packet = bytearray(random_.randbytes(0x824))
        struct.pack_into("<I", packet, 0, random_.choice((0, 1, 2, 3, 3, 4)))
        packet[8:0x1C] = random_.choice(pool)
        struct.pack_into("<i", packet, 0x1C, random_.choice((0, 0, 1, 0x123, 0x600, random_.randrange(0x601))))
        struct.pack_into("<i", packet, 0x620, random_.choice((0, 0, 1, 0x200, random_.randrange(0x201))))
        t.check(
            "network_link_receive_packet",
            Buffer(bytes(packet)),
            link(False),
            globals={"unknown_004d87d4": Buffer(bytes(channels))},
            externals={
                "network_rate_update": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "network_channel_read_packet": lambda harness, arguments: (
                    1,
                    (stream_of(harness, arguments[0]), harness.normalize(arguments[1]), arguments[2], arguments[3]),
                ),
                "game_voice_read_packet": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                "network_channel_set_time": lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1]))),
            },
            mocks=[
                Mock(
                    lambda call: 0,
                    stack_arguments=2,
                    registers=("ecx",),
                    record=lambda call: (
                        t.harness.normalize(call.registers["ecx"]),
                        t.harness.normalize(call.stack[0]),
                        stream_of(t.harness, call.stack[1]),
                    ),
                )
            ],
        )

        # what comes for the sockets, one thing each time that one is asked: bytes from an address, or nothing
        script = []
        for index in range(random_.choice((0, 1, 2, 3, 6))):
            own = random_.randrange(0x40)
            data = struct.pack("<H", random_.choice((own, own, own, 0x700))) + random_.randbytes(own + random_.randrange(0x20))
            address = bytearray(random_.choice(pool))
            if random_.random() < 0.2:
                address[0:4] = bytes(4)
            if random_.random() < 0.1:
                address[0:0x10] = bytes(0x10)
            if random_.random() < 0.1:
                struct.pack_into("<H", address, 0x12, random_.choice((0xFFFF, 8, 0)))
            script.append((random_.choice((len(data), len(data), len(data), 1, 0, 0xFFFF, 0xFFFD)), data, bytes(address)))

        def take(harness, arguments, script=script):
            index = calls_so_far(harness, "transport_endpoint_read_from")
            if index >= len(script):
                return 0, (harness.normalize(arguments[0]), arguments[2])
            count, data, address = script[index]
            harness.mu.mem_write(arguments[1], data)
            harness.mu.mem_write(arguments[3], address)
            return count, (harness.normalize(arguments[0]), arguments[2])

        t.check(
            "network_link_receive",
            link(True),
            externals={
                "transport_endpoint_read_from": take,
                "network_link_receive_packet": lambda harness, arguments: (
                    0,
                    (read(harness, arguments[0], 0x824), harness.normalize(arguments[1])),
                ),
            },
        )
