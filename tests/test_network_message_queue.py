# source/networking/network_message_queue.c

import random
import struct

from difftest import Buffer

SIZE = 0x2850


def run(t) -> None:
    random_ = random.Random(439)

    def queue() -> tuple:
        """One of the things, with some messages to send; the numbers of the first and the last."""
        data = bytearray(random_.randbytes(SIZE))
        size = random_.choice((1, 8, 0x40, 0x100))
        oldest = random_.choice((0, 5, 1000, random_.getrandbits(20)))
        count = random_.randrange(0, min(size, 12) + 1)
        start = random_.randrange(size)
        struct.pack_into("<B3x6i", data, 0x10, 1, size, oldest + count, oldest, size, start, count)
        pending = random_.choice((None, None, 0, count - 1, random_.randrange(max(count, 1))))
        for message in range(count):
            place = 0x2C + ((message + start) % size) * 12
            flags = random_.getrandbits(8) & ~4
            data[place] = flags | 4 if message == pending else flags
        return data, oldest, oldest + count

    for n in range(300):
        data = bytearray(random_.randbytes(SIZE))
        data[4] = random_.choice((0, 1, 1, 0x80))
        data[5] = random_.choice((0, 1, 1, 0x80))
        t.check("network_message_queue_closes", Buffer(bytes(data)), random_.choice((0, 1)) and Buffer(random_.randbytes(4)))
        t.check("network_message_queue_minimum_bits", Buffer(bytes(data)), random_.getrandbits(32), random_.getrandbits(32))

    for n in range(1500):
        data, oldest, newest = queue()
        t.check("network_message_queue_has_something", Buffer(bytes(data)), Buffer(random_.randbytes(4)))
