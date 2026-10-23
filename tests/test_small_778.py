# five small functions of the service of games: whether a service is there,
# forgetting a key of the network, the length of a text, what is said of an
# error, and seven numbers of 64 bits as 32

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
SERVICES, SERVICE = 13, 12
KEYS, KEY = 8, 0x4A
ERRORS = (
    0x80152100, 0x80152108, 0x8015210C, 0x80152112, 0x80152002, 0x8015200A, 0x80151000,
    0x80151001, 0x80151101, 0x001512F0, 0x8015B006, 0x80151300,
)


def run(t) -> None:
    random_ = random.Random(778)

    def says(*values):
        return lambda harness, arguments, said=random_.choice(values): (said, tuple(arguments))

    for n in range(600):
        services = bytearray(random_.randbytes(SERVICES * SERVICE))
        for index in range(SERVICES):
            struct.pack_into("<I", services, index * SERVICE, random_.choice((0x4D530064, 2, 4, 0x4D530065)))
            services[index * SERVICE + 8] = random_.choice((0, 0, 1, 0x80))
        t.check(
            "online_unknown_000b36c0_service_available",
            globals={
                "unknown_00467214": random_.choice((NONE, 0, 1, 3)),
                "unknown_00467178": bytes(services),
            },
            externals={"online_tasks_get_logon_state": says(0, 1, 1, 2, NONE)},
        )

        keys = random_.randbytes(KEYS * KEY)

        def unregister_key(harness, arguments, said=random_.choice((0, 1))):
            return said, (arguments[0] - harness.global_addresses["unknown_00510588"],)

        t.check(
            "online_unknown_000b36c0_unregister",
            globals={
                "unknown_00510588": keys,
                "unknown_005107dc": random_.choice((NONE, 0, 1, 7, random_.randrange(KEYS))),
                "unknown_005107e0": random_.choice((0, 0, 0x0100007F, random_.getrandbits(32))),
                "unknown_005107e4": random_.choice((0, 1, 8, random_.getrandbits(32))),
            },
            externals={"XNetUnregisterKey": unregister_key, "XNetUnregisterInAddr": says(0, 1)},
        )

        length = random_.choice((0, 1, 4, 5, 6, 0xFFA, 0xFFE, 0xFFF, 0x1000, 0x1004, random_.randrange(0x1010)))
        text = bytes(random_.randrange(1, 256) for _ in range(length)) + b"\0" + random_.randbytes(8)
        t.check("online_unknown_000b36c0_string_length", Buffer(text + bytes(max(0, 0x1010 - len(text)))))

        t.check(
            "online_error_get_message",
            random_.choice(ERRORS + (0, NONE, 0x80152107, random_.getrandbits(32))),
        )

        t.check("online_copy_low_halves", Buffer(random_.randbytes(7 * 4 + 4)), Buffer(random_.randbytes(14 * 4)))
