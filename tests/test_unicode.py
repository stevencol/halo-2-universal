# source/text/unicode.c

import random
import struct

from difftest import Buffer


def wide(text: list[int], room: int, random_: random.Random) -> Buffer:
    """A wide string in room for `room` characters; after its zero there is anything."""
    characters = (text + [0])[:room]
    characters += [random_.randrange(1, 0x10000) for _ in range(room - len(characters))]
    return Buffer(struct.pack(f"<{room}H", *characters))


def some_text(random_: random.Random, length: int) -> list[int]:
    """Characters that are not zero: letters, the accented ones the game knows, and any other."""
    return [
        random_.choice((random_.randrange(0x20, 0x7F), random_.randrange(0x61, 0x7B), 0xE9, 0xF3, 0x80, 0x25A1))
        for _ in range(length)
    ]


def run(t) -> None:
    random_ = random.Random(9)

    for _ in range(150):
        length = random_.choice((0, 1, 2, 5, 12))
        text = some_text(random_, length)
        other = some_text(random_, random_.choice((0, 1, 3, 9)))
        room = 24

        for count in (1, 2, length, length + 1, length + 2, room):
            if count < 1:
                continue
            t.check("ustrnzcpy", Buffer(random_.randbytes(room * 2)), wide(text, room, random_), count)
            t.check("wchar_string_to_ascii_string", wide(text, room, random_), Buffer(random_.randbytes(room)), count)
            ascii_text = bytes(character & 0xFF or 1 for character in text) + b"\0"
            t.check("ascii_string_to_wchar_string", Buffer(ascii_text), Buffer(random_.randbytes(room * 2)), count)
        for count in (0, -1, -5):
            t.check("wchar_string_to_ascii_string", wide(text, room, random_), Buffer(random_.randbytes(room)), count)
            t.check("ascii_string_to_wchar_string", Buffer(b"abc\0"), Buffer(random_.randbytes(room * 2)), count)

        # the count of ustrnzcat is of the whole room of the destination
        for count in (length + 1, length + 2, length + 1 + len(other), room):
            t.check("ustrnzcat", wide(text, room, random_), wide(other, room, random_), count)
        for count in (-1, 0, 1, length - 1, length, room):
            t.check("ustrnupr", wide(text, room, random_), count)

    # ranges in order, with gaps between them
    for _ in range(40):
        range_count = random_.choice((0, 1, 2, 5, 16))
        edges = sorted(random_.sample(range(0x10000), range_count * 2))
        ranges = Buffer(struct.pack(f"<{range_count * 2}H", *edges) + bytes(4))
        characters = [0, 0xFFFF, *random_.sample(range(0x10000), 10)]
        for edge in edges[:8]:
            characters += [edge - 1, edge, edge + 1]
        for character in characters:
            t.check("character_in_ranges", character & 0xFFFF, range_count, ranges)

    for edge in (0x1100, 0x11FF, 0x3000, 0xD7AF, 0xF900, 0xFAFF, 0xFF00, 0xFFDC):
        for character in (edge - 1, edge, edge + 1):
            t.check("character_is_east_asian", character)
    for character in (0, 0x41, 0x10000, -1, 0x7FFFFFFF):
        t.check("character_is_east_asian", character)

    # ---------- formatted text

    def wide_text(value: str) -> Buffer:
        return Buffer(value.encode("utf-16-le") + bytes(2))

    # no %c or %s: the C library of the tests takes the format as one of
    # characters of 8 bits
    for format_ in ("Halo%04d", "%d", "x%5dy", "", "%-6d|", "%x and %X", "no number"):
        for count in (0x80, 9, 5, 2, 1):
            for value in (0, 1, 42, 4096, 65, 0x7FFFFFFF, 0xFFFFFFFF):
                t.check("usnzprintf", Buffer(random_.randbytes(0x108)), count, wide_text(format_), value)

    # with no end if the text does not fit
    for format_ in ("Halo%04d", "%d", "x%5dy", "", "%x and %X", "no number"):
        for count in (0x80, 9, 8, 5, 2, 1, 0):
            for value in (0, 42, 4096, 0xFFFFFFFF):
                t.check("usnprintf", count, Buffer(random_.randbytes(0x108)), wide_text(format_), value)

    # ---------- what a letter after a bar stands for

    for character in (*"|lrcnt", *"LRCNT", "a", "m", "\\", "/", "}", "{", "0"):
        t.check("unicode_escape_character", ord(character), Buffer(random_.randbytes(4)), externals={"GetTickCount": lambda harness, arguments: 77})
    for character in (0, 0x10000, 0x1007C, 0x1006C, 0xFFFF, 0xE405, 9, 0xD, 0x17C, 0x7C00):
        t.check("unicode_escape_character", character, Buffer(random_.randbytes(4)), externals={"GetTickCount": lambda harness, arguments: 77})
    # a letter that is none is noted, at most once a minute
    for character in (ord("x"), ord("l"), 0):
        for time, noted in ((1000, 999), (1000, 1000), (1000, 1001), (0xFFFFFFF0, 5), (5, 0xFFFFFFF0), (0, 0), (0xFFFF3CB0, 0)):
            t.check(
                "unicode_escape_character",
                character,
                Buffer(random_.randbytes(4)),
                globals={"unicode_escape_note_time": noted},
                externals={"GetTickCount": lambda harness, arguments, time=time: time},
            )

    # ---------- where a line can end

    starts = (
        0x21, 0x22, 0x25, 0x29, 0x2C, 0x2E, 0x3A, 0x3B, 0x3F, 0x5D, 0x7D, 0xB0, 0xB7,
        0x2013, 0x2014, 0x2019, 0x201D, 0x2022, 0x2026, 0x2027, 0x2032, 0x2033, 0x2103,
        0x3001, 0x3002, 0x3009, 0x300B, 0x300D, 0x300F, 0x3011, 0x3015, 0x301E,
        0x3041, 0x3043, 0x3045, 0x3047, 0x3049, 0x3063, 0x3083, 0x3085, 0x3087, 0x308E,
        0x30A1, 0x30A3, 0x30A5, 0x30A7, 0x30A9, 0x30C3, 0x30E3, 0x30E5, 0x30E7, 0x30EE, 0x30FC,
        0xFE30, 0xFE50, 0xFE51, 0xFE52, 0xFE54, 0xFE55, 0xFE56, 0xFE57, 0xFE5A, 0xFE5C, 0xFE5E,
        0xFF01, 0xFF05, 0xFF09, 0xFF0C, 0xFF0E, 0xFF1A, 0xFF1B, 0xFF1F, 0xFF3D, 0xFF5D,
        0xFF61, 0xFF64, 0xFF70, 0xFF9E, 0xFF9F, 0xFFE0,
    )
    ends = (
        0x22, 0x24, 0x28, 0x5B, 0x5C, 0x7B, 0x2018, 0x201C, 0x2035,
        0x3008, 0x300A, 0x300C, 0x300E, 0x3010, 0x3014, 0x301D,
        0xFE59, 0xFE5B, 0xFE5D, 0xFF04, 0xFF08, 0xFF3B, 0xFF5B, 0xFFE1, 0xFFE5, 0xFFE6,
    )
    # every character near one that the game tells apart, and some of all
    # the others; with more than 16 bits too
    near = sorted({character + step for character in (*starts, *ends) for step in range(-4, 5) if character + step >= 0})
    spread = list(range(0, 0x10000, 61))
    wide_ones = [0x10000 | character for character in (0x21, 0x22, 0x3001, 0xFF04)] + [0xFFFFFFFF, 0x80000021, 0x10000, 0x12345678]
    for character in (*near, *spread, *wide_ones):
        t.check("character_cannot_start_line", character)
        t.check("character_cannot_end_line", character)

    spaces = (9, 0xD, 0x20, 0x85, 0xA0, 0x1680, 0x180E, 0x2000, 0x200A, 0x2028, 0x2029, 0x202F, 0x205F, 0x3000)
    firsts = (0, 0x41, 0x61, *spaces, 8, 0xE, 0x1F, 0x21, 0x200B, 0x2FFF, 0x3001, 0x3042, 0xAC00, 0x1100, 0xFF21, 0x28, 0x5B, 0x300C, 0xFF08, 0x10020, 0x13042)
    seconds = (0, 0x41, 0x20, 0x21, 0x29, 0x3001, 0x3002, 0x3042, 0x3041, 0xAC00, 0xFF01, 0xFF21, 0x28, 0x10021, 0x13042)
    for character in firsts:
        for next_ in seconds:
            t.check("character_can_break_line", character, next_)
    for _ in range(300):
        t.check(
            "character_can_break_line",
            random_.choice((*spaces, *starts, *ends, 0x41, 0x3042, 0xAC00, random_.getrandbits(16))),
            random_.choice((*spaces, *starts, *ends, 0x41, 0x3042, 0xAC00, random_.getrandbits(16))),
        )
