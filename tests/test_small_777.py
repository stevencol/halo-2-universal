# five small functions: taking a flag from some recent players, and four of
# two things of 0x50 bytes that look for something in the network

import random
import struct

from difftest import Buffer, GlobalData, Pointer

NONE = 0xFFFFFFFF
PLAYERS, PLAYER = 350, 0x68
SEARCH, ENTRY = 0x50, 0x784


def run(t) -> None:
    random_ = random.Random(777)

    def says(*values):
        return lambda harness, arguments, said=random_.choice(values): (said, tuple(harness.normalize(a) for a in arguments))

    for n in range(300):
        # the things: 0x18 bytes each, with a pointer at 0x14 to four more
        # of 0x10 bytes each, which are after them
        count = random_.choice((0, 1, 2, 4, 6))
        data = bytearray(random_.randbytes(max(count, 1) * (0x18 + 0x40)))
        pointers = []
        for index in range(count):
            at = index * 0x18
            more = count * 0x18 + index * 0x40
            struct.pack_into("<II", data, at + 0xC, random_.choice((0x61, 0x61, 0x60, 0)), random_.choice((4, 4, 3)))
            struct.pack_into("<I", data, at + 0x14, more)
            pointers.append(at + 0x14)
            for offset, value in ((4, 4), (0x14, 1), (0x24, 1), (0x34, 1)):
                struct.pack_into("<I", data, more + offset, value if random_.random() < 0.85 else random_.choice((0, 2, 5)))
        things = Buffer(bytes(data), pointers=tuple(pointers))

        # what is said of each thing, the same for the game and source/
        found = [random_.choice((NONE, 0, 1, 7, 349, random_.randrange(PLAYERS))) for _ in range(max(count, 1))]

        def find(harness, arguments, things=things, found=found):
            at = arguments[0] - things.address
            return found[at // 0x18], (at,)

        t.check(
            "network_recent_players_unknown_00080d70",
            count,
            things,
            globals={"unknown_004cf98c": random_.randbytes(PLAYERS * PLAYER)},
            externals={"network_recent_players_find": find},
        )

        def searches() -> bytearray:
            data = bytearray(random_.randbytes(2 * SEARCH))
            for at in (0, SEARCH):
                data[at] = random_.choice((0, 0, 1, 0x80))
                struct.pack_into("<II", data, at + 0x3C, random_.choice((NONE, 0, 5)), random_.choice((NONE, 0, 3)))
            return data

        entry_count = random_.choice((0, 1, 2, 3))
        entries = Buffer(random_.randbytes(max(entry_count, 1) * ENTRY))

        def identifier(harness, arguments, value=random_.randbytes(8)):
            harness.mu.mem_write(arguments[0], value)
            return 0, (harness.normalize(arguments[0]),)

        t.check(
            "network_unknown_000b2e30_start",
            entries,
            entry_count,
            globals={
                "unknown_004d8eb4": GlobalData(bytes(searches())),
                "unknown_004d8b18": bytes((random_.choice((0, 1, 1)),)),
                "unknown_004d8b19": bytes((random_.choice((0, 1, 1)),)),
            },
            externals={"transport_security_random_identifier": identifier},
        )

        t.check(
            "network_unknown_000b2e30_stop",
            globals={"unknown_004d8eb4": GlobalData(bytes(searches()))},
            externals={"online_task_close": says(0), "transport_qos_delete": says(0)},
        )

        t.check(
            "network_unknown_000b2e30_fill_entry",
            Buffer(random_.randbytes(0x40)),
            Buffer(random_.randbytes(ENTRY + 8)),
            globals={
                "unknown_00510548": bytes((random_.choice((0, 1)),)),
                "unknown_0051054c": random_.getrandbits(32),
            },
            externals={"GetTickCount": says(0, 1, random_.getrandbits(32))},
        )

        data = searches()
        struct.pack_into("<i", data, SEARCH + 0x10, random_.choice((0, 1, 3, 4, -1)))
        entries = bytearray(random_.randbytes(4 * ENTRY))
        for index in range(4):
            entries[index * ENTRY] = random_.choice((0, 1, 1, 0x80))
            struct.pack_into("<H", entries, index * ENTRY + 0x72, random_.choice((0, 0, 1, 0x8000)))
        entries = Buffer(bytes(entries))
        t.check(
            "network_unknown_000b2e30_get_entry",
            random_.choice((NONE, 0x80000000, 0, 1, 2, 3, 4)),
            extra=[entries],
            globals={"unknown_004d8eb4": GlobalData(bytes(data)), "unknown_004d8eb4+0x64": Pointer(entries, 0)},
        )
