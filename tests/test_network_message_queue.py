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

    # ---------- one of the things starts over, and the messages that come

    from difftest import Mock, Pointer, mock_address

    # the heap of the network, what gives its memory with its functions, how many times it was asked, what it answers,
    # and the memory that it gives: a piece of its own each time
    HEAP, ALLOCATOR, TABLE, ASKED, ANSWERS, GIVEN, END = 0x0, 0x8, 0xC, 0x38, 0x3C, 0x40, 0x440

    def heap() -> Buffer:
        data = bytearray(random_.randbytes(END))
        struct.pack_into("<Ii", data, HEAP, ALLOCATOR, random_.choice((0, 5, 5000)))
        struct.pack_into("<I", data, ALLOCATOR, TABLE)
        struct.pack_into("<11I", data, TABLE, mock_address(0), mock_address(1), 0x33, 0x44, 0x55, mock_address(2), 0x77, 0x88, 0x99, 0xAA, mock_address(3))
        struct.pack_into("<I4B", data, ASKED, 0, *random_.choice(((1, 1, 1, 1), (1, 1, 1, 1), (0, 1, 1, 1), (0, 0, 0, 0))))
        return Buffer(bytes(data), pointers=(HEAP, ALLOCATOR))

    def heap_mocks() -> list:
        def gives(call):
            mu = t.harness.mu
            base = call.registers["ecx"] - ALLOCATOR
            asked = struct.unpack("<I", mu.mem_read(base + ASKED, 4))[0]
            mu.mem_write(base + ASKED, struct.pack("<I", asked + 1))
            return base + GIVEN + (asked & 3) * 0x100 if mu.mem_read(base + ANSWERS + (asked & 3), 1)[0] else 0

        return [
            Mock(lambda call: 0, stack_arguments=2, registers=("ecx",)),
            # where the size goes is in the stack
            Mock(lambda call, known=random_.choice((0, 1)): known, stack_arguments=2, registers=("ecx",), record=lambda call: (call.registers["ecx"], call.stack[0])),
            Mock(gives, stack_arguments=3, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
        ]

    def with_received(data: bytearray, wide: bool = False) -> tuple:
        """The messages that came: some, of which some are there; the numbers of the first and the last.

        With `wide`, there can be many, so that a number half of all the numbers before the last is not too far
        before the first; and the first can be just before nothing, so that NONE is a number that can come."""
        size = random_.choice((1, 8, 0x40, 0x200))
        oldest = random_.choice((0, 5, 1000, random_.getrandbits(20)))
        count = random_.randrange(0, min(size, 12) + 1)
        if wide and random_.random() < 0.3:
            size = 0x200
            count = random_.randrange(0x80, 0x200)
        if wide and random_.random() < 0.2:
            oldest = -random_.choice((1, 2, 5, 9)) - count
        start = random_.randrange(size)
        struct.pack_into("<B3x6i", data, 0x182C, 1, size, oldest + count, oldest, size, start, count)
        for message in range(count):
            place = 0x1848 + ((message + start) % size) * 8
            there = random_.random() < 0.5
            # where a message is is not looked at here: a number
            struct.pack_into("<BBHI", data, place, (random_.getrandbits(8) & ~1) | there, random_.getrandbits(8), random_.getrandbits(16), 0x1000 + message if there else 0)
        return oldest, oldest + count, size

    for n in range(600):
        data, oldest, newest = queue()
        start, count = struct.unpack_from("<2i", data, 0x24)
        size = struct.unpack_from("<i", data, 0x14)[0]
        for message in range(newest - oldest):
            struct.pack_into("<I", data, 0x2C + ((message + start) % size) * 12 + 4, random_.choice((0, 0x2000 + message)))
        with_received(data)
        data[4] = random_.choice((0, 1, 1, 1, 0x80))
        memory = heap()
        t.check(
            "network_message_queue_reset",
            Buffer(bytes(data)),
            extra=[memory],
            globals={"unknown_004d87f8": Pointer(memory, HEAP)},
            mocks=heap_mocks(),
        )

    for n in range(2000):
        data, _, _ = queue()
        oldest, newest, size = with_received(data)
        sequence = random_.choice((newest + 1, newest + 1, newest, oldest + 1, oldest, oldest - 2, newest + 3, oldest + size, oldest + size + 1, random_.randrange(oldest, newest + 2)))
        t.check("network_message_queue_get_received", sequence & 0xFFFFFFFF, Buffer(bytes(data)))
        memory = heap()
        length = random_.choice((0, 1, 4, 0x30, 0xFF))
        t.check(
            "network_message_queue_add_received",
            sequence & 0xFFFFFFFF,
            length,
            Buffer(bytes(data)),
            random_.choice((0, 1, 0x100)),
            random_.getrandbits(16),
            Buffer(random_.randbytes(length + 4)),
            extra=[memory],
            globals={"unknown_004d87f8": Pointer(memory, HEAP)},
            mocks=heap_mocks(),
        )

    # ---------- what one of the things writes in a packet, what it reads, and what became of a packet

    MESSAGES, MESSAGE_SPACE = 40, 0x40

    def sending(packet=None) -> Buffer:
        """One of the things with messages to send, each with its bits after the thing; some were sent in a packet."""
        data = bytearray(random_.randbytes(SIZE + MESSAGES * MESSAGE_SPACE + 8))
        size = random_.choice((0x40, 0x100))
        oldest = random_.choice((0, 5, 1000, 0x3F0, random_.getrandbits(20)))
        count = random_.choice((0, 1, 2, 5, 12, MESSAGES, random_.randrange(MESSAGES + 1)))
        start = random_.randrange(size)
        struct.pack_into("<B3x6i", data, 0x10, 1, size, oldest + count, oldest, size, start, count)
        chance = random_.choice((0.05, 0.3, 0.6, 1.0))
        first = random_.random() < 0.5
        places = []
        for message in range(count):
            place = 0x2C + ((message + start) % size) * 12
            flags = random_.getrandbits(8) & ~5
            if random_.random() < chance:
                flags |= 4
            if packet is not None and (random_.random() < 0.7 if message else first):
                flags |= 1
            bits = random_.choice((1, 8, 13, 64, 0x100, 0x100, 0x1F3, 0, random_.randrange(1, 0x101)))
            where = random_.choice((0, SIZE + message * MESSAGE_SPACE)) if packet is not None else SIZE + message * MESSAGE_SPACE
            sent_in = random_.choice((packet, packet, 0xFFFFFFFF, random_.getrandbits(32))) if packet is not None else random_.getrandbits(32)
            struct.pack_into("<BBHII", data, place, flags, random_.getrandbits(8), bits, where, sent_in)
            if where:
                places.append(place + 4)
        data[4] = random_.choice((0, 1, 1, 0x80))
        data[5] = random_.choice((0, 0, 1))
        return Buffer(bytes(data), pointers=tuple(places))

    def stream(size: int, mode: int, position: int, contents: bytes = b"") -> Buffer:
        header = struct.pack("<I4iB3x7i", 0x34, size, 1, mode, position, 0, random_.randrange(4), *(random_.getrandbits(8) for value in range(6)))
        return Buffer(header + contents.ljust(size, b"\0")[:size] + random_.randbytes(8), pointers=(0,))

    for n in range(3000):
        t.check(
            "network_message_queue_write",
            sending(),
            random_.choice((0xFFFFFFFF, 0, 7, random_.getrandbits(32), random_.getrandbits(32))),
            stream(random_.choice((4, 8, 16, 40, 64, 200, 200, 1200)), 1, random_.choice((0, 0, 0, 3, 8, 17))),
            random_.getrandbits(32),
            random_.choice((0, 0, 0, 5, 30, 100, 0x7FFFFFFF, 0xFFFFFFFE, random_.getrandbits(32))),
        )

    class Bits:
        """What a packet has, bit after bit, from the low bit of each byte."""

        def __init__(self) -> None:
            self.value = 0
            self.count = 0

        def put(self, value: int, count: int) -> None:
            self.value |= (value & ((1 << count) - 1)) << self.count
            self.count += count

        def bytes(self) -> bytes:
            return self.value.to_bytes((self.count + 7) // 8 + 1, "little")

    for n in range(3000):
        data, _, _ = queue()
        oldest, newest, size = with_received(data, wide=True)
        bits = Bits()
        number = None
        for message in range(random_.choice((0, 1, 1, 2, 3, 6))):
            how = random_.choice((0, 0, 1, 1, 2)) if number is not None else random_.choice((0, 0, 0, 0, 1, 2))
            if how == 0:
                number = random_.choice((newest + 1, newest + 1, newest + 2, newest, oldest + 1, oldest, oldest - 3, oldest + size, oldest + size + 1, newest + 0x180, newest + 0x181, oldest - 0x180, oldest - 0x181, newest + 0x200, newest - 0x200, newest + 0x1FF, random_.getrandbits(10)))
                bits.put(2, 2)
                bits.put(number, 10)
            elif how == 1:
                number = (number or 0) + 1
                bits.put(3, 2)
            else:
                distance = random_.randrange(16)
                number = (number or 0) + distance + 1
                bits.put(1, 2)
                bits.put(distance, 4)
            if random_.random() < 0.6:
                count = random_.choice((1, 8, 13, 64, 0x100, random_.randrange(1, 0x101)))
                bits.put(1, 1)
                bits.put(count - 1, 8)
            else:
                count = 0x100
                bits.put(0, 1)
            bits.put(random_.getrandbits(count), count)
        if random_.random() < 0.9:
            bits.put(0, 2)
        bits.put(random_.getrandbits(64), 64)
        contents = bits.bytes() if random_.random() < 0.9 else random_.randbytes(40)
        length = random_.choice((len(contents), len(contents), len(contents) + 4, max(len(contents) - 9, 1), 4))
        memory = heap()
        t.check(
            "network_message_queue_read",
            Buffer(bytes(data)),
            Buffer(struct.pack("<I", random_.choice((0xFFFFFFFF, 0, 7, random_.getrandbits(32), random_.getrandbits(32))))),
            stream(length, random_.choice((3, 3, 3, 3, 4)), 0, contents),
            extra=[memory],
            globals={"unknown_004d87f8": Pointer(memory, HEAP)},
            mocks=heap_mocks(),
        )

    for n in range(2000):
        packet = random_.choice((0, 7, 0xFFFFFFFF, random_.getrandbits(32)))
        memory = heap()
        t.check(
            "network_message_queue_packet_result",
            sending(packet),
            packet,
            random_.choice((0, 1, 1, 0x100)),
            extra=[memory],
            globals={"unknown_004d87f8": Pointer(memory, HEAP)},
            mocks=heap_mocks(),
        )

    # ---------- a message is cut in pieces to be sent, and the pieces that came are put together

    KINDS = 0x2D
    PIECES, PIECE = 64, 0x20

    def big_heap() -> Buffer:
        """The heap of the network, with a piece of memory of its own for each of many things that it gives."""
        data = bytearray(random_.randbytes(GIVEN + PIECES * PIECE))
        struct.pack_into("<Ii", data, HEAP, ALLOCATOR, random_.choice((0, 5, 5000)))
        struct.pack_into("<I", data, ALLOCATOR, TABLE)
        struct.pack_into("<11I", data, TABLE, mock_address(0), mock_address(1), 0x33, 0x44, 0x55, mock_address(2), 0x77, 0x88, 0x99, 0xAA, mock_address(3))
        struct.pack_into("<I4B", data, ASKED, 0, *random_.choice(((1, 1, 1, 1), (1, 1, 1, 1), (1, 1, 1, 1), (0, 1, 1, 1), (1, 0, 1, 1), (1, 1, 0, 0), (0, 0, 0, 0))))
        return Buffer(bytes(data), pointers=(HEAP, ALLOCATOR))

    def big_heap_mocks() -> list:
        def gives(call):
            mu = t.harness.mu
            base = call.registers["ecx"] - ALLOCATOR
            asked = struct.unpack("<I", mu.mem_read(base + ASKED, 4))[0]
            mu.mem_write(base + ASKED, struct.pack("<I", asked + 1))
            return base + GIVEN + (asked % PIECES) * PIECE if mu.mem_read(base + ANSWERS + (asked & 3), 1)[0] else 0

        return [
            Mock(lambda call: 0, stack_arguments=2, registers=("ecx",)),
            Mock(lambda call, known=random_.choice((0, 1)): known, stack_arguments=2, registers=("ecx",), record=lambda call: (call.registers["ecx"], call.stack[0])),
            Mock(gives, stack_arguments=3, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            # whether one of the things cannot send: what its byte says
            Mock(lambda call: t.harness.mu.mem_read(call.registers["ecx"] + 5, 1)[0], stack_arguments=1, registers=("ecx",)),
        ]

    def with_kinds(data: bytearray, kinds=None) -> tuple:
        """After one of the things, its functions and the kinds of message; what is a pointer."""
        functions = len(data)
        data += struct.pack("<2I", 0x11, mock_address(4))
        table = len(data)
        for kind in range(KINDS):
            valid, least, most = kinds[kind] if kinds else (1, 0, 0xFFFF)
            data += struct.pack("<B3x2i2i3I", valid, 0, 0, least, most, mock_address(5), mock_address(5), 0x33)
        struct.pack_into("<I", data, 0, functions)
        struct.pack_into("<I", data, 0xC, table)
        return (0, 0xC)

    def stream_state(call) -> bytes:
        # but for the number of 0x14 of the stream: the game does not set it before this, so it has what was on the
        # stack, which is not the same when other tests ran before (the full battery of milestone 533)
        state = bytes(t.harness.mu.mem_read(call.stack[0] + 4, 0x18))
        return state[:0x10] + state[0x14:]

    for n in range(700):
        data = bytearray(random_.randbytes(SIZE))
        size = random_.choice((1, 4, 0x40, 0x100, 0x100))
        oldest = random_.choice((0, 5, 1000, random_.getrandbits(20)))
        count = random_.choice((0, 0, 1, size - 1, size, random_.randrange(size + 1)))
        struct.pack_into("<B3x6i", data, 0x10, 1, size, oldest + count, oldest, size, random_.randrange(size), count)
        data[5] = random_.choice((0, 0, 0, 0, 0, 1))
        pointers = with_kinds(data)
        length = random_.choice((0, 1, 5, 24, 25, 26, 27, 57, 58, 100, 300, 1000))
        message = Buffer(random_.randbytes(length + 4))

        def encodes(call):
            """What the function of a kind writes: the message, and some bits more with nothing."""
            mu = t.harness.mu
            where, position = struct.unpack("<I12xi", mu.mem_read(call.stack[0], 0x14))
            mu.mem_write(where + position // 8, bytes(mu.mem_read(call.stack[2], call.stack[1])))
            mu.mem_write(call.stack[0] + 0x10, struct.pack("<i", position + call.stack[1] * 8 + call.stack[1] % 5))
            return 0

        memory = big_heap()
        t.check(
            "network_message_queue_send",
            Buffer(bytes(data), pointers=pointers),
            random_.randrange(KINDS),
            length,
            message,
            extra=[memory],
            globals={"unknown_004d87f8": Pointer(memory, HEAP)},
            mocks=big_heap_mocks() + [Mock(encodes, stack_arguments=3, record=lambda call: (stream_state(call), bytes(t.harness.mu.mem_read(call.stack[0] + 0x2C, 8)), call.stack[1], call.stack[2]))],
        )

    def checksum(data: bytes) -> int:
        crc = 0xFFFFFFFF
        for value in data:
            for bit in range(8):
                crc = (crc >> 1) ^ (0xEDB88320 if (crc ^ value) & 1 else 0)
                value >>= 1
        return crc

    assert checksum(b"123456789") == 0x340BC6D9

    for n in range(1200):
        data = bytearray(random_.randbytes(SIZE))
        size = random_.choice((8, 0x40, 0x200))
        oldest = random_.choice((0, 5, 1000, random_.getrandbits(20)))
        start = random_.randrange(size)
        kinds = [(random_.choice((1, 1, 1, 1, 0)), random_.choice((0, 0, 0, 3)), random_.choice((0xFFFF, 0xFFFF, 40))) for kind in range(KINDS)]
        pieces = []
        # the messages that came: each a checksum, its kind, its size and its bytes, in pieces
        for message in range(random_.choice((0, 1, 1, 1, 2, 3))):
            length = random_.choice((0, 1, 5, 24, 25, 26, 27, 57, 58, 100))
            kind = random_.choice((random_.randrange(KINDS), random_.randrange(KINDS), random_.randrange(KINDS), KINDS, 0xFF))
            body = struct.pack("<BH", kind, length) + random_.randbytes(length)
            bits = len(body) * 8 + 32 - random_.choice((0, 0, 0, 3))
            packed = struct.pack("<I", checksum(b"\xff\xff\xff\xff" + body)) + body
            if random_.random() < 0.1:
                packed = bytearray(packed)
                packed[random_.randrange(len(packed))] ^= 1 << random_.randrange(8)
                packed = bytes(packed)
            while bits > 0x100:
                pieces.append((packed[:0x20], 0x100, False))
                packed = packed[0x20:]
                bits -= 0x100
            pieces.append((packed, bits, True))
        pieces = pieces[:size]
        how = random_.choice((0, 0, 0, 0, 1, 2, 3))
        pointers = []
        for index, (piece, bits, last) in enumerate(pieces):
            place = 0x1848 + ((index + start) % size) * 8
            flags = (random_.getrandbits(8) & ~3) | 1 | (2 if last else 0)
            if how == 1 and index == 0 or how == 2 and index == len(pieces) - 1:
                # one that did not come
                flags &= ~1
            if how == 3:
                # none is the last piece
                flags &= ~2
            where = len(data)
            data += piece.ljust(0x20, b"\xee")
            struct.pack_into("<BBHI", data, place, flags, random_.getrandbits(8), bits, where)
            pointers.append(place + 4)
        struct.pack_into("<B3x6i", data, 0x182C, 1, size, oldest + len(pieces), oldest, size, start, len(pieces))
        data[5] = random_.choice((0, 0, 1))
        pointers += with_kinds(data, kinds)
        good = random_.choice((1, 1, 1, 0))

        def decodes(call, good=good):
            """What the function of a kind reads: the message is the bytes that there are."""
            mu = t.harness.mu
            where, position = struct.unpack("<I12xi", mu.mem_read(call.stack[0], 0x14))
            mu.mem_write(call.stack[2], bytes(mu.mem_read(where + position // 8, call.stack[1])))
            mu.mem_write(call.stack[0] + 0x10, struct.pack("<i", position + call.stack[1] * 8))
            return good

        memory = heap()
        t.check(
            "network_message_queue_receive",
            Buffer(bytes(data), pointers=tuple(pointers)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(0x80)),
            extra=[memory],
            globals={"unknown_004d87f8": Pointer(memory, HEAP)},
            mocks=big_heap_mocks() + [Mock(decodes, stack_arguments=3, record=lambda call: (stream_state(call), call.stack[1], call.stack[2], bytes(t.harness.mu.mem_read(call.stack[2], call.stack[1]))))],
        )
