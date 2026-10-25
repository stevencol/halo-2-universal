# four small functions of the way to the main menu: three places of the
# entry of the language in a table, and whether what Havok gives changed

import random

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(818)

    for n in range(400):
        for name in ("unknown_002afd10", "unknown_002afd40", "unknown_002afd70"):
            xbox = random_.randrange(10)
            language = random_.randrange(10)
            t.check(
                name,
                Buffer(random_.randbytes(0x400 + 10 * 0x100)),
                globals={"global_language": random_.choice((NONE, random_.randrange(10)))},
                externals={
                    "XGetLanguage": lambda harness, arguments, xbox=xbox: (xbox, tuple(arguments)),
                    "language_from_xbox_language": lambda harness, arguments, language=language: (language, tuple(arguments)),
                },
            )
        old = random_.getrandbits(32)
        value = random_.choice((old, random_.getrandbits(32)))
        t.check(
            "unknown_00278e80",
            Buffer(random_.randbytes(0xB0)),
            globals={"unknown_0055e504": old, "unknown_0055e508": random_.getrandbits(32), "unknown_0055e4fc": bytes((random_.randrange(2),))},
            externals={"havok_unknown_00147090": lambda harness, arguments, value=value: (value, tuple(arguments))},
        )
