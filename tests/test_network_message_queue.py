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

    def with_received(data: bytearray) -> tuple:
        """The messages that came: some, of which some are there; the numbers of the first and the last."""
        size = random_.choice((1, 8, 0x40, 0x200))
        oldest = random_.choice((0, 5, 1000, random_.getrandbits(20)))
        count = random_.randrange(0, min(size, 12) + 1)
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
