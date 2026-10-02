# source/cache/xbox_texture_cache.c
#
# A cache is made as the game has it. In one buffer: the lruv_cache with its
# blocks, the data array of the cache's blocks, that of the bitmaps that are
# about to be drawn, and the bitmaps of the tags. In the console's memory: the
# memory of the cache, where what it lends has a structure before it

import random
import struct
from dataclasses import dataclass, field

from difftest import Buffer, Memory, Mock, Pointer, f32, mock_address
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
    FIRST_PAGE_INDEX,
    HOLE_ALGORITHM,
    LOCKED_BLOCK_PROC,
    MAXIMUM_TICK,
    PAGE_COUNT,
    PAGE_SIZE_BITS,
    cache,
)
from test_scenario_load import physical_memory

NONE = 0xFFFFFFFF
BASE_ADDRESS = 0x80200000
REGISTERS = 0x80000000
PAGE_BITS = 12
PAGE_SIZE = 1 << PAGE_BITS

# a data array
ALLOCATION = 0x30
VALID = 0x29

# the lruv_cache
TYPE_BLOCK_PROC = 0x28
CACHE_ALLOCATION = 0x6C

# a block of the cache
CACHE_BLOCK_SIZE = 0x28
FLAGS = 0x2
READ_DONE = 0x3
LEVEL_INDEX = 0x4
READ_TASK_ID = 0xC
BLOCK_BITMAP = 0x10
STOLEN, REQUIRED = 1, 2

# a bitmap that is about to be drawn
PREDICTED_SIZE = 0x8
PREDICTED_BITMAP = 0x4

# a bitmap
BITMAP_SIZE = 0x74
BITMAP_WIDTH = 0x4
BITMAP_FLAGS = 0xE
BITMAP_MIPMAP_COUNT = 0x14
BITMAP_OFFSETS = 0x1C
BITMAP_BLOCK_INDICES = 0x28
BITMAP_SIZES = 0x34
BITMAP_MAXIMUM_MIP_BIAS = 0x58
BITMAP_TEXTURE = 0x5C
LINEAR = 0x10
# the formats that Direct3D has one for
FORMATS = (0, 1, 2, 3, 6, 8, 9, 10, 11, 14, 15, 16, 17, 18, 22, 23)
BITMAP_HARDWARE_FORMAT = 0x50
BITMAP_FRAME_INDEX = 0x70
CACHED, PREDICTED = 0x200, 0x400

# what is before the memory that the cache lends
STOLEN_SIZE = 0x24
SIGNATURE = 0x02281972
STOLEN_PREVIOUS = 0x1C
STOLEN_NEXT = 0x20

# the globals
STOLEN_MEMORY = 0x8

DELETED = "texture_cache_block_deleted"
LOCKED = "texture_cache_block_locked"
TYPE = "texture_cache_block_type"

BLOCKS_MEMORY_SIZE = 0x508C
PREDICTED_MEMORY_SIZE = 0x6A8
CACHE_MEMORY_SIZE = 0x30FC

# the mocks
ALLOCATE, DEALLOCATE, IDLE_PROC, RECLAIM_PROC = range(4)


@dataclass
class World:
    buffer: Buffer
    memory: Memory
    blocks: int
    predicted: int
    maximum_blocks: int
    indices: list[int]
    page_count: int
    # the addresses of the memory that is lent, in the order of the list
    stolen: list[int] = field(default_factory=list)
    stolen_indices: list[int] = field(default_factory=list)
    # the offsets of the bitmaps
    bitmaps: list[int] = field(default_factory=list)

    def block(self, index: int) -> int:
        return self.blocks + DATA_ARRAY_SIZE + (index & 0xFFFF) * CACHE_BLOCK_SIZE

    def pointer(self, offset: int) -> Pointer:
        return Pointer(self.buffer, offset)


def world(
    random_: random.Random,
    block_count: int | None = None,
    maximum_blocks: int | None = None,
    maximum_predicted: int | None = None,
    predicted_count: int | None = None,
    reading: float = 0.2,
    required: bool = False,
    stolen: float = 0.25,
    tick: int | None = None,
    valid: bool = True,
    predict_cached: float = 0.0,
) -> World:
    lruv, indices = cache(
        random_,
        block_count=block_count,
        tick=tick if tick is not None else random_.choice((400, 1000, 0x101, MAXIMUM_TICK)),
        maximum_count=maximum_blocks,
        random_ticks=False,
        agos=(0, 1, 2, 4, 5, 6, 30, 100, 200),
    )
    data = bytearray(lruv.data)
    pointers = list(lruv.pointers)
    functions = {DELETE_BLOCK_PROC: DELETED, LOCKED_BLOCK_PROC: LOCKED, TYPE_BLOCK_PROC: TYPE}
    (maximum_blocks,) = struct.unpack_from("<i", data, CACHE_SIZE + 0x20)
    (page_count,) = struct.unpack_from("<I", data, PAGE_COUNT)
    struct.pack_into("<I", data, PAGE_SIZE_BITS, PAGE_BITS)
    struct.pack_into("<I", data, HOLE_ALGORITHM, random_.choice((0, 1, 2, 2)))
    for n in range(8):
        struct.pack_into("<I", data, FAILED_PAGE_COUNTS + n * 4, MAXIMUM_TICK)

    def append(buffer: Buffer) -> int:
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

    in_use = {index & 0xFFFF: signed(index >> 16) for index in indices}
    blocks = append(array(random_, maximum_blocks, CACHE_BLOCK_SIZE, in_use))
    data[blocks + VALID] = valid

    if maximum_predicted is None:
        maximum_predicted = random_.choice((4, 8, 16))
    if predicted_count is None:
        predicted_count = random_.randint(0, maximum_predicted)
    predicted_count = min(predicted_count, maximum_predicted)
    predicted_slots = sorted(random_.sample(range(maximum_predicted), predicted_count))
    predicted_in_use = {slot: random_.choice((-0x8000, -1, 1, 0x1234)) for slot in predicted_slots}
    predicted = append(array(random_, maximum_predicted, PREDICTED_SIZE, predicted_in_use))
    data[predicted + VALID] = valid

    allocator = append(Buffer(struct.pack("<3I", 4, mock_address(ALLOCATE), mock_address(DEALLOCATE)), pointers=(0,)))
    if random_.random() < 0.8:
        struct.pack_into("<I", data, blocks + ALLOCATION, allocator)
        pointers.append(blocks + ALLOCATION)
    struct.pack_into("<I", data, CACHE_ALLOCATION, allocator)
    pointers.append(CACHE_ALLOCATION)

    memory = bytearray(random_.randbytes((page_count + 2) * PAGE_SIZE))
    result = World(Buffer(b""), Memory(b""), blocks, predicted, maximum_blocks, indices, page_count)

    def bitmap(flags: int) -> int:
        at = len(data)
        piece = bytearray(random_.randbytes(BITMAP_SIZE))
        if random_.random() < 0.8:
            # the depth is its low byte, with a sign
            sizes = [1 << random_.randint(0, 9) for _ in range(2)] + [random_.choice((1, 1, 1, 4, 0x104, 0x80))]
        else:
            sizes = [random_.randint(1, 640), random_.randint(1, 480), random_.choice((1, 3))]
        struct.pack_into("<5h", piece, BITMAP_WIDTH, *sizes, random_.choice((0, 0, 1, 2)), random_.choice(FORMATS))
        struct.pack_into(
            "<H",
            piece,
            BITMAP_FLAGS,
            (random_.getrandbits(16) & ~(CACHED | PREDICTED | LINEAR)) | flags | (LINEAR if random_.random() < 0.2 else 0),
        )
        struct.pack_into("<hBB", piece, BITMAP_MIPMAP_COUNT, random_.randint(0, 9), random_.choice((0, 0, 10, 100, 255)), random_.randint(0, 20))
        # the levels of detail that the map has pixels for
        for level in range(3):
            there = random_.random() < (0.9 if level == 0 else 0.6)
            struct.pack_into("<I", piece, BITMAP_OFFSETS + level * 4, random_.getrandbits(32) & ~1 if there else NONE)
            struct.pack_into(
                "<I",
                piece,
                BITMAP_SIZES + level * 4,
                random_.choice((0, 0x200, 0x400, 0x401, PAGE_SIZE, 2 * PAGE_SIZE - 5, 5 * PAGE_SIZE)),
            )
        struct.pack_into("<f", piece, BITMAP_MAXIMUM_MIP_BIAS, random_.choice((0.0, 0.5, 1.0, 2.5, 100.0)))
        # the pixels read with the map: a texture made already, the pixels of one to make, or none
        kind = random_.choice(("made", "to make", "none", "none"))
        struct.pack_into(
            "<5I",
            piece,
            BITMAP_TEXTURE,
            0x40001 if kind == "made" else 0,
            random_.getrandbits(20),
            random_.randint(1, 0x10000) if kind == "to make" else 0,
            random_.choice(FORMATS) | (1 << random_.randint(0, 9)) << 16,
            (1 << random_.randint(0, 9)) | random_.choice((1, 4)) << 16,
        )
        struct.pack_into("<3I", piece, BITMAP_BLOCK_INDICES, NONE, NONE, NONE)
        struct.pack_into("<I", piece, BITMAP_HARDWARE_FORMAT, 0)
        struct.pack_into("<i", piece, BITMAP_FRAME_INDEX, random_.choice((0, 3, 4, 5, 6, 100)))
        data.extend(piece)
        result.bitmaps.append(at)
        return at

    # each block is memory that the cache lent, or a level of a bitmap
    current = None
    levels: list[int] = []
    for index in indices:
        slot = index & 0xFFFF
        block = result.block(index)
        pages, first_page = struct.unpack_from("<2I", data, BLOCKS + slot * BLOCK_SIZE + BLOCK_PAGE_COUNT)
        assert FIRST_PAGE_INDEX == BLOCK_PAGE_COUNT + 4
        if pages >= 2 and random_.random() < stolen:
            data[block + FLAGS : block + CACHE_BLOCK_SIZE] = bytes(CACHE_BLOCK_SIZE - FLAGS)
            data[block + FLAGS] = STOLEN
            result.stolen.append(BASE_ADDRESS + (first_page + 1) * PAGE_SIZE)
            result.stolen_indices.append(index)
            continue
        if current is None or not levels or random_.random() < 0.4:
            current = bitmap(CACHED)
            levels = [0, 1, 2]
            random_.shuffle(levels)
        level = levels.pop()
        data[block + FLAGS] = REQUIRED if required and random_.random() < 0.3 else 0
        if random_.random() < reading:
            data[block + READ_DONE] = 0
            task = 0x500 + slot
        else:
            data[block + READ_DONE] = 1
            task = NONE if random_.random() < 0.3 else 0x500 + slot
        struct.pack_into("<I", data, block + LEVEL_INDEX, level)
        struct.pack_into("<I", data, block + READ_TASK_ID, task)
        struct.pack_into("<I", data, block + BLOCK_BITMAP, current)
        pointers.append(block + BLOCK_BITMAP)
        struct.pack_into("<I", data, current + BITMAP_BLOCK_INDICES + level * 4, index)
        # a level that is in the cache has pixels in the map
        struct.pack_into("<I", data, current + BITMAP_OFFSETS + level * 4, random_.getrandbits(32) & ~1)
        struct.pack_into("<I", data, current + BITMAP_SIZES + level * 4, pages * PAGE_SIZE - random_.choice((0, 1, 0x7FF)))
        if level == 0:
            struct.pack_into("<I", data, current + BITMAP_HARDWARE_FORMAT, block + 0x14)
            pointers.append(current + BITMAP_HARDWARE_FORMAT)

    # the list of what is lent, in an order of its own
    order = list(zip(result.stolen, result.stolen_indices))
    random_.shuffle(order)
    result.stolen = [address for address, _ in order]
    result.stolen_indices = [index for _, index in order]
    for n, (address, index) in enumerate(order):
        (pages,) = struct.unpack_from("<I", data, BLOCKS + (index & 0xFFFF) * BLOCK_SIZE + BLOCK_PAGE_COUNT)
        struct.pack_into(
            "<9I",
            memory,
            address - BASE_ADDRESS - STOLEN_SIZE,
            index,
            address ^ SIGNATURE,
            address,
            (pages - 1) * PAGE_SIZE - random_.choice((0, 1, 0x800)),
            # the mocks give back the memory of those with an odd one
            random_.getrandbits(16),
            mock_address(IDLE_PROC) if random_.random() < 0.7 else 0,
            mock_address(RECLAIM_PROC),
            order[n - 1][0] - STOLEN_SIZE if n else 0,
            order[n + 1][0] - STOLEN_SIZE if n + 1 < len(order) else 0,
        )

    cached_bitmaps = list(result.bitmaps)
    # the bitmaps that are about to be drawn, and some that nobody knows of
    at = None
    for slot in predicted_slots:
        # most are marked as being in the list, and are there once
        if cached_bitmaps and random_.random() < predict_cached:
            # one whose pixels got into the cache since
            at = random_.choice(cached_bitmaps)
        elif at is None or random_.random() < 0.85:
            at = bitmap(CACHED | (PREDICTED if random_.random() < 0.85 else 0))
        datum = predicted + DATA_ARRAY_SIZE + slot * PREDICTED_SIZE
        struct.pack_into("<I", data, datum + PREDICTED_BITMAP, at)
        pointers.append(datum + PREDICTED_BITMAP)
    for _ in range(random_.randint(1, 3)):
        bitmap(random_.choice((0, CACHED, CACHED, PREDICTED)))

    result.buffer = Buffer(bytes(data), pointers=tuple(pointers), functions=functions)
    result.memory = Memory(bytes(memory))
    return result


def cache_globals(random_: random.Random, w: World, **fields: object) -> dict:
    """The globals; `fields` are those that a test sets, by their offset."""
    values = {
        0x0: w.pointer(w.blocks),
        0x4: w.pointer(w.predicted),
        STOLEN_MEMORY: w.stolen[0] - STOLEN_SIZE if w.stolen else 0,
        0xC: BASE_ADDRESS,
        0x10: w.pointer(0),
        0x14: bytes([random_.getrandbits(1)]),
        0x18: random_.choice((0, 0x80700000)),
        0x1C: random_.choice((0, 1, 2)),
        0x20: random_.choice((0, 1)),
        # unknown, flushing, missed, unknown
        0x24: bytes([random_.getrandbits(1), fields.pop("flushing", 0), fields.pop("missed", 0), random_.getrandbits(1)]),
        0x28: struct.unpack("<I", struct.pack("<f", fields.pop("mip_bias", 0.0)))[0],
        0x2C: fields.pop("mip_bias_time", 5000),
        0x30: random_.randint(0, 12),
        0x34: fields.pop("frame_index", random_.choice((4, 5, 100, 0x7FFFFFFF))),
    }
    values.update({int(key[1:], 16): value for key, value in fields.items() if key.startswith("x")})
    result = {f"texture_cache_globals+{offset:#x}": value for offset, value in values.items()}
    result.update(
        {
            "async_globals+0x8": 0x2F,
            "async_globals+0x2598": 0,
            "async_globals+0x259c": random_.choice((0, 3, 25)),
            "d3d_device": Buffer(bytes(0x934) + struct.pack("<I", REGISTERS) + bytes(0x10)),
            "texture_cache_warning_time": random_.choice((0, 5000, 0xFFFFFF00)),
            "texture_cache_mip_bias_enabled": bytes([fields.get("enabled", 1)]),
            "texture_cache_low_detail": bytes([fields.get("low_detail", 0)]),
            "texture_cache_full_detail": bytes([fields.get("full_detail", 0)]),
            "texture_cache_default_textures": struct.pack("<3I", 0x0D000010, 0x0D000020, 0x0D000030),
        }
    )
    return result


def run(t) -> None:
    random_ = random.Random(33)
    harness = t.harness

    def give_back(address: int) -> None:
        """What a function that is asked for its memory does, without the
        cache's own work: the memory leaves the list."""
        head = harness.global_addresses["texture_cache_globals"] + STOLEN_MEMORY
        previous, next_ = struct.unpack("<2I", harness.mu.mem_read(address - STOLEN_SIZE + STOLEN_PREVIOUS, 8))
        if next_:
            harness.mu.mem_write(next_ + STOLEN_PREVIOUS, struct.pack("<I", previous))
        if previous:
            harness.mu.mem_write(previous + STOLEN_NEXT, struct.pack("<I", next_))
        else:
            harness.mu.mem_write(head, struct.pack("<I", next_))

    def idle_proc(call):
        address, user_data = call.stack
        if user_data & 1:
            give_back(address)
        return 0

    def reclaim_proc(call):
        give_back(call.stack[0])
        return 0

    mocks = [
        Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
        Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
        Mock(idle_proc, stack_arguments=2),
        Mock(reclaim_proc, stack_arguments=2),
    ]

    pending: list[int] = []

    def externals(w: World, waits: int, busy: int, ticks: tuple[int, ...], instant: bool = True, stand_in: bool = True) -> dict:
        def wait_for_single_object(harness, arguments):
            # the task's thread ends the reads while the game waits
            if calls_so_far(harness, "WaitForSingleObject") >= waits - 1:
                for slot in range(w.maximum_blocks):
                    harness.mu.mem_write(w.buffer.address + w.block(slot) + READ_DONE, b"\x01")
            return 0

        def is_busy(harness, arguments):
            return calls_so_far(harness, "D3DResource_IsBusy") < busy

        def tick_count(harness, arguments):
            return ticks[min(calls_so_far(harness, "GetTickCount"), len(ticks) - 1)]

        def read_async(harness, arguments):
            # the flag can be in the stack, which is not at the same place in
            # the game and in source/: it is not compared
            if not calls_so_far(harness, "cache_file_read_async"):
                pending.clear()
            done = arguments[4]
            if instant:
                harness.mu.mem_write(done, b"\x01")
            else:
                pending.append(done)
            inside = w.buffer.address <= done < w.buffer.address + len(w.buffer.data)
            record = (*arguments[:4], done if inside else "elsewhere", *arguments[5:])
            return 0x1000 + calls_so_far(harness, "cache_file_read_async"), record

        def switch_to_thread(harness, arguments):
            if not calls_so_far(harness, "cache_file_read_async"):
                pending.clear()
            for done in pending:
                harness.mu.mem_write(done, b"\x01")
            for slot in range(w.maximum_blocks):
                harness.mu.mem_write(w.buffer.address + w.block(slot) + READ_DONE, b"\x01")
            return 0

        def mipmap_count(harness, arguments):
            # the registers have more than the arguments
            width, depth, height, format_, none, maximum = arguments
            record = (width & 0xFFFF, depth & 0xFFFF, height & 0xFFFF, format_ & 0xFFFF, none & 0xFF, maximum & 0xFFFF)
            maximum = (maximum & 0xFFFF) - (0x10000 if maximum & 0x8000 else 0)
            return min(maximum, (width & 0xF) % 5) & 0xFFFF, record

        def row_size(harness, arguments):
            return 4 * struct.unpack("<h", harness.mu.mem_read(arguments[1] + BITMAP_WIDTH, 2))[0], (arguments[0] & 0xFF, arguments[1])

        return {
            "cache_file_read_async": read_async,
            "async_task_set_priority": lambda harness, arguments: 1,
            "SwitchToThread": switch_to_thread,
            "main_loading_idle": lambda harness, arguments: 0,
            "bitmap_mipmap_count": mipmap_count,
            "bitmap_row_size": row_size,
            "tag_iterator_next": lambda harness, arguments: (NONE, ()),
            "WaitForSingleObject": wait_for_single_object,
            "ReleaseMutex": lambda harness, arguments: 1,
            "D3DResource_IsBusy": is_busy,
            "D3DResource_BlockUntilNotBusy": lambda harness, arguments: 0,
            "D3DDevice_KickOff": lambda harness, arguments: 0,
            "XPhysicalProtect": lambda harness, arguments: (0, tuple(arguments)),
            "GetTickCount": tick_count,
            **({"texture_cache_update_predicted": lambda harness, arguments: 0} if stand_in else {}),
        }

    def check(name, w, *arguments, ticks=(6000, 6010), more=None, stand_in=True, extra=(), **fields):
        t.check(
            name,
            *arguments,
            globals={**cache_globals(random_, w, **fields), **(more or {})},
            mocks=mocks,
            extra=[w.buffer, *extra],
            externals=externals(
                w, random_.choice((1, 2)), random_.choice((0, 0, 1, 3)), ticks, random_.random() < 0.5, stand_in
            ),
            memory={BASE_ADDRESS: w.memory},
        )

    # ---------- a new cache

    for give_arrays in (True, False):
        for _ in range(3):
            blocks_memory = Buffer(random_.randbytes(BLOCKS_MEMORY_SIZE))
            predicted_memory = Buffer(random_.randbytes(PREDICTED_MEMORY_SIZE))
            cache_memory = Buffer(
                random_.randbytes(CACHE_MEMORY_SIZE), places=(DELETE_BLOCK_PROC, LOCKED_BLOCK_PROC, TYPE_BLOCK_PROC)
            )
            allocator = Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,))

            def allocate(call, give_arrays=give_arrays, memory={BLOCKS_MEMORY_SIZE: blocks_memory, PREDICTED_MEMORY_SIZE: predicted_memory, CACHE_MEMORY_SIZE: cache_memory}):
                if call.stack[0] != CACHE_MEMORY_SIZE and not give_arrays:
                    return 0
                return memory[call.stack[0]].address

            t.check(
                "texture_cache_new",
                globals={
                    "heap_allocation": allocator,
                    "texture_cache_globals": random_.randbytes(0x38),
                    "lruv_blocks_allocation": random_.choice((0, 0x60F00100)),
                },
                mocks=[
                    Mock(allocate, stack_arguments=1, registers=("ecx",)),
                    Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                ],
                extra=[blocks_memory, predicted_memory, cache_memory],
            )

    # ---------- the end of the cache

    for _ in range(40):
        w = world(random_)
        nothing = {key: 0 for key in ("x0", "x10") if random_.random() < 0.2}
        check("texture_cache_delete", w, **nothing)

    # ---------- a map comes and goes

    for _ in range(80):
        w = world(random_, valid=random_.random() < 0.5)
        stage = random_.randint(0, 4)
        low = random_.choice((0x61000, 0x1000000, 0x2F00000))
        pages = random_.choice((0, 1, 5, 20, w.page_count, 100, 0x2000))
        high = low + pages * PAGE_SIZE + random_.choice((0, 0, 1, 0xFFF, -1, -0xFFF))
        check("texture_cache_open", w, more={"physical_memory_globals": physical_memory(stage, low, high & 0xFFFFFFFF)})

    for _ in range(80):
        check("texture_cache_close", world(random_, required=True))

    for _ in range(80):
        flushing = random_.getrandbits(1)
        check("texture_cache_flush", world(random_, required=bool(flushing), valid=random_.random() < 0.9), flushing=flushing)

    # ---------- once a frame

    for _ in range(300):
        w = world(random_, required=True, block_count=random_.choice((None, 0)) if random_.random() < 0.2 else None)
        check(
            "texture_cache_update_mip_bias",
            w,
            mip_bias=random_.choice((0.0, 0.005, 0.01, 0.02, 0.05, 1.0, 1.95, 1.96, 1.99, 2.0, -1.0, 3.0)),
            missed=random_.random() < 0.2,
            enabled=random_.random() < 0.9,
        )

    # how much of the cache is in use, all the way: blocks that were all just used
    for pages_in_use in range(0, 21):
        for missed in (0, 1):
            w = world(random_, block_count=1, stolen=0.0, tick=1000)
            data = bytearray(w.buffer.data)
            if w.indices:
                at = BLOCKS + (w.indices[0] & 0xFFFF) * BLOCK_SIZE
                struct.pack_into("<3I", data, at + BLOCK_PAGE_COUNT, pages_in_use, 0, 0xFFFFFFFF)
                struct.pack_into("<I", data, at + 0x14, 1000)
            struct.pack_into("<I", data, PAGE_COUNT, 20)
            w.buffer = Buffer(bytes(data), pointers=w.buffer.pointers, functions=w.buffer.functions)
            check("texture_cache_update_mip_bias", w, mip_bias=random_.choice((0.5, 1.0)), missed=missed)

    for _ in range(150):
        w = world(random_, stolen=random_.choice((0.3, 0.8)))
        check("texture_cache_stolen_memory_idle", w)

    for _ in range(100):
        w = world(random_, valid=random_.random() < 0.9)
        check("texture_cache_reset_frame_index", w)
        check("texture_cache_tick", w, frame_index=random_.choice((4, 100, NONE, 0x7FFFFFFF, NONE - 1)))

    for _ in range(200):
        w = world(random_, required=True, stolen=random_.choice((0.0, 0.5)))
        check(
            "texture_cache_idle",
            w,
            frame_index=random_.choice((4, 100, NONE)),
            mip_bias=random_.choice((0.0, 0.5, 2.0)),
            mip_bias_time=random_.choice((0, 5000, 0xFFFFFF00)),
            ticks=random_.choice(((6000, 6010), (4000, 4010), (5000, 5001), (5001, 5002), (0xFFFFFF01, 0xFFFFFF02))),
        )

    # ---------- the bitmaps

    for _ in range(200):
        w = world(random_)
        for offset in random_.sample(w.bitmaps, min(len(w.bitmaps), 3)):
            check("texture_cache_bitmap_delete", w, w.pointer(offset))

    for _ in range(300):
        w = world(
            random_,
            maximum_predicted=random_.choice((4, 8)),
            predicted_count=random_.choice((None, 4, 8)),
            valid=random_.random() < 0.9,
        )
        for offset in random_.sample(w.bitmaps, min(len(w.bitmaps), 3)):
            check(
                "texture_cache_bitmap_predict",
                w,
                w.pointer(offset),
                ticks=random_.choice(((6000, 6010), (4000, 4010), (5000, 5001), (5001, 5002), (0xFFFFFF01, 0xFFFFFF02))),
            )

    # ---------- what the lruv_cache asks

    for _ in range(200):
        w = world(random_, block_count=random_.randint(1, 9), required=True)
        index = random_.choice(w.indices)
        flushing = random_.getrandbits(1)
        check("texture_cache_block_type", w, index)
        check("texture_cache_block_locked", w, index, flushing=flushing)

    for _ in range(200):
        flushing = random_.getrandbits(1)
        w = world(random_, block_count=random_.randint(1, 9), required=bool(flushing))
        index = random_.choice(w.indices)
        if index not in w.stolen_indices:
            check("texture_cache_block_deleted", w, index, flushing=flushing)

    # ---------- the memory that the cache lends

    for _ in range(300):
        w = world(random_, required=True, maximum_blocks=random_.choice((4, 8, 33)))
        size = random_.choice((1, 0x800, PAGE_SIZE, 3 * PAGE_SIZE, 0x3FDD, 0x3FDE, max(w.page_count - 3, 1) * PAGE_SIZE, w.page_count * PAGE_SIZE))
        check(
            "texture_cache_steal_memory_now",
            w,
            size,
            random_.getrandbits(32),
            random_.choice((0, mock_address(IDLE_PROC))),
            mock_address(RECLAIM_PROC),
        )

    for _ in range(300):
        w = world(random_, stolen=random_.choice((0.3, 0.8)))
        if w.stolen:
            check("texture_cache_return_memory", w, random_.choice(w.stolen))

    # ---------- the pixels of a bitmap

    for usage in (*range(-1, 26), 0x101, 0x10001, NONE - 5):
        t.check("texture_cache_usage_is_preloaded", usage & 0xFFFFFFFF)

    biases = (0.0, 0.5, 0.99, 1.0, 1.5, 2.0, 2.5, -1.0, 100.0)

    def some_fields() -> dict:
        """The state of the cache that the functions that look for pixels look at."""
        return {
            "x14": bytes([random_.random() < 0.6]),
            "x1c": random_.choice((0, 0, 0, 1, 2)),
            "x20": random_.choice((0, 1, 2)),
            "x30": random_.choice((0, 5, 9, 10, 11)),
            "mip_bias": random_.choice((0.0, 0.0, 0.5, 1.0, 2.0)),
            "enabled": random_.random() < 0.8,
            "frame_index": random_.choice((4, 5, 6, 100)),
            "low_detail": random_.random() < 0.1,
            "full_detail": random_.random() < 0.1,
        }

    for _ in range(300):
        w = world(random_)
        offset = random_.choice(w.bitmaps)
        check("texture_cache_bitmap_choose_level", w, w.pointer(offset), f32(random_.choice(biases)))
        for level in range(3):
            check(
                "texture_cache_initialize_hardware_format",
                w,
                w.pointer(offset),
                level,
                Buffer(random_.randbytes(0x14)),
            )
        check("texture_cache_bitmap_initialize_preloaded_format", w, w.pointer(offset), **some_fields())
        check("texture_cache_bitmap_get_preloaded_format", w, w.pointer(offset), **some_fields())

    for _ in range(400):
        w = world(random_, required=True, maximum_blocks=random_.choice((4, 8, 33)))
        offset = random_.choice(w.bitmaps)
        level = random_.randint(0, 2)
        if struct.unpack_from("<I", w.buffer.data, offset + BITMAP_BLOCK_INDICES + level * 4)[0] == NONE:
            check(
                "texture_cache_start_loading_bitmap",
                w,
                random_.getrandbits(1),
                w.pointer(offset),
                level,
                random_.getrandbits(1),
                random_.choice((1, 2, 5, 7)),
                **some_fields(),
            )

    for _ in range(800):
        w = world(random_, required=True, maximum_blocks=random_.choice((4, 8, 33)), reading=random_.choice((0.2, 0.6)))
        offset = random_.choice(w.bitmaps)
        check(
            "texture_cache_bitmap_get_level",
            w,
            random_.randint(0, 2),
            random_.choice((0, 1, 1, 1)),
            random_.choice((1, 2, 3, 5, 7)),
            w.pointer(offset),
            random_.choice((0, 0, 1)),
            random_.getrandbits(1),
            Buffer(b"\x00"),
            Buffer(b"\x00"),
            more={"async_globals+0x259c": random_.choice((0, 25, 26))},
            **some_fields(),
        )

    for _ in range(300):
        w = world(random_, required=True, maximum_blocks=random_.choice((4, 8, 33)), reading=random_.choice((0.2, 0.6)))
        check("texture_cache_bitmap_load", w, w.pointer(random_.choice(w.bitmaps)), **some_fields())

    for _ in range(800):
        w = world(random_, required=True)
        offset = random_.choice(w.bitmaps)
        check(
            "texture_cache_bitmap_get_loaded",
            w,
            random_.choice((0, 1, 2, 4, 6, 0x1F)),
            w.pointer(offset),
            f32(random_.choice(biases)),
            **some_fields(),
        )

    for _ in range(1500):
        w = world(random_, required=True, maximum_blocks=random_.choice((4, 8, 33)), reading=random_.choice((0.2, 0.6)))
        offset = random_.choice(w.bitmaps)
        check(
            "texture_cache_bitmap_get_hardware_format",
            w,
            w.pointer(offset),
            random_.choice((0, 1, 2, 3, 3, 3, 4, 6, 7, 0xA, 0x12, 0x1A, 0x1F)),
            f32(random_.choice(biases)),
            more={"async_globals+0x259c": random_.choice((0, 0, 25, 26))},
            **some_fields(),
        )

    # ---------- once a frame, with the bitmaps that are about to be drawn

    for _ in range(400):
        w = world(
            random_,
            required=True,
            maximum_blocks=random_.choice((4, 8, 33)),
            maximum_predicted=random_.choice((4, 8, 16)),
            valid=random_.random() < 0.9,
            predict_cached=random_.choice((0.0, 0.5)),
            reading=random_.choice((0.0, 0.2)),
        )
        check(
            "texture_cache_update_predicted",
            w,
            stand_in=False,
            more={"async_globals+0x259c": random_.choice((0, 0, 3, 25, 26, 100))},
            **some_fields(),
        )

    for _ in range(100):
        w = world(random_, required=True, maximum_blocks=random_.choice((4, 8, 33)))
        check("texture_cache_idle", w, stand_in=False, **some_fields())

    # ---------- the pixels that are read with the map

    for _ in range(60):
        w = world(random_)
        stage = random_.randint(0, 4)
        low = random_.choice((0x61000, 0x1000000))
        size = random_.choice((0, 1, 0x1FF, 0x200, 0x201, PAGE_SIZE, PAGE_SIZE + 1, 0x12345))
        high = low + random_.choice((0, PAGE_SIZE, 0x12000, 0x13000, 0x14000, 0x100000))
        header = bytearray(random_.randbytes(0x810))
        struct.pack_into("<2I", header, 0x160, random_.choice((0, 0x4000, 0x40004000)), size)
        check(
            "texture_cache_load_preloaded_pixels",
            w,
            more={
                "physical_memory_globals": physical_memory(stage, low, high),
                "cache_file_globals": bytes(header),
            },
        )

    # ---------- the memory that the cache lends, waiting for it

    # with no waiting, whatever the cache has; with it, a cache that will have
    # room when its blocks get old: how long the game waits when there is
    # none is a matter of the processor's clock, which is not the same twice
    for _ in range(200):
        wait = random_.choice((0, 0, 1, 2))
        if wait:
            w = world(random_, reading=0.0, stolen=0.0, maximum_blocks=random_.choice((33, 40)), predicted_count=0)
            size = random_.choice((1, PAGE_SIZE, max(w.page_count - 3, 1) * PAGE_SIZE))
            if (size + 0x2023 + PAGE_SIZE - 1) // PAGE_SIZE > w.page_count:
                continue
        else:
            w = world(random_, required=True)
            size = random_.choice((0, NONE, 1, PAGE_SIZE, 3 * PAGE_SIZE, w.page_count * PAGE_SIZE))
        check(
            "texture_cache_steal_memory",
            w,
            wait,
            size,
            random_.getrandbits(32),
            random_.choice((0, mock_address(IDLE_PROC))),
            mock_address(RECLAIM_PROC),
            stand_in=False,
            more={"texture_cache_globals+0x2c": 0xFFFFFFF0},
        )

    for ticks in (0, 1, 733333332, 733333333, 733333334, 1466666666, 0x7FFFFFFF, 0x100000000, 0x123456789AB, 366666666, 73333333):
        t.check("system_ticks_to_seconds", ticks & 0xFFFFFFFF, ticks >> 32)
