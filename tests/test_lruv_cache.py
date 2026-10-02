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
PAGE_COUNT = 0x30
TICK = 0x38
FIRST_BLOCK_INDEX = 0x3C
LAST_BLOCK_INDEX = 0x40
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
) -> tuple[Buffer, list[int]]:
    """A cache with some blocks, in a list in the order of their pages; and the indices of the blocks."""
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
        page += random_.randint(0, 3)
        ago = random_.choice((0, 1, 5, 29, 30, 31, 0x100, 0x101, 500, 0x7FFFFFF0))
        struct.pack_into(
            "<5I",
            data,
            at + BLOCK_PAGE_COUNT,
            pages,
            page,
            indices[order[position + 1]] if position + 1 < len(order) else NONE,
            indices[order[position - 1]] if position else NONE,
            (tick - ago) & MAXIMUM_TICK if random_.random() < 0.8 else random_.getrandbits(31),
        )
        page += pages
    struct.pack_into("<I", data, FIRST_BLOCK_INDEX, indices[order[0]] if order else NONE)
    struct.pack_into("<I", data, LAST_BLOCK_INDEX, indices[order[-1]] if order else NONE)
    struct.pack_into("<I", data, PAGE_COUNT, page + random_.randint(0, 4))

    return Buffer(bytes(data), pointers=(*buffer.pointers, BLOCKS_POINTER)), [indices[slot] for slot in order]


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
