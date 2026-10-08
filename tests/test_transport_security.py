# source/networking/transport_security.c

import random
import struct

from difftest import Buffer, GlobalData

FAILED = 0x80004005
NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(316)

    def keys() -> GlobalData:
        data = bytearray(random_.randbytes(0x100))
        for index in range(8):
            data[index * 0x20] = random_.choice((1, 1, 0x80, 0))
        return GlobalData(bytes(data))

    # ---------- what a session is known by, and its key: they can be in the
    # stack, so what is told of them is compared by what they are

    for n in range(160):
        result = random_.choice((0, 0, 0, 1, FAILED))
        t.check(
            "transport_security_set_key",
            random_.randrange(8),
            Buffer(random_.randbytes(16)),
            random_.choice((0, 0, 1, random_.getrandbits(32))),
            random_.choice((0, 1, 0x80, 0x1FF, 0x100)),
            Buffer(random_.randbytes(8)),
            globals={"unknown_004cf7d4": keys()},
            externals={
                "XNetUnregisterKey": lambda harness, arguments: 0,
                "XNetRegisterKey": lambda harness, arguments, result=result: (
                    result,
                    (bytes(harness.mu.mem_read(arguments[0], 8)), bytes(harness.mu.mem_read(arguments[1], 16))),
                ),
            },
        )

    for n in range(200):
        made = random_.randbytes(24)
        # the library always makes them here: what the game does when it
        # cannot is done with what there was in the stack
        makes = random_.choice((0, 0, 0, 1, FAILED))
        result = random_.choice((0, 0, 0, 1, FAILED))

        def make(harness, arguments, made=made, makes=makes):
            harness.mu.mem_write(arguments[0], made[:8])
            harness.mu.mem_write(arguments[1], made[8:])
            return makes, ()

        t.check(
            "transport_security_create_key",
            random_.choice((0, 0, 0, 1, random_.getrandbits(32))),
            random_.randrange(8),
            random_.choice((0, 1, 1, 0x80, 0x100)),
            globals={"unknown_004cf7d4": keys()},
            externals={
                "XNetCreateKey": make,
                "XNetUnregisterKey": lambda harness, arguments: 0,
                "XNetRegisterKey": lambda harness, arguments, result=result: (
                    result,
                    (bytes(harness.mu.mem_read(arguments[0], 8)), bytes(harness.mu.mem_read(arguments[1], 16))),
                ),
            },
        )

    # ---------- what this console is known by

    for asked in (1, 0, 0x80):
        for known in (1, 0):
            t.check(
                "transport_security_forget_address",
                random_.getrandbits(32),
                globals={"unknown_004cf791": bytes((asked,)), "unknown_004cf792": bytes((known,))},
            )

    for n in range(200):
        address = random_.randbytes(0x24)
        status = random_.choice((0, 0, 1, 1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 0x84, 0x8000, 0x8002, 0xFFFFFFFF))

        def says(harness, arguments, address=address, status=status):
            # where it goes is in the stack
            harness.mu.mem_write(arguments[0], address)
            return status, ()

        t.check(
            "transport_security_get_address",
            globals={
                "unknown_004cf791": bytes((random_.choice((0, 0, 0, 1, 0x80)),)),
                "unknown_004cf792": bytes((random_.choice((0, 1)),)),
                "unknown_004cf793": random_.randbytes(0x24),
                "unknown_004cf7b8": random_.randbytes(0x14),
            },
            externals={"XNetGetTitleXnAddr": says},
        )

    # ---------- the addresses of the game

    def an_address() -> bytes:
        size = random_.choice((4, 4, 4, 4, 16, 16, 0xFFFF, 0, 6))
        way = random_.randrange(6)
        if way == 0:
            data = bytearray(0x10)
        elif way == 1:
            # only the last of the sixteen bytes is not zero
            data = bytearray(15) + bytes((random_.randrange(1, 256),))
        elif way == 2:
            # one that the console's library gave: when it is the other way round, it starts with a zero
            data = bytearray(random_.randbytes(3) + b"\0" + random_.randbytes(12))
        elif way == 3:
            data = bytearray(4) + bytearray(random_.randbytes(12))
        else:
            data = bytearray(random_.randbytes(0x10))
        return bytes(data) + random_.randbytes(2) + struct.pack("<H", size)

    for n in range(300):
        t.check("transport_address_valid", random_.choice((Buffer(an_address()), Buffer(an_address()), Buffer(an_address()), 0)))
    for n in range(300):
        t.check(
            "transport_address_get_secure_in_address",
            random_.choice((Buffer(an_address()), Buffer(an_address()), Buffer(an_address()), 0)),
            Buffer(random_.randbytes(4)),
        )
    for n in range(200):
        t.check(
            "transport_address_connect",
            Buffer(an_address()),
            externals={"XNetConnect": lambda harness, arguments, result=random_.choice((0, 0, 1, FAILED)): result},
        )
    for n in range(200):
        t.check(
            "transport_address_get_connect_status",
            Buffer(an_address()),
            externals={
                "XNetGetConnectStatus": lambda harness, arguments, result=random_.choice((0, 1, 2, 3, 4, 5, 0xFFFFFFFF)): result
            },
        )

    # ---------- what this console is known by, again

    for n in range(120):
        address = random_.randbytes(0x24)
        status = random_.choice((0, 1, 2, 4, 8, 0x10, 0x20, 0x8000))

        def says(harness, arguments, address=address, status=status):
            harness.mu.mem_write(arguments[0], address)
            return status, ()

        t.check(
            "transport_security_get_addresses",
            random_.choice((Buffer(random_.randbytes(0x24)), Buffer(random_.randbytes(0x24)), 0)),
            random_.choice((Buffer(random_.randbytes(0x14)), Buffer(random_.randbytes(0x14)), 0)),
            globals={
                "unknown_004d8b18": bytes((random_.choice((1, 1, 0x80, 0)),)),
                "unknown_004d8b19": bytes((random_.choice((1, 1, 0x80, 0)),)),
                "unknown_004cf791": bytes((random_.choice((0, 0, 1)),)),
                "unknown_004cf792": bytes((random_.choice((0, 1, 0x80)),)),
                "unknown_004cf793": random_.randbytes(0x24),
                "unknown_004cf7b8": random_.randbytes(0x14),
            },
            externals={"XNetGetTitleXnAddr": says},
        )

    for n in range(20):
        address = random_.randbytes(0x24)

        def says_always(harness, arguments, address=address, short=n % 5 == 0):
            # what was there has to be zeros
            before = bytes(harness.mu.mem_read(arguments[0], 0x24))
            harness.mu.mem_write(arguments[0], address[:0xA] if short else address)
            return 2, (before,)

        t.check(
            "transport_security_get_unknown7cc",
            globals={"unknown_004cf7cc": random_.randbytes(6)},
            externals={"XNetGetTitleXnAddr": says_always},
        )

    # ---------- bytes made at random

    def at_random(harness, arguments):
        # they are made from what there was, so that the two sides get the same
        size = arguments[1] & 0xFF
        before = bytes(harness.mu.mem_read(arguments[0], size))
        harness.mu.mem_write(arguments[0], bytes((byte * 7 + 3) & 0xFF for byte in before))
        return 0, (harness.normalize(arguments[0]), arguments[1])

    for n in range(80):
        size = random_.choice((0, 1, 8, 8, 16, 33))
        t.check(
            "transport_security_random_bytes",
            size,
            Buffer(random_.randbytes(40)),
            globals={"unknown_004cf790": bytes((random_.choice((0, 0, 1, 0x80)),))},
            externals={
                "XNetRandom": at_random,
                "time": lambda harness, arguments, value=random_.getrandbits(31): value,
                "rand": lambda harness, arguments, value=random_.getrandbits(15): value,
                "GetTickCount": lambda harness, arguments, value=random_.getrandbits(32): value,
            },
        )

    for n in range(40):
        zeros = random_.choice((1, 1, 2, 0))

        def zeros_first(harness, arguments, zeros=zeros):
            # all zeros the first times: the count of them is kept after the eight bytes
            count = bytes(harness.mu.mem_read(arguments[0] + 8, 1))[0]
            if count < zeros:
                harness.mu.mem_write(arguments[0], bytes(8))
                harness.mu.mem_write(arguments[0] + 8, bytes((count + 1,)))
            else:
                harness.mu.mem_write(arguments[0], bytes(range(1, 9)))
            return 0, (arguments[1], count)

        t.check(
            "transport_security_random_identifier",
            Buffer(random_.randbytes(8) + bytes(1) + random_.randbytes(7)),
            globals={"unknown_004cf790": bytes((1,)), "unknown_0046725c": Buffer(bytes(8))},
            externals={"XNetRandom": zeros_first},
        )

    # ---------- the address of a console that is known by something

    def gives(harness, arguments, could=0, value=0):
        harness.mu.mem_write(arguments[2], struct.pack("<I", value))
        return could, (arguments[0], harness.normalize(arguments[1]))

    for name in ("transport_security_address_from_key", "transport_security_address_from_peer_address"):
        for n in range(240):
            could = random_.choice((0, 0, 0, 1, FAILED))
            value = random_.choice((0, random_.getrandbits(32), random_.getrandbits(32), random_.getrandbits(24) << 8))
            table = bytearray(keys().data)
            for index in range(8):
                struct.pack_into("<I", table, index * 0x20 + 4, random_.choice((0, 0, 1)))
                if name.endswith("peer_address") and n % 3 == 0:
                    table[index * 0x20] = 0
            xnaddr = Buffer(random_.randbytes(0x24))
            out = Buffer(random_.randbytes(0x14))
            unknown = random_.choice((0, 0, 0, 1, random_.getrandbits(32)))
            port = random_.getrandbits(32)
            index = random_.randrange(8)
            if name.endswith("from_key"):
                arguments = (index, unknown, out, xnaddr, port)
            else:
                arguments = (random_.choice((index, NONE, NONE)), out, xnaddr, unknown, port)
            t.check(
                name,
                *arguments,
                globals={"unknown_004cf7d4": GlobalData(bytes(table))},
                externals={"XNetXnAddrToInAddr": lambda harness, arguments, could=could, value=value: gives(harness, arguments, could, value)},
            )
