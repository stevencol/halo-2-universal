# source/text/unicode_string_list.c
#
# The texts of a language, the tag that has some of them and the globals tag
# are the test's, all in one buffer whose pointers are to itself

import random
import struct

from difftest import Buffer, Pointer
from test_scenario_load import physical_memory

NONE = 0xFFFFFFFF
TAGS = 0x140
LANGUAGES = 9
TEXTS = 0x188
TEXTS_SIZE = 0x1C
STRINGS = (b"", b"Checkpoint... done", b"a", b"Guardado", b"\xe3\x83\x97\xe3\x83\xac\xe3\x82\xa4", b"x" * 300)


def text(harness, address: int) -> bytes:
    return bytes(harness.mu.mem_read(address, 0x200)).split(b"\0", 1)[0]


class World:
    def __init__(self, random_: random.Random, count: int = 10, loaded: int = 1, list_index: int | None = None, stored: int | None = None):
        """`count` texts in each language; `stored` is how many the table
        says it has, if it is not that."""
        self.list_index = random_.randrange(TAGS) if list_index is None else list_index
        data = bytearray(random_.randbytes(TAGS * 0x10))
        pointers = []

        def add(part: bytes) -> int:
            offset = len(data)
            data.extend(part)
            return offset

        def point(offset: int, to: int) -> None:
            struct.pack_into("<I", data, offset, to)
            pointers.append(offset)

        # the tag with the texts: for each language, so many from one on
        self.ranges = [(random_.randint(-2, count), random_.randint(0, count + 2)) for _ in range(LANGUAGES)]
        tag = add(random_.randbytes(0x10) + b"".join(struct.pack("<2h", *range_) for range_ in self.ranges))
        point(self.list_index * 0x10 + 8, tag)

        # the globals tag: the texts of each language
        self.game_globals = add(random_.randbytes(TEXTS + LANGUAGES * TEXTS_SIZE))
        self.ids = []
        self.ghosts = []
        self.tables = []
        for language in range(LANGUAGES):
            ids = [random_.getrandbits(32) for _ in range(count)]
            if count > 3:
                # a text that is there twice: the first is the one
                ids[count - 1] = ids[1]
            strings = bytearray()
            references = bytearray()
            for string_id in ids:
                references += struct.pack("<2I", string_id, len(strings))
                strings += random_.choice(STRINGS) + b"\0"
            table = self.game_globals + TEXTS + language * TEXTS_SIZE
            # before the first text, what looks like one more: a place
            # before the first is no place
            ghost = random_.getrandbits(32)
            point(table, add(struct.pack("<2I", ghost, 0) + bytes(references) + random_.randbytes(16)) + 8)
            self.ghosts.append(ghost)
            point(table + 4, add(bytes(strings)))
            struct.pack_into("<2i2IB", data, table + 8, count if stored is None else stored, len(strings), random_.getrandbits(32), random_.getrandbits(32), loaded)
            self.ids.append(ids)
            self.tables.append(table)
        self.buffer = Buffer(bytes(data), pointers=tuple(pointers))

    def globals(self, language: int) -> dict:
        return {
            "global_tag_instances": self.buffer,
            "global_game_globals": Pointer(self.buffer, self.game_globals),
            "global_language": language,
        }

    def some_id(self, random_: random.Random, language: int) -> int:
        ids = self.ids[language]
        return random_.choice(ids) if ids and random_.random() < 0.8 else random_.getrandbits(32)


def run(t) -> None:
    random_ = random.Random(54)

    # ---------- the texts of a language

    for n in range(300):
        count = random_.choice((0, 1, 4, 10))
        stored = None if n % 6 else random_.choice((0, 1, -1, count - 1, count + 1))
        w = World(random_, count, loaded=random_.choice((1, 1, 1, 0, 0x80)), stored=stored)
        language = random_.randrange(LANGUAGES)
        first = random_.choice((0, 0, 1, -1, -3, count - 1, count, random_.randint(-2, count + 1)))
        many = random_.choice((0, 1, count, count + 2, random_.randint(0, count + 3)))
        string_id = w.some_id(random_, language) if n % 7 else w.ghosts[language]
        table = Pointer(w.buffer, w.tables[language])
        t.check("language_texts_find", table, first, many, string_id, extra=[w.buffer])
        t.check("language_texts_has", table, first, string_id, many, extra=[w.buffer])

        def to_wide(harness, arguments):
            utf8, where, length = arguments
            written = text(harness, utf8)
            # what is written is not the text, to see that it is what is copied
            harness.mu.mem_write(where, (written[:40].decode("latin-1") + "!").encode("utf-16-le") + bytes(2))
            return 0, (written, length)

        t.check(
            "language_texts_get",
            table,
            string_id,
            many,
            Buffer(random_.randbytes(0x208)),
            first,
            extra=[w.buffer],
            externals={"unknown_00140440": to_wide},
        )

    # a text that the function of the console gives with no end: 255 characters are kept
    w = World(random_)

    def no_end(harness, arguments):
        harness.mu.mem_write(arguments[1], b"z\0" * 0x100)
        return 0, (text(harness, arguments[0]), arguments[2])

    t.check(
        "language_texts_get",
        Pointer(w.buffer, w.tables[0]),
        w.ids[0][0],
        10,
        Buffer(random_.randbytes(0x208)),
        0,
        extra=[w.buffer],
        externals={"unknown_00140440": no_end},
    )

    # ---------- the texts of a tag, in the language of the console

    for n in range(200):
        w = World(random_, random_.choice((1, 4, 10)), loaded=random_.choice((1, 1, 1, 0)))
        # no language yet: the console is asked
        asked = n % 4 == 3
        xbox_language = random_.choice((0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 20))
        language = random_.randrange(LANGUAGES)
        known = language if not asked else {2: 1, 3: 2, 4: 3, 5: 4, 6: 5, 7: 6, 8: 7}.get(xbox_language, 0)
        string_id = w.some_id(random_, known)
        index = NONE if n % 10 == 9 else random_.getrandbits(16) << 16 | w.list_index
        globals_ = w.globals(NONE if asked else language)
        externals = {"XGetLanguage": lambda harness, arguments, value=xbox_language: value}
        t.check("unicode_string_list_has_string", index, string_id, globals=globals_, externals=externals)

        def to_wide(harness, arguments):
            utf8, where, length = arguments
            written = text(harness, utf8)
            harness.mu.mem_write(where, (written[:40].decode("latin-1") + "?").encode("utf-16-le") + bytes(2))
            return 0, (written, length)

        t.check(
            "unicode_string_list_get_string",
            index,
            string_id,
            Buffer(random_.randbytes(0x208)),
            globals=globals_,
            externals={**externals, "unknown_00140440": to_wide},
        )

    # ---------- reading the texts of a language

    def read(harness, arguments):
        return 1, tuple(arguments)

    for n in range(120):
        stage = random_.randrange(3)
        low = random_.choice((0x61000, 0x1000000))
        count = random_.choice((0, 1, 63, 64, 65, 511, 512, 513, 2000))
        data_size = random_.choice((0, 1, 0x1FF, 0x200, 0x201, 0xFFF, 0x1000, 0x1001, 0x12345))
        # room for both, for the first only, for none; and memory that ends at 0
        room = random_.choice((0x100000, 0x100000, count * 8 + 0xFFF & ~0xFFF, 0, 0x1000))
        high = low + room if n % 13 else random_.choice((0x1000, 0x2000))
        table = struct.pack("<2I2i2IB3s", random_.getrandbits(32), random_.getrandbits(32), count, data_size, random_.getrandbits(32), random_.getrandbits(32), random_.choice((0, 1)), random_.randbytes(3))
        t.check(
            "language_texts_load",
            Buffer(table),
            globals={"physical_memory_globals": physical_memory(stage, low, high)},
            externals={"XPhysicalProtect": lambda harness, arguments: 0, "cache_file_blocking_read": read},
        )
