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

    # ---------- the pixels of a character

    def coded(length: int) -> bytes:
        """Bytes of pixels as a font has them: runs, single pixels, pairs and
        new colours, of all kinds."""
        data = bytearray()
        while len(data) < length:
            kind = random_.randrange(6)
            if kind == 0:
                # a new colour, with its two bytes
                data += bytes([0]) + random_.randbytes(2)
            elif kind == 1:
                data.append(random_.choice((1, 2, 5, 0x3F, 0x20)))
            elif kind == 2:
                data.append(0x40 | random_.choice((0, 1, 2, 5, 0x3F)))
            elif kind == 3:
                data.append(0x80 | random_.getrandbits(6))
            elif kind == 4:
                data.append(0xC0 | random_.getrandbits(6))
            else:
                data.append(random_.getrandbits(8))
        return bytes(data)

    # every byte, alone and after a colour
    for code in range(256):
        for before in (b"", b"\x00\x12\x34"):
            data = before + bytes([code])
            t.check("font_character_decode_pixels", Buffer(data + random_.randbytes(4)), len(data), Buffer(random_.randbytes(0x100)))
    for n in range(300):
        length = random_.choice((0, 1, 2, 3, 8, 20, 40))
        data = coded(length)
        # a size that cuts a colour, and sizes that are not of anything
        size = random_.choice((len(data), len(data), len(data), max(len(data) - 1, 0), 0, -1 & 0xFFFFFFFF, 0x80000000))
        t.check(
            "font_character_decode_pixels",
            Buffer(data + random_.randbytes(4)),
            size,
            # with nowhere to write them, they are counted
            0 if n % 5 == 4 else Buffer(random_.randbytes(0x1500)),
        )

    # ---------- loading a font

    FONT_FILE, FONT_HEADER_SIZE = 0x1C4, 0x1C4
    TASK_SIZE, TASK_PARAMETERS, TASK_CATEGORY, TASK_FUNCTION, TASK_DONE = 0x40, 8, 0x30, 0x34, 0x38
    PREFERENCES_SIZE, PREFERENCES_FONTS = 0x408, 0x24
    SIGNATURE = 0xF0000001

    def text(harness, address: int) -> bytes:
        return bytes(harness.mu.mem_read(address, 0x200)).split(b"\0", 1)[0]

    def preferences() -> bytes:
        data = bytearray(random_.randbytes(PREFERENCES_SIZE))
        data[1] = random_.choice((0, 1))
        struct.pack_into("<I", data, PREFERENCES_FONTS, random_.choice((NONE, 0, 3)))
        return bytes(data)

    names = (b"conduit", b"", b"a name of a font that is longer than thirty-one", b"x" * 31, b"x" * 32)
    for n in range(120):
        slot = random_.randrange(SLOTS)
        data = slots()
        # what the task leaves in the place of the font, if the game waits
        signature = random_.choice((SIGNATURE, SIGNATURE, 0xF0000000, 0, random_.getrandbits(32)))
        waits = random_.choice((0, 1, 3))
        # the task: the two addresses that it gets are places of the game, or of source/
        task = Buffer(random_.randbytes(TASK_SIZE), places=(TASK_FUNCTION, TASK_DONE))

        def loaded(harness, arguments, slot=slot, signature=signature, waits=waits):
            if calls_so_far(harness, "SwitchToThread") >= waits - 1:
                place = harness.global_addresses["font_slots"] + slot * FONT_SIZE
                harness.mu.mem_write(place, struct.pack("<I", signature))
                harness.mu.mem_write(place + LOADED, b"\x01")
            return 0

        def queue(harness, arguments, slot=slot, signature=signature, waits=waits):
            if waits == 0:
                place = harness.global_addresses["font_slots"] + slot * FONT_SIZE
                harness.mu.mem_write(place, struct.pack("<I", signature))
                harness.mu.mem_write(place + LOADED, b"\x01")
            return 0x00120000 | slot, ()

        t.check(
            "font_load",
            slot,
            Buffer(random_.choice(names) + bytes(1)),
            random_.choice((0, 0, 1, 1, 0x80, 0x100, 0xFF01)),
            globals={
                "font_slots": GlobalData(bytes(data)),
                "async_globals+9628": random_.getrandbits(32),
                "global_preferences_globals": preferences(),
            },
            extra=[task],
            externals={
                "async_task_allocate": lambda harness, arguments, task=task: (task.address, ()),
                "async_task_queue": queue,
                "SwitchToThread": loaded,
                "global_preferences_flush_blocking": lambda harness, arguments: 0,
            },
        )

    for n in range(80):
        slot = random_.randrange(SLOTS)
        data = slots()
        opened = n % 2
        struct.pack_into("<I", data, slot * FONT_SIZE + FONT_FILE, 0x700 + slot if opened else NONE)
        header = random_.randbytes(FONT_HEADER_SIZE)
        handle = random_.choice((0x900, 0x900, NONE))

        def read(harness, arguments, header=header):
            file, buffer, size, bytes_read, overlapped = arguments
            harness.mu.mem_write(buffer, header[:size])
            return 1, (file, harness.normalize(buffer), size, overlapped)

        t.check(
            "font_load_work",
            Buffer(struct.pack("<2I", slot, random_.getrandbits(32)) + (random_.choice(names) + bytes(32))[:31] + bytes(1) + random_.randbytes(4)),
            globals={"font_slots": GlobalData(bytes(data))},
            externals={
                "CreateFileA": lambda harness, arguments, handle=handle: (handle, (text(harness, arguments[0]), *arguments[1:])),
                "SetFilePointer": lambda harness, arguments: arguments[1],
                "ReadFile": read,
            },
        )

    # ---------- the fonts that there are

    font_names = (b"conduit", b"fixedsys", b"handel_gothic", b"conduit", b"x", b"a" * 40, b"", b"terminal")
    separators = (b" ", b"\t", b"\r\n", b"\n", b"  \t ", b"\r")

    for n in range(300):
        words = [random_.choice(font_names[:-3] + font_names[-1:]) for _ in range(random_.choice((0, 1, 2, 4, 6, 11, 12, 15)))]
        content = random_.choice((b"", b" ", b"\r\n")) + b"".join(word + random_.choice(separators) for word in words)
        if n % 6 == 0:
            content = content.rstrip()
        if n % 25 == 24:
            # longer than what is read of it
            content = (b"conduit " * 300)[: random_.choice((0x7FE, 0x7FF, 0x800, 0x900))]
        size = random_.choice((len(content), len(content), len(content), max(len(content) - 3, 0), 0x900))
        waits = random_.choice((0, 0, 1, 3))
        read = random_.choice((1, 1, 1, 1, 0, 0x100))

        def start(harness, arguments, content=content, size=size, waits=waits, read=read):
            done, read_, size_, path, buffer, buffer_size, unknown7, unknown8 = arguments
            harness.mu.mem_write(buffer, content[:buffer_size] + bytes(1))
            harness.mu.mem_write(read_, bytes((read & 0xFF,)))
            harness.mu.mem_write(size_, struct.pack("<I", size))
            # the flag is cleared when the read starts, and set when it ends
            harness.mu.mem_write(done, bytes((0 if waits else 1,)))
            return 0, (text(harness, path), buffer_size, unknown7, unknown8)

        def wait(harness, arguments, waits=waits):
            if calls_so_far(harness, "SwitchToThread") >= waits - 1:
                # where the flag is: the first argument of the read
                harness.mu.mem_write(harness._fonts_done, b"\x01")
            return 0

        def remember(handler):
            def wrapped(harness, arguments):
                harness._fonts_done = arguments[0]
                return handler(harness, arguments)
            return wrapped

        def name_of_list(harness, arguments, name=random_.choice((b"fonts.txt", b"english.txt", b""))):
            buffer, size_ = arguments
            harness.mu.mem_write(buffer, name + bytes(1))
            return 0, (size_,)

        t.check(
            "fonts_initialize",
            globals={
                "font_slot_indices": random_.randbytes(44),
                "font_slots": random_.randbytes(SLOTS * FONT_SIZE),
                "fonts_initialized": bytes((random_.choice((0, 0, 0, 0, 1, 0x80)),)),
                "unknown_0051ea00": random_.randbytes(1),
                "main_globals+1": random_.randbytes(1),
                "unknown_004ed39e": random_.randbytes(1),
                "unknown_004ed39d": random_.randbytes(1),
                "unknown_004ed3a0": random_.randbytes(4),
                "unknown_00527330": bytes((random_.choice((0, 1, 0x80)),)),
                "unknown_00527334": struct.pack("<I", random_.choice((3, 8, 0, 4, 0x103))),
            },
            externals={
                "unknown_00121b00": lambda harness, arguments, right=random_.choice((1, 1, 0, 0x100)): right,
                "GetTickCount": lambda harness, arguments: 0x12345,
                "unknown_000593e0": lambda harness, arguments: 0,
                "unknown_00121570": name_of_list,
                "async_read_whole_file": remember(start),
                "SwitchToThread": wait,
                "font_load": lambda harness, arguments: (0, (arguments[0], text(harness, arguments[1]), arguments[2] & 0xFF)),
                "font_characters_new": lambda harness, arguments: 0,
                "font_cache_new": lambda harness, arguments: 0,
            },
        )

    # ---------- files of the game that are damaged (source/main/main.c)

    for n in range(40):
        t.check(
            "main_files_damaged",
            globals={
                "main_globals": random_.randbytes(0x54),
                "unknown_004ed39e": random_.randbytes(1),
                "unknown_004ed39d": random_.randbytes(1),
                "unknown_004ed3a0": random_.randbytes(4),
                "unknown_00527330": bytes((random_.choice((0, 1, 0x80)),)),
                "unknown_00527334": struct.pack("<I", random_.choice((3, 8, 0, 4, 0x103))),
            },
            externals={
                "GetTickCount": lambda harness, arguments: 0x12345,
                "unknown_000593e0": lambda harness, arguments: 0,
            },
        )
