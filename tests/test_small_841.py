# three functions of the way to the main menu: how one of the four is in a
# session, the colors of an object kept in its cached render state, and the
# first entry of a sound of a thing

import random
import struct

from difftest import Buffer

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
PLAYER = 0x13C
ENTRY = 0x18


def run(t) -> None:
    random_ = random.Random(841)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        four = bytearray(random_.randbytes(4 * 0xD0))
        same = bytearray(random_.randbytes(0x98))
        for index in range(4):
            at = index * 0xD0
            four[at] = random_.choice((0, 1, 1))
            struct.pack_into("<I", four, at + 9, random_.choice((0xBAD00000, 1)))
            four[at + 0x10:at + 0x10 + 0x94] = same[:0x94]
            struct.pack_into("<i", four, at + 0xA4, 7)
        session = bytearray(random_.randbytes(0x1120 + 3 * PLAYER + 0x6400))
        struct.pack_into("<i", session, 0x741C, random_.choice((2, 3, 5, 8, 9)))
        struct.pack_into("<i", session, 0x72D8, random_.randrange(2))
        struct.pack_into("<i", session, 0x10, random_.randrange(4))
        for slot in range(2 * 0x43 + 4):
            struct.pack_into("<I", session, 0x154 + slot * 4, random_.choice((NONE, 0, 1, 2)))
        for player in range(3):
            at = 0x1120 + player * PLAYER
            if random_.random() < 0.5:
                session[at + 0x14:at + 0x14 + 0x94] = same[:0x94]
                struct.pack_into("<i", session, at + 0x138, random_.choice((7, 8)))
            struct.pack_into("<I", session, at + 0x14, random_.choice((NONE, struct.unpack_from("<I", same, 0)[0])))
        state = random_.choice((0, 1, 2))
        t.check(
            "game_voice_unknown_000646b0",
            random_.randrange(4),
            Buffer(bytes(session)),
            globals={"unknown_004cd904": bytes(four), "unknown_00467214": random_.choice((NONE, 0, 1))},
            externals={"online_tasks_get_logon_state": lambda harness, arguments, state=state: (state, tuple(arguments))},
        )

        # objects with entries of twelve bytes (reals of colors) at their short at 0x126, the size at 0x124
        table = some_objects(random_)
        objects = bytearray(table.data)
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<I", objects, at, salted(random_.randrange(2)))
            struct.pack_into("<IHh", objects, at + 0xCC, random_.choice((NONE, salted(0), salted(1))), 0, 0)
            struct.pack_into("<hh", objects, at + 0x124, random_.choice((0, 24, 48, 96)), 0x140)
            struct.pack_into("<24f", objects, at + 0x140, *(random_.uniform(0, 1) for _ in range(24)))
        tags = bytearray(random_.randbytes(0x20 + 2 * 0x20))
        for tag in range(2):
            struct.pack_into("<I", tags, tag * 0x10 + 8, 0x20 + tag * 0x20)
        states = bytearray(random_.randbytes(0x50 + 2 * 0x100))
        struct.pack_into("<I", states, 0x44, 0x50)
        t.check(
            "cached_object_render_state_unknown_0003dd10",
            salted(random_.randrange(COUNT)),
            random_.choice((0, 1)),
            globals={
                "object_header_data": Buffer(bytes(objects), pointers=table.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=(8, 0x18)),
                "cached_object_render_state_data": Buffer(bytes(states), pointers=(0x44,)),
            },
        )

        # a data array of six entries, some in use (bits at 0x48)
        entries = bytearray(random_.randbytes(0x60 + 6 * ENTRY))
        struct.pack_into("<i", entries, 0x24, ENTRY)
        struct.pack_into("<i", entries, 0x38, random_.choice((0, 3, 6)))
        struct.pack_into("<II", entries, 0x44, 0x60, 0x50)
        struct.pack_into("<I", entries, 0x50, random_.getrandbits(6))
        for entry in range(6):
            at = 0x60 + entry * ENTRY
            struct.pack_into("<I", entries, at + 0x0C, random_.choice((5, 6)))
        t.check(
            "sound_unknown_0018d1c0",
            random_.choice((5, 6)),
            globals={"unknown_004ed28c": Buffer(bytes(entries), pointers=(0x44, 0x48))},
        )
