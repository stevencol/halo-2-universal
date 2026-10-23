# six small functions of the list of games of the service: whether there is
# one and whether it is being read, and the name, the text and two more
# things of one of it

import random
import struct

from difftest import Buffer, GlobalData
from test_network_hoppers import COUNT, hopper

NONE = 0xFFFFFFFF
LANGUAGES = 9


def blob(random_: random.Random) -> bytes:
    data = bytearray(random_.randbytes(0x30))
    struct.pack_into(
        "<HHHH",
        data,
        4,
        random_.choice((2, 2, 2, 3, 0, 0xFFFD)),
        random_.getrandbits(16),
        random_.choice((0, 1, 1, 4, 0xFFFF)),
        random_.choice((1, 1, 0, 2, 0x101)),
    )
    return bytes(data)


def texts(random_: random.Random, data: bytearray) -> None:
    """Names and texts in UTF-8, ended or not, with characters of more than a byte."""
    for language in range(LANGUAGES):
        for at, size in ((0xC + language * 0x10, 0x10), (0x9C + language * 0x80, 0x80)):
            text = "".join(random_.choice("abcXYZ éñü€") for _ in range(random_.randrange(size))).encode("utf-8")[: size - 1]
            data[at : at + size] = text + bytes(size - len(text)) if random_.random() < 0.8 else random_.randbytes(size)


def run(t) -> None:
    random_ = random.Random(757)

    for n in range(300):
        table = bytearray()
        for _ in range(COUNT):
            one = hopper(random_)
            texts(random_, one)
            # the byte of 0x5E8 and the one after it, often 0: only whether
            # it is 0 is compared
            one[0x5E8] = random_.choice((0, 0, 1, 0xFF))
            one[0x5E9] = random_.choice((0, 0, 1, 0xFF))
            table += one
        globals_ = {"unknown_0047d8f4": blob(random_), "network_hoppers": GlobalData(bytes(table))}
        good = lambda harness, arguments, so=random_.choice((0, 1, 1)): (so, (harness.normalize(arguments[0]),))
        index = random_.choice((0, 1, 7, 15, 16, NONE, 0x80000000, random_.randrange(16), random_.randrange(16)))
        language = {"global_language": random_.choice((NONE, 0, 1, 5, 8))}

        t.check("network_hoppers_list_available", globals=globals_)
        t.check(
            "network_hoppers_list_reading",
            random_.choice((Buffer(random_.randbytes(4)), 0)),
            globals=globals_,
            externals={
                "network_blob_get_progress": lambda harness, arguments: (
                    1,
                    (arguments[0] - harness.global_addresses["unknown_0047d8f4"], harness.normalize(arguments[1])),
                ),
            },
        )
        for name, size in (("network_hoppers_get_name", 0x20), ("network_hoppers_get_description", 0x100)):
            t.check(
                name,
                index,
                Buffer(random_.randbytes(size)),
                globals={**globals_, **language},
                externals={
                    "network_hopper_valid": good,
                    "XGetLanguage": lambda harness, arguments, said=random_.randrange(10): (said, ()),
                    "language_from_xbox_language": lambda harness, arguments, said=random_.randrange(LANGUAGES): (said, tuple(arguments)),
                },
            )
        t.check("network_hoppers_get_unknown5E4", index, globals=globals_, externals={"network_hopper_valid": good})
        t.check("network_hoppers_get_unknown5E8", index, globals=globals_, externals={"network_hopper_valid": good})
