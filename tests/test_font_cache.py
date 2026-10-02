# source/text/font_cache.c

import random
import struct

from difftest import Buffer, Mock, Pointer, mock_address

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
