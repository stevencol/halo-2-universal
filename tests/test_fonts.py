# source/text/fonts.c

import random
import struct

from difftest import Buffer, GlobalData
from test_async import calls_so_far

NONE = 0xFFFFFFFF
FONT_SIZE = 0x1D0
SLOTS = 10
LOADED, LOADING, TASK = 0x1C8, 0x1C9, 0x1CC
PAIR_COUNT, PAIRS, CHARACTERS = 0x20, 0x24, 0x1A4
MAXIMUM_PAIRS = 96


def run(t) -> None:
    random_ = random.Random(82)

    def slots() -> bytearray:
        return bytearray(random_.randbytes(SLOTS * FONT_SIZE))

    def loaded_after(waits: int, slot: int):
        """SwitchToThread: the font is loaded after so many."""

        def handler(harness, arguments):
            if calls_so_far(harness, "SwitchToThread") >= waits - 1:
                harness.mu.mem_write(harness.global_addresses["font_slots"] + slot * FONT_SIZE + LOADED, b"\x01")
            return 0

        return handler

    def hurried(becomes_loaded: bool, slot: int):
        def handler(harness, arguments):
            if becomes_loaded:
                harness.mu.mem_write(harness.global_addresses["font_slots"] + slot * FONT_SIZE + LOADED, b"\x01")
            return 1

        return handler

    # ---------- the font of a place

    for n in range(160):
        data = slots()
        slot = random_.choice((0, 1, 5, 9, 9, 10, 11, -1, -5, 0x7FFFFFFF, 0x80000000)) & 0xFFFFFFFF if n % 4 == 0 else random_.randrange(SLOTS)
        at = slot if slot < SLOTS else 0
        loaded, loading = random_.choice(((1, 0), (1, 1), (0x80, 0), (0, 1), (0, 1), (0, 0x80), (0, 0)))
        data[at * FONT_SIZE + LOADED] = loaded
        data[at * FONT_SIZE + LOADING] = loading
        waits = random_.choice((0, 1, 2, 3))
        t.check(
            "font_get",
            slot,
            globals={"font_slots": GlobalData(bytes(data))},
            externals={
                "async_task_set_priority": hurried(waits == 0, at),
                "SwitchToThread": loaded_after(waits, at),
            },
        )

    # ---------- how high its lines are

    for n in range(60):
        data = slots()
        slot = random_.randrange(SLOTS)
        indices = [random_.randrange(SLOTS) for _ in range(11)]
        font_index = random_.randrange(11)
        slot = indices[font_index]
        there = n % 5 != 4
        data[slot * FONT_SIZE + LOADED] = there
        data[slot * FONT_SIZE + LOADING] = 0
        struct.pack_into("<3h", data, slot * FONT_SIZE + 4, *(random_.choice((0, 1, 12, 300, -3, 0x7FFF, -0x8000)) for _ in range(3)))
        t.check(
            "font_get_line_height",
            font_index,
            globals={"font_slots": bytes(data), "font_slot_indices": struct.pack("<11I", *indices)},
        )

    # ---------- how near two characters are

    def font(pairs: list, bits: set | None = None, ghost: tuple | None = None) -> Buffer:
        """A font with pairs of characters, in order, and what there is for each."""
        data = bytearray(random_.randbytes(FONT_SIZE))
        struct.pack_into("<i", data, PAIR_COUNT, len(pairs))
        data[CHARACTERS : CHARACTERS + 32] = bytes(32)
        for index, (first, second, offset) in enumerate(pairs):
            struct.pack_into("<2Bh", data, PAIRS + 4 * index, first, second, offset)
        if ghost:
            # what is after the pairs, which is not one of them
            struct.pack_into("<2Bh", data, PAIRS + 4 * len(pairs), *ghost)
        for first in {pair[0] for pair in pairs} if bits is None else bits:
            data[CHARACTERS + first // 8] |= 1 << (first % 8)
        return Buffer(bytes(data))

    for n in range(120):
        firsts = sorted(random_.sample(range(1, 256), random_.choice((1, 2, 5, 12))))
        pairs = []
        for first in firsts:
            for second in sorted(random_.sample(range(1, 256), random_.choice((1, 1, 2, 6)))):
                if len(pairs) < MAXIMUM_PAIRS - 1:
                    pairs.append((first, second, random_.randrange(-0x8000, 0x8000)))
        asked = [random_.choice(pairs)[:2] for _ in range(4)]
        # a first character that has no pair, one that has it with another
        # second, and characters that are none or of more than 8 bits
        asked += [(random_.randrange(256), random_.randrange(256)), (pairs[0][0], pairs[0][1] + 1), (pairs[-1][0], 255), (pairs[-1][0], 1)]
        asked += [(0, pairs[0][1]), (pairs[0][0], 0), (pairs[0][0] + 0x100, pairs[0][1]), (pairs[0][0], pairs[0][1] + 0x100), (0xFFFFFFFF, 1), (1, 0xFFFFFFFF)]
        for first, second in asked:
            t.check("font_get_kerning", font(pairs), first, second)
        # the bit of a character that is the first of no pair, and of one
        # that is after all those that are: the game goes past the pairs
        missing = random_.choice([first for first in range(1, 255) if first not in firsts and first < firsts[-1]] or [firsts[-1]])
        t.check("font_get_kerning", font(pairs, {*firsts, missing}), missing, pairs[0][1])
    for first, second in ((5, 5), (5, 6), (4, 5), (255, 255)):
        t.check("font_get_kerning", 0, first, second)
        # a font with no pairs, and a bit: the first place is looked at all the same
        t.check("font_get_kerning", font([], {first}), first, second)

    # the characters that are none can be in a pair all the same: they have
    # no kerning
    t.check("font_get_kerning", font([(0, 5, 77), (3, 4, 9)], {0, 3}), 0, 5)
    t.check("font_get_kerning", font([(3, 0, 55), (3, 4, 9)]), 3, 0)
    # a pair of a character whose bit is not set is not looked for
    for _ in range(6):
        pairs = [(first, second, random_.randrange(1, 100)) for first in (7, 40, 200) for second in (3, 90)]
        first, second, _offset = random_.choice(pairs)
        t.check("font_get_kerning", font(pairs, set()), first, second)
        t.check("font_get_kerning", font(pairs, {first ^ 1, first + 32 & 0xFF}), first, second)
    # after the pairs there is something that looks like the pair that is
    # asked for: it is not one of them
    for _ in range(10):
        pairs = [(first, second, random_.randrange(1, 100)) for first in (7, 40, 200) for second in (3, 90)]
        # a first character after all those that there are
        t.check("font_get_kerning", font(pairs, {7, 40, 200, 201}, ghost=(201, 3, 0x1234)), 201, 3)
        # the last first character, with a second one after all of its own
        t.check("font_get_kerning", font(pairs, ghost=(200, 91, 0x1234)), 200, 91)
        t.check("font_get_kerning", font(pairs, ghost=(200, 95, 0x1234)), 200, 91)
        # no pairs at all
        t.check("font_get_kerning", font([], {9}, ghost=(9, 4, 0x1234)), 9, 4)
        t.check("font_get_kerning", font([], {9}, ghost=(8, 4, 0x1234)), 9, 4)
