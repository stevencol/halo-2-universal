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

    # ---------- one of the things starts over, and the packets that it sends

    from difftest import Mock, mock_address

    def at_time() -> dict:
        now = random_.getrandbits(32)
        while 0x20000000 <= now < 0x21000000:
            now = random_.getrandbits(32)
        return {
            "now": now,
            "globals": {"unknown_00510548": bytes((random_.choice((0, 0, 1)),)), "unknown_0051054c": struct.pack("<I", now)},
            "externals": {"GetTickCount": lambda harness, arguments, now=now: now},
        }

    for n in range(300):
        setup = at_time()
        t.check(
            "network_connection_reset",
            Buffer(random_.randbytes(SIZE)),
            globals=dict(
                setup["globals"],
                unknown_004cf6e8=struct.pack("<I", random_.choice((0, 1, 7, 1000, random_.getrandbits(32)))),
                **{name: struct.pack("<I", random_.getrandbits(32)) for name in ("unknown_004cf70c", "unknown_004cf710", "unknown_004cf714", "unknown_004cf718")},
            ),
            externals=setup["externals"],
        )

    for n in range(1000):
        data, (came, (oldest, newest)) = connection(empty=random_.random() < 0.2)
        size, _, _, capacity, start, count = struct.unpack_from("<6i", data, 0x2C)
        struct.pack_into("<i", data, 0x40, random_.choice((newest - oldest, newest - oldest, 0)))
        t.check(
            "network_connection_window_advance",
            Buffer(bytes(data[0x28:0x44])),
            random_.choice((oldest, oldest + 1, newest, newest + 1, oldest - 1, oldest + random_.randrange(0, 0x100))) & 0xFFFFFFFF,
        )

    def sending(full: bool = False) -> tuple:
        """One of the things, with its functions after it, and packets that it sent, each at a time."""
        setup = at_time()
        data, (came, (oldest, newest)) = connection()
        size = struct.unpack_from("<i", data, 0x2C)[0]
        if full:
            newest = oldest + size
            struct.pack_into("<i", data, 0x30, newest)
        struct.pack_into("<i", data, 0x40, newest - oldest)
        struct.pack_into("<i", data, 0x38, size)
        start = struct.unpack_from("<i", data, 0x3C)[0]
        wait = random_.choice((0, 100, 1000))
        struct.pack_into("<i", data, 0x974, wait)
        for packet in range(newest - oldest):
            place = 0x14C + ((packet + start) % size) * 16
            # sent just so long ago that it has been waited for, or not
            struct.pack_into("<I", data, place, (setup["now"] - wait - 50 - random_.choice((-1, 0, 0, 1, 500, -500))) & 0xFFFFFFFF)
            struct.pack_into("<H", data, place + 0xE, random_.choice((0, 0, 0, 1, 4, 2, 0xFFF8)))
        struct.pack_into("<I", data, 0, SIZE)
        buffer = bytes(data) + struct.pack("<II", 0x11, mock_address(0))
        return setup, buffer, oldest, newest

    for n in range(1500):
        setup, buffer, oldest, newest = sending(random_.random() < 0.3)
        data = bytearray(buffer)
        # the number that they are counted from: the first one, mostly, or one before or far from it
        struct.pack_into("<i", data, 0x44, random_.choice((oldest + 1, oldest + 1, newest, oldest, newest - 0x7F, newest - 0x7E, newest + 5)))
        struct.pack_into("<i", data, 0x94C, random_.choice((newest, newest, oldest, newest - 0x7F, newest - 0x80, newest - 0x81)))
        globals_ = dict(setup["globals"], unknown_004cf6f4=struct.pack("<i", 50))
        t.check(
            "network_connection_drop_oldest_sent",
            Buffer(bytes(data), pointers=(0,)),
            random_.choice((0, 0, 0, 1, 0x100)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            globals=globals_,
            externals=setup["externals"],
        )
        t.check(
            "network_connection_new_sequence",
            Buffer(bytes(data), pointers=(0,)),
            random_.getrandbits(32),
            globals=globals_,
            externals=setup["externals"],
            mocks=[Mock(lambda call, cannot=random_.choice((0, 0, 0, 1, 0x100)): cannot, stack_arguments=1, registers=("ecx",))],
        )

    # ---------- what became of the packets that were sent

    for n in range(2500):
        setup, buffer, oldest, newest = sending(random_.random() < 0.2)
        data = bytearray(buffer)
        size, _, _, _, start, count = struct.unpack_from("<6i", data, 0x2C)
        turn = random_.choice((oldest, oldest, oldest, newest, newest - 1, random_.randrange(oldest, newest + 1)))
        struct.pack_into("<i", data, 0x44, max(turn, oldest))
        struct.pack_into("<i", data, 0x978, random_.choice((0, 0, 30, 1000)))
        struct.pack_into("<i", data, 0x94C, random_.choice((turn, turn + 1, turn + 3, turn + 4, turn + 5, newest)))
        for packet in range(newest - oldest):
            # answered or not, said before its turn or not, and with other bits
            place = 0x14C + ((packet + start) % size) * 16
            struct.pack_into("<H", data, place + 0xE, random_.choice((0, 0, 0, 1, 1, 3, 2, 0x20, 0x21)))
        globals_ = dict(
            setup["globals"],
            unknown_004cf6f0=struct.pack("<i", random_.choice((3, 3, 100))),
            unknown_004cf71c=struct.pack("<i", random_.choice((0, 10, 100))),
            unknown_004cf720=struct.pack("<i", random_.choice((50, 1000))),
        )
        t.check(
            "network_connection_next_result",
            Buffer(bytes(data), pointers=(0,)),
            *(Buffer(random_.randbytes(4)) for value in range(4)),
            globals=globals_,
            externals=setup["externals"],
        )
        t.check(
            "network_connection_next_event",
            Buffer(bytes(data), pointers=(0,)),
            *(Buffer(random_.randbytes(4)) for value in range(3)),
            globals=globals_,
            externals=setup["externals"],
        )

    # ---------- what the other console says of the packets

    for n in range(3000):
        setup, buffer, oldest, newest = sending(random_.random() < 0.3)
        data = bytearray(buffer)
        size, _, _, _, start, count = struct.unpack_from("<6i", data, 0x2C)
        turn = random_.choice((oldest, oldest, newest, random_.randrange(oldest, newest + 1)))
        struct.pack_into("<i", data, 0x44, turn)
        for packet in range(newest - oldest):
            place = 0x14C + ((packet + start) % size) * 16
            struct.pack_into("<H", data, place + 0xC, random_.choice((0, 1, 5, newest - oldest, 0xFFFF)))
            struct.pack_into("<H", data, place + 0xE, random_.choice((0, 0, 0, 0, 1, 8, 9, 0x20)))
        # the answer: of the last one, of one that is kept, of the first, or of one before
        of = random_.choice((newest, newest, random_.randrange(oldest, newest + 1), oldest + 1, oldest, oldest - 3, oldest - 40, newest - 255, newest + 1))
        bit_count = random_.choice((0, 0, 1, 8, 32, 33, 64, 128))
        bits = random_.choice((bytes(16), bytes((0xFF,)) * 16, random_.randbytes(16), random_.randbytes(16)))
        t.check(
            "network_connection_acknowledge",
            Buffer(bytes(data), pointers=(0,)),
            of & 0xFF,
            bit_count,
            Buffer(bits + random_.randbytes(4)),
            random_.choice((0, 1, 1, 0x100)),
            globals=dict(
                setup["globals"],
                unknown_004cf6ec=struct.pack("<i", random_.choice((0, 5, 5, 0x40))),
                unknown_004cf6f8=struct.pack("<I", 3),
                unknown_004cf6fc=struct.pack("<I", 2),
                unknown_004cf700=struct.pack("<I", 4),
                unknown_004cf704=struct.pack("<I", random_.choice((0, 50))),
                unknown_004cf708=struct.pack("<I", random_.choice((0, 100))),
                unknown_004cf724=struct.pack("<I", random_.choice((0, 10))),
            ),
            externals=setup["externals"],
        )
