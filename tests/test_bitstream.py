# source/networking/bitstream.c

import random
import struct

from difftest import Buffer

STREAM_SIZE = 0x14


def stream(random_: random.Random, size: int, position: int, blank_from_position: bool = False) -> Buffer:
    """A stream over `size` bytes of anything, at a position in bits; the data follows the stream in the buffer.

    With blank_from_position, the data from the position on is zeros, as the
    functions that write take it to be.
    """
    data = bytearray(random_.randbytes(size))
    if blank_from_position:
        for bit in range(position, size * 8):
            data[bit >> 3] &= ~(1 << (bit & 7))
    header = struct.pack("<I4i", STREAM_SIZE, size, random_.getrandbits(31), random_.getrandbits(31), position)
    return Buffer(header + bytes(data), pointers=(0,))


def positions(random_: random.Random, size: int) -> list[int]:
    """Positions at whole bytes, at whole words, anywhere, and at and past the end."""
    bits = size * 8
    return [
        0,
        8,
        32,
        bits - 1,
        bits,
        bits + 5,
        *(random_.randrange(0, bits) for _ in range(6)),
        *(random_.randrange(0, bits, 8) for _ in range(3)),
    ]


def run(t) -> None:
    random_ = random.Random(7)

    for size in (4, 8, 12, 40):
        for position in positions(random_, size):
            t.check("bitstream_read_bool", stream(random_, size, position))

            for bit_count in (0, 1, 2, 7, 8, 9, 16, 17, 31, 32):
                t.check("bitstream_read_integer", stream(random_, size, position), bit_count)
                t.check(
                    "bitstream_write_integer",
                    stream(random_, size, position, blank_from_position=True),
                    random_.getrandbits(32),
                    bit_count,
                )

            for bit_count in (0, 1, 5, 8, 13, 31, 32, 33, 64, 70, 100, size * 8, size * 8 + 9):
                # the buffer has the words that the count needs
                room = ((bit_count + 31) >> 5) * 4 + 4
                t.check("bitstream_read_bits", stream(random_, size, position), Buffer(random_.randbytes(room)), bit_count)
                t.check(
                    "bitstream_write_bits",
                    stream(random_, size, position, blank_from_position=True),
                    Buffer(random_.randbytes(room)),
                    bit_count,
                )
