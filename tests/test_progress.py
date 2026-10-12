# saying that a long piece of work starts and ends

import random
import struct

from difftest import Buffer, GlobalData, Mock, mock_address

SIZE = 0x12C


def globals_(random_) -> tuple:
    """What is kept, with some of the four functions that are called; and their mocks."""
    data = bytearray(random_.randbytes(SIZE))
    mocks = []
    for offset in (0, 8, 0x10, 0x18):
        if random_.random() < 0.6:
            struct.pack_into("<I", data, offset, mock_address(len(mocks)))
            mocks.append(Mock(lambda call: 0, stack_arguments=2))
        else:
            struct.pack_into("<I", data, offset, 0)
    data[0x20] = random_.choice((0, 0, 0, 1, 0x80))
    data[0x21] = random_.choice((0, 1, 1, 0x80))
    return GlobalData(bytes(data)), mocks


def run(t) -> None:
    random_ = random.Random(643)

    for n in range(400):
        data, mocks = globals_(random_)
        t.check("progress_done", globals={"progress_globals": data}, mocks=mocks)

        data, mocks = globals_(random_)
        length = random_.choice((0, 1, 22, 0xFE, 0xFF, 0x100, 0x140))
        name = bytes(random_.randint(1, 255) for _ in range(length)) + bytes(1) + random_.randbytes(8)
        t.check("progress_new", Buffer(name), globals={"progress_globals": data}, mocks=mocks)
