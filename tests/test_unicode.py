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
