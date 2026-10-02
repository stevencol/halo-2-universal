# source/text/font_cache.c

import random
import struct

from difftest import Buffer, FunctionPointer, Mock, Pointer, mock_address
from test_async import calls_so_far

DATA_SIZE = 0x4C
DATA_ALLOCATION = 0x30
TABLE_SIZE = 0x38
TABLE_ALLOCATION = 0x34
LRUV_SIZE = 0x70
LRUV_PAGE_SIZE_BITS, LRUV_BLOCKS, LRUV_ALLOCATION = 0x34, 0x64, 0x6C
BLOCK_SIZE = 0x18
ALLOCATOR_SIZE = 12


def run(t) -> None:
    random_ = random.Random(83)

    for character in (0, 0x41, 0xDFFF, 0xE000, 0xE001, 0xE405, 0xF8FE, 0xF8FF, 0xF900, 0xFFFF, 0x1E000, 0xABCDE123, 0x8000DFFF):
        t.check("character_is_private_use", character)

    # ---------- the end of what the game knows of the characters

    def things(sizes: tuple, allocations: tuple, allocated: tuple) -> tuple:
        """Some things one after another in a buffer, each with the thing that
        gives its memory back after it; and where each is."""
        blob = bytearray()
        pointers = []
        places = []
        for size, allocation, has in zip(sizes, allocations, allocated):
            place = len(blob)
            places.append(place)
            blob += random_.randbytes(size)
            allocator = len(blob)
            # the two functions of the allocator: the first is not called
            blob += struct.pack("<3I", allocator + 4, 0x11111111, mock_address(0))
            pointers.append(allocator)
            if has:
                struct.pack_into("<I", blob, place + allocation, allocator)
                pointers.append(place + allocation)
            else:
                struct.pack_into("<I", blob, place + allocation, 0)
        return Buffer(bytes(blob), pointers=tuple(pointers)), places

    freed = Mock(lambda call: 0, stack_arguments=1, registers=("ecx",))

    for n in range(40):
        # the table always has what frees it; an array may have nothing
        blob, places = things(
            (TABLE_SIZE, DATA_SIZE, DATA_SIZE),
            (TABLE_ALLOCATION, DATA_ALLOCATION, DATA_ALLOCATION),
            (True, random_.random() < 0.7, random_.random() < 0.7),
        )
        there = [random_.random() < 0.75 for _ in range(3)]
        t.check(
            "font_characters_dispose",
            globals={
                "font_cache_globals": Pointer(blob, places[0]) if there[0] else 0,
                "font_cache_globals+4": Pointer(blob, places[1]) if there[1] else 0,
                "font_cache_globals+8": Pointer(blob, places[2]) if there[2] else 0,
                "font_cache_globals+12": random_.getrandbits(32),
                "font_cache_globals+16": random_.getrandbits(32),
            },
            extra=[blob],
            mocks=[freed],
        )

    for n in range(40):
        blob, places = things((LRUV_SIZE, DATA_SIZE), (LRUV_ALLOCATION, DATA_ALLOCATION), (True, random_.random() < 0.7))
        there = [random_.random() < 0.75 for _ in range(2)]
        t.check(
            "font_cache_dispose",
            globals={
                "font_cache_globals": random_.getrandbits(32),
                "font_cache_globals+4": random_.getrandbits(32),
                "font_cache_globals+12": Pointer(blob, places[1]) if there[1] else 0,
                "font_cache_globals+16": Pointer(blob, places[0]) if there[0] else 0,
            },
            extra=[blob],
            mocks=[freed],
        )

    # ---------- where the pixels of a block are

    BLOCKS = 6
    for n in range(80):
        # the cache, the array of its blocks and the blocks, in a buffer
        data_place = LRUV_SIZE
        blocks_place = LRUV_SIZE + DATA_SIZE
        blob = bytearray(random_.randbytes(blocks_place + BLOCKS * BLOCK_SIZE))
        struct.pack_into("<I", blob, LRUV_PAGE_SIZE_BITS, random_.choice((0, 1, 9, 12, 16, 31, 32, 44)))
        struct.pack_into("<I", blob, LRUV_BLOCKS, data_place)
        struct.pack_into("<I", blob, data_place + 0x44, blocks_place)
        for block in range(BLOCKS):
            struct.pack_into("<I", blob, blocks_place + block * BLOCK_SIZE + 8, random_.choice((0, 1, 5, 0x1234, 0xFFFFF, random_.getrandbits(32))))
        t.check(
            "font_cache_block_get_address",
            random_.randrange(BLOCKS) | random_.choice((0, 0, 0x10000, 0xE1740000)),
            globals={
                "font_cache_globals+16": Buffer(bytes(blob), pointers=(LRUV_BLOCKS, data_place + 0x44)),
                "font_cache_globals+20": random_.choice((0, 0x80400000, random_.getrandbits(32))),
            },
        )

    # ---------- finding a character

    def key(font: int, character: int) -> Buffer:
        return Buffer(struct.pack("<2I", font, character) + random_.randbytes(4))

    values = (0, 1, 2, 3, 9, 0x41, 0x3042, 0xFFFF, 0x10000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF, 0x55555556, 0xAAAAAAAB)
    for font in values:
        for character in values:
            t.check("font_character_key_hash", key(font, character))
    for _ in range(200):
        font, character = random_.choice(values), random_.choice(values)
        other = random_.choice(((font, character), (font, character), (font ^ 1, character), (font, character ^ 0x100), (character, font), (random_.choice(values), random_.choice(values))))
        t.check("font_character_key_compare", key(font, character), key(*other))

    HEADER, ENTRY = 0x3C, 0x10
    NONE = 0xFFFFFFFF

    def hash_of(font: int, character: int) -> int:
        return font * 3 + character & 0xFFFFFFFF

    def characters(list_count: int, known: dict) -> tuple:
        """The table that finds the characters, with some in it: the key of
        each is after the entries, as it would be in what the game knows of
        the character. The functions of the table are those of the game, and
        of source/."""
        entries = HEADER + 4 * list_count
        keys = entries + ENTRY * (len(known) + 2)
        blob = bytearray(random_.randbytes(keys + 8 * len(known) + 8))
        blob[0x1F] = 0
        struct.pack_into("<3I", blob, 0x20, list_count, len(known) + 2, 4)
        struct.pack_into("<I", blob, 0x34, 0x80AA0000)
        struct.pack_into("<I", blob, 0x38, 0)
        pointers = []
        lists = [0] * list_count
        for n, ((font, character), index) in enumerate(known.items()):
            place = entries + ENTRY * n
            key_place = keys + 8 * n
            struct.pack_into("<2I", blob, key_place, font, character)
            list_ = hash_of(font, character) % list_count
            struct.pack_into("<4I", blob, place, key_place, hash_of(font, character), lists[list_], index)
            pointers.append(place)
            if lists[list_]:
                pointers.append(place + 8)
            lists[list_] = place
        for list_, place in enumerate(lists):
            struct.pack_into("<I", blob, HEADER + 4 * list_, place)
            if place:
                pointers.append(HEADER + 4 * list_)
        return Buffer(bytes(blob), pointers=tuple(pointers), functions={0x2C: "font_character_key_hash", 0x30: "font_character_key_compare"})

    for n in range(200):
        list_count = random_.choice((1, 3, 16, 0x400))
        known = {}
        for _ in range(random_.randrange(0, 9)):
            known[(random_.randrange(4), random_.choice((0x41, 0x42, 0x44, 0x47, 0x3042, random_.getrandbits(16))))] = random_.getrandbits(32)
        asked = [(random_.randrange(4), random_.choice((0x41, 0x44, 0x3042)))]
        if known:
            font, character = random_.choice(list(known))
            # a character that is known; the same of another font; one with
            # the same hash, which is three times the font and the character
            asked += [(font, character), (font, character), (font ^ 1, character), (font + 1, (character - 3) & 0xFFFFFFFF)]
        font, character = random_.choice(asked)
        t.check(
            "font_character_find",
            font,
            character,
            globals={"font_cache_globals": characters(list_count, known)},
        )

    # ---------- the characters that the game knows

    CHARACTER = 0x38
    CHARACTERS = 6
    STATE, BLOCK, FRAME, SLOT = 0x10, 0x2C, 0x30, 0x34
    SLOTS = 512

    def known_characters(values: dict) -> Buffer:
        """The array of the characters and, after it, the characters."""
        blob = bytearray(random_.randbytes(DATA_SIZE + CHARACTERS * CHARACTER))
        struct.pack_into("<I", blob, 0x44, DATA_SIZE)
        for index, (state, block, frame, slot) in values.items():
            place = DATA_SIZE + index * CHARACTER
            struct.pack_into("<I", blob, place + STATE, state)
            struct.pack_into("<3I", blob, place + BLOCK, block, frame & 0xFFFFFFFF, slot)
        return Buffer(bytes(blob), pointers=(0x44,))

    for n in range(200):
        index = random_.randrange(CHARACTERS)
        other = random_.randrange(CHARACTERS)
        slot = random_.choice((NONE, NONE, 0, 7, 511))
        values = {
            index: (random_.randrange(5), random_.choice((NONE, NONE, 0xE1740003, 5)), random_.getrandbits(32), slot),
            other: (random_.randrange(5), NONE, 0, random_.choice((NONE, 3, slot))),
        }
        # its place among the pictures: its own, another character's, or no one's
        slots = bytearray(random_.randbytes(SLOTS * 8))
        if slot != NONE:
            struct.pack_into("<I", slots, slot * 8, random_.choice((NONE, index, other, index | 0xE1740000, other | 0x10000)))
        t.check(
            "font_character_delete",
            index | random_.choice((0, 0, 0x10000, 0xE1740000)),
            globals={
                "font_cache_globals": 0x80111100,
                "font_cache_globals+4": known_characters(values),
                "font_cache_globals+16": 0x80222200,
                "font_glyph_slots": bytes(slots),
            },
            externals={
                "lruv_block_delete": lambda harness, arguments: 0,
                "hash_table_remove": lambda harness, arguments: 1,
                "datum_delete": lambda harness, arguments: 0,
            },
        )

    BLOCKS = 8
    BLOCK_SIZE_ = 0xC
    for n in range(300):
        index = random_.randrange(CHARACTERS)
        frame = random_.choice((0, 5, 1000, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF))
        last = (frame - random_.choice((0, 1, 1, 2, 2, 3, 100, -1, -5))) & 0xFFFFFFFF
        block = random_.choice((NONE, NONE, random_.randrange(BLOCKS), random_.randrange(BLOCKS) | 0xE1740000))
        values = {index: (random_.choice((0, 1, 2, 3, 4, 4, 4, 5)), block, last, NONE)}
        blocks = bytearray(random_.randbytes(DATA_SIZE + BLOCKS * BLOCK_SIZE_))
        struct.pack_into("<I", blocks, 0x44, DATA_SIZE)
        for at in range(BLOCKS):
            blocks[DATA_SIZE + at * BLOCK_SIZE_ + 2] = random_.choice((0, 0, 1, 0x80))
        t.check(
            "font_character_is_busy",
            index | random_.choice((0, 0, 0x10000, 0xE1740000)),
            globals={
                "font_cache_globals+4": known_characters(values),
                "font_cache_globals+12": Buffer(bytes(blocks), pointers=(0x44,)),
                "font_cache_globals+24": frame,
            },
        )

    # more characters than a number of 8 bits tells apart
    for index in (0x100, 0x1FF, 0x101):
        MANY = 0x200
        blob = bytearray(random_.randbytes(DATA_SIZE + MANY * CHARACTER))
        struct.pack_into("<I", blob, 0x44, DATA_SIZE)
        # those that would be taken for it have used in this frame; it, long ago
        for at in (index, index & 0xFF):
            struct.pack_into("<I", blob, DATA_SIZE + at * CHARACTER + STATE, 4)
            struct.pack_into("<3I", blob, DATA_SIZE + at * CHARACTER + BLOCK, NONE, 1000 if at != index else 5, NONE)
        t.check(
            "font_character_is_busy",
            index,
            globals={"font_cache_globals+4": Buffer(bytes(blob), pointers=(0x44,)), "font_cache_globals+24": 1000},
        )

    # ---------- making the characters and the cache

    CHARACTERS_SIZE, PREDICTED_SIZE, TABLE_MEMORY_SIZE = 0x708C, 0xC6C, 0x303C
    BLOCKS_SIZE, CACHE_SIZE = 0x188C, 0x30FC

    def gives(memory: dict):
        """The function that gives memory: each size has its own."""

        def handler(call):
            buffer = memory.get(call.stack[0])
            return buffer.address if buffer is not None else 0

        return handler

    for n in range(6):
        characters = Buffer(random_.randbytes(CHARACTERS_SIZE + 8))
        predicted = Buffer(random_.randbytes(PREDICTED_SIZE + 8))
        # the two functions of the table are the game's, or those of source/
        table = Buffer(random_.randbytes(TABLE_MEMORY_SIZE + 8), places=(0x2C, 0x30))
        memory = {CHARACTERS_SIZE: characters, PREDICTED_SIZE: predicted}
        if n != 5:
            memory[TABLE_MEMORY_SIZE] = table
        allocator = Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,))
        t.check(
            "font_characters_new",
            globals={"heap_allocation": allocator, "font_cache_globals": random_.randbytes(0x1C)},
            mocks=[Mock(gives(memory), stack_arguments=1, registers=("ecx",)), Mock(lambda call: 0, stack_arguments=1, registers=("ecx",))],
            extra=[characters, predicted, table],
        )

    for n in range(6):
        blocks = Buffer(random_.randbytes(BLOCKS_SIZE + 8))
        cache = Buffer(random_.randbytes(CACHE_SIZE + 8), places=(0x20, 0x24))
        allocator = Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,))
        base = 0 if n == 5 else 0x80400000
        t.check(
            "font_cache_new",
            globals={
                "heap_allocation": allocator,
                "font_cache_globals": random_.randbytes(0x1C),
                "lruv_blocks_allocation": random_.choice((0, 0x60F00100)),
            },
            mocks=[
                Mock(gives({BLOCKS_SIZE: blocks, CACHE_SIZE: cache}), stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
            extra=[blocks, cache],
            externals={
                "VirtualAlloc": lambda harness, arguments, base=base: base,
                "GetLastError": lambda harness, arguments: 8,
            },
        )

    # ---------- what the cache asks of its blocks

    def cache_blocks(values: dict) -> Buffer:
        blob = bytearray(random_.randbytes(DATA_SIZE + BLOCKS * BLOCK_SIZE_))
        struct.pack_into("<I", blob, 0x44, DATA_SIZE)
        for index, (loaded, character) in values.items():
            blob[DATA_SIZE + index * BLOCK_SIZE_ + 2] = loaded
            struct.pack_into("<I", blob, DATA_SIZE + index * BLOCK_SIZE_ + 4, character)
        return Buffer(bytes(blob), pointers=(0x44,))

    for n in range(60):
        index = random_.randrange(BLOCKS)
        t.check(
            "font_cache_block_locked",
            index | random_.choice((0, 0, 0x10000, 0xE1740000)),
            globals={"font_cache_globals+12": cache_blocks({index: (random_.choice((0, 0, 1, 0x80)), 0)})},
        )

    TASK_SIZE, TASK_NEXT = 0x40, 0x3C
    for n in range(80):
        index = random_.randrange(BLOCKS)
        character = random_.randrange(CHARACTERS)
        waits = random_.choice((0, 0, 1, 2, 3))
        blocks = cache_blocks({index: (waits == 0, character | random_.choice((0, 0x10000, 0xE1740000)))})
        # the tasks that are not done: the game counts them while it waits
        task_count = random_.choice((0, 1, 2, 5))
        tasks = bytearray(random_.randbytes(max(task_count, 1) * TASK_SIZE))
        for task in range(task_count):
            struct.pack_into("<I", tasks, task * TASK_SIZE + TASK_NEXT, (task + 1) * TASK_SIZE if task + 1 < task_count else 0)
        task_list = Buffer(bytes(tasks), pointers=tuple(task * TASK_SIZE + TASK_NEXT for task in range(task_count - 1)))

        def wait(harness, arguments, waits=waits, blocks=blocks, index=index):
            if calls_so_far(harness, "WaitForSingleObject") >= waits - 1:
                harness.mu.mem_write(blocks.address + DATA_SIZE + index * BLOCK_SIZE_ + 2, b"\x01")
            return 0

        t.check(
            "font_cache_block_deleted",
            index | random_.choice((0, 0, 0x10000, 0xE1740000)),
            globals={
                "font_cache_globals+4": known_characters({character: (4, 0xE1740000 | index, 0, NONE)}),
                "font_cache_globals+12": blocks,
                "async_globals+8": 0x1234,
                "async_globals+9624": Pointer(task_list, 0) if task_count else 0,
                "async_globals+9628": random_.getrandbits(32),
            },
            extra=[task_list],
            externals={
                "WaitForSingleObject": wait,
                "ReleaseMutex": lambda harness, arguments: 1,
                "datum_delete": lambda harness, arguments: 0,
            },
        )

    # ---------- a new character

    for n in range(300):
        font, character = random_.randrange(11), random_.choice((0x41, 0x3042, random_.getrandbits(32)))
        frame = random_.choice((5, 1000, 0x7FFFFFFF, 0x80000000))
        found = NONE if n % 5 else random_.getrandbits(32)
        fresh = NONE if n % 3 == 0 else random_.randrange(CHARACTERS) | random_.choice((0, 0x10000, 0xE1740000))
        # the characters that there are: whether each is busy, and when it was used
        there = sorted(random_.sample(range(CHARACTERS), random_.randrange(0, CHARACTERS + 1)))
        busy = {index: random_.random() < 0.4 for index in there}
        values = {index: (4, NONE, (frame - random_.choice((0, 1, 2, 2, 7, 7, 100, -3))) & 0xFFFFFFFF, NONE) for index in there}
        blob = bytearray(known_characters(values).data)
        struct.pack_into("<I", blob, 0x24, CHARACTER)
        for index in range(CHARACTERS):
            struct.pack_into("<h", blob, DATA_SIZE + index * CHARACTER, random_.choice((1, 0x7FFF, -0x8000, -2)))
        again = NONE if n % 11 == 10 else None

        def next_one(harness, arguments, there=there):
            data, start = arguments
            later = [index for index in there if index >= start]
            return (later[0] if later else NONE), (start,)

        def is_busy(harness, arguments, busy=busy):
            return busy[arguments[0] & 0xFFFF]

        def new_at(harness, arguments, again=again):
            data, index = arguments
            return (index if again is None else again), (index,)

        def add(harness, arguments):
            table, key, data = arguments
            # the index that goes with the key is in the stack
            return 1, (table, harness.normalize(key), bytes(harness.mu.mem_read(data, 4)))

        t.check(
            "font_character_new",
            font,
            character,
            globals={
                "font_cache_globals": 0x80111100,
                "font_cache_globals+4": Buffer(bytes(blob), pointers=(0x44,)),
                "font_cache_globals+24": frame,
            },
            externals={
                "font_character_find": lambda harness, arguments, found=found: found,
                "datum_new": lambda harness, arguments, fresh=fresh: (fresh, ()),
                "data_next_absolute_index": next_one,
                "font_character_is_busy": is_busy,
                "font_character_delete": lambda harness, arguments: 0,
                "datum_new_at_index": new_at,
                "hash_table_add": add,
            },
        )

    # ---------- getting a character ready

    FONT_SIZE, FONT_SLOTS, FONT_FILE = 0x1D0, 10, 0x1C4
    LOCATION_LOADED, HEADER_LOADED, TASK, LOCATION, HEADER = 0xC, 0xD, 0x14, 0x18, 0x1C
    PREFERENCES_SIZE, PREFERENCES_DIRTY, PREFERENCES_FONTS = 0x408, 1, 0x24

    def fonts() -> bytes:
        data = bytearray(random_.randbytes(FONT_SLOTS * FONT_SIZE))
        for slot in range(FONT_SLOTS):
            struct.pack_into("<I", data, slot * FONT_SIZE + FONT_FILE, 0x700 + slot)
        return bytes(data)

    def a_character(state: int, location_loaded: int = 0, header_loaded: int = 0, header: tuple | None = None) -> bytearray:
        data = bytearray(random_.randbytes(CHARACTER))
        struct.pack_into("<2I", data, 4, random_.randrange(FONT_SLOTS), random_.choice((0, 0x41, 0x3042, 0xFFFF, random_.getrandbits(32))))
        data[LOCATION_LOADED] = location_loaded
        data[HEADER_LOADED] = header_loaded
        struct.pack_into("<I", data, STATE, state)
        if header:
            struct.pack_into("<H2h", data, HEADER + 2, *header)
        return data

    def read(harness, arguments):
        file, size, offset, category, priority, bytes_read, buffer, done = arguments
        return 0x00AB0000 | size, (file, size, offset, category, priority, bytes_read, harness.normalize(buffer), harness.normalize(done))

    sizes =(0, 1, 2, 0x7F, 0x80, 0x81, 0x1000, 0x1001, 0x7FFF, -1, -0x8000)

    def a_header() -> tuple:
        if random_.random() < 0.5:
            # one that can be
            return random_.choice((1, 16, 0x1000)), random_.choice((0, 5, 0x80)), random_.choice((0, 5, 0x80))
        return random_.choice(sizes) & 0xFFFF, random_.choice(sizes), random_.choice(sizes)

    def preferences() -> bytes:
        data = bytearray(random_.randbytes(PREFERENCES_SIZE))
        data[PREFERENCES_DIRTY] = random_.choice((0, 1))
        struct.pack_into("<I", data, PREFERENCES_FONTS, random_.choice((NONE, 0, 3)))
        return bytes(data)

    def externals_of(buffer: Buffer, waits: int, flag: int, character_at: int = 0) -> dict:
        """What the game waits for is read when its task is hurried, or after
        so many times that the game stands by."""

        def where(harness):
            return buffer.address + character_at + flag

        def switch(harness, arguments):
            if calls_so_far(harness, "SwitchToThread") >= waits - 1:
                harness.mu.mem_write(where(harness), b"\x01")
            return 0

        def hurry(harness, arguments):
            if waits == 0:
                harness.mu.mem_write(where(harness), b"\x01")
            return 1

        return {
            "font_get": lambda harness, arguments, there=random_.random() < 0.8: there,
            "async_read_position": read,
            "async_task_set_priority": hurry,
            "SwitchToThread": switch,
            "global_preferences_flush_blocking": lambda harness, arguments: 0,
        }

    for n in range(120):
        for name in ("font_character_read_location", "font_character_read_header"):
            data = Buffer(bytes(a_character(random_.randrange(5))))
            t.check(
                name,
                data,
                random_.choice((0, 0, 1, 0x80, 0x100, 0xFF01)),
                globals={"font_slots": fonts()},
                externals=externals_of(data, 0, 0),
            )
    for n in range(200):
        t.check(
            "font_character_header_validate",
            Buffer(struct.pack("<2sH2h8s", random_.randbytes(2), *a_header(), random_.randbytes(8)) + random_.randbytes(4)),
            globals={"global_preferences_globals": preferences()},
            externals={"global_preferences_flush_blocking": lambda harness, arguments: 0},
        )
    for n in range(200):
        for name, flag in (("font_character_location_wait", LOCATION_LOADED), ("font_character_header_wait", HEADER_LOADED)):
            loaded = random_.choice((0, 0, 0, 1, 0x80))
            data = Buffer(bytes(a_character(random_.randrange(5), loaded, loaded, a_header())))
            t.check(
                name,
                data,
                random_.choice((0, 0, 1, 1, 0x80, 0x100, 0xFF01)),
                globals={"font_slots": fonts(), "global_preferences_globals": preferences()},
                externals=externals_of(data, random_.choice((0, 0, 1, 2)), flag),
            )
    for n in range(400):
        index = random_.randrange(CHARACTERS)
        state = random_.choice((0, 1, 2, 3, 4, 5, 9))
        loaded = random_.choice((0, 0, 1))
        blob = bytearray(random_.randbytes(DATA_SIZE + CHARACTERS * CHARACTER))
        struct.pack_into("<I", blob, 0x44, DATA_SIZE)
        place = DATA_SIZE + index * CHARACTER
        blob[place : place + CHARACTER] = a_character(state, loaded, loaded, a_header())
        data = Buffer(bytes(blob), pointers=(0x44,))
        t.check(
            "font_character_update",
            index | random_.choice((0, 0, 0x10000, 0xE1740000)),
            random_.choice((0, 0, 1, 1, 0x80, 0x100, 0xFF01)),
            globals={"font_cache_globals+4": data, "font_slots": fonts(), "global_preferences_globals": preferences()},
            externals=externals_of(data, random_.choice((0, 0, 1, 2)), LOCATION_LOADED if state == 1 else HEADER_LOADED, place),
        )

    # ---------- asking for a character and for its pixels

    LRUV_TICK = 0x38
    LRUV_BLOCK, LRUV_BLOCK_FIRST_PAGE, LRUV_BLOCK_LAST_USED = 0x18, 8, 0x14
    BLOCK_LOADED, BLOCK_CHARACTER, BLOCK_TASK = 2, 4, 8
    PIXELS_SIZE, PIXELS_OFFSET = 0x1E, 0x28

    def a_cache() -> Buffer:
        """The cache, the array of its blocks and the blocks, in a buffer."""
        data_place = LRUV_SIZE
        blocks_place = LRUV_SIZE + DATA_SIZE
        blob = bytearray(random_.randbytes(blocks_place + BLOCKS * LRUV_BLOCK))
        struct.pack_into("<I", blob, LRUV_PAGE_SIZE_BITS, random_.choice((0, 4, 4, 12)))
        struct.pack_into("<I", blob, LRUV_BLOCKS, data_place)
        struct.pack_into("<I", blob, data_place + 0x44, blocks_place)
        return Buffer(bytes(blob), pointers=(LRUV_BLOCKS, data_place + 0x44))

    def some_blocks(values: dict) -> Buffer:
        blob = bytearray(random_.randbytes(DATA_SIZE + BLOCKS * BLOCK_SIZE_))
        struct.pack_into("<I", blob, 0x44, DATA_SIZE)
        for index, loaded in values.items():
            blob[DATA_SIZE + index * BLOCK_SIZE_ + BLOCK_LOADED] = loaded
        return Buffer(bytes(blob), pointers=(0x44,))

    def block_loaded(blocks: Buffer, index: int, waits: int, name: str = "SwitchToThread"):
        """The pixels of a block are read after the game stands by so many times."""

        def handler(harness, arguments):
            if calls_so_far(harness, name) >= waits - 1:
                harness.mu.mem_write(blocks.address + DATA_SIZE + index * BLOCK_SIZE_ + BLOCK_LOADED, b"\x01")
            return 0

        return handler

    for n in range(40):
        t.check(
            "font_cache_read",
            random_.randrange(FONT_SLOTS),
            0x80500000 + random_.randrange(0x100),
            0x80600000 + random_.randrange(0x100),
            random_.getrandbits(16),
            random_.getrandbits(32),
            random_.choice((2, 6)),
            random_.choice((0, 0x80700000)),
            globals={"font_slots": fonts()},
            externals={
                "font_get": lambda harness, arguments, there=n % 4 != 3: there,
                "async_read_position": lambda harness, arguments: 0x00CD0000 | arguments[1],
            },
        )

    for n in range(200):
        index = random_.randrange(BLOCKS)
        loaded = random_.choice((0, 0, 0, 1, 0x80))
        waits = random_.choice((0, 0, 1, 2))
        blocks = some_blocks({index: loaded})

        def hurry(harness, arguments, blocks=blocks, index=index, waits=waits):
            if waits == 0:
                harness.mu.mem_write(blocks.address + DATA_SIZE + index * BLOCK_SIZE_ + BLOCK_LOADED, b"\x01")
            return 1

        t.check(
            "font_cache_block_wait",
            NONE if n % 13 == 12 else index | random_.choice((0, 0, 0x10000, 0xE1740000)),
            random_.choice((0, 0, 1, 1, 0x80, 0x100, 0xFF01)),
            globals={
                "font_cache_globals+12": blocks,
                "font_cache_globals+16": a_cache(),
                "font_cache_globals+20": random_.choice((0, 0x80400000)),
            },
            externals={"async_task_set_priority": hurry, "SwitchToThread": block_loaded(blocks, index, waits)},
        )

    def a_known(index: int, state: int, block: int) -> Buffer:
        blob = bytearray(random_.randbytes(DATA_SIZE + CHARACTERS * CHARACTER))
        struct.pack_into("<I", blob, 0x44, DATA_SIZE)
        place = DATA_SIZE + index * CHARACTER
        struct.pack_into("<I", blob, place + 4, random_.randrange(FONT_SLOTS))
        struct.pack_into("<I", blob, place + STATE, state)
        struct.pack_into("<I", blob, place + BLOCK, block)
        return Buffer(bytes(blob), pointers=(0x44,))

    for n in range(300):
        index = random_.randrange(CHARACTERS)
        state = random_.choice((4, 4, 4, 4, 0, 1, 3, 5))
        block = random_.choice((NONE, NONE, NONE, 3))
        new_block = random_.randrange(BLOCKS) | random_.choice((0, 0x10000, 0xE1740000))
        fits = n % 7 != 6
        made = n % 11 != 10
        waits = random_.choice((0, 1, 2))
        blocks = some_blocks({new_block & 0xFFFF: 0})

        def read_pixels(harness, arguments, blocks=blocks, new_block=new_block, waits=waits):
            slot, done, buffer, size, offset, priority, bytes_read = arguments
            if waits == 0:
                harness.mu.mem_write(done, b"\x01")
            return 0x00EF0000 | size, (slot, harness.normalize(done), buffer, size, offset, priority, bytes_read)

        t.check(
            "font_character_pixels_new",
            index | random_.choice((0, 0, 0x10000, 0xE1740000)),
            random_.choice((0, 0, 1, 1, 0x80, 0x100, 0xFF01)),
            globals={
                "font_cache_globals+4": a_known(index, state, block),
                "font_cache_globals+12": blocks,
                "font_cache_globals+16": 0x80222200,
            },
            externals={
                "lruv_block_new": lambda harness, arguments, value=new_block if fits else NONE: value,
                "datum_new_at_index": lambda harness, arguments, made=made: (arguments[1] if made else NONE, (arguments[1],)),
                "font_cache_block_get_address": lambda harness, arguments: 0x80401230,
                "font_cache_read": read_pixels,
                "SwitchToThread": block_loaded(blocks, new_block & 0xFFFF, waits),
            },
        )

    for n in range(200):
        index = random_.randrange(CHARACTERS)
        block = random_.choice((NONE, NONE, 3, 3 | 0xE1740000))
        new_block = random_.choice((NONE, 5, 5 | 0x10000))
        known = a_known(index, random_.choice((4, 4, 1)), block)
        there = random_.choice((0, 0x80401230, 0x80401230))

        def new_pixels(harness, arguments, known=known, index=index, new_block=new_block):
            character_index, wait = arguments
            harness.mu.mem_write(known.address + DATA_SIZE + index * CHARACTER + BLOCK, struct.pack("<I", new_block))
            return new_block != NONE, (character_index, wait & 0xFF)

        t.check(
            "font_character_pixels_request",
            NONE if n % 17 == 16 else index | random_.choice((0, 0, 0x10000, 0xE1740000)),
            random_.choice((0, 1, 2, 3, 0xFE, 0xFF, 0x100, 0x12345601)),
            globals={
                "font_cache_globals+4": known,
                "font_cache_globals+16": a_cache(),
            },
            externals={
                "font_character_pixels_new": new_pixels,
                "font_cache_block_wait": lambda harness, arguments, there=there: (there, (arguments[0], arguments[1] & 0xFF)),
            },
        )

    for n in range(300):
        index = random_.randrange(CHARACTERS)
        state = random_.choice((4, 4, 0, 1, 2, 3))
        known = a_known(index, state, NONE)
        found = random_.choice((index, index, index | 0xE1740000, NONE))
        made = random_.choice((index, index | 0x10000, NONE))
        steps = random_.choice((1, 1, 2, 4))
        fonts_ = [random_.randrange(FONT_SLOTS) for _ in range(11)]

        def update(harness, arguments, known=known, index=index, steps=steps):
            character_index, wait = arguments
            ready = calls_so_far(harness, "font_character_update") >= steps - 1
            if ready:
                harness.mu.mem_write(known.address + DATA_SIZE + index * CHARACTER + STATE, struct.pack("<I", 4))
            return ready, (character_index, wait & 0xFF)

        t.check(
            "font_character_request",
            random_.randrange(11),
            random_.choice((0x41, 0x3042, random_.getrandbits(32))),
            random_.choice((0, 1, 2, 3, 3, 0xFFFFFFFC, 0xFFFFFFFF, 0x12345602)),
            globals={
                "font_cache_globals+4": known,
                "font_cache_globals+24": random_.getrandbits(32),
                "font_slot_indices": struct.pack("<11I", *fonts_),
            },
            externals={
                "font_character_find": lambda harness, arguments, found=found: found,
                "font_character_new": lambda harness, arguments, made=made: made,
                "font_character_update": update,
            },
        )

    for n in range(200):
        index = random_.randrange(CHARACTERS)
        known = a_known(index, random_.choice((4, 4, 4, 3, 0)), NONE)
        asked = random_.choice((index, index, index | 0xE1740000, NONE))
        t.check(
            "font_character_get_header",
            random_.choice((0x41, 0x3042, random_.getrandbits(32))),
            random_.randrange(11),
            random_.choice((0, 1, 2, 3, 0x12345603)),
            globals={"font_cache_globals+4": known},
            externals={
                "font_character_request": lambda harness, arguments, asked=asked: asked,
                "font_character_pixels_request": lambda harness, arguments, there=random_.choice((0, 1, 1, 0x80)): there,
            },
        )

    # ---------- the characters that are waited for

    PREDICTED, PREDICTED_SIZE_ = 8, 0xC
    DATA_ELEMENT_SIZE, DATA_FIRST_UNALLOCATED, DATA_BITS = 0x24, 0x38, 0x48
    LRUV_FAILED = 0x44

    def waited_for(entries: dict, first_unallocated: int | None = None) -> Buffer:
        """The array of the characters that are waited for: each has an
        identifier, a font and a character. After the array are its things,
        and after them a bit for each that is in use."""
        things = DATA_SIZE
        bits = things + PREDICTED * PREDICTED_SIZE_
        blob = bytearray(random_.randbytes(bits + 4))
        struct.pack_into("<I", blob, DATA_ELEMENT_SIZE, PREDICTED_SIZE_)
        struct.pack_into("<i", blob, DATA_FIRST_UNALLOCATED, max(entries, default=-1) + 1 if first_unallocated is None else first_unallocated)
        struct.pack_into("<2I", blob, 0x44, things, bits)
        in_use = 0
        for index, (identifier, font, character) in entries.items():
            struct.pack_into("<h2x2I", blob, things + index * PREDICTED_SIZE_, identifier, font, character)
            in_use |= 1 << index
        # bits of things past the last one are not looked at
        struct.pack_into("<I", blob, bits, in_use | random_.getrandbits(32) << PREDICTED & 0xFFFFFFFF)
        return Buffer(bytes(blob), pointers=(0x44, 0x48))

    def some_waited(count: int) -> dict:
        return {
            index: (random_.choice((1, 0x7FFF, -0x8000, -2)), random_.randrange(11), random_.choice((0x41, 0x42, 0x3042)))
            for index in random_.sample(range(PREDICTED), count)
        }

    for n in range(200):
        entries = some_waited(random_.randrange(0, PREDICTED + 1))
        order = sorted(entries)
        # what there is of each when it is asked for again: nothing, the
        # character with no pixels, the character not ready, or all of it
        there = {index: random_.choice(("none", "no pixels", "not ready", "all", "all")) for index in order}
        blob = bytearray(random_.randbytes(DATA_SIZE + CHARACTERS * CHARACTER))
        struct.pack_into("<I", blob, 0x44, DATA_SIZE)
        answers = []
        for position, index in enumerate(order):
            character_index = position % CHARACTERS
            answers.append(NONE if there[index] == "none" else character_index | random_.choice((0, 0x10000)))
        for character_index in range(CHARACTERS):
            # a character is ready, but for those that are asked for and are not
            states = [there[index] for position, index in enumerate(order) if position % CHARACTERS == character_index]
            struct.pack_into("<I", blob, DATA_SIZE + character_index * CHARACTER + STATE, 3 if "not ready" in states else 4)
        pixels = [there[index] != "no pixels" for index in order if there[index] != "none"]
        cache = bytearray(a_cache().data)
        struct.pack_into("<I", cache, LRUV_TICK, random_.choice((0, 5, 0x7FFFFFFE, 0x7FFFFFFF)))
        data_place = LRUV_SIZE
        t.check(
            "font_cache_frame",
            globals={
                "font_cache_globals+4": Buffer(bytes(blob), pointers=(0x44,)),
                "font_cache_globals+8": waited_for(entries, random_.choice((None, None, PREDICTED))),
                "font_cache_globals+16": Buffer(bytes(cache), pointers=(LRUV_BLOCKS, data_place + 0x44)),
                "font_cache_globals+24": random_.choice((0, 5, 0x7FFFFFFF, 0xFFFFFFFF)),
            },
            externals={
                "lruv_wrap_ticks": lambda harness, arguments: 0,
                "font_character_request": lambda harness, arguments, answers=answers: (answers[calls_so_far(harness, "font_character_request")], tuple(arguments)),
                "font_character_pixels_request": lambda harness, arguments, pixels=pixels: (pixels[calls_so_far(harness, "font_character_pixels_request")], tuple(arguments)),
                "datum_delete": lambda harness, arguments: (0, (arguments[1],)),
            },
        )

    for n in range(200):
        entries = some_waited(random_.randrange(0, PREDICTED + 1))
        font, character = random_.randrange(11), random_.choice((0x41, 0x42, 0x3042))
        if entries and n % 3 == 0:
            # one of those that are waited for
            _identifier, font, character = entries[random_.choice(list(entries))]
        elif entries and n % 3 == 1:
            # the same character of another font, or another of the same font
            _identifier, font, character = entries[random_.choice(list(entries))]
            if random_.getrandbits(1):
                font ^= 0x10
            else:
                character ^= 0x100
        free = [index for index in range(PREDICTED) if index not in entries]
        fresh = random_.choice(free) | random_.choice((0, 0x10000, 0xE1740000)) if free and n % 7 != 6 else NONE
        t.check(
            "font_character_precache",
            font,
            character,
            globals={"font_cache_globals+8": waited_for(entries, random_.choice((None, None, PREDICTED)))},
            externals={
                "font_character_get_header": lambda harness, arguments, there=random_.choice((0, 0, 0, 0x80123400)): there,
                "datum_new": lambda harness, arguments, fresh=fresh: (fresh, ()),
            },
        )
