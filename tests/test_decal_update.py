# a decal, each frame

import random
import struct

from difftest import Buffer

from test_decals_delete import COUNT, SIZE, decals


def run(t) -> None:
    random_ = random.Random(664)

    for n in range(400):
        decal_index = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        table = decals(random_)
        data = bytearray(table.data)
        if random_.random() < 0.75:
            # it is the first of its chain
            struct.pack_into("<I", data, 0x4C + (decal_index & 0xFFFF) * SIZE + 0x38, decal_index)
        gone = random_.choice((0, 0, 1))
        t.check(
            "decal_update",
            decal_index,
            globals={"decal_data": Buffer(bytes(data), pointers=table.pointers)},
            externals={"unknown_0017ccd0": lambda harness, arguments, gone=gone: gone},
        )
