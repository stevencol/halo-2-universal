# four small functions of the things of the interface: two bytes and a
# number, a real of where a thing is moved, its place moved, and the text of
# the kind of what the player of a gamepad has

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(770)

    for n in range(600):
        thing = bytearray(random_.randbytes(0x70))
        thing[0x6D] = random_.choice((0, 1, 1, 0x80))
        thing[0x6E] = random_.choice((0, 1, 1, 0x80))
        limit = random_.choice((0, 5, 100, 0x7FFFFFFF, NONE))
        struct.pack_into("<I", thing, 0x50, random_.choice((0, 5, 6, 99, 100, 101, 0x80000000, NONE, limit)))
        reals = [random_.choice((0.0, -0.0, 0.5, -0.5, 1.99, -1.99, 7.0, -300.75, 32767.9, -40000.25, random_.uniform(-1000, 1000))) for _ in range(3)]
        struct.pack_into("<3f", thing, 0x58, *reals)
        thing = bytes(thing)
        t.check("user_interface_thing_unknown_0022e3b4", Buffer(thing), globals={"unknown_0054d5b8": limit})
        t.check("user_interface_thing_get_offset_z", Buffer(thing))
        t.check("user_interface_thing_get_bounds", Buffer(thing), Buffer(random_.randbytes(8)))

        text = Buffer(random_.randbytes(0x40))
        unknown = bytearray(random_.randbytes(0x92))
        if random_.random() < 0.25:
            unknown[0:8] = bytes(8)
        elif random_.random() < 0.3:
            unknown[0:8] = random_.choice((b"\1\0\0\0\0\0\0\0", b"\0\0\0\0\0\0\0\x80"))
        struct.pack_into("<I", unknown, 0x1C, random_.choice((0, 1, 2, 3, 3, 4, NONE)))

        def got(harness, arguments, there=random_.choice((0, 1, 1, 1)), unknown=bytes(unknown)):
            harness.mu.mem_write(arguments[1], unknown)
            return there, (arguments[0],)

        t.check(
            "user_interface_text_unknownB82_kind",
            random_.getrandbits(32),
            text,
            globals={"unknown_0046e7b8": random_.choice((NONE, 0, 1, 3))},
            externals={
                "user_interface_controller_get_unknownB82": got,
                "unknown_0023620d": lambda harness, arguments, text=text: (0, (arguments[0], arguments[1] == text.address)),
            },
        )
