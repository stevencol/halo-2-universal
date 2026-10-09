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

    # ---------- a whole stream: with what it keeps after its position

    def whole(size: int, position: int, state: int | None = None, blank: bool = False) -> Buffer:
        data = bytearray(random_.randbytes(size))
        if blank:
            for bit in range(max(position, 0), size * 8):
                data[bit >> 3] &= ~(1 << (bit & 7))
        count = random_.choice((1, 2, 3, 4))
        kept = [random_.choice((0, 1, 7, 8, 9, 31, 33, max(size * 8 - 12, 0), size * 8 - 1, size * 8, size * 8 + 5, max(position - 1, 0), position, position + 1)) for n in range(4)]
        header = struct.pack(
            "<I4iB3xi4iii",
            0x34,
            size,
            random_.getrandbits(31),
            random_.choice((1, 1, 3, 2)) if state is None else state,
            position,
            random_.choice((0, 0, 1, 0x80)),
            count,
            *kept,
            random_.getrandbits(31),
            random_.getrandbits(31),
        )
        # what is after the data is not the stream's to write
        return Buffer(header + bytes(data) + random_.randbytes(8), pointers=(0,))

    for size in (1, 4, 8, 12, 40):
        for position in (*positions(random_, size), -1, -8, -9):
            t.check("bitstream_overflowed", whole(size, position))
            for value in (0, 1, 0x80, 1):
                t.check("bitstream_write_bool", whole(size, position, blank=True), value)
                t.check("bitstream_pop_position", whole(size, position), value)
            for bit_count in (0, 1, 2, 7, 8, 16, 31, 32, 33):
                for value in (0, 1, (1 << (bit_count & 31)) - 1, 1 << (bit_count & 31), random_.getrandbits(32)):
                    t.check(
                        "bitstream_write_integer_checked",
                        value & 0xFFFFFFFF,
                        bit_count,
                        whole(size, max(position, 0), blank=True),
                        # the line of text that is made for a number that does not fit
                        externals={"csprintf256": lambda harness, arguments: (0, (arguments[2],))},
                    )

    for n in range(200):
        size = random_.choice((4, 8, 12, 40, 3))
        data = bytearray(whole(size, random_.getrandbits(8)).data)
        if n % 2:
            # the word of a build for testing, whole or not
            data[0x34 : 0x34 + min(size, 4)] = b"gbed"[: min(size, 4)]
            if n % 8 == 1:
                data[0x34] ^= 1
        t.check("bitstream_begin_reading", Buffer(bytes(data), pointers=(0,)))

    for n in range(300):
        size = random_.choice((2, 4, 8, 12, 40))
        data = bytearray(whole(size, random_.choice((0, 0, 8, 16, 3, size * 8 - 16, size * 8))).data)
        # a text that ends, sooner or later, or does not
        end = random_.choice((0, 2, 4, 6, 10, None))
        if end is not None and 0x34 + end + 2 <= len(data):
            data[0x34 + end : 0x34 + end + 2] = b"\0\0"
        maximum = random_.choice((1, 2, 3, 5, 8, 30))
        t.check("bitstream_read_wide_string", Buffer(bytes(data), pointers=(0,)), Buffer(random_.randbytes(maximum * 2 + 2)), maximum)