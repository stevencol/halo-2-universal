# source/networking/network_memory.c

import random
import struct

from difftest import Buffer, GlobalRange, Mock, mock_address


def run(t) -> None:
    random_ = random.Random(489)

    for n in range(300):
        def maybe(value):
            return value if random_.random() < 0.8 else 0

        # the heap of the network: what gives its memory, with its functions, of which the fourteenth is asked
        heap = bytearray(random_.randbytes(0x50))
        there = random_.random() < 0.8
        struct.pack_into("<I", heap, 0, 0x8 if there else 0)
        struct.pack_into("<I", heap, 0x8, 0xC)
        struct.pack_into("<14I", heap, 0xC, *(0x11 * (index + 1) for index in range(13)), mock_address(0))
        t.check(
            "network_memory_free",
            globals={
                "unknown_005291a0": GlobalRange(4),
                "unknown_00450cb8": GlobalRange(4),
                "unknown_00450d60": GlobalRange(4),
                "unknown_004d87e8": maybe(Buffer(random_.randbytes(0xA0B0), places=(0x2098, 0xA0AC))),
                "unknown_004d87f0": maybe(Buffer(random_.randbytes(0x4C))),
                "unknown_004d87ec": maybe(Buffer(random_.randbytes(0x4C))),
                "unknown_004d87d4": maybe(0x100000),
                "unknown_004d87d8": maybe(0x200000),
                "unknown_004d87dc": maybe(0x300000),
                "unknown_004d87f8": maybe(Buffer(bytes(heap), pointers=(0, 0x8) if there else (0x8,))),
                "unknown_004d87fc": maybe(0x400000),
                "unknown_004d87d0": random_.getrandbits(32),
                "unknown_004d87e0": random_.getrandbits(32),
                "unknown_004d87e4": bytes((random_.choice((0, 1)),)),
                "unknown_004d87c4": random_.getrandbits(32),
                "unknown_004d87c8": random_.getrandbits(32),
                "unknown_004d87cc": random_.getrandbits(32),
            },
            externals={
                "network_observer_release_game_channels": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "unknown_00321379": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
            mocks=[Mock(lambda call: 0, stack_arguments=1, registers=("ecx",))],
        )

    for n in range(200):
        t.check("network_memory_unknown_00081310", Buffer(random_.randbytes(0x4E0)))

    for n in range(100):
        t.check(
            "network_memory_unknown_00081440",
            Buffer(random_.randbytes(0x4E58), places=(0,)),
            globals={"unknown_00450e10": GlobalRange(4)},
        )
    for n in range(30):
        t.check(
            "network_memory_construct",
            Buffer(
                random_.randbytes(0x1EA7C),
                places=(0xB28, 0x1154, 0x11A0, 0x60F8, 0x60F8 + 0x78B8, 0x60F8 + 2 * 0x78B8, 0x1DD40),
            ),
            globals={
                "unknown_00450e10": GlobalRange(4),
                "unknown_00450e28": GlobalRange(4),
                "unknown_00450df8": GlobalRange(4),
                "unknown_00450da0": GlobalRange(4),
            },
        )

    # ---------- all of it is unmade

    for n in range(60):
        data = bytearray(random_.randbytes(0x1EA7C))
        data[0xB2C] = random_.choice((0, 1, 1, 0x40))
        has = random_.random() < 0.7
        struct.pack_into("<I", data, 0xB30, 0x1E000 if has else 0)
        t.check(
            "network_memory_destruct",
            Buffer(
                bytes(data),
                pointers=(0xB30,) if has else (),
                places=(0xB28, 0x11A0, 0x60F8, 0x60F8 + 0x78B8, 0x60F8 + 2 * 0x78B8, 0x1DD40),
            ),
            globals={
                "unknown_00450e10": GlobalRange(4),
                "unknown_00450e28": GlobalRange(4),
                "unknown_00450df8": GlobalRange(4),
                "unknown_00450da0": GlobalRange(4),
            },
            externals={
                "network_link_close_channels": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "network_link_close_sockets": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
        )

    # ---------- the network takes its memory

    import hashlib

    from test_async import calls_so_far
    from test_scenario_load import physical_memory

    for n in range(24):
        unknown = random_.choice((0, 1, 1, 2, 5, 0xFFFFFFFF, 0x80000000))
        many = unknown <= 1 or unknown >= 0x80000000
        count = 15 if many else 1
        heap = Buffer(random_.randbytes(0x10), places=(0,))
        allocator = Buffer(random_.randbytes(0x34), places=(0,))
        seed = random_.getrandbits(32)
        news = random_.choice((2, 2, 2, 2, 1, 0))
        now = random_.getrandbits(31)
        while 0x20000000 <= now < 0x21000000:
            now = random_.getrandbits(31)

        def protect(harness, arguments, seed=seed):
            # what the console's memory has is not known: the same in the game and in source/, each time
            index = calls_so_far(harness, "XPhysicalProtect")
            harness.mu.mem_write(arguments[0], random.Random(seed + index).randbytes(arguments[1]))
            return 0, tuple(arguments)

        def new(harness, arguments, news=news, heap=heap, allocator=allocator):
            index = calls_so_far(harness, "operator_new")
            return ((heap, allocator)[index].address if index < news else 0), (arguments[0],)

        def take(harness, arguments, count=count, many=many):
            # all the memory that was taken, with what points to the functions of things looked at apart
            regions = [call[1] for call in harness._calls if call[0] == "XPhysicalProtect"]
            pointers = [[index * 0xF8 + 0x18 for index in range(count)], [index * 0x97C for index in range(count)], [index * 0x2850 for index in range(count)], []]
            if many:
                pointers += [[0x2098, 0xA0AC], []]
            pointers.append([0, 0xFFC])
            seen = []
            for (address, size, _), offsets in zip(regions, pointers):
                data = bytearray(harness.mu.mem_read(address, size))
                found = []
                for offset in offsets:
                    found.append(harness.normalize(struct.unpack_from("<I", data, offset)[0]))
                    struct.pack_into("<I", data, offset, 0)
                seen.append((hashlib.sha1(bytes(data)).hexdigest(), tuple(found)))
            return 0, (harness.normalize(arguments[0]), len(regions), tuple(seen))

        t.check(
            "network_memory_allocate",
            unknown,
            globals={
                "physical_memory_globals": physical_memory(random_.randrange(5), 0x100000, 0x700000),
                "unknown_004cf760": random_.choice((0x20000, 0x40000, 0x21234)),
                "unknown_004cf764": random_.choice((0x8000, 0x10000, 0x8765)),
                "unknown_004d87f4": random_.getrandbits(32),
                "unknown_004d87e4": bytes((random_.choice((0, 1)),)),
                "unknown_004d87d0": random_.getrandbits(32),
                "unknown_004d87e0": random_.getrandbits(32),
                "unknown_004d87d4": random_.getrandbits(32),
                "unknown_004d87d8": random_.getrandbits(32),
                "unknown_004d87dc": random_.getrandbits(32),
                "unknown_004d87ec": random_.getrandbits(32),
                "unknown_004d87f0": random_.getrandbits(32),
                "unknown_004d87e8": random_.getrandbits(32),
                "unknown_004d87fc": random_.getrandbits(32),
                "unknown_004d87f8": random_.getrandbits(32),
                "unknown_004d87c4": random_.choice((0, 0, 0x1234)),
                "unknown_004d87c8": random_.choice((0, 0x80300000, 0x80690000, 0xFFFFFFFF)),
                "unknown_004d87cc": random_.choice((0, 0x80300000, 0x806FF000, 0xFFFFFFFF)),
                "lruv_blocks_allocation": random_.choice((0, 0x60F00100)),
                "unknown_00510548": bytes((random_.choice((0, 0, 1)),)),
                "unknown_0051054c": struct.pack("<I", now),
                "unknown_004e6398": random_.randbytes(8),
                "unknown_005291a0": GlobalRange(4),
                "unknown_00450d40": GlobalRange(4),
                "unknown_00450dd8": GlobalRange(4),
                "unknown_00450db8": GlobalRange(4),
                "unknown_00450d60": GlobalRange(4),
                "unknown_00450cb8": GlobalRange(4),
                "unknown_00454970": GlobalRange(4),
                "unknown_00454980": GlobalRange(4),
                "unknown_0046e490": GlobalRange(4),
                "unknown_0046e690": GlobalRange(4),
                "unknown_0046e7a0": GlobalRange(4),
            },
            extra=[heap, allocator],
            externals={
                "XPhysicalProtect": protect,
                "GetTickCount": lambda harness, arguments, now=now: now,
                "operator_new": new,
                "unknown_00329d20": lambda harness, arguments: (0, (arguments[0],)),
                "unknown_0032a850": lambda harness, arguments: (
                    0,
                    (
                        harness.normalize(arguments[0]),
                        harness.normalize(arguments[1]),
                        harness.normalize(arguments[2]),
                        *arguments[3:7],
                        harness.normalize(arguments[7]),
                        *arguments[8:],
                    ),
                ),
                "network_observer_take_game_channels": take,
            },
        )
