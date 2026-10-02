# source/cseries/sort.c
#
# The function that tells whether a number goes after another is the test's:
# the game and source/ have to ask it the same things in the same order

import random
import struct

from difftest import Buffer, FunctionPointer, Mock, Pointer, mock_address


def after(call) -> int:
    element, other, context = call.stack
    return int(element > other)


def before(call) -> int:
    element, other, context = call.stack
    return int(element < other)


def after_signed(call) -> int:
    element, other, context = call.stack
    return int((element ^ 0x80000000) > (other ^ 0x80000000))


def after_or_same(call) -> int:
    element, other, context = call.stack
    return int(element >= other)


def after_by_low_bits(call) -> int:
    # many numbers are the same for it
    element, other, context = call.stack
    return int(element & 7 > other & 7)


def never(call) -> int:
    return 0


def always(call) -> int:
    return 1


def odd(call) -> int:
    # not an order at all
    element, other, context = call.stack
    return (element ^ other >> 1) & 1


COMPARES = (after, before, after_signed, after_or_same, after_by_low_bits, never, always, odd)
# a value that is not 0 or 1 in the byte that is looked at, and one that is
# zero in it and not in the rest
RETURNS = {0: 0xFFFFFF00, 1: 0x180}


def values(random_: random.Random, count: int, bits: int, kinds: tuple = (0, 1, 2, 3, 4, 5)) -> list[int]:
    kind = random_.choice(kinds)
    if kind == 0:
        numbers = sorted(random_.getrandbits(bits) for _ in range(count))
    elif kind == 1:
        numbers = sorted((random_.getrandbits(bits) for _ in range(count)), reverse=True)
    elif kind == 2:
        numbers = [random_.getrandbits(bits)] * count
    elif kind == 3:
        few = [random_.getrandbits(bits) for _ in range(3)]
        numbers = [random_.choice(few) for _ in range(count)]
    else:
        numbers = [random_.getrandbits(bits) for _ in range(count)]
    return numbers


def run(t) -> None:
    random_ = random.Random(69)

    # with many numbers that are the same for the function, the sort asks
    # about as many times as the square of how many there are: the test
    # allows 10,000 calls, so many numbers are sorted only when they differ
    counts = (0, 1, 2, 3, 4, 7, 8, 9, 10, 11, 16, 17, 18, 31, 64, 100, 257, 500)
    for name, format_, bits, size in (("qsort_4byte", "I", 32, 4), ("qsort_2byte", "H", 16, 2)):
        for count in counts:
            for n in range(3 if count > 64 else 6):
                compare = COMPARES[n] if count > 64 else random_.choice(COMPARES)
                # the numbers, and two more after them that are not sorted
                numbers = values(random_, count + 2, bits, (0, 1, 4) if count > 64 else (0, 1, 2, 3, 4, 5))
                wide = random_.getrandbits(1)

                def handler(call, compare=compare, wide=wide):
                    result = compare(call)
                    return RETURNS[result] if wide else result

                t.check(
                    name,
                    Buffer(struct.pack(f"<{count + 2}{format_}", *numbers)),
                    count,
                    random_.getrandbits(32),
                    mock_address(0),
                    random_.getrandbits(32),
                    mocks=[Mock(handler, stack_arguments=3)],
                )

    # with a function of the game: that of the indices of the files
    for count in (2, 5, 9, 40):
        for _ in range(4):
            numbers = values(random_, count, 32)
            t.check(
                "qsort_4byte",
                Buffer(struct.pack(f"<{count}I", *numbers)),
                count,
                0,
                FunctionPointer("saved_game_file_index_after"),
                0,
            )

    # ---------- a few numbers

    for name, format_, bits, size in (("short_sort_4byte", "I", 32, 4), ("short_sort_2byte", "H", 16, 2)):
        for count in (1, 2, 3, 5, 8, 12):
            for _ in range(8):
                compare = random_.choice(COMPARES)
                # one number before those that are sorted, and one after
                numbers = values(random_, count + 2, bits)
                buffer = Buffer(struct.pack(f"<{count + 2}{format_}", *numbers))
                wide = random_.getrandbits(1)

                def handler(call, compare=compare, wide=wide):
                    result = compare(call)
                    return RETURNS[result] if wide else result

                t.check(
                    name,
                    Pointer(buffer, size),
                    Pointer(buffer, size * count),
                    mock_address(0),
                    random_.getrandbits(32),
                    mocks=[Mock(handler, stack_arguments=3)],
                    extra=[buffer],
                )
        # the last before the first: nothing to sort
        for first, last in ((2, 1), (3, 0), (1, 1)):
            numbers = values(random_, 5, bits)
            buffer = Buffer(struct.pack(f"<5{format_}", *numbers))
            t.check(
                name,
                Pointer(buffer, size * first),
                Pointer(buffer, size * last),
                mock_address(0),
                0,
                mocks=[Mock(after, stack_arguments=3)],
                extra=[buffer],
            )
