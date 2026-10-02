# source/saved games/saved_game_file_names.c

import random

from difftest import Buffer

NONE = 0xFFFFFFFF

NAMES = (
    "saved_game_file_default_name_profile",
    "saved_game_file_default_name_slayer",
    "saved_game_file_default_name_koth",
    "saved_game_file_default_name_race",
    "saved_game_file_default_name_oddball",
    "saved_game_file_default_name_juggernaut",
    "saved_game_file_default_name_headhunter",
    "saved_game_file_default_name_ctf",
    "saved_game_file_default_name_assault",
    "saved_game_file_default_name_territories",
    "saved_game_file_default_name_playlist",
    "saved_game_file_default_name_unknown",
)
STRING_IDS = (
    0x1B0006E9,
    0x130006EA,
    0x110006EB,
    0x110006EC,
    0x140006ED,
    0x170006EE,
    0x170006EF,
    0x100006F0,
    0x140006F1,
    0x180006F2,
    0x150006F3,
    0x1B0006E8,
)
LANGUAGES = list(range(-1, 11)) + [NONE, 0x80000000, 0x7FFFFFFF, 0x100, 0x101]


def run(t) -> None:
    random_ = random.Random(53)

    # each name in each language, and in those that there are not
    for name in NAMES:
        for language in LANGUAGES:
            t.check(name, language)

    # the texts of the types, those next to them, and others
    string_ids = list(STRING_IDS) + [0, NONE, 0x1B0006E7, 0x150006F4, 0x100006EF, 0x100006F1, 0x130006E9, 0x130006EB]
    string_ids += [string_id ^ 0x01000000 for string_id in STRING_IDS[:4]] + [string_id & 0xFFFF for string_id in STRING_IDS[:4]]
    for string_id in string_ids:
        for language in (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, NONE):
            t.check("saved_game_file_get_default_name", string_id, language, Buffer(random_.randbytes(0x208)))

    # the length of a name
    for length in list(range(0, 13)) + [100, 250, 253, 254, 255, 256, 257, 259, 260, 300]:
        data = bytearray()
        for _ in range(length):
            data += random_.choice((b"a\0", b"\0\x30", b"\xff\xff", b"\x01\0"))
        data += b"\0\0" + random_.randbytes(8)
        t.check("saved_game_file_name_length", Buffer(bytes(data) + bytes(0x260)))
