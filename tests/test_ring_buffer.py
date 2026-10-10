# source/memory/ring_buffer.c

import random
import struct

from difftest import Buffer


def run(t) -> None:
    random_ = random.Random(472)

    def a_ring(size: int, start: int, count: int) -> Buffer:
        """A ring, and after it its memory."""
        return Buffer(struct.pack("<iI2i", size, 0x10, start, count) + random_.randbytes(size + 4), pointers=(4,))

    for n in range(3000):
        size = random_.choice((1, 16, 64, 255))
        offset = random_.randrange(size)
        count = random_.choice((0, 1, size, random_.randrange(size + 1), random_.randrange(size + 1), 0xFFFFFFFF))
        length = 0 if count == 0xFFFFFFFF else count
        t.check("ring_buffer_write", a_ring(size, 0, 0), count, offset, Buffer(random_.randbytes(length + 4)))
        t.check("ring_buffer_read", a_ring(size, 0, 0), offset, count, Buffer(random_.randbytes(length + 4)))
        kept = random_.randrange(size + 1)
        more = random_.choice((0, 1, size - kept, size - kept + 1, random_.randrange(size + 1)))
        t.check("ring_buffer_append", a_ring(size, offset, kept), more, Buffer(random_.randbytes(more + 4)))
