# five small functions of sixteen entries of 0xB bytes, each known by six
# bytes, and a number with its bytes the other way round

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
ENTRIES, ENTRY, AT = 16, 0xB, 0xDC24


def entries(random_: random.Random, identifiers: list[bytes]) -> bytearray:
    data = bytearray(random_.randbytes(ENTRIES * ENTRY))
    full = random_.random() < 0.2
    for index in range(ENTRIES):
        at = index * ENTRY
        data[at : at + 6] = random_.choice(identifiers)
        data[at + 6] = random_.choice((1, 0x80)) if full else random_.choice((0, 0, 1, 1, 0x80))
        data[at + 8] = random_.choice((0, 0, 0, 1, 0x40))
    return data


def run(t) -> None:
    random_ = random.Random(765)

    for n in range(600):
        identifiers = [random_.randbytes(6) for _ in range(3)]
        # two that differ only in their last or first two bytes
        identifiers.append(identifiers[0][:4] + random_.randbytes(2))
        identifiers.append(random_.randbytes(2) + identifiers[0][2:])

        def data():
            whole = bytearray(random_.randbytes(AT))
            return Buffer(bytes(whole + entries(random_, identifiers)))

        t.check("game_unknown_001991f0_new", data(), Buffer(random_.choice(identifiers)))
        t.check("game_unknown_001991f0_find", data(), Buffer(random_.choice(identifiers)))
        t.check("game_unknown_001991f0_find_marked", data())
        t.check(
            "game_unknown_001991f0_mark",
            random_.choice((NONE, 0, 1, 7, 15, 16, random_.randrange(16), random_.getrandbits(32))),
            globals={"unknown_0051e8d4": bytes(entries(random_, identifiers))},
        )
        t.check(
            "game_unknown_001991f0_read_swapped",
            random_.choice((0, 1, 3, 4, 5, 8, 0x80000000, NONE, random_.getrandbits(32))),
            Buffer(struct.pack("<I", random_.getrandbits(32))),
        )
