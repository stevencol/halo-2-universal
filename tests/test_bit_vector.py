# source/memory/bit_vector.c

import random
import struct

from difftest import Buffer, Pointer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(86)

    values = [0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 0x7FFFFFFF, 0x80000000, 0x80000001, 0xFFFFFFFF]
    values += [1 << bit for bit in range(32)] + [(1 << bit) + 1 for bit in range(1, 32)] + [(1 << bit) - 1 for bit in range(2, 32)]
    values += [random_.getrandbits(random_.randrange(1, 33)) for _ in range(60)]
    for value in values:
        t.check("highest_bit_set", value)
        t.check("bits_to_count", value)

    def words(count: int) -> list:
        kind = random_.randrange(5)
        if kind == 0:
            return [0] * count
        if kind == 1:
            return [0xFFFFFFFF] * count
        if kind == 2:
            # one bit only
            numbers = [0] * count
            if count:
                numbers[random_.randrange(count)] = 1 << random_.randrange(32)
            return numbers
        if kind == 3:
            # nothing in the last numbers
            numbers = [random_.getrandbits(32) for _ in range(count)]
            for index in range(random_.randrange(count + 1), count):
                numbers[index] = 0
            return numbers
        return [random_.getrandbits(32) for _ in range(count)]

    def vector(bit_count: int) -> tuple:
        """A vector of so many bits, in a buffer with a number before it and
        two after it; and where it is in the buffer."""
        count = (bit_count + 31) // 32
        numbers = [random_.getrandbits(32), *words(count), random_.getrandbits(32), random_.getrandbits(32)]
        if random_.random() < 0.3 and count:
            # bits past the end of the vector, and none in it
            numbers[count] &= ~((1 << (bit_count % 32 or 32)) - 1) & 0xFFFFFFFF
        return Buffer(struct.pack(f"<{len(numbers)}I", *numbers)), 4

    bit_counts = (0, 1, 2, 5, 31, 32, 33, 40, 63, 64, 65, 96, 100, 128, 200)
    for bit_count in bit_counts:
        for _ in range(14):
            data, place = vector(bit_count)
            t.check("bit_vector_count_bits", Pointer(data, place), bit_count, extra=[data])
            data, place = vector(bit_count)
            t.check("bit_vector_highest_bit_set", Pointer(data, place), bit_count, extra=[data])
            a, place_a = vector(bit_count)
            b, place_b = vector(bit_count)
            result, place_result = vector(bit_count)
            t.check("bit_vector_or", bit_count, Pointer(a, place_a), Pointer(b, place_b), Pointer(result, place_result), extra=[a, b, result])
    # a count of bits that is less than zero: nothing is done
    for bit_count in (-1, -31, -32, -100):
        a, place_a = vector(64)
        b, place_b = vector(64)
        result, place_result = vector(64)
        t.check("bit_vector_or", bit_count & 0xFFFFFFFF, Pointer(a, place_a), Pointer(b, place_b), Pointer(result, place_result), extra=[a, b, result])
