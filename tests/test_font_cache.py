# source/text/font_cache.c

import random
import struct

from difftest import Buffer, FunctionPointer, Mock, Pointer, mock_address

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
