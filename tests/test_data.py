# source/memory/data.c

import random
import struct

from difftest import SCRATCH_ADDRESS, Buffer, Mock, mock_address

DATA_ARRAY_SIZE = 0x4C
FILL_DELETED_FLAG = 1 << 3
NONE = -1


def array(
    random_: random.Random,
    maximum_count: int,
    size: int,
    in_use: dict[int, int],
    prefix: bytes = b"",
    flags: int = 0,
    next_index: int | None = None,
) -> Buffer:
    """A data array with the elements of in_use (the identifier of each absolute
    index), as the game keeps one: the array, its elements and its vector of bits.

    `prefix` goes before the array in the buffer; the pointers of the prefix
    are the caller's to give.
    """
    first_unallocated = max(in_use, default=-1) + 1
    free = [n for n in range(maximum_count) if n not in in_use]
    if next_index is None:
        # at the first free element, or before it
        next_index = random_.randint(0, free[0]) if free else maximum_count

    datums = bytearray()
    for n in range(maximum_count):
        # what is left of an element after its identifier is anything
        datum = bytearray(random_.randbytes(size))
        struct.pack_into("<h", datum, 0, in_use.get(n, 0))
        datums += datum
    bits = bytearray(((maximum_count + 31) >> 5) * 4)
    for n in in_use:
        bits[n >> 3] |= 1 << (n & 7)

    base = len(prefix)
    header = bytearray(DATA_ARRAY_SIZE)
    header[0:5] = b"test\0"
    struct.pack_into(
        "<2iBBHIIiiihHII",
        header,
        0x20,
        maximum_count,
        size,
        0,  # alignment bits
        1,  # valid
        flags,
        0x64407440,  # 'd@t@'
        0,  # allocation
        next_index,
        first_unallocated,
        len(in_use),
        random_.choice((-0x8000, -2, -1 & 0x7FFF, 0x1234 - 0x8000)),  # next identifier
        0,
        base + DATA_ARRAY_SIZE,  # the elements
        base + DATA_ARRAY_SIZE + len(datums),  # the vector of bits
    )
    return Buffer(prefix + bytes(header) + bytes(datums) + bytes(bits), pointers=(base + 0x44, base + 0x48))


def identifier(random_: random.Random) -> int:
    return random_.choice((-0x8000, -0x7FFF, -1, 1, 2, 0x7FFF, random_.randint(-0x8000, 0x7FFF) or 1))


def arrays(random_: random.Random, count: int, **options: object):
    """Arrays of different sizes and with different elements in use; each with its elements in use."""
    for _ in range(count):
        maximum_count = random_.choice((1, 2, 5, 31, 32, 33, 40, 64, 70))
        size = random_.choice((2, 3, 4, 6, 12, 17))
        fill = random_.choice((0.0, 0.3, 0.7, 1.0))
        in_use = {n: identifier(random_) for n in range(maximum_count) if random_.random() < fill}
        yield array(random_, maximum_count, size, in_use, **options), in_use, maximum_count


def index(absolute_index: int, identifier_: int) -> int:
    return ((identifier_ & 0xFFFF) << 16 | absolute_index) - (1 << 32 if identifier_ < 0 else 0)


def some_indices(random_: random.Random, in_use: dict[int, int], maximum_count: int) -> list[int]:
    """Indices of elements in use, of elements not in use, and with the wrong identifier."""
    indices = [NONE, 0, index(maximum_count - 1, 1), index(maximum_count, 1), index(0xFFFF, 0x7FFF)]
    for absolute_index in random_.sample(range(maximum_count), min(maximum_count, 6)):
        indices.append(index(absolute_index, in_use.get(absolute_index, 5)))
        indices.append(index(absolute_index, (in_use.get(absolute_index, 5) + 1) or 1))
        indices.append(absolute_index)
    return indices


def run(t) -> None:
    random_ = random.Random(2)

    # the functions that only read
    for data, in_use, maximum_count in arrays(random_, 60):
        for value in some_indices(random_, in_use, maximum_count):
            t.check("datum_try_and_get", data, value)
            t.check("data_next_index", data, value)
            t.check("data_prev_index", data, value)
        for absolute_index in (NONE, -2, 0, 1, maximum_count - 1, maximum_count, maximum_count + 40, 0x10000):
            t.check("datum_try_and_get_absolute", data, absolute_index)
            t.check("data_next_absolute_index", data, absolute_index)
        for absolute_index in [NONE, *random_.sample(range(maximum_count), min(maximum_count, 4))]:
            t.check("datum_absolute_index_to_index", data, absolute_index)
        t.check("data_last_index", data)

    # the functions that make and delete elements, with and without the fill of the deleted ones
    for flags in (0, FILL_DELETED_FLAG):
        for data, in_use, maximum_count in arrays(random_, 60, flags=flags):
            t.check("datum_new", data)
            # the elements where they are, after the array, which is the
            # first buffer; and fewer of them than the array has
            for count in {maximum_count, max(maximum_count - 3, 0)}:
                t.check("data_connect", data, count, SCRATCH_ADDRESS + DATA_ARRAY_SIZE)
            t.check("data_delete_all", data)
            t.check("data_make_valid", data)
            for value in some_indices(random_, in_use, maximum_count)[1:]:
                t.check("datum_new_at_index", data, value & 0xFFFFFFFF)
            for absolute_index in (NONE, 0, 1, maximum_count - 1, maximum_count, *random_.sample(range(maximum_count), min(maximum_count, 4))):
                t.check("datum_new_at_absolute_index", data, absolute_index)
            for absolute_index in random_.sample(sorted(in_use), min(len(in_use), 5)):
                t.check("datum_delete", data, index(absolute_index, in_use[absolute_index]))

    # an element of each size, made from each next identifier
    for size in (2, 3, 4, 5, 8, 13, 64):
        for next_identifier in (-0x8000, -2, -1, 0, 1, 0x7FFE, 0x7FFF):
            data = bytearray(array(random_, 4, size, {}).data)
            struct.pack_into("<h", data, 0x40, next_identifier)
            # the element is in a buffer of its own
            t.check("datum_initialize", Buffer(bytes(data)), Buffer(random_.randbytes(size)))

    # the iterator, before each element; it is before the array in the buffer
    for _ in range(40):
        maximum_count = random_.choice((1, 5, 32, 33, 70))
        in_use = {n: identifier(random_) for n in range(maximum_count) if random_.random() < 0.4}
        for absolute_index in (NONE, 0, maximum_count - 1, maximum_count, *random_.sample(range(maximum_count), min(maximum_count, 3))):
            iterator = struct.pack("<Iii4x", 16, random_.randint(-5, 5), absolute_index)
            data = array(random_, maximum_count, 8, in_use, prefix=iterator)
            t.check("data_iterator_next", Buffer(data.data, pointers=(0, *data.pointers)))

    # an array made by an allocator and freed by it. The allocator is an
    # object with a table of two functions, which the test stands in for
    allocator = Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,))
    for alignment_bits in (0, 2, 5):
        for maximum_count, size in ((1, 2), (5, 12), (33, 6), (70, 3)):
            memory = Buffer(random_.randbytes(0x400))
            # with memory, and with none
            for give in (True, False):
                mocks = [
                    Mock(lambda call, memory=memory, give=give: memory.address if give else 0, stack_arguments=1, registers=("ecx",)),
                    Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                ]
                t.check(
                    "data_new", Buffer(b"some objects\0"), maximum_count, size, alignment_bits, allocator,
                    mocks=mocks, extra=[memory],
                )
    for data, _, _ in arrays(random_, 20):
        # an array with an allocator, which is after it in the buffer, and one with none
        for allocated in (True, False):
            array_ = bytearray(data.data)
            pointers = list(data.pointers)
            struct.pack_into("<I", array_, 0x30, len(array_) if allocated else 0)
            if allocated:
                pointers += [0x30, len(array_)]
            array_ += struct.pack("<3I", len(array_) + 4, mock_address(0), mock_address(1))
            mocks = [
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ]
            t.check("data_dispose", Buffer(bytes(array_), pointers=tuple(pointers)), mocks=mocks)

    # a new array: the memory is the array, then room for its elements and its vector of bits
    for alignment_bits in (0, 1, 2, 4, 5, 7):
        for maximum_count, size in ((1, 2), (5, 12), (32, 4), (33, 6), (70, 3)):
            for name in (b"x", b"objects", b"a name of 31 characters, no more", b"a name that is longer than the array keeps"):
                room = DATA_ARRAY_SIZE + (1 << alignment_bits) + maximum_count * size + ((maximum_count + 31) >> 5) * 4
                t.check(
                    "data_initialize",
                    Buffer(random_.randbytes(room)),
                    Buffer(name + b"\0"),
                    maximum_count,
                    size,
                    alignment_bits,
                    0x12345678,
                )
                t.check(
                    "data_initialize_disconnected",
                    Buffer(random_.randbytes(DATA_ARRAY_SIZE)),
                    Buffer(name + b"\0"),
                    maximum_count,
                    size,
                    alignment_bits,
                    0x12345678,
                    0x0BADF00D,
                )
