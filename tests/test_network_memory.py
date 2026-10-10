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
