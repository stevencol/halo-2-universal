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

    # ---------- addresses, and the sockets that have them

    from test_async import calls_so_far

    def an_address() -> Buffer:
        return Buffer(random_.randbytes(0x12) + struct.pack("<H", random_.choice((4, 4, 16, 16, 0, 0xFFFF, 8))))

    def sockaddr_of(harness, address: int, length: int) -> bytes:
        # what is set of an address of the libraries, which for sixteen bytes is not all of it
        raw = read(harness, address, 0x1C)
        return raw[:8] if length == 0x10 else raw[:4] + raw[8:0x18]

    def socket_externals() -> dict:
        return {
            "socket": lambda harness, arguments, made=random_.choice((7, 0x1234, 0x1234, NONE)): (made, tuple(arguments)),
            "getsockopt": lambda harness, arguments, value=random_.choice((bytes(4), bytes(4), b"\x00\x20\x00\x00")): (
                harness.mu.mem_write(arguments[3], value) or 0,
                (arguments[0], arguments[1], arguments[2]),
            ),
            "setsockopt": lambda harness, arguments: (
                0,
                (arguments[0], arguments[1], arguments[2], read(harness, arguments[3], 4), arguments[4]),
            ),
            "ioctlsocket": lambda harness, arguments, failed=random_.choice((0, 0, 0, NONE)): (
                failed,
                (arguments[0], arguments[1], read(harness, arguments[2], 4)),
            ),
            "WSAGetLastError": lambda harness, arguments, why=random_.choice((10035, 10035, 10065, 10054)): why,
        }

    for n in range(1500):
        t.check("transport_address_to_sockaddr", Buffer(random_.randbytes(0x1C)), Buffer(random_.randbytes(4)), an_address())
        t.check(
            "transport_address_from_sockaddr",
            Buffer(random_.randbytes(0x14)),
            random_.choice((0x10, 0x10, 0x1C, 0x1C, 0, 4, 0x14, NONE)),
            Buffer(random_.randbytes(0x1C)),
        )

        def typed() -> Buffer:
            return Buffer(
                struct.pack(
                    "<IHH",
                    random_.choice((NONE, NONE, 5, 0x1234)),
                    random_.choice((0, 1, 0x10, 0x40, 0x50, 0xFFFF, random_.getrandbits(16))),
                    random_.choice((2, 3, 4, 0, 1, 5, 0xFFFF)),
                )
            )

        t.check("transport_endpoint_open", typed(), an_address(), globals=network(), externals=socket_externals())
        t.check(
            "transport_endpoint_bind",
            an_address(),
            typed(),
            globals=network(),
            externals=dict(
                socket_externals(),
                bind=lambda harness, arguments, failed=random_.choice((0, 0, NONE)): (
                    failed,
                    (arguments[0], sockaddr_of(harness, arguments[1], arguments[2]), arguments[2]),
                ),
            ),
        )
        t.check(
            "transport_endpoint_connect",
            an_address(),
            typed(),
            globals=dict(network(), unknown_0048480c=Buffer(random_.randbytes(4))),
            externals=dict(
                socket_externals(),
                connect=lambda harness, arguments, failed=random_.choice((0, NONE, NONE)): (
                    failed,
                    (arguments[0], sockaddr_of(harness, arguments[1], arguments[2]), arguments[2]),
                ),
            ),
        )

        sets = random_.choice(((0, 0), (0, 1), (1, 0), (1, 1), (0, 5)))
        t.check(
            "transport_endpoint_connect_finished",
            typed(),
            Buffer(random_.randbytes(4)),
            globals=network(),
            externals={
                "select": lambda harness, arguments, count=random_.choice((0, 1, 1, 2, NONE)): (
                    count,
                    (arguments[0], arguments[1], read(harness, arguments[2], 0x104), read(harness, arguments[3], 0x104), read(harness, arguments[4], 8)),
                ),
                "__WSAFDIsSet": lambda harness, arguments, sets=sets: (
                    sets[min(calls_so_far(harness, "__WSAFDIsSet"), 1)],
                    (arguments[0], read(harness, arguments[1], 8)),
                ),
                "WSAGetLastError": error,
            },
        )

        count = random_.choice((0, 1, 5, 0x100, 0xFFFF, 0xFFFF, NONE, 0x7FFF))
        why = random_.choice((10035, 10035, 10065, 10054, 0))
        size = random_.choice((0, 1, 0x100, 0x1000, 0xFFFF)) | random_.choice((0, 0, 0xABCD0000))

        def receive(harness, arguments, count=count, came=random_.randbytes(0x1C), length=random_.choice((0x10, 0x10, 0x1C, 0x1C, 0))):
            before = read(harness, arguments[4], 0x1C), read(harness, arguments[5], 4)
            if count & 0xFFFF != 0xFFFF:
                harness.mu.mem_write(arguments[4], came)
                harness.mu.mem_write(arguments[5], struct.pack("<I", length))
            return count, (arguments[0], harness.normalize(arguments[1]), arguments[2], arguments[3], before)

        t.check(
            "transport_endpoint_read_from",
            endpoint(),
            Buffer(random_.randbytes(0x10)),
            size,
            Buffer(random_.randbytes(0x14)),
            globals=network(),
            externals={"recvfrom": receive, "WSAGetLastError": lambda harness, arguments, why=why: why},
        )
        t.check(
            "transport_endpoint_write_to",
            an_address(),
            endpoint(),
            Buffer(random_.randbytes(0x10)),
            size,
            globals=network(),
            externals={
                "sendto": lambda harness, arguments, count=count: (
                    count,
                    (
                        arguments[0],
                        harness.normalize(arguments[1]),
                        arguments[2],
                        arguments[3],
                        sockaddr_of(harness, arguments[4], arguments[5]),
                        arguments[5],
                    ),
                ),
                "WSAGetLastError": lambda harness, arguments, why=why: why,
            },
        )
