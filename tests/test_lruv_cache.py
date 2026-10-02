# source/memory/lruv_cache.c
#
# A cache is made in a buffer as the game keeps one: the cache, the data array
# of its blocks right after it, the blocks and the vector of bits

import random
import struct

from difftest import Buffer, Mock, mock_address
from test_data import array

NONE = 0xFFFFFFFF
CACHE_SIZE = 0x70
DATA_ARRAY_SIZE = 0x4C
BLOCK_SIZE = 0x18
BLOCKS = CACHE_SIZE + DATA_ARRAY_SIZE

# a cache
DELETE_BLOCK_PROC = 0x20
LOCKED_BLOCK_PROC = 0x24
HOLE_ALGORITHM = 0x2C
PAGE_COUNT = 0x30
PAGE_SIZE_BITS = 0x34
TICK = 0x38
FIRST_BLOCK_INDEX = 0x3C
LAST_BLOCK_INDEX = 0x40
FAILED_PAGE_COUNTS = 0x44
BLOCKS_POINTER = 0x64

# a block
BLOCK_PAGE_COUNT = 0x4
FIRST_PAGE_INDEX = 0x8
NEXT_BLOCK_INDEX = 0xC
PREVIOUS_BLOCK_INDEX = 0x10
LAST_USED_TICK = 0x14

MAXIMUM_TICK = 0x7FFFFFFF


def cache(
    random_: random.Random,
    block_count: int | None = None,
    tick: int | None = None,
    delete_proc: bool = True,
    locked_proc: bool = True,
    maximum_count: int | None = None,
    agos: tuple[int, ...] = (0, 1, 5, 29, 30, 31, 0x100, 0x101, 500, 0x7FFFFFF0),
    gaps: tuple[int, int] = (0, 3),
    random_ticks: bool = True,
    tick_mask: int = MAXIMUM_TICK,
) -> tuple[Buffer, list[int]]:
    """A cache with some blocks, in a list in the order of their pages; and the indices of the blocks."""
    if maximum_count is None:
        maximum_count = random_.choice((4, 8, 33, 40))
    if block_count is None:
        block_count = random_.randint(0, min(maximum_count, 9))
    block_count = min(block_count, maximum_count)
    slots = sorted(random_.sample(range(maximum_count), block_count))
    in_use = {slot: random_.choice((-0x8000, -1 & 0x7FFF, 1, 0x1234)) for slot in slots}
    tick = tick if tick is not None else random_.choice((1, 400, 0x101, MAXIMUM_TICK, 0x80000005))

    prefix = bytearray(random_.randbytes(CACHE_SIZE))
    prefix[0:6] = b"cache\0"
    struct.pack_into("<I", prefix, DELETE_BLOCK_PROC, mock_address(0) if delete_proc else 0)
    struct.pack_into("<I", prefix, LOCKED_BLOCK_PROC, mock_address(1) if locked_proc else 0)
    struct.pack_into("<I", prefix, TICK, tick)
    struct.pack_into("<I", prefix, BLOCKS_POINTER, CACHE_SIZE)

    buffer = array(random_, maximum_count, BLOCK_SIZE, in_use, prefix=bytes(prefix))
    data = bytearray(buffer.data)

    # the list, in an order of its own
    order = slots[:]
    random_.shuffle(order)
    indices = {slot: ((in_use[slot] & 0xFFFF) << 16) | slot for slot in slots}
    page = 0
    for position, slot in enumerate(order):
        at = BLOCKS + slot * BLOCK_SIZE
        pages = random_.randint(1, 6)
        page += random_.randint(*gaps)
        ago = random_.choice(agos)
        struct.pack_into(
            "<5I",
            data,
            at + BLOCK_PAGE_COUNT,
            pages,
            page,
            indices[order[position + 1]] if position + 1 < len(order) else NONE,
            indices[order[position - 1]] if position else NONE,
            (tick - ago) & tick_mask if not random_ticks or random_.random() < 0.8 else random_.getrandbits(31),
        )
        page += pages
    struct.pack_into("<I", data, FIRST_BLOCK_INDEX, indices[order[0]] if order else NONE)
    struct.pack_into("<I", data, LAST_BLOCK_INDEX, indices[order[-1]] if order else NONE)
    struct.pack_into("<I", data, PAGE_COUNT, page + random_.randint(0, 4))

    return Buffer(bytes(data), pointers=(*buffer.pointers, BLOCKS_POINTER)), [indices[slot] for slot in order]


def with_fields(buffer: Buffer, fields: dict[int, int]) -> Buffer:
    """The cache with other values in some of its fields."""
    data = bytearray(buffer.data)
    for offset, value in fields.items():
        struct.pack_into("<I", data, offset, value & 0xFFFFFFFF)
    return Buffer(bytes(data), pointers=buffer.pointers)


def run(t) -> None:
    random_ = random.Random(30)

    def deleted(call):
        return 0

    def locked(call, answers={}):
        # some blocks cannot be deleted: by their index, the same in both runs
        return (call.stack[0] >> 3) & 1

    mocks = [Mock(deleted, stack_arguments=1), Mock(locked, stack_arguments=1)]

    # ---------- a new cache

    for name in (b"xbox animation cache", b"", b"x" * 31, b"y" * 40):
        for maximum_block_count in (1, 8, 0x40):
            size = CACHE_SIZE + DATA_ARRAY_SIZE + maximum_block_count * BLOCK_SIZE + ((maximum_block_count + 31) >> 5) * 4
            t.check(
                "lruv_initialize",
                Buffer(random_.randbytes(size)),
                Buffer(name + b"\0"),
                random_.choice((0, 0x800)),
                random_.choice((0xB, 0xC)),
                maximum_block_count,
                mock_address(0),
                random_.choice((mock_address(1), 0)),
                random_.choice((0, 0x12345678)),
                random_.choice((0, 0x60F00000)),
                globals={"lruv_blocks_allocation": random_.choice((0, 0x60F00100))},
            )

    # ---------- the ticks

    for _ in range(60):
        buffer, _indices = cache(random_, tick=MAXIMUM_TICK)
        t.check("lruv_wrap_ticks", buffer)

    for tick in (1, 400, 0x7FFFFFFF, 0x80000010):
        for last_used in (0, 1, 100, 249, 250, 251, 399, 400):
            for other in (0, 100, 249, 250, 251, 399, 400, 0x7FFFFFFF):
                for pages, other_pages in ((1, 2), (2, 1), (3, 3), (-1, 0)):
                    t.check(
                        "lruv_hole_is_better",
                        Buffer(random_.randbytes(TICK) + struct.pack("<I", tick) + random_.randbytes(CACHE_SIZE - TICK - 4)),
                        Buffer(struct.pack("<IIIi", 7, last_used, 9, pages)),
                        Buffer(struct.pack("<IIIi", 7, other, 9, other_pages)),
                    )

    # ---------- deleting blocks

    for _ in range(150):
        delete_proc = random_.random() < 0.8
        buffer, indices = cache(random_, block_count=random_.randint(1, 9), delete_proc=delete_proc)
        t.check("lruv_block_delete", buffer, random_.choice(indices), mocks=mocks)

    for _ in range(60):
        buffer, indices = cache(random_, delete_proc=random_.random() < 0.8)
        t.check("lruv_flush", buffer, mocks=mocks)

    for _ in range(150):
        buffer, indices = cache(random_, delete_proc=random_.random() < 0.8)
        (page_count,) = struct.unpack_from("<I", buffer.data, PAGE_COUNT)
        t.check("lruv_resize", buffer, random_.choice((0, page_count, page_count // 2, page_count - 1, 5, 0x1000)), mocks=mocks)

    # ---------- what is in use

    for _ in range(200):
        buffer, indices = cache(random_, locked_proc=random_.random() < 0.7)
        t.check("lruv_compute_pages_in_use", buffer, random_.choice((0, 1, 5, 30, 31, 0x100, 0x7FFFFFFF, NONE)), mocks=mocks)

    # ---------- a new block

    def new_block(buffer, age, algorithm=None, failed=None, bits=None, pages=None, mocks=mocks):
        (page_count,) = struct.unpack_from("<I", buffer.data, PAGE_COUNT)
        bits = random_.choice((0, 0xB, 0xC)) if bits is None else bits
        fields = {
            PAGE_SIZE_BITS: bits,
            HOLE_ALGORITHM: random_.choice((0, 1, 2, 7)) if algorithm is None else algorithm,
        }
        for index in range(8):
            fields[FAILED_PAGE_COUNTS + index * 4] = (
                MAXIMUM_TICK if failed is None else random_.choice((MAXIMUM_TICK, *failed))
            )
        if pages is None:
            pages = random_.choice((1, 2, 3, 5, 8, 13, max(page_count // 2, 1), page_count, page_count + 1))
        size = pages << bits
        if bits and random_.random() < 0.5:
            # not a whole number of pages
            size -= random_.randint(1, (1 << bits) - 1)
        t.check("lruv_block_new", with_fields(buffer, fields), size, age, mocks=mocks)

    ages = (-1, 0, 1, 2, 5, 7, 8, 9, 30, 31, 300, 0x7FFFFFFF)

    # caches of all kinds
    for _ in range(600):
        buffer, _indices = cache(random_, locked_proc=random_.random() < 0.6, delete_proc=random_.random() < 0.8)
        new_block(buffer, random_.choice(ages))

    # old blocks only, so that it is the algorithm that chooses
    for algorithm in (0, 1, 2):
        for _ in range(300):
            buffer, _indices = cache(
                random_,
                block_count=random_.randint(2, 30),
                tick=random_.choice((1000, 5000, 0x7FFFFFFF)),
                locked_proc=random_.random() < 0.3,
                maximum_count=random_.choice((33, 40)),
                agos=(40, 41, 100, 150, 189, 190, 191, 250, 339, 340, 341, 400, 600, 900),
            )
            new_block(buffer, random_.choice((0, 1, 5, 30)), algorithm=algorithm, pages=random_.randint(1, 12))

    # no blocks left in the array: the one used the longest ago goes
    for _ in range(300):
        maximum_count = random_.choice((4, 8))
        buffer, _indices = cache(
            random_,
            block_count=maximum_count,
            locked_proc=random_.random() < 0.3,
            maximum_count=maximum_count,
            tick=random_.choice((1000, 0x7FFFFFFF)),
            agos=(0, 1, 5, 31, 40, 100, 150, 400, 900),
        )
        new_block(buffer, random_.choice((0, 1, 5, 30)), pages=random_.randint(1, 4))

    # nothing in it, and no pages
    for block_count in (0, 1):
        for page_count in (0, 1, 7):
            for age in (0, 3, 8, 9):
                buffer, _indices = cache(random_, block_count=block_count)
                if not block_count:
                    buffer = with_fields(buffer, {PAGE_COUNT: page_count})
                new_block(buffer, age, pages=random_.choice((1, 7, 8)))

    # sizes that there was no room for already
    for _ in range(300):
        buffer, _indices = cache(random_, locked_proc=random_.random() < 0.6)
        new_block(buffer, random_.choice(ages), failed=(0, 1, 2, 3, 5, 8, 13))

    # more places to start at than are kept at a time
    for _ in range(40):
        buffer, _indices = cache(
            random_,
            block_count=random_.choice((140, 200, 300)),
            tick=5000,
            locked_proc=random_.random() < 0.3,
            maximum_count=300,
            agos=(40, 100, 400, 900),
            gaps=(0, 2),
        )
        (page_count,) = struct.unpack_from("<I", buffer.data, PAGE_COUNT)
        new_block(
            buffer,
            random_.choice((0, 5)),
            pages=random_.choice((3, 600, page_count - 5, page_count, page_count + 1)),
        )

    # the same, with no block that stays: the first place to start at is
    # still there when there are that many
    for _ in range(80):
        buffer, _indices = cache(
            random_,
            block_count=random_.choice((200, 300)),
            tick=5000,
            locked_proc=False,
            maximum_count=300,
            agos=(40, 100, 400, 900),
            gaps=(0, 2),
            random_ticks=False,
        )
        (page_count,) = struct.unpack_from("<I", buffer.data, PAGE_COUNT)
        new_block(
            buffer,
            random_.choice((0, 5)),
            algorithm=random_.choice((0, 0, 1, 2)),
            pages=random_.choice((1, 2, 3, 7, 600, 900, page_count - 5, page_count)),
        )

    # ticks of all 32 bits: the game compares them without a sign
    for algorithm in (0, 1, 2):
        for _ in range(150):
            maximum_count = random_.choice((4, 8, 33))
            buffer, _indices = cache(
                random_,
                block_count=random_.randint(2, 20),
                tick=random_.choice((0xFFFFFF00, 0xC0000000)),
                locked_proc=random_.random() < 0.3,
                maximum_count=maximum_count,
                agos=(0x10, 0x100, 0x3FFFFF00, 0x40000000, 0x40000100, 0x7FFFFE00, 0x7FFFFF80, 0xB0000000),
                random_ticks=False,
                tick_mask=0xFFFFFFFF,
            )
            new_block(buffer, random_.choice((0, 1, 5)), algorithm=algorithm, pages=random_.randint(1, 12))

    # no blocks left in the array, and some of them cannot be deleted
    low_mocks = [Mock(deleted, stack_arguments=1), Mock(lambda call: call.stack[0] & 1, stack_arguments=1)]
    for _ in range(200):
        maximum_count = random_.choice((4, 8))
        buffer, _indices = cache(
            random_,
            block_count=maximum_count,
            maximum_count=maximum_count,
            tick=1000,
            agos=(40, 100, 150, 400, 900),
            random_ticks=False,
        )
        new_block(buffer, random_.choice((0, 1, 5, 30)), pages=random_.randint(1, 4), mocks=low_mocks)
