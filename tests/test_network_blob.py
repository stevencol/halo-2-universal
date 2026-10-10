# source/networking/network_blob.c

import random
import struct
import zlib

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(491)

    for n in range(3000):
        size = random_.choice((0x10, 0x10, 0x11, 0x40, 0x123))
        blob = bytearray(random_.randbytes(size))
        version = random_.choice((1, 5, 0x80000000))
        said = random_.choice((size, size, size, size, size + 1, size - 1))
        struct.pack_into("<2I", blob, 4, random_.choice((version, version, version, version - 1, version + 1)), said)
        # the checksum that the game makes is the common one, not turned over at the end
        good = zlib.crc32(bytes(blob[4:])) ^ 0xFFFFFFFF
        struct.pack_into("<I", blob, 0, random_.choice((good, good, good, good ^ 1)))
        t.check("network_blob_checksum", Buffer(bytes(blob)), size)
        definition = bytearray(random_.randbytes(0x40))
        struct.pack_into("<I", definition, 0, 0x30)
        struct.pack_into("<H", definition, 8, random_.choice((4, 4, 1, 3)))
        struct.pack_into("<I", definition, 0x28, random_.choice((size, size, size, size + 8, size - 1, 0x1000)))
        struct.pack_into("<I", definition, 0x38, version)
        t.check(
            "network_blob_verify",
            random_.choice((size, size, size, size, 0xF, 8, 0)),
            Buffer(bytes(blob)),
            Buffer(bytes(definition), pointers=(0,)),
        )
