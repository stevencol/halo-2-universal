# source/cache/xbox_geometry_cache.c
#
# A cache is made in one buffer, as the game has it: the lruv_cache with its
# blocks, the data array of the cache's blocks, that of the geometry that is
# about to be drawn, and the blocks of geometry of the tags, each with the
# fields of its tag after it. The reads from the map are the test's

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
CACHE_BLOCK_SIZE = 0xC
READ_DONE = 0x2
READ_PRIORITY = 0x3
READ_TASK_ID = 0x4
BLOCK_INFO = 0x8

# geometry that is about to be drawn
PREDICTED_SIZE = 0x8
PREDICTED_INFO = 0x4

# a block of geometry of a tag, and the two fields of the tag after it
INFO_SIZE = 0x24
INFO_BLOCK_SIZE = 0x4
INFO_OWNER_TAG = 0x18
INFO_LOADED = 0x1E
INFO_FLAGS = 0x1F
INFO_CACHE_BLOCK = 0x20
PREDICTED = 4

DELETED = "geometry_cache_block_deleted"
LOCKED = "geometry_cache_block_locked"

BLOCKS_MEMORY_SIZE = 0x188C
PREDICTED_MEMORY_SIZE = 0xB68
CACHE_MEMORY_SIZE = 0x30FC

# the tags that the blocks of geometry are of: those of the levels, and others
GROUPS = (b"pmtl", b"psbs", b"edom", b"dmaj", b"pmtl", b"edom", b"psbs", b"tmib")


@dataclass
class World:
    buffer: Buffer
    blocks: int
    predicted: int
    maximum_blocks: int
    indices: list[int]
    page_count: int
    # the offsets of the blocks of geometry: by the index of their block of
    # the cache, those that are about to be drawn, and those that nobody knows of
    cached: dict[int, int] = field(default_factory=dict)
    predicted_infos: list[int] = field(default_factory=list)
    other_infos: list[int] = field(default_factory=list)

    def block(self, index: int) -> int:
        return self.blocks + DATA_ARRAY_SIZE + (index & 0xFFFF) * CACHE_BLOCK_SIZE

    def pointer(self, offset: int) -> Pointer:
        return Pointer(self.buffer, offset)

    def infos(self) -> list[int]:
        return [*self.cached.values(), *self.predicted_infos, *self.other_infos]


def world(
    random_: random.Random,
    block_count: int | None = None,
    maximum_blocks: int | None = None,
    maximum_predicted: int | None = None,
    predicted_count: int | None = None,
    reading: float = 0.3,
    tick: int | None = None,
    large: bool = False,
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
    functions = {DELETE_BLOCK_PROC: DELETED, LOCKED_BLOCK_PROC: LOCKED}
    (maximum_blocks,) = struct.unpack_from("<i", data, CACHE_SIZE + 0x20)
    (page_count,) = struct.unpack_from("<I", data, PAGE_COUNT)
    struct.pack_into("<I", data, PAGE_SIZE_BITS, PAGE_BITS)
    struct.pack_into("<I", data, HOLE_ALGORITHM, random_.choice((0, 1, 2, 2)))
    struct.pack_into("<I", data, TYPE_BLOCK_PROC, 0)
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

    allocator = append(Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,)))
    for at in (blocks + ALLOCATION, predicted + ALLOCATION):
        if random_.random() < 0.8:
            struct.pack_into("<I", data, at, allocator)
            pointers.append(at)
    struct.pack_into("<I", data, CACHE_ALLOCATION, allocator)
    pointers.append(CACHE_ALLOCATION)

    result = World(Buffer(b""), blocks, predicted, maximum_blocks, indices, page_count)

    def info(size: int, block_index: int, loaded: int, flags: int = 0, usable: bool = True) -> int:
        at = len(data)
        data.extend(
            struct.pack(
                "<4i2IIhBBI",
                random_.getrandbits(31),
                size,
                # with no geometry in it, the cache does nothing
                random_.choice((1, 0x100, 0x4000)) if usable else random_.choice((0, -1, 5)),
                random_.choice((0, 0x100)) if usable or random_.random() < 0.5 else -1,
                random_.randint(0, 3),
                0x40000000,
                random_.randrange(len(GROUPS)),
                INFO_SIZE,
                loaded,
                (random_.getrandbits(8) & ~PREDICTED) | flags,
                block_index,
            )
        )
        # the fields of the tag
        data.extend(random_.randbytes(8))
        return at

    for index in indices:
        (pages,) = struct.unpack_from("<I", data, BLOCKS + (index & 0xFFFF) * BLOCK_SIZE + BLOCK_PAGE_COUNT)
        block = result.block(index)
        if random_.random() < reading:
            done, loaded = 0, 0
        else:
            done, loaded = 1, random_.random() < 0.7
        at = info(pages * PAGE_SIZE - random_.choice((0, 1, 0x1FF, 0x200, 0x201, PAGE_SIZE - 1)), index, loaded)
        result.cached[index] = at
        data[block + READ_DONE] = done
        data[block + READ_PRIORITY] = random_.choice((1, 2, 5, 7))
        struct.pack_into("<I", data, block + READ_TASK_ID, 0x500 + (index & 0xFFFF))
        struct.pack_into("<I", data, block + BLOCK_INFO, at)
        pointers.append(block + BLOCK_INFO)

    def new_size() -> int:
        if large and random_.random() < 0.5:
            pages = random_.choice((page_count, page_count // 2 + 1, page_count + 1, page_count - 1))
            return max(pages, 1) * PAGE_SIZE
        return random_.randint(1, 6) * PAGE_SIZE - random_.choice((0, 1, 0x200, 0x7FF))

    at = None
    for slot in predicted_slots:
        if indices and random_.random() < predict_cached:
            at = result.cached[random_.choice(indices)]
            data[at + INFO_FLAGS] |= PREDICTED
        elif at is None or at in result.cached.values() or random_.random() < 0.9:
            at = info(new_size(), NONE, 0, PREDICTED if random_.random() < 0.9 else 0)
            result.predicted_infos.append(at)
        datum = predicted + DATA_ARRAY_SIZE + slot * PREDICTED_SIZE
        struct.pack_into("<I", data, datum + PREDICTED_INFO, at)
        pointers.append(datum + PREDICTED_INFO)

    for _ in range(random_.randint(1, 3)):
        result.other_infos.append(
            info(new_size(), NONE, random_.random() < 0.1, usable=random_.random() < 0.85)
        )

    result.buffer = Buffer(bytes(data), pointers=tuple(pointers), functions=functions)
    return result


def cache_globals(random_: random.Random, w: World, **fields: object) -> dict:
    """The globals; `fields` are those that a test sets, by their offset."""
    values = {
        0x0: w.pointer(w.blocks),
        0x4: w.pointer(w.predicted),
        0x8: BASE_ADDRESS,
        0xC: 0x70000000 + w.page_count,
        0x10: w.pointer(0),
        0x14: random_.choice((0, 0, 0, 1, 2)),
        0x18: random_.choice((0, 1, 2)),
        0x1C: random_.getrandbits(16) | 0x40000000,
        0x20: random_.choice((0, 0x40001234)),
        0x24: bytes([random_.getrandbits(1)]),
    }
    values.update({int(key[1:], 16): value for key, value in fields.items() if key.startswith("x")})
    result = {f"geometry_cache_globals+{offset:#x}": value for offset, value in values.items()}
    result.update(
        {
            "async_globals+0x8": 0x2F,
            "async_globals+0x2598": 0,
            "async_globals+0x259c": fields.get("tasks", random_.choice((0, 3, 25))),
            "d3d_device": Buffer(bytes(0x934) + struct.pack("<I", REGISTERS) + bytes(0x10)),
            "geometry_cache_warning_time": random_.choice((0, 5000, 0xFFFFFF00)),
            "geometry_cache_never_block": bytes([fields.get("never_block", random_.random() < 0.5)]),
            "game_state_globals+0x2538": bytes([random_.getrandbits(1)]),
            "global_tag_instances": Buffer(b"".join(group + bytes(12) for group in GROUPS)),
        }
    )
    return result


def run(t) -> None:
    random_ = random.Random(35)

    allocator_mocks = [
        Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
        Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
    ]

    def externals(w: World, instant: bool, waits: int, busy: int, ticks: tuple[int, ...]) -> dict:
        def finish_reads(harness):
            for slot in range(w.maximum_blocks):
                harness.mu.mem_write(w.buffer.address + w.block(slot) + READ_DONE, b"\x01")

        def read_async(harness, arguments):
            if instant:
                harness.mu.mem_write(arguments[4], b"\x01")
            # a size can be the address of a function of the game: it is not one
            return 0x1000 + calls_so_far(harness, "cache_file_read_async"), tuple(arguments)

        def switch_to_thread(harness, arguments):
            finish_reads(harness)
            return 0

        def wait_for_single_object(harness, arguments):
            if calls_so_far(harness, "WaitForSingleObject") >= waits - 1:
                finish_reads(harness)
            return 0

        def block_busy(harness, arguments):
            return calls_so_far(harness, "geometry_block_busy") < busy

        def tick_count(harness, arguments):
            return ticks[min(calls_so_far(harness, "GetTickCount"), len(ticks) - 1)]

        return {
            "cache_file_read_async": read_async,
            "async_task_set_priority": lambda harness, arguments: 1,
            "SwitchToThread": switch_to_thread,
            "main_loading_idle": lambda harness, arguments: 0,
            "WaitForSingleObject": wait_for_single_object,
            "ReleaseMutex": lambda harness, arguments: 1,
            "geometry_block_attach_data": lambda harness, arguments: 0,
            "geometry_block_busy": block_busy,
            "geometry_block_wait": lambda harness, arguments: 0,
            "unknown_0012e480": lambda harness, arguments: 0,
            "D3DDevice_KickOff": lambda harness, arguments: 0,
            "XPhysicalProtect": lambda harness, arguments: (0, tuple(arguments)),
            "GetTickCount": tick_count,
        }

    def check(name, w, *arguments, ticks=(6000, 6010), more=None, **fields):
        t.check(
            name,
            *arguments,
            globals={**cache_globals(random_, w, **fields), **(more or {})},
            mocks=allocator_mocks,
            extra=[w.buffer],
            externals=externals(
                w, random_.random() < 0.5, random_.choice((1, 2)), random_.choice((0, 0, 1, 3)), ticks
            ),
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
                "geometry_cache_new",
                globals={
                    "heap_allocation": allocator,
                    "geometry_cache_globals": random_.randbytes(0x28),
                    "lruv_blocks_allocation": random_.choice((0, 0x60F00100)),
                },
                mocks=[
                    Mock(allocate, stack_arguments=1, registers=("ecx",)),
                    Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                ],
                extra=[blocks_memory, predicted_memory, cache_memory],
            )

    for _ in range(40):
        w = world(random_)
        nothing = {key: 0 for key in ("x0", "x4", "x10") if random_.random() < 0.2}
        check("geometry_cache_delete", w, **nothing)

    # ---------- its memory

    for configuration in range(6):
        for _ in range(8):
            w = world(random_)
            stage = random_.randint(0, 4)
            low = random_.choice((0x61000, 0x1000000, 0x2F00000))
            high = random_.choice((0x3145000, low + 0x700000, low + 0x6FFFFF, low + 0x680000, low + 0x67F000, low))
            more = {"physical_memory_globals": physical_memory(stage, low, high & 0xFFFFFFFF)}
            nothing = {"x10": 0} if random_.random() < 0.15 else {}
            check("geometry_cache_allocate_memory", w, configuration, more=more, **nothing)

    for _ in range(40):
        w = world(random_)
        check("geometry_cache_free_memory", w, **({"x10": 0} if random_.random() < 0.15 else {}))

    # ---------- a map comes and goes

    for _ in range(40):
        w = world(random_, valid=random_.random() < 0.5)
        nothing = {key: 0 for key in ("x0", "x4") if random_.random() < 0.2}
        check("geometry_cache_open", w, **nothing)

    for _ in range(80):
        w = world(random_, valid=random_.random() < 0.9)
        check("geometry_cache_close", w)
        check("geometry_cache_flush", w, **({"x10": 0} if random_.random() < 0.1 else {}))

    # ---------- what the lruv_cache asks

    for _ in range(300):
        w = world(random_, block_count=random_.randint(1, 9))
        index = random_.choice(w.indices)
        check("geometry_cache_block_locked", w, index)
        if random_.random() < 0.15:
            # a block that was not given its geometry yet
            at = w.block(index) + BLOCK_INFO
            data = bytearray(w.buffer.data)
            struct.pack_into("<I", data, at, 0)
            w.buffer = Buffer(
                bytes(data),
                pointers=tuple(offset for offset in w.buffer.pointers if offset != at),
                functions=w.buffer.functions,
            )
        check("geometry_cache_block_deleted", w, index)

    # ---------- the blocks of geometry

    for _ in range(100):
        check("geometry_cache_reading_count", world(random_, reading=random_.choice((0.0, 0.5, 1.0)), valid=random_.random() < 0.8))

    for _ in range(300):
        w = world(random_, predict_cached=random_.choice((0.0, 0.3)))
        for offset in random_.sample(w.infos(), min(len(w.infos()), 3)):
            check("geometry_cache_block_delete", w, w.pointer(offset))

    for _ in range(500):
        w = world(random_, large=random_.random() < 0.4, maximum_blocks=random_.choice((4, 8, 33)))
        offset = random_.choice(w.predicted_infos + w.other_infos)
        if struct.unpack_from("<I", w.buffer.data, offset + INFO_CACHE_BLOCK)[0] == NONE:
            check(
                "geometry_cache_start_loading",
                w,
                random_.getrandbits(1),
                w.pointer(offset),
                random_.getrandbits(1),
                random_.choice((1, 2, 5, 7)),
            )

    for _ in range(2000):
        w = world(
            random_,
            large=random_.random() < 0.3,
            maximum_blocks=random_.choice((4, 8, 33)),
            reading=random_.choice((0.0, 0.3, 0.9)),
        )
        check(
            "geometry_cache_block_load",
            w,
            w.pointer(random_.choice(w.infos())),
            random_.choice((0, 1, 2, 3, 3, 6, 7, 0xA, 0xB, 0xE, 0xF, 4, 8)),
        )

    for _ in range(600):
        w = world(
            random_,
            large=random_.random() < 0.3,
            maximum_predicted=random_.choice((4, 8)),
            predicted_count=random_.choice((None, 4, 8)),
        )
        check(
            "geometry_cache_block_predict",
            w,
            w.pointer(random_.choice(w.infos())) if random_.random() < 0.97 else 0,
            ticks=random_.choice(((6000, 6010), (4000, 4010), (5000, 5001), (5001, 5002), (0xFFFFFF01, 0xFFFFFF02))),
        )

    for _ in range(200):
        w = world(random_, predict_cached=random_.choice((0.0, 0.3)), valid=random_.random() < 0.9)
        nothing = {"x0": 0} if random_.random() < 0.1 else {}
        check("geometry_cache_delete_tag_blocks", w, random_.randrange(len(GROUPS)), **nothing)

    # ---------- once a frame

    for _ in range(400):
        w = world(
            random_,
            large=random_.random() < 0.3,
            maximum_blocks=random_.choice((4, 8, 33)),
            reading=random_.choice((0.0, 0.3, 0.9)),
            valid=random_.random() < 0.9,
            predict_cached=random_.choice((0.0, 0.3)),
        )
        check("geometry_cache_update_predicted", w, tasks=random_.choice((0, 3, 25, 26, 100, NONE)))

    for _ in range(200):
        w = world(random_, maximum_blocks=random_.choice((4, 8, 33)), valid=random_.random() < 0.9)
        check("geometry_cache_idle", w, **({"x10": 0} if random_.random() < 0.1 else {}))
