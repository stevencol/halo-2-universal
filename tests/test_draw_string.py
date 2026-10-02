# source/text/draw_string.c

import random
import struct

from difftest import Buffer, f32

ITERATOR_SIZE = 0x2C
CHARACTERS, INDEX, CHARACTER, WRAP, ELEMENT, PREVIOUS_ELEMENT = 8, 0xC, 0x12, 0x1B, 0x1C, 0x20


def run(t) -> None:
    random_ = random.Random(77)

    # ---------- the pictures of the game

    for character in range(0xDFF0, 0xE410):
        t.check("character_is_private_picture", character)
    for character in (0, 1, 0x41, 0xE000 - 0x10000, 0xFFFFE000, 0xFFFFFFFF, 0x80000000, 0x7FFFFFFF, 0x1E010, 0x1E120, 0xF000):
        t.check("character_is_private_picture", character & 0xFFFFFFFF)

    # ---------- going through a text

    drawn = (0x41, 0x61, 0x20, 0x21, 0x28, 0x29, 0x3042, 0x3001, 0x300C, 0xAC00, 0xE100, 0x85, 0x1F, 0xE, 0xE404, 0xE408, 8, 0xB, 0xC, 1)
    special = (0, 0, 9, 0xA, 0xA, 0xA, 0xD, 0xD, 0xE405, 0xE406, 0xE407)

    def text() -> list[int]:
        characters = [random_.choice(drawn + special) for _ in range(random_.randrange(1, 12))]
        # something that is not a line feed is after them
        characters.append(random_.choice((0, 0x41, 0xD)))
        return characters

    for n in range(600):
        characters = text()
        # from any place of the text from which there is something that is
        # not a line feed
        index = random_.randrange(len(characters))
        iterator = bytearray(random_.randbytes(ITERATOR_SIZE))
        first = ITERATOR_SIZE
        if n % 9 == 8:
            # the index can be less than zero: the character before those
            # that the iterator has
            characters = [random_.choice(drawn), *text()]
            index, first = -1, ITERATOR_SIZE + 4
        struct.pack_into("<I", iterator, CHARACTERS, first)
        struct.pack_into("<h", iterator, INDEX, index)
        struct.pack_into("<I", iterator, CHARACTER, random_.choice((*drawn, 0, 0xD, 0x10041)))
        iterator[WRAP] = random_.choice((0, 1, 1, 0x80))
        struct.pack_into("<2I", iterator, ELEMENT, random_.choice((0, 1, 2, 3, 4, 4, 4, 5)), random_.choice((0, 1, 2, 3, 4, 4, 4, 5, 0x104)))
        t.check(
            "text_iterator_next",
            Buffer(bytes(iterator) + struct.pack(f"<{len(characters)}I", *characters), pointers=(CHARACTERS,)),
        )

    # ---------- starting to go through a text

    def color(*values: float) -> Buffer:
        return Buffer(struct.pack("<4f", *values) + random_.randbytes(4))

    parts = (0.0, 1.0, 0.5, 0.25, 1 / 255, 0.999, 0.9999999, 0.003, 0.0039, 0.00393, 254.5 / 255, 1.004, 2.0, -0.5, 100.0)

    def a_color() -> Buffer:
        if random_.random() < 0.7:
            return color(*(random_.choice(parts[:11]) for _ in range(4)))
        return color(*(random_.choice(parts) for _ in range(4)))

    for n in range(400):
        slots = [random_.getrandbits(32) for _ in range(11)]
        font = random_.choice((0, 0x80123400, 0x80123400))
        t.check(
            "text_iterator_new",
            Buffer(random_.randbytes(ITERATOR_SIZE + 4), places=(CHARACTERS,)),
            random_.randrange(11),
            random_.getrandbits(32),
            Buffer(random_.randbytes(16)),
            random_.getrandbits(32),
            a_color(),
            Buffer(bytes([random_.choice((0, 1, 1, 0x80))]) + random_.randbytes(3)),
            a_color(),
            globals={
                "draw_string_globals+4": bytes([random_.choice((0, 1, 2, 3, 0xFE, 0xFF))]),
                "font_slot_indices": struct.pack("<11I", *slots),
            },
            externals={"unknown_001224c0": lambda harness, arguments, font=font: font},
        )

    # ---------- how the texts are drawn

    GLOBALS_SIZE = 0x60

    def how(font_index: int | None = None) -> bytes:
        """How the texts are drawn: with colours that are colours."""
        data = bytearray(random_.randbytes(GLOBALS_SIZE))
        if font_index is not None:
            struct.pack_into("<I", data, 0, font_index)
        struct.pack_into("<4f", data, 0x10, *(random_.choice(parts[:11]) for _ in range(4)))
        data[0x20] = random_.choice((0, 1, 1))
        struct.pack_into("<4f", data, 0x24, *(random_.choice(parts[:11]) for _ in range(4)))
        return bytes(data)

    def fonts() -> bytes:
        return struct.pack("<11I", *(random_.getrandbits(32) for _ in range(11)))

    for count in (-0x8000, -1, 0, 1, 2, 5, 15, 16, 17, 100, 0x7FFF):
        for _ in range(3):
            t.check(
                "draw_string_set_tab_stops",
                count & 0xFFFFFFFF,
                Buffer(random_.randbytes(0x30)),
                globals={"draw_string_globals": how()},
            )
    for n in range(12):
        t.check("draw_string_set_shadow_color", color(0.25, 0.5, 0.75, 1.0) if n % 3 else 0, globals={"draw_string_globals": how()})
    for font_index in range(11):
        t.check(
            "draw_string_set_font",
            font_index,
            globals={"draw_string_globals": how(), "font_slot_indices": fonts()},
            externals={"unknown_001224c0": lambda harness, arguments: 0x80123400},
        )
    for n in range(60):
        t.check(
            "draw_string_set_font_and_format",
            random_.randrange(11),
            a_color() if n % 3 else 0,
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            a_color(),
            globals={"draw_string_globals": how(), "font_slot_indices": fonts()},
            externals={
                "unknown_001224c0": lambda harness, arguments: 0x80123400,
                "draw_string_set_color": lambda harness, arguments: (0, (bytes(harness.mu.mem_read(arguments[0], 16)),)),
            },
        )

    # ---------- getting the characters of a text ready

    def ready(harness, arguments):
        font_index, character = arguments
        # some characters cannot be got ready
        return character % 5 != 3

    for n in range(200):
        characters = [random_.choice(drawn + special[2:]) for _ in range(random_.randrange(0, 14))] + [0, 0x41, 0]
        font_index = random_.randrange(11)
        t.check(
            "draw_string_precache_characters",
            Buffer(struct.pack(f"<{len(characters)}I", *characters)),
            font_index,
            globals={"draw_string_globals": how(), "font_slot_indices": fonts(), "unicode_escape_note_time": 0},
            externals={
                "unknown_001224c0": lambda harness, arguments, font=random_.choice((0, 0x80123400, 0x80123400, 0x80123400)): font,
                "unknown_001414d0": ready,
                "GetTickCount": lambda harness, arguments: 77,
            },
        )

    def wide(text_: str) -> Buffer:
        return Buffer(text_.encode("utf-16-le") + bytes(2))

    texts = [
        "", "a", "plain text", "one|ntwo", "||", "|", "a|", "|x unknown", "|l left |r right |c centre |t tab", "|n|n|n", "bar | space",
        "\u3042\u3044 |n \u3046", "a\rb\nc\td", "|N upper", "x" * 0x7FE, "x" * 0x7FF, "x" * 0x800, "x" * 0x900,
        "x" * 0x7FD + "|n", "x" * 0x7FE + "|n", "x" * 0x7FE + "||y", "|n" * 0x500,
    ]
    for text_ in texts:
        for name in ("draw_string_precache_string", "draw_string_precache"):
            font_index = random_.randrange(11)
            t.check(
                name,
                wide(text_),
                *((font_index,) if name == "draw_string_precache_string" else ()),
                globals={"draw_string_globals": how(font_index), "font_slot_indices": fonts(), "unicode_escape_note_time": 0},
                externals={
                    "unknown_001224c0": lambda harness, arguments: 0x80123400,
                    "unknown_001414d0": ready,
                    "GetTickCount": lambda harness, arguments: 77,
                },
            )

    # ---------- the colour of the texts

    nan = float("nan")
    infinite = float("inf")
    exact = (0.0, 1.0, 0.5, 0.68, 0.6800001, 0.6799999, 0.7, 0.9, 0.999, 1.0000001, 1.5, 2.0, -0.0, -0.001, -3.0, 1e-30, nan, infinite, -infinite)
    for n in range(500):
        values = [random_.choice(exact) if random_.random() < 0.6 else random_.uniform(-0.5, 1.5) for _ in range(4)]
        if n % 3 == 0:
            # near white: its three parts are over the most
            values[1:] = [random_.choice((0.6800001, 0.69, 0.7, 0.85, 1.0, 1.2, 3.0, infinite, random_.uniform(0.68, 1.3))) for _ in range(3)]
        for name in ("draw_string_set_color_exact", "draw_string_set_color"):
            t.check(name, color(*values), globals={"draw_string_globals": how()})
    # one part is no number and the others are near white: nothing is scaled
    for at in (1, 2, 3):
        for others in (0.7, 0.9, 1.0):
            values = [0.5, others, others, others]
            values[at] = nan
            t.check("draw_string_set_color", color(*values), globals={"draw_string_globals": how()})
