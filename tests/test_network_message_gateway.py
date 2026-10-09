# source/networking/network_message_gateway.c

import random
import struct

from difftest import Buffer, Mock, mock_address

NONE = 0xFFFFFFFF
SIZE = 0x680
PENDING = 0x14
ADDRESS = 0x618
STREAM = 0x62C


def run(t) -> None:
    random_ = random.Random(394)

    def gateway() -> bytearray:
        data = bytearray(random_.randbytes(SIZE))
        data[PENDING] = random_.choice((0, 1, 1, 1, 0x80))
        # what writes the packet: where it writes, in the gateway itself
        struct.pack_into("<I", data, STREAM, 0x15)
        return data

    # ---------- the packet that has not been sent is

    for n in range(400):
        data = gateway()
        struct.pack_into(
            "<iiIi",
            data,
            STREAM + 4,
            random_.getrandbits(31),
            random_.choice((1, 1, 2, 4, 8, 16, 3)),
            random_.getrandbits(32),
            random_.choice((0, 1, 6, 7, 8, 9, 31, 32, 63, 64, 100, 0x28BF, 0x7FFFFFF7, 0x7FFFFFFF, -1, -2, -9, -100, random_.getrandbits(16))),
        )
        t.check(
            "network_message_gateway_flush",
            Buffer(bytes(data), pointers=(STREAM,)),
            externals={
                "unknown_00093100": lambda harness, arguments: (
                    0,
                    (harness.normalize(arguments[0]), harness.normalize(arguments[1]), arguments[2], arguments[3]),
                )
            },
        )

    # ---------- and is if it is for an address

    for n in range(600):
        data = gateway()
        size = random_.choice((4, 4, 16, 16, 6, 0, -1, -4))
        struct.pack_into("<h", data, ADDRESS + 0x12, size)
        address = bytearray(data[ADDRESS : ADDRESS + 0x14])
        way = random_.randrange(6)
        if way == 0:
            # another one, of the same size
            address[0:16] = random_.randbytes(16)
        elif way == 1:
            # the same but for its last byte, or for the one after
            address[max(size, 1) - 1 + random_.choice((0, 1))] ^= 0x10
        elif way == 2:
            # of another size
            struct.pack_into("<h", address, 0x12, random_.choice((4, 16, 0)))
        elif way == 3:
            # with another port, which does not count
            struct.pack_into("<H", address, 0x10, random_.getrandbits(16))
        t.check(
            "network_message_gateway_flush_address",
            Buffer(bytes(address)),
            Buffer(bytes(data), pointers=(STREAM,)),
            externals={"network_message_gateway_flush": lambda harness, arguments: (0, (harness.normalize(arguments[0]),))},
        )

    # ---------- the kind and the size that a message starts with

    def a_stream(size: int, position: int, blank: bool = False) -> Buffer:
        data = bytearray(random_.randbytes(size))
        if blank:
            for bit in range(position, size * 8):
                data[bit >> 3] &= ~(1 << (bit & 7))
        header = struct.pack("<I4iB3x7i", 0x34, size, random_.getrandbits(31), 1, position, random_.choice((0, 0, 0, 1)), *(random_.getrandbits(8) for n in range(7)))
        return Buffer(header + bytes(data) + random_.randbytes(8), pointers=(0,))

    for n in range(400):
        size = random_.choice((2, 3, 4, 8, 40))
        t.check(
            "network_message_header_write",
            a_stream(size, random_.choice((0, 0, 3, 8, size * 8 - 24, size * 8 - 8, size * 8)), blank=True),
            random_.choice((0, 5, 0x2C, 0xFF, 0x100, random_.getrandbits(32))),
            random_.choice((0, 8, 0x518, 0xFFFF, 0x10000, random_.getrandbits(32))),
            externals={"csprintf256": lambda harness, arguments: (0, (arguments[2], arguments[3] if len(arguments) > 3 else None))},
        )

    for n in range(1500):
        size = random_.choice((2, 3, 4, 8, 40))
        stream_ = bytearray(a_stream(size, random_.choice((0, 0, 0, 3, 8, size * 8 - 24, size * 8 - 16, size * 8))).data)
        # a kind that there is, mostly, and a size that it can be of, mostly
        kind = random_.choice((0, 1, 5, 0x2C, 0x2C, 0x2D, 0x80, 0xFF))
        message_size = random_.choice((0, 8, 9, 15, 16, 17, 0x518))
        if size >= 3 and stream_[0x10] == 0 and n % 4:
            stream_[0x34] = kind
            stream_[0x35:0x37] = struct.pack("<H", message_size)
        kinds = bytearray()
        for other in range(0x2D):
            kinds += struct.pack("<B3x2i2i3I", random_.choice((1, 1, 1, 0, 0x80)), 0, 0, random_.choice((0, 8, 9, 16)), random_.choice((8, 16, 16, 0x518)), 0x11, 0x22, 0x33)
        # what is after the last kind is not one, whatever it looks like
        kinds += struct.pack("<B3x2i2i3I", 1, 0, 0, 0, 0xFFFF, 0x11, 0x22, 0x33)
        t.check(
            "network_message_header_read",
            Buffer(random_.randbytes(4)),
            Buffer(bytes(stream_), pointers=(0,)),
            Buffer(bytes(kinds)),
            Buffer(random_.randbytes(4)),
        )

    # ---------- a message goes in the packet

    for n in range(1200):
        data = gateway() + bytearray()
        # the kinds of message go after the gateway: each has the function that writes one
        kinds = len(data)
        struct.pack_into("<I", data, 0xC, kinds)
        for other in range(0x2D):
            data += struct.pack("<B3x2i2i3I", 1, 0, 0, 0, 0xFFFF, mock_address(0), 0x22, 0x33)
        size = random_.choice((4, 16))
        address = bytearray(random_.randbytes(0x14))
        struct.pack_into("<h", address, 0x12, size)
        way = random_.randrange(4)
        if way == 0:
            # the packet that has not been sent is for this address
            data[ADDRESS : ADDRESS + 0x14] = address
        elif way == 1:
            # or for this address with another port, which is the same
            data[ADDRESS : ADDRESS + 0x14] = address[0:0x10] + random_.randbytes(2) + address[0x12:0x14]
        # the packet that has not been sent, with room for more or with next to none
        position = random_.choice((0, 9, 800, 0x518 * 8 - 400, 0x518 * 8 - 60, 0x518 * 8 - 27, 0x518 * 8 - 26, 0x518 * 8 - 25, 0x518 * 8 - 1))
        struct.pack_into("<iiIi", data, STREAM + 4, 0x518, random_.choice((1, 1, 4)), 1, position)
        data[STREAM + 0x14] = 0
        struct.pack_into("<i", data, STREAM + 0x18, random_.choice((0, 1)))
        for bit in range(position, 0x518 * 8):
            data[0x15 + (bit >> 3)] &= ~(1 << (bit & 7))
        # how many bits the function of the kind writes of the message
        wrote = random_.choice((0, 8, 32, 300, 0x518 * 8 - 26, 0x518 * 8 - 25, 0x518 * 8))

        def writes(call, wrote=wrote):
            stream_ = call.stack[0]
            at = struct.unpack("<i", t.harness.mu.mem_read(stream_ + 0x10, 4))[0]
            t.harness.mu.mem_write(stream_ + 0x10, struct.pack("<i", at + wrote))
            return 0

        t.check(
            "network_message_gateway_send",
            Buffer(bytes(data), pointers=(0xC, STREAM)),
            Buffer(bytes(address)),
            random_.randrange(0x2D),
            random_.choice((0, 8, 0x518)),
            Buffer(random_.randbytes(16)),
            externals={
                "unknown_00093100": lambda harness, arguments: (
                    0,
                    (harness.normalize(arguments[0]), harness.normalize(arguments[1]), arguments[2], arguments[3]),
                )
            },
            mocks=[Mock(writes, stack_arguments=3)],
        )