# the things that give the physics its memories are closed

import random
import struct

from difftest import Buffer, Mock, mock_address


def thing(random_, mock: int) -> Buffer:
    """Something that gives memory: where its functions are, and after it the nine of them, of which the last is a mock."""
    data = bytearray(random_.randbytes(0x24 + 9 * 4))
    struct.pack_into("<I", data, 0, 0x24)
    struct.pack_into("<I", data, 0x24 + 8 * 4, mock_address(mock))
    return Buffer(bytes(data), pointers=(0,))


def run(t) -> None:
    random_ = random.Random(650)

    for n in range(200):
        t.check(
            "havok_unknown_00146de0",
            globals={
                "unknown_0047989c": thing(random_, 0) if random_.random() < 0.7 else 0,
                "unknown_004798a0": thing(random_, 1) if random_.random() < 0.7 else 0,
                "unknown_0047988c": random_.getrandbits(32),
                "unknown_00510c48": random_.randbytes(1),
            },
            mocks=[
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
            externals={"texture_cache_return_memory": lambda harness, arguments: 0},
        )
