# source/text/draw_string.c

import random
import struct

from difftest import Buffer

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
