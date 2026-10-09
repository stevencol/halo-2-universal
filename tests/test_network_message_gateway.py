# source/networking/network_message_gateway.c

import random
import struct

from difftest import Buffer

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
