# source/networking/transport.c

import random
import struct

from difftest import Buffer, Mock, mock_address

USERS = 8


def run(t) -> None:
    random_ = random.Random(506)

    def read(harness, address: int, size: int) -> bytes:
        return bytes(harness.mu.mem_read(address, size))

    def users() -> dict:
        """What is told that the network started or stopped: for each, what it has for the one and the other, or nothing."""
        count = random_.choice((0, 1, 3, 8))
        starts = [mock_address(index) if random_.random() < 0.7 else 0 for index in range(USERS)]
        stops = [mock_address(USERS + index) if random_.random() < 0.7 else 0 for index in range(USERS)]
        return {
            "unknown_004d8b1c": count,
            "unknown_004d8b20": struct.pack("<8I", *starts),
            "unknown_004d8b40": struct.pack("<8I", *stops),
            # not numbers that are the address of a function of source/, which would be taken for one
            "unknown_004d8b80": struct.pack("<8I", *(random_.getrandbits(28) for index in range(USERS))),
        }

    def mocks() -> list:
        return [Mock(lambda call: 0, stack_arguments=1) for index in range(2 * USERS)]

    said = lambda harness, arguments: (0, tuple(harness.normalize(value) for value in arguments))

    for n in range(1500):
        t.check(
            "transport_global_update",
            globals={
                "unknown_004d8b18": bytes((random_.choice((1, 1, 1, 0)),)),
                "unknown_004d8b1a": bytes((random_.choice((0, 1)),)),
                "unknown_004cf791": bytes((random_.choice((0, 1)),)),
                "unknown_0055e704": random_.choice((0, 1, 3)),
            },
            externals={
                "XNetGetEthernetLinkStatus": lambda harness, arguments, status=random_.choice((0, 1, 1, 3, 2, 0x10)): status,
                "transport_startup": said,
                "transport_shutdown": said,
                "transport_security_get_address": lambda harness, arguments: (1, ()),
            },
        )

        fails = random_.choice((None, None, None, "XNetStartup", "WSAStartup", "XOnlineStartup"))

        def starts(name: str, sizes: tuple):
            def stand_in(harness, arguments, name=name, sizes=sizes, fails=fails):
                seen = tuple(read(harness, value, size) if size else value for value, size in zip(arguments, sizes))
                return (0x2726 if fails == name else 0), seen

            return stand_in

        t.check(
            "transport_startup",
            globals=dict(
                users(),
                unknown_004d8b19=bytes((random_.choice((0, 0, 0, 1)),)),
                unknown_004cf8d4=bytes((random_.choice((0, 0, 1)),)),
                transport_qos_attempts=Buffer(random_.randbytes(0x4C)),
            ),
            externals={
                "XNetGetEthernetLinkStatus": lambda harness, arguments: 1,
                "XNetStartup": starts("XNetStartup", (12,)),
                "WSAStartup": starts("WSAStartup", (0, 0x190)),
                "XOnlineStartup": starts("XOnlineStartup", (4,)),
                "WSACleanup": said,
                "XNetCleanup": said,
                "transport_security_get_unknown7cc": said,
                "transport_security_get_address": lambda harness, arguments: (1, ()),
                "data_delete_all": said,
            },
            mocks=mocks(),
        )

        keys = bytearray(random_.randbytes(8 * 0x20))
        for index in range(8):
            keys[index * 0x20] = random_.choice((0, 1, 1, 0x80))
        t.check(
            "transport_shutdown",
            globals=dict(
                users(),
                unknown_004d8b19=bytes((random_.choice((1, 1, 1, 0)),)),
                unknown_004cf7d4=bytes(keys),
            ),
            externals={
                "transport_qos_dispose": said,
                "XNetUnregisterKey": lambda harness, arguments: (0, (read(harness, arguments[0], 8),)),
                "XOnlineCleanup": said,
                "WSACleanup": said,
                "XNetCleanup": said,
            },
            mocks=mocks(),
        )

    # ---------- the network of the console for the game, and who is told

    for n in range(1500):
        every = dict(
            users(),
            unknown_004d8b18=bytes((random_.choice((0, 1)),)),
            unknown_004d8b19=bytes((random_.choice((0, 1)),)),
            unknown_004d8b1a=bytes((random_.choice((0, 1)),)),
            unknown_004d8b60=random_.randbytes(USERS * 4),
            unknown_0055e704=random_.choice((0, 1, 3)),
        )
        t.check(
            "transport_initialize",
            globals=every,
            externals={
                "unknown_0007a840": said,
                "transport_qos_initialize": said,
                "XNetGetEthernetLinkStatus": lambda harness, arguments, status=random_.choice((0, 1, 1, 3, 2)): status,
                "transport_startup": said,
            },
        )

        # the table of the attempts, with what gives its memory after it: of its two functions, the second takes it back
        table = bytearray(random_.randbytes(0x58))
        has = random_.random() < 0.8
        struct.pack_into("<I", table, 0x30, 0x4C if has else 0)
        struct.pack_into("<3I", table, 0x4C, 0x50, mock_address(0), mock_address(1))
        t.check(
            "transport_dispose",
            globals=dict(
                users(),
                unknown_004d8b18=bytes((random_.choice((1, 1, 1, 0)),)),
                transport_qos_attempts=random_.choice(
                    (Buffer(bytes(table), pointers=(0x30, 0x4C) if has else (0x4C,)),) * 4 + (0,)
                ),
            ),
            externals={"transport_shutdown": said, "transport_qos_dispose": said},
            mocks=[Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)) for index in range(2)],
        )

        thirds = [mock_address(index) if random_.random() < 0.7 else 0 for index in range(USERS)]
        t.check(
            "transport_unknown_0008d730",
            globals=dict(users(), unknown_004d8b60=struct.pack("<8I", *thirds)),
            mocks=mocks(),
        )
        t.check(
            "transport_register",
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={
                "unknown_004d8b1c": random_.randrange(USERS),
                "unknown_004d8b20": random_.randbytes(USERS * 4),
                "unknown_004d8b40": random_.randbytes(USERS * 4),
                "unknown_004d8b60": random_.randbytes(USERS * 4),
                "unknown_004d8b80": random_.randbytes(USERS * 4),
            },
        )
        t.check(
            "transport_available",
            globals={
                "unknown_004d8b18": bytes((random_.choice((0, 1, 1, 0x80)),)),
                "unknown_004d8b19": bytes((random_.choice((0, 1, 1, 0x80)),)),
            },
        )
