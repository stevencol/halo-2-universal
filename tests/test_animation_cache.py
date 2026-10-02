# source/cache/xbox_animation_cache.c
#
# A cache is made in one buffer, as the game has it: the lruv_cache with its
# blocks, the data array of the cache's blocks, that of the animations that
# are about to be used, and the animations of the tags. The reads from the
# map are the test's: one ends at once, or when the game waits for it

import random
import struct
from dataclasses import dataclass, field

from difftest import Buffer, Mock, Pointer, mock_address
from test_async import calls_so_far
from test_data import array
from test_lruv_cache import (
    BLOCK_PAGE_COUNT,
    BLOCK_SIZE,
    BLOCKS,
    CACHE_SIZE,
    DATA_ARRAY_SIZE,
    DELETE_BLOCK_PROC,
    FAILED_PAGE_COUNTS,
    HOLE_ALGORITHM,
    LOCKED_BLOCK_PROC,
    MAXIMUM_TICK,
    PAGE_COUNT,
    PAGE_SIZE_BITS,
    cache,
)
from test_scenario_load import physical_memory

NONE = 0xFFFFFFFF
BASE_ADDRESS = 0x80400000
PAGE_BITS = 11
PAGE_SIZE = 1 << PAGE_BITS

# a data array
ALLOCATION = 0x30
VALID = 0x29

# the lruv_cache's allocation
CACHE_ALLOCATION = 0x6C

# a block of the cache
CACHE_BLOCK_SIZE = 0x10
READ_DONE = 0x4
READ_TASK_ID = 0x8
BLOCK_ANIMATION = 0xC

# an animation that is about to be used
PREDICTED_SIZE = 0xC
PREDICTED_PRIORITY = 0x4
PREDICTED_ANIMATION = 0x8

# the data of animations, in a tag
ANIMATION_SIZE = 0x10
ANIMATION_DATA_SIZE = 0x4
ANIMATION_BLOCK_INDEX = 0xC

DELETED = "animation_cache_block_deleted"
LOCKED = "animation_cache_block_locked"

# the memory of a new cache: the two data arrays and the lruv_cache
BLOCKS_MEMORY_SIZE = DATA_ARRAY_SIZE + 0x200 * CACHE_BLOCK_SIZE + 0x40
PREDICTED_MEMORY_SIZE = DATA_ARRAY_SIZE + 0x80 * PREDICTED_SIZE + 0x10
CACHE_MEMORY_SIZE = 0x30FC


@dataclass
class World:
    buffer: Buffer
    # the offsets of the two data arrays
    blocks: int
    predicted: int
    maximum_blocks: int
    # the indices of the blocks, in the order of their pages
    indices: list[int]
    # the offsets of the animations: by the index of their block, those that
    # are about to be used, and those that nobody knows of
    cached: dict[int, int] = field(default_factory=dict)
    predicted_animations: list[int] = field(default_factory=list)
    other_animations: list[int] = field(default_factory=list)
    page_count: int = 0

    def block(self, index: int) -> int:
        """The offset of a block of the cache."""
        return self.blocks + DATA_ARRAY_SIZE + (index & 0xFFFF) * CACHE_BLOCK_SIZE

    def pointer(self, offset: int) -> Pointer:
        return Pointer(self.buffer, offset)

    def animations(self) -> list[int]:
        return [*self.cached.values(), *self.predicted_animations, *self.other_animations]


def world(
    random_: random.Random,
    block_count: int | None = None,
    maximum_blocks: int | None = None,
    maximum_predicted: int | None = None,
    predicted_count: int | None = None,
    reading: float = 0.3,
    tick: int | None = None,
    large: bool = False,
    allocations: bool = True,
    predicted_valid: bool = True,
    predict_cached: float = 0.0,
) -> World:
    lruv, indices = cache(
        random_,
        block_count=block_count,
        tick=tick if tick is not None else random_.choice((400, 1000, 0x101, MAXIMUM_TICK)),
        maximum_count=maximum_blocks,
        random_ticks=False,
        agos=(0, 1, 5, 9, 10, 11, 19, 20, 21, 29, 30, 31, 49, 50, 51, 100, 200),
    )
    data = bytearray(lruv.data)
    pointers = list(lruv.pointers)
    functions = {DELETE_BLOCK_PROC: DELETED, LOCKED_BLOCK_PROC: LOCKED}
    (maximum_blocks,) = struct.unpack_from("<i", data, CACHE_SIZE + 0x20)
    (page_count,) = struct.unpack_from("<I", data, PAGE_COUNT)
    struct.pack_into("<I", data, PAGE_SIZE_BITS, PAGE_BITS)
    struct.pack_into("<I", data, HOLE_ALGORITHM, random_.choice((0, 0, 1, 2)))
    for n in range(8):
        struct.pack_into(
            "<I", data, FAILED_PAGE_COUNTS + n * 4, MAXIMUM_TICK if random_.random() < 0.9 else random_.randint(1, 8)
        )

    def append(buffer: Buffer) -> int:
        """Puts a buffer that has pointers to itself after the rest; where it is."""
        while len(data) % 4:
            data.append(0)
        base = len(data)
        piece = bytearray(buffer.data)
        for offset in buffer.pointers:
            struct.pack_into("<I", piece, offset, struct.unpack_from("<I", piece, offset)[0] + base)
            pointers.append(base + offset)
        data.extend(piece)
        return base

    def signed(identifier: int) -> int:
        return identifier - 0x10000 if identifier & 0x8000 else identifier

    # the blocks of the cache: one for each block of the lruv_cache, with its index
    in_use = {index & 0xFFFF: signed(index >> 16) for index in indices}
    blocks = append(array(random_, maximum_blocks, CACHE_BLOCK_SIZE, in_use))

    # the animations that are about to be used
    if maximum_predicted is None:
        maximum_predicted = random_.choice((4, 8, 16))
    if predicted_count is None:
        predicted_count = random_.randint(0, maximum_predicted)
    predicted_count = min(predicted_count, maximum_predicted)
    predicted_slots = sorted(random_.sample(range(maximum_predicted), predicted_count))
    predicted_in_use = {slot: random_.choice((-0x8000, -1, 1, 0x1234)) for slot in predicted_slots}
    predicted = append(array(random_, maximum_predicted, PREDICTED_SIZE, predicted_in_use))
    data[predicted + VALID] = predicted_valid

    # who frees the arrays and the cache
    allocator = append(Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,)))
    for at in (blocks + ALLOCATION, predicted + ALLOCATION):
        if allocations and random_.random() < 0.8:
            struct.pack_into("<I", data, at, allocator)
            pointers.append(at)
    struct.pack_into("<I", data, CACHE_ALLOCATION, allocator)
    pointers.append(CACHE_ALLOCATION)

    result = World(Buffer(b""), blocks, predicted, maximum_blocks, indices, page_count=page_count)

    def animation(size: int, block_index: int) -> int:
        at = len(data)
        data.extend(struct.pack("<4I", random_.getrandbits(32), size, random_.getrandbits(32), block_index))
        return at

    # the animations in the cache
    for index in indices:
        (pages,) = struct.unpack_from("<I", data, BLOCKS + (index & 0xFFFF) * BLOCK_SIZE + BLOCK_PAGE_COUNT)
        at = animation(pages * PAGE_SIZE - random_.choice((0, 1, 0x1FF, 0x200, 0x201, PAGE_SIZE - 1)), index)
        result.cached[index] = at
        block = result.block(index)
        if random_.random() < reading:
            # being read
            data[block + READ_DONE] = 0
            struct.pack_into("<I", data, block + READ_TASK_ID, 0x500 + (index & 0xFFFF))
        else:
            # read; a few, as a block is before its read starts
            data[block + READ_DONE] = random_.random() < 0.9
            struct.pack_into(
                "<I", data, block + READ_TASK_ID, NONE if random_.random() < 0.6 else 0x500 + (index & 0xFFFF)
            )
            if not data[block + READ_DONE]:
                struct.pack_into("<I", data, block + READ_TASK_ID, NONE)
        struct.pack_into("<I", data, block + BLOCK_ANIMATION, at)
        pointers.append(block + BLOCK_ANIMATION)

    def new_size() -> int:
        if large and random_.random() < 0.5:
            pages = random_.choice((page_count, page_count // 2 + 1, page_count + 1, page_count - 1))
            return max(pages, 1) * PAGE_SIZE
        return random_.randint(1, 6) * PAGE_SIZE - random_.choice((0, 1, 0x200, 0x7FF))

    # those that are not, which are about to be used
    for slot in predicted_slots:
        if indices and random_.random() < predict_cached:
            # one that got into the cache since
            at = result.cached[random_.choice(indices)]
        else:
            at = animation(new_size(), NONE)
            result.predicted_animations.append(at)
        datum = predicted + DATA_ARRAY_SIZE + slot * PREDICTED_SIZE
        struct.pack_into("<I", data, datum + PREDICTED_PRIORITY, random_.choice((1, 2)))
        struct.pack_into("<I", data, datum + PREDICTED_ANIMATION, at)
        pointers.append(datum + PREDICTED_ANIMATION)

    # and those that are not
    for _ in range(random_.randint(1, 3)):
        result.other_animations.append(animation(new_size(), NONE))

    result.buffer = Buffer(bytes(data), pointers=tuple(pointers), functions=functions)
    return result


def cache_globals(
    random_: random.Random,
    w: World,
    initialized: int = 1,
    opened: int = 1,
    tasks: int | None = None,
) -> dict:
    return {
        "animation_cache_globals": w.pointer(w.blocks),
        "animation_cache_globals+0x4": w.pointer(w.predicted),
        "animation_cache_globals+0x8": BASE_ADDRESS,
        "animation_cache_globals+0xc": w.pointer(0),
        "animation_cache_globals+0x10": 0x70000000 + w.page_count,
        "animation_cache_globals+0x14": w.page_count,
        "animation_cache_initialized": bytes([initialized]),
        "animation_cache_opened": bytes([opened]),
        "async_globals+0x259c": random_.choice((0, 3, 25)) if tasks is None else tasks,
        "animation_cache_warning_time": random_.choice((0, 5000, 0xFFFFFF00)),
        "animation_cache_idle_count": random_.getrandbits(32),
    }


def cache_externals(w: World, instant: bool, switches: int = 1, ticks: tuple[int, ...] = (6000, 6010)) -> dict:
    """The reads: one ends at once, or the n-th time that the game gives the
    task's thread its turn."""

    def read_async(harness, arguments):
        if instant:
            harness.mu.mem_write(arguments[4], b"\x01")
        # a size can be the address of a function of the game: it is not one
        return 0x1000 + calls_so_far(harness, "cache_file_read_async"), tuple(arguments)

    def switch_to_thread(harness, arguments):
        if calls_so_far(harness, "SwitchToThread") >= switches - 1:
            for slot in range(w.maximum_blocks):
                harness.mu.mem_write(w.buffer.address + w.block(slot) + READ_DONE, b"\x01")
        return 0

    def tick_count(harness, arguments):
        return ticks[min(calls_so_far(harness, "GetTickCount"), len(ticks) - 1)]

    return {
        "cache_file_read_async": read_async,
        "async_task_set_priority": lambda harness, arguments: 1,
        "SwitchToThread": switch_to_thread,
        "GetTickCount": tick_count,
        "XPhysicalProtect": lambda harness, arguments: 0,
    }


def run(t) -> None:
    random_ = random.Random(32)

    allocator_mocks = [
        Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
        Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
    ]

    def check(name, w, *arguments, instant=None, switches=None, ticks=(6000, 6010), **options):
        instant = random_.random() < 0.5 if instant is None else instant
        switches = random_.choice((1, 2)) if switches is None else switches
        t.check(
            name,
            *arguments,
            globals={**cache_globals(random_, w, **options.pop("state", {})), **options.pop("globals", {})},
            mocks=allocator_mocks,
            extra=[w.buffer],
            externals=cache_externals(w, instant, switches, ticks),
        )

    # ---------- a new cache

    for initialized in (0, 1):
        for _ in range(3):
            blocks_memory = Buffer(random_.randbytes(BLOCKS_MEMORY_SIZE))
            predicted_memory = Buffer(random_.randbytes(PREDICTED_MEMORY_SIZE))
            cache_memory = Buffer(random_.randbytes(CACHE_MEMORY_SIZE), places=(DELETE_BLOCK_PROC, LOCKED_BLOCK_PROC))
            allocator = Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,))

            def allocate(call, memory={BLOCKS_MEMORY_SIZE: blocks_memory, PREDICTED_MEMORY_SIZE: predicted_memory, CACHE_MEMORY_SIZE: cache_memory}):
                return memory[call.stack[0]].address

            t.check(
                "animation_cache_new",
                globals={
                    "heap_allocation": allocator,
                    "animation_cache_globals": random_.randbytes(0x18),
                    "animation_cache_initialized": bytes([initialized]),
                    "lruv_blocks_allocation": random_.choice((0, 0x60F00100)),
                },
                mocks=[
                    Mock(allocate, stack_arguments=1, registers=("ecx",)),
                    Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                ],
                extra=[blocks_memory, predicted_memory, cache_memory],
            )

    # ---------- its memory

    for _ in range(60):
        w = world(random_)
        stage = random_.randint(0, 4)
        low = random_.choice((0x61000, 0x1000000, 0x2F00000))
        high = random_.choice((0x3145000, low + 0x400000, low + 0x3FFFFF, low + 0x300000, low + 0x2FF000, low, 0x400000))
        memory = {"physical_memory_globals": physical_memory(stage, low, high & 0xFFFFFFFF)}
        if random_.random() < 0.15:
            memory["animation_cache_globals+0xc"] = 0
        check("animation_cache_allocate_memory", w, random_.choice((0, 1, 2, 4, NONE)), globals=memory)

    for _ in range(40):
        w = world(random_)
        check(
            "animation_cache_free_memory",
            w,
            globals={"animation_cache_globals+0xc": 0} if random_.random() < 0.15 else {},
        )

    # ---------- the end of the cache

    for _ in range(40):
        w = world(random_)
        nothing = {key: 0 for key in ("animation_cache_globals", "animation_cache_globals+0x4", "animation_cache_globals+0xc") if random_.random() < 0.2}
        check("animation_cache_delete", w, state={"initialized": random_.random() < 0.8}, globals=nothing)

    # ---------- a map comes and goes

    for _ in range(40):
        w = world(random_)
        nothing = {key: 0 for key in ("animation_cache_globals", "animation_cache_globals+0x4") if random_.random() < 0.2}
        state = {"initialized": random_.random() < 0.8, "opened": random_.random() < 0.3}
        data = bytearray(w.buffer.data)
        data[w.blocks + VALID] = random_.random() < 0.5
        data[w.predicted + VALID] = random_.random() < 0.5
        w.buffer = Buffer(bytes(data), pointers=w.buffer.pointers, functions=w.buffer.functions)
        check("animation_cache_open", w, state=state, globals=nothing)

    for _ in range(60):
        w = world(random_)
        nothing = {key: 0 for key in ("animation_cache_globals", "animation_cache_globals+0x4") if random_.random() < 0.2}
        state = {"initialized": random_.random() < 0.8, "opened": random_.random() < 0.8}
        if nothing and state["initialized"] and state["opened"]:
            # the cache has blocks, and their array is gone: the game does not look
            w = world(random_, block_count=0)
        check("animation_cache_close", w, state=state, globals=nothing)

    # ---------- the blocks

    for _ in range(100):
        w = world(random_, block_count=random_.randint(1, 9))
        index = random_.choice(w.indices)
        check("animation_cache_block_locked", w, index)
        check("animation_cache_block_deleted", w, index)
        check("animation_cache_block_wait", w, w.pointer(w.block(index)))
        check("animation_cache_reading_count", w)

    for _ in range(20):
        check("animation_cache_reading_count", world(random_, reading=random_.choice((0.0, 0.5, 1.0))))

    for force in (0, 1):
        for _ in range(250):
            w = world(random_, large=True, maximum_blocks=random_.choice((4, 8, 33)), reading=random_.choice((0.0, 0.3, 0.9)))
            offset = random_.choice(w.predicted_animations + w.other_animations)
            check("animation_cache_block_new", w, w.pointer(offset), force, random_.choice((0, 0, 5, 30, 50, 1000)))

    for _ in range(300):
        w = world(random_, large=random_.random() < 0.4, maximum_blocks=random_.choice((4, 8, 33)))
        offset = random_.choice(w.predicted_animations + w.other_animations)
        check("animation_cache_block_request", w, random_.randint(0, 2), w.pointer(offset))

    for _ in range(200):
        w = world(random_, block_count=random_.randint(1, 9))
        index = random_.choice(w.indices)
        check(
            "animation_cache_block_read",
            w,
            w.pointer(w.block(index)),
            random_.getrandbits(32),
            random_.randint(0, 2),
        )

    # ---------- the animations

    for _ in range(150):
        w = world(random_, predict_cached=random_.choice((0.0, 0.5)))
        for offset in random_.sample(w.animations(), min(len(w.animations()), 4)):
            check("animation_cache_find_predicted", w, w.pointer(offset))

    for _ in range(400):
        w = world(
            random_,
            large=random_.random() < 0.3,
            maximum_predicted=random_.choice((4, 8)),
            predicted_count=random_.choice((None, 4, 8)),
        )
        offset = random_.choice(w.animations())
        check(
            "animation_cache_predict",
            w,
            w.pointer(offset),
            random_.choice((0, 1, 0x100, 0x7701)),
            state={"opened": random_.random() < 0.9},
            ticks=random_.choice(((6000, 6010), (4000, 4010), (5000, 5001), (5001, 5002), (0xFFFFFF01, 0xFFFFFF02))),
        )

    # an animation that the cache has no room for at all cannot be asked for:
    # the game does not look whether it got a block
    for _ in range(500):
        w = world(
            random_,
            large=random_.random() < 0.3,
            maximum_blocks=random_.choice((4, 8, 33)),
            reading=random_.choice((0.0, 0.3, 0.9)),
        )
        fit = [
            offset
            for offset in w.animations()
            if struct.unpack_from("<I", w.buffer.data, offset + ANIMATION_DATA_SIZE)[0] <= w.page_count * PAGE_SIZE
        ]
        if fit:
            check("animation_cache_get_data", w, w.pointer(random_.choice(fit)), random_.getrandbits(32))

    # ---------- once a frame

    for _ in range(400):
        w = world(
            random_,
            large=random_.random() < 0.3,
            maximum_blocks=random_.choice((4, 8, 33)),
            reading=random_.choice((0.0, 0.3, 0.9)),
            predicted_valid=random_.random() < 0.9,
            predict_cached=random_.choice((0.0, 0.0, 0.3)),
        )
        check(
            "animation_cache_update_predicted",
            w,
            state={"opened": random_.random() < 0.9, "tasks": random_.choice((0, 3, 25, 26, 100, NONE))},
        )

    for _ in range(300):
        w = world(random_, maximum_blocks=random_.choice((4, 8, 33)), predicted_valid=random_.random() < 0.9)
        state = {"initialized": random_.random() < 0.9, "opened": random_.random() < 0.9}
        check("animation_cache_idle", w, state=state)
