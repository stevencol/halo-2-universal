# source/networking/transport_endpoint.c

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(498)

    def network() -> dict:
        return {
            "unknown_004d8b18": bytes((random_.choice((1, 1, 1, 1, 0)),)),
            "unknown_004d8b19": bytes((random_.choice((1, 1, 1, 1, 0)),)),
        }

    def endpoint() -> Buffer:
        return Buffer(
            struct.pack(
                "<IHH",
                random_.choice((NONE, 5, 0x1234, random_.getrandbits(32))),
                random_.choice((0, 1, 1, 0xFFFF, 0x25, random_.getrandbits(16))),
                random_.getrandbits(16),
            )
        )

    def read(harness, address: int, size: int) -> bytes:
        return bytes(harness.mu.mem_read(address, size))

    def an_option() -> int:
        return random_.choice((0, 1, 2, 3, 4, 5, 5, 6, 0xFFFF, 0x8000)) | random_.choice((0, 0, 0xABCD0000))

    error = lambda harness, arguments: 10035

    for n in range(1500):
        made = Buffer(random_.randbytes(8))
        t.check(
            "transport_endpoint_create",
            random_.getrandbits(16) | random_.choice((0, 0xABCD0000)),
            globals=network(),
            extra=[made],
            externals={
                "VirtualAlloc": lambda harness, arguments, made=made, there=random_.random() < 0.8: (
                    made.address if there else 0,
                    tuple(arguments),
                ),
                "GetLastError": lambda harness, arguments: 8,
            },
        )
        t.check("transport_endpoint_option_name", an_option())

        def get(harness, arguments, value=random_.randbytes(4), failed=random_.choice((0, 0, 0, NONE))):
            length = read(harness, arguments[4], 4)
            if not failed:
                harness.mu.mem_write(arguments[3], value)
            return failed, (arguments[0], arguments[1], arguments[2], length)

        t.check(
            "transport_endpoint_get_option",
            endpoint(),
            an_option(),
            globals=network(),
            externals={"getsockopt": get, "WSAGetLastError": error},
        )
        option = an_option()
        t.check(
            "transport_endpoint_set_option",
            option,
            endpoint(),
            Buffer(random_.randbytes(4)) if option & 0xFFFF == 5 else random_.getrandbits(32),
            globals=network(),
            externals={
                "setsockopt": lambda harness, arguments, failed=random_.choice((0, 0, 0, NONE)): (
                    failed,
                    (arguments[0], arguments[1], arguments[2], read(harness, arguments[3], 4), arguments[4]),
                ),
                "WSAGetLastError": error,
            },
        )
        t.check(
            "transport_endpoint_close",
            endpoint(),
            globals=network(),
            externals={
                "shutdown": lambda harness, arguments, failed=random_.choice((0, 0, NONE)): (failed, tuple(arguments)),
                "closesocket": lambda harness, arguments, failed=random_.choice((0, 0, NONE)): (failed, tuple(arguments)),
                "WSAGetLastError": error,
            },
        )
        count = random_.choice((0, 0, 1, 5, 0x100, 0xFFFF, 0xFFFF, NONE, 0x7FFF))
        why = random_.choice((10035, 10035, 10065, 10054, 0))
        size = random_.choice((0, 1, 0x100, 0x7FFF, 0xFFFF)) | random_.choice((0, 0, 0xABCD0000))
        t.check(
            "transport_endpoint_read",
            endpoint(),
            Buffer(random_.randbytes(0x10)),
            size,
            globals=network(),
            externals={
                "recv": lambda harness, arguments, count=count: (
                    count,
                    (arguments[0], harness.normalize(arguments[1]), arguments[2], arguments[3]),
                ),
                "WSAGetLastError": lambda harness, arguments, why=why: why,
            },
        )
        t.check(
            "transport_endpoint_write",
            endpoint(),
            Buffer(random_.randbytes(0x10)),
            size,
            globals=network(),
            externals={
                "send": lambda harness, arguments, count=count: (
                    count,
                    (arguments[0], harness.normalize(arguments[1]), arguments[2], arguments[3]),
                ),
                "WSAGetLastError": lambda harness, arguments, why=why: why,
            },
        )
