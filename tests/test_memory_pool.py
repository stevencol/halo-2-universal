# source/memory/memory_pool.c
#
# A pool is made in a buffer as the game has one: the pool, its memory right
# after it with some blocks in it, and after that the pointers that the
# owners of the blocks have to them

import random
import struct
from dataclasses import dataclass, field

from difftest import Buffer, Mock, Pointer, mock_address

POOL_SIZE = 0x40
BLOCK_SIZE = 0x10
DEBUG_SIZE = 0x10

# a pool
ALLOCATION = 0x24
BASE_ADDRESS = 0x28
SIZE = 0x2C
FREE_SIZE = 0x30
FIRST_BLOCK = 0x34
LAST_BLOCK = 0x38
NO_REFERENCES = 0x3C
FILL_GAPS = 0x3D
DEBUG = 0x3F

# a block
REFERENCE = 0x4
NEXT_BLOCK = 0x8
PREVIOUS_BLOCK = 0xC


@dataclass
class Pool:
    buffer: Buffer
    size: int
    debug: bool
    # the offsets of the blocks, in the order of the list, and of the
    # pointers of their owners
    blocks: list[int] = field(default_factory=list)
    references: list[int] = field(default_factory=list)
    # the pointer of an owner that has no block
    free_reference: int = 0
    # where each block starts and ends, with what is before it
    spans: list[tuple[int, int]] = field(default_factory=list)

    def pointer(self, offset: int) -> Pointer:
        return Pointer(self.buffer, offset)


def pool(
    random_: random.Random,
    block_count: int | None = None,
    debug: bool | None = None,
    fill_gaps: bool | None = None,
    no_references: bool | None = None,
    size: int | None = None,
    gaps: tuple[int, ...] = (0, 0, 0x10, 0x20, 0x40, 0x80),
) -> Pool:
    debug = random_.random() < 0.4 if debug is None else debug
    fill_gaps = random_.random() < 0.5 if fill_gaps is None else fill_gaps
    no_references = random_.random() < 0.15 if no_references is None else no_references
    block_count = random_.randint(0, 6) if block_count is None else block_count
    before = BLOCK_SIZE + (DEBUG_SIZE if debug else 0)

    # where the blocks are, from the start of the pool's memory
    spans = []
    at = 0
    for _ in range(block_count):
        at += random_.choice(gaps)
        length = before + random_.choice((0x10, 0x10, 0x20, 0x30, 0x60))
        spans.append((at, at + length))
        at += length
    used = at
    if size is None:
        size = used + random_.choice((0, 0, 0x10, 0x20, 0x40, 0x100))
    size = max(size, used)

    references = POOL_SIZE + size
    data = bytearray(random_.randbytes(references + 4 * (block_count + 1) + 12))
    pointers = [BASE_ADDRESS]
    data[4:9] = b"pool\0"
    struct.pack_into("<4I", data, ALLOCATION, references + 4 * (block_count + 1), POOL_SIZE, size, size - sum(end - start for start, end in spans))
    pointers.append(ALLOCATION)
    # who gives the pool's memory back
    struct.pack_into("<3I", data, references + 4 * (block_count + 1), references + 4 * (block_count + 1) + 4, mock_address(0), mock_address(1))
    pointers.append(references + 4 * (block_count + 1))
    data[NO_REFERENCES] = no_references
    data[FILL_GAPS] = fill_gaps
    data[DEBUG] = debug

    result = Pool(Buffer(b""), size, debug, free_reference=references + 4 * block_count)
    blocks = [POOL_SIZE + start + (DEBUG_SIZE if debug else 0) for start, _ in spans]
    for n, (block, (start, end)) in enumerate(zip(blocks, spans)):
        reference = references + 4 * n
        if debug:
            struct.pack_into("<4s", data, block - DEBUG_SIZE, b"daeh")
        struct.pack_into(
            "<4I",
            data,
            block,
            end - start,
            0 if no_references else reference,
            blocks[n + 1] if n + 1 < len(blocks) else 0,
            blocks[n - 1] if n else 0,
        )
        if not no_references:
            pointers.append(block + REFERENCE)
        if n + 1 < len(blocks):
            pointers.append(block + NEXT_BLOCK)
        if n:
            pointers.append(block + PREVIOUS_BLOCK)
        struct.pack_into("<I", data, reference, block + BLOCK_SIZE)
        pointers.append(reference)
        result.blocks.append(block)
        result.references.append(reference)
    struct.pack_into("<2I", data, FIRST_BLOCK, blocks[0] if blocks else 0, blocks[-1] if blocks else 0)
    if blocks:
        pointers += [FIRST_BLOCK, LAST_BLOCK]
    result.spans = [(POOL_SIZE + start, POOL_SIZE + end) for start, end in spans]
    result.buffer = Buffer(bytes(data), pointers=tuple(pointers))
    return result


def run(t) -> None:
    random_ = random.Random(37)
    tick = {"GetTickCount": lambda harness, arguments: 0x12345}
    sizes = (0, 1, 8, 0x10, 0x11, 0x20, 0x30, 0x40, 0x70, 0x100, 0x1000)

    # ---------- a pool

    for name in (b"some pool", b"", b"x" * 31, b"y" * 40):
        for size in (0, 0x100, 0x12345):
            # the pool can be anywhere: its memory starts at a multiple of 16
            memory = Buffer(random_.randbytes(0x70))
            t.check(
                "memory_pool_initialize",
                Pointer(memory, random_.choice((0, 1, 4, 8, 0xC, 0xF))),
                size,
                Buffer(name + b"\0"),
                random_.choice((0, 0x60F00000)),
                extra=[memory],
            )

    for give in (True, False):
        for with_allocation in (True, False):
            memory = Buffer(random_.randbytes(0x200))
            allocator = Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,))
            other = Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,))
            t.check(
                "memory_pool_new",
                other if with_allocation else 0,
                random_.choice((0x100, 0x180)),
                Buffer(b"a pool\0"),
                globals={"heap_allocation": allocator},
                mocks=[
                    Mock(lambda call, memory=memory, give=give: memory.address if give else 0, stack_arguments=1, registers=("ecx",)),
                    Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                ],
                extra=[memory],
            )

    mocks = [
        Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
        Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
    ]
    for _ in range(10):
        t.check("memory_pool_delete", pool(random_).buffer, mocks=mocks)

    # ---------- the blocks

    for _ in range(600):
        p = pool(random_)
        t.check(
            "memory_pool_block_allocate",
            p.buffer,
            p.pointer(p.free_reference),
            random_.choice(sizes),
            random_.getrandbits(32),
            random_.getrandbits(32),
            externals=tick,
        )

    for _ in range(600):
        p = pool(random_, block_count=random_.randint(1, 6))
        n = random_.randrange(len(p.blocks))
        t.check(
            "memory_pool_block_reallocate",
            p.buffer,
            p.pointer(p.references[n]),
            random_.choice(sizes),
            random_.getrandbits(32),
            random_.getrandbits(32),
            externals=tick,
        )

    for _ in range(200):
        p = pool(random_, block_count=random_.randint(1, 6))
        t.check("memory_pool_block_free", p.buffer, p.pointer(random_.choice(p.references)))

    for _ in range(300):
        p = pool(random_, gaps=random_.choice(((0, 0, 0x10, 0x20, 0x40, 0x80), (0,), (0x10, 0x40))))
        t.check("memory_pool_compact", p.buffer)

    # ---------- where a new block goes

    actual_sizes = (0x10, 0x20, 0x30, 0x40, 0x50, 0x80, 0x90, 0x100, 0x1000)
    for _ in range(300):
        p = pool(random_)
        arguments = (random_.choice(actual_sizes), random_.getrandbits(32), random_.getrandbits(32))
        t.check("memory_pool_block_new_at_end", p.buffer, *arguments, externals=tick)
        t.check("memory_pool_block_new_at_start", p.buffer, *arguments, externals=tick)
        t.check("memory_pool_block_new_in_gap", p.buffer, *arguments, externals=tick)

    # a block where there is room for it: before the first, between two, after the last
    for _ in range(300):
        p = pool(random_, gaps=(0x40, 0x60, 0x80), size=0x400)
        n = random_.randint(0, len(p.blocks))
        address = p.spans[n - 1][1] if n else POOL_SIZE
        t.check(
            "memory_pool_block_link",
            p.buffer,
            p.pointer(address),
            random_.choice((0x20, 0x30, 0x40)),
            p.pointer(p.blocks[n - 1]) if n else 0,
            random_.getrandbits(32),
            p.pointer(p.blocks[n]) if n < len(p.blocks) else 0,
            random_.getrandbits(32),
            externals=tick,
        )
