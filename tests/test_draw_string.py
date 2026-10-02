# source/text/draw_string.c

import random
import struct

from difftest import Buffer, f32
from test_async import calls_so_far

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
            externals={"font_get": lambda harness, arguments, font=font: font},
        )

    # ---------- how the texts are drawn

    GLOBALS_SIZE = 0x64

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
            externals={"font_get": lambda harness, arguments: 0x80123400},
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
                "font_get": lambda harness, arguments: 0x80123400,
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
                "font_get": lambda harness, arguments, font=random_.choice((0, 0x80123400, 0x80123400, 0x80123400)): font,
                "font_character_precache": ready,
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
                    "font_get": lambda harness, arguments: 0x80123400,
                    "font_character_precache": ready,
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

    # ---------- measuring a text

    BOUNDS_SIZE, FONT_SIZE = 12, 0x1D0

    def a_font():
        # the font that there is, or none: the same in all the calls of a check
        font = Buffer(random_.randbytes(FONT_SIZE))
        there = random_.choice((True, True, False))
        return font, lambda harness, arguments: font.address if there else 0

    for n in range(40):
        t.check(
            "draw_string_initialize",
            globals={"draw_string_globals": random_.randbytes(GLOBALS_SIZE)},
        )

    def real_bits(value: float) -> int:
        return struct.unpack("<I", struct.pack("<f", value))[0]

    reals = (0.0, 1.0, -1.0, 0.5, -0.5, 100.25, -100.75, 639.0, 479.5, 32766.5, -32768.5, 40000.0, float("nan"))
    for n in range(300):
        font, get = a_font()
        bounds = [random_.choice((0x7FFF, -0x8000, 0, random_.randrange(-700, 700))) for _ in range(4)]
        x = random_.choice((*reals, float(bounds[1]), bounds[1] + 0.5, bounds[1] - 0.5, random_.uniform(-800, 800)))
        y = random_.choice((*reals, float(bounds[0]), bounds[0] + 0.5, bounds[0] - 0.5, random_.uniform(-800, 800)))
        scale = random_.choice((1.0, 1.0, 0.5, 2.0, 0.0, -1.0, random_.uniform(0, 3), float("nan")))
        # so that the right and the bottom of the character are at times
        # where those of the bounds are
        width = random_.choice((0.0, 1.0, 13.0, random_.uniform(0, 60), float(bounds[3]) - x - 1.0, float("nan")))
        height = random_.choice((0.0, 1.0, 17.0, random_.uniform(0, 60), float(bounds[2]) - y - 1.0, float("nan")))
        t.check(
            "draw_string_bounds_character",
            random_.getrandbits(32),
            random_.randrange(11),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            real_bits(x),
            real_bits(y),
            random_.getrandbits(32),
            random_.getrandbits(32),
            real_bits(width),
            real_bits(height),
            real_bits(scale),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={
                "draw_string_bounds": struct.pack("<4h", *bounds) + random_.randbytes(4),
                "font_slot_indices": fonts(),
            },
            extra=[font],
            externals={"font_get": get},
        )

    def measure(harness, arguments, end, characters_seen):
        proc, unknown1, place, unknown3, unknown4, unknown5, characters = arguments
        harness.mu.mem_write(place, end)
        text = []
        while len(text) < 0x900:
            (value,) = struct.unpack("<i", harness.mu.mem_read(characters + 4 * len(text), 4))
            text.append(value)
            if not value:
                break
        characters_seen.append(tuple(text))
        return 0, (harness.normalize(proc), unknown1, unknown3, unknown4, unknown5, tuple(text))

    for n in range(150):
        font, get = a_font()
        end = struct.pack("<2h", *(random_.choice((0, 1, -1, 0x7FFF, -0x8000, random_.randrange(-700, 700))) for _ in range(2)))
        text = [random_.randrange(1, 0x10000) for _ in range(random_.choice((0, 1, 5, 40)))] + [0]
        seen = []
        t.check(
            "draw_string_get_characters_bounds",
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(8)),
            random_.getrandbits(32),
            Buffer(struct.pack(f"<{len(text)}i", *text)),
            random_.getrandbits(32),
            globals={
                "draw_string_bounds": random_.randbytes(BOUNDS_SIZE),
                "draw_string_globals": struct.pack("<i", random_.randrange(11)) + random_.randbytes(GLOBALS_SIZE - 4),
                "font_slot_indices": fonts(),
            },
            extra=[font],
            externals={
                "font_get": get,
                "unknown_0013f0e0": lambda harness, arguments, end=end, seen=seen: measure(harness, arguments, end, seen),
            },
        )

    # the stand-in of the function that goes through the text changes the
    # bounds, as the game's does with the function that it is given
    for n in range(60):
        font, get = a_font()
        end = struct.pack("<2h", random_.randrange(-700, 700), random_.randrange(-700, 700))
        grown = struct.pack("<6h", *(random_.randrange(-700, 700) for _ in range(6)))
        text = [random_.randrange(1, 0x10000) for _ in range(3)] + [0]

        def grow(harness, arguments, end=end, grown=grown):
            # both heights, only at times: so that it is seen that they are
            # read after the text is gone through
            harness.mu.mem_write(harness.global_addresses["draw_string_bounds"], grown)
            return measure(harness, arguments, end, [])

        t.check(
            "draw_string_get_characters_bounds",
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(8)),
            random_.getrandbits(32),
            Buffer(struct.pack(f"<{len(text)}i", *text)),
            random_.getrandbits(32),
            globals={
                "draw_string_bounds": random_.randbytes(BOUNDS_SIZE),
                "draw_string_globals": struct.pack("<i", random_.randrange(11)) + random_.randbytes(GLOBALS_SIZE - 4),
                "font_slot_indices": fonts(),
            },
            extra=[font],
            externals={"font_get": get, "unknown_0013f0e0": grow},
        )

    def game_text(count: int) -> bytes:
        units = []
        while len(units) < count:
            kind = random_.randrange(8)
            if kind == 0:
                units.append(ord("|"))
                units.append(random_.choice((ord("n"), ord("t"), ord("r"), ord("l"), ord("c"), ord("x"), ord("|"), ord("N"))))
            else:
                units.append(random_.choice((random_.randrange(1, 0x80), random_.randrange(0x80, 0x10000))))
        return struct.pack(f"<{len(units) + 1}H", *units, 0)

    for n in range(60):
        count = random_.choice((0, 1, 2, 7, 30, 30, 200))
        if n < 6:
            # longer than what is taken of a text
            count = (2046, 2047, 2048, 2049, 2500, 4000)[n]
        end = struct.pack("<2h", random_.randrange(-700, 700), random_.randrange(-700, 700))
        seen = []
        font, get = a_font()
        data = game_text(count)
        if n % 10 == 9:
            # a bar at the end
            data = data[:-2] + struct.pack("<2H", ord("|"), 0)
        t.check(
            "draw_string_get_bounds",
            Buffer(data + random_.randbytes(8)),
            random_.getrandbits(32),
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(8)),
            random_.getrandbits(32),
            globals={
                "draw_string_bounds": random_.randbytes(BOUNDS_SIZE),
                "draw_string_globals": struct.pack("<i", random_.randrange(11)) + random_.randbytes(GLOBALS_SIZE - 4),
                "font_slot_indices": fonts(),
                "unicode_escape_note_time": 0,
            },
            extra=[font],
            externals={
                "font_get": get,
                "GetTickCount": lambda harness, arguments: 77,
                "unknown_0013f0e0": lambda harness, arguments, end=end, seen=seen: measure(harness, arguments, end, seen),
            },
        )

    # ---------- how much of a text fits in a line

    KNOWN_DATA, KNOWN, KNOWN_COUNT, KNOWN_STATE, KNOWN_HEADER = 0x4C, 0x38, 6, 0x10, 0x1C
    PREVIOUS_CHARACTER, CAN_BREAK = 0x16, 0x1A
    nan = float("nan")

    def known_characters(narrow: bool) -> Buffer:
        """The characters that the game knows of: all ready but, at times, one."""
        blob = bytearray(random_.randbytes(KNOWN_DATA + KNOWN_COUNT * KNOWN))
        struct.pack_into("<I", blob, 0x44, KNOWN_DATA)
        for index in range(KNOWN_COUNT):
            place = KNOWN_DATA + index * KNOWN
            struct.pack_into("<I", blob, place + KNOWN_STATE, 4 if random_.randrange(12) else random_.choice((0, 3, 5)))
            # how far the next character is, and where its pixels are
            struct.pack_into("<h", blob, place + KNOWN_HEADER, random_.choice((0, 3, 8, 8, 13, 21) if narrow else (8, 13, 300, -5, 0x7FFF)))
            struct.pack_into("<2h", blob, place + KNOWN_HEADER + 8, random_.choice((0, 0, 1, -1, 2, -3)), random_.randrange(-5, 20))
        return Buffer(bytes(blob), pointers=(0x44,))

    for n in range(900):
        # what going through the text finds: its characters and what is
        # after them
        count = random_.choice((0, 1, 2, 3, 5, 8, 12, 20))
        steps = []
        index = random_.choice((0, 0, 0, 1, 7))
        for step in range(count):
            index += random_.choice((1, 1, 1, 2))
            character = random_.choice((0x41, 0x61, 0x20, 0x3042, 0xE100, 0x1F))
            steps.append((4, character, random_.choice((0, 0x41, 0x20)), random_.randrange(10) < (2 if n % 3 else 6), index if n % 50 != 49 else 0))
        steps.append((random_.choice((0, 0, 1, 2, 3)), random_.choice((0, 0xA, 9)), 0, random_.getrandbits(1), index + 1))
        if n % 20 == 19:
            # an index that is not kept whole
            steps = [(element, character, previous, can_break, at | 0x8000) for element, character, previous, can_break, at in steps]

        def next_(harness, arguments, steps=steps):
            (iterator,) = arguments
            element, character, previous, can_break, at = steps[min(calls_so_far(harness, "text_iterator_next"), len(steps) - 1)]
            harness.mu.mem_write(iterator + INDEX, struct.pack("<H", at & 0xFFFF))
            harness.mu.mem_write(iterator + CHARACTER, struct.pack("<2I", character, previous))
            harness.mu.mem_write(iterator + CAN_BREAK, bytes((can_break,)))
            harness.mu.mem_write(iterator + ELEMENT, struct.pack("<I", element))
            return 0, ()

        def request(harness, arguments):
            font_index, character, flags = arguments
            # some characters are not there
            asked = 0xFFFFFFFF if character == 0x1F and font_index % 2 else (character + font_index) % KNOWN_COUNT | 0xE1740000
            return asked, (font_index, character, flags)

        def kerning(harness, arguments):
            font, first, second = arguments
            return (first * 7 + second) % 5 - 2

        scale = random_.choice((1.0, 1.0, 1.0, 0.5, 2.0, 1.5, 0.0, -1.0, 0.3)) if n % 40 != 39 else nan
        x = random_.choice((0.0, 0.0, 20.0, -30.5, 100.25, 640.0)) if n % 41 != 40 else nan
        right = x + random_.choice((0.0, 5.0, 9.0, 20.0, 40.0, 60.0, 100.0, 200.0, -10.0, 1e9)) if n % 43 != 42 else nan
        iterator = bytearray(random_.randbytes(ITERATOR_SIZE))
        struct.pack_into("<I", iterator, 0, random_.randrange(11))
        t.check(
            "draw_string_measure_line",
            Buffer(bytes(iterator)),
            Buffer(struct.pack("<2f", x, random_.uniform(-50, 500))),
            Buffer(struct.pack("<4f", random_.uniform(-50, 50), right, random_.uniform(-50, 50), random_.uniform(50, 500))),
            real_bits(scale),
            globals={
                "draw_string_globals": struct.pack("<2I", random_.randrange(11), random_.choice((0, 1, 1, 1, 2, 0xFFFFFFFE, 0xFFFFFFFF)))
                + random_.randbytes(GLOBALS_SIZE - 8),
                "font_cache_globals+4": known_characters(n % 5 != 4),
            },
            externals={
                "text_iterator_next": next_,
                "font_character_request": request,
                "font_character_pixels_request": lambda harness, arguments, there=random_.choice((1, 1, 1, 1, 1, 0x80, 0)): there,
                "font_get_kerning": kerning,
            },
        )
