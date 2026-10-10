# source/networking/network_connection.c

import random
import struct

from difftest import Buffer

SIZE = 0x97C


def run(t) -> None:
    random_ = random.Random(432)

    def number() -> int:
        return random_.choice((0, 1, 50, 100, 1000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0xFFFFFF00, random_.getrandbits(32), random_.getrandbits(10)))

    for n in range(3000):
        data = bytearray(random_.randbytes(SIZE))
        for offset in (0x94C, 0x968, 0x96C, 0x970, 0x978):
            struct.pack_into("<I", data, offset, number())
        time = number()
        if random_.random() < 0.2:
            # just as the least, the most and the smoothed ones
            time = struct.unpack_from("<I", data, random_.choice((0x968, 0x96C)))[0]
        t.check(
            "network_connection_update_times",
            random_.choice((1, 1, 3, 3, 4, 5, 0, 2, 6, random_.getrandbits(32))),
            Buffer(bytes(data)),
            time,
            random_.choice((number(), struct.unpack_from("<I", data, 0x94C)[0])),
            number(),
            globals={
                "unknown_004cf6f8": struct.pack("<I", random_.choice((0, 1, 3, 3, 31, 32, 35))),
                "unknown_004cf6fc": struct.pack("<I", random_.choice((0, 1, 2, 2, 31, 34))),
                "unknown_004cf700": struct.pack("<I", random_.choice((0, 1, 4, 4, 0xFFFFFFFF, 1000))),
                "unknown_004cf704": struct.pack("<I", number()),
                "unknown_004cf708": struct.pack("<I", number()),
                "unknown_004cf71c": struct.pack("<I", number()),
                "unknown_004cf720": struct.pack("<I", number()),
                "unknown_004cf724": struct.pack("<I", number()),
            },
        )

    # ---------- the packets that came, and the ones that were sent

    def connection(most: int | None = None, empty: bool = False) -> tuple:
        """One of the things, with some packets that came and some that it sent; the two numbers of each window."""
        data = bytearray(random_.randbytes(SIZE))
        numbers = []
        for window in (0x10, 0x2C):
            size = random_.choice((1, 8, 0x40, 0x80))
            oldest = random_.choice((0, 5, 1000, 0x7FFFFF00, random_.getrandbits(20)))
            count = 0 if empty else random_.randrange(0, size + 1)
            if most is not None:
                count = min(count, most)
            start = random_.randrange(size)
            struct.pack_into("<6i", data, window, size, oldest + count, oldest, random_.choice((size, size, size + 3)), start, random_.choice((count, count, count, 0)))
            numbers.append((oldest, oldest + count))
        return data, numbers

    for n in range(1500):
        data, ((came_oldest, came_newest), (sent_oldest, sent_newest)) = connection()
        sequence = random_.choice((sent_oldest, sent_oldest + 1, sent_newest, sent_newest + 1, sent_oldest - 3, 0xFFFFFFFF, random_.randrange(sent_oldest, sent_newest + 2)))
        t.check("network_connection_get_sent_packet", sequence & 0xFFFFFFFF, Buffer(bytes(data)))
        if sent_newest > sent_oldest:
            # one that there is: the game does not look
            struct.pack_into("<i", data, 0x40, sent_newest - sent_oldest)
            t.check(
                "network_connection_set_sent_size",
                random_.randrange(sent_oldest + 1, sent_newest + 1),
                Buffer(bytes(data)),
                random_.choice((0, 48, 1200, 0xFFFFFFFF, random_.getrandbits(32))),
            )

    for n in range(1500):
        data, ((came_oldest, came_newest), sent) = connection(most=0x30)
        size = struct.unpack_from("<i", data, 0x10)[0]
        # the count is right, and the packet is one that there is or one after them, but no more than the ring has
        struct.pack_into("<i", data, 0x24, came_newest - came_oldest)
        room = size - (came_newest - came_oldest)
        sequence = random_.choice((0xFFFFFFFF, came_newest, came_newest + min(room, 1), came_newest + room, came_oldest + 1 if came_newest > came_oldest else came_newest + min(room, 1)))
        if sequence != 0xFFFFFFFF and not came_oldest < sequence:
            sequence = 0xFFFFFFFF
        t.check("network_connection_received", sequence, Buffer(bytes(data)))

    for n in range(1500):
        data, ((came_oldest, came_newest), sent) = connection()
        size, newest, oldest, capacity, start, count = struct.unpack_from("<6i", data, 0x10)
        data[0xC] = random_.choice((0, 1, 1, 1, 0x80))
        # the count is right: with a wrong one the game looks where there is nothing
        struct.pack_into("<i", data, 0x24, came_newest - came_oldest)
        # the first ones came, up to one that did not
        came = random_.randrange(0, came_newest - came_oldest + 1)
        for packet in range(came_newest - came_oldest):
            place = (packet + start) % size
            word = random_.getrandbits(16)
            word = word | 1 if packet < came else word & ~1
            struct.pack_into("<H", data, 0x4C + place * 2, word)
        t.check("network_connection_advance_received", Buffer(bytes(data)))
