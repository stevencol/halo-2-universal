# twenty-two small functions of the AI, of Havok's part and of others

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
ACTORS, ACTOR = 3, 0x888
TAGS, TAG = 3, 0x20


def run(t) -> None:
    random_ = random.Random(736)

    for n in range(300):
        def actors() -> Buffer:
            data = bytearray(random_.randbytes(0x4C + ACTORS * ACTOR))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(ACTORS):
                at = 0x4C + index * ACTOR
                data[at + 0x50C] = random_.choice((0, 0, 1, 0xFF))
                struct.pack_into("<h", data, at + 0x504, random_.choice((0, 1, 1, 2, 3, 4, -1)))
                struct.pack_into("<H", data, at + 0x508, random_.choice((0, 5, 0x7FFF, 0xFFFF)))
            return Buffer(bytes(data), pointers=(0x44,))

        actor_index = lambda: random_.getrandbits(16) << 16 | random_.randrange(ACTORS)
        for name in (
            "actor_get_unknown50C",
            "actor_unknown504_is_1_with_unknown50C",
            "actor_unknown504_is_2",
            "actor_unknown504_is_3",
            "actor_set_unknown504_to_3",
        ):
            t.check(name, actor_index(), globals={"actor_data": actors()})

        # something with four numbers after it
        thing = bytearray(random_.randbytes(0x18 + 16))
        struct.pack_into("<i", thing, 0x10, random_.choice((0, 1, 4, -1)))
        struct.pack_into("<I", thing, 0x14, 0x18)
        t.check(
            "unknown10_get_element_for_none",
            random_.choice((0, 1, 3, 4, NONE, 0x80000000)),
            Buffer(bytes(thing), pointers=(0x14,)),
            random_.choice((NONE, NONE, 0, random_.getrandbits(32))),
        )

        # the time, and four pairs of a time and a key
        now = random_.choice((0, 2, 3, 1000, 0x7FFFFFFF, 0x80000000, 0x80000002, 0xFFFFFFFF))

        def time() -> Buffer:
            data = bytearray(random_.randbytes(0x24))
            struct.pack_into("<I", data, 8, now)
            return Buffer(bytes(data))

        times = b"".join(
            struct.pack("<IHH", (now - random_.choice((0, 1, 30, 31, 1000, -5))) & 0xFFFFFFFF, random_.choice((0, 1, 2, 0xFFFF)), random_.getrandbits(16))
            for _ in range(4)
        )
        t.check(
            "unknown_times_has_recent",
            Buffer(times),
            random_.choice((0, 1, 2, 0xFFFF, 0x10001, NONE)),
            random_.choice((0, 1, 30, 31, 32, 2000, 0xFFFFFFFF)),
            globals={"game_time_globals": time()},
        )

        # the two things of the AI
        some = (0, 7, NONE, random_.getrandbits(32))

        def two() -> Buffer:
            data = bytearray(random_.randbytes(2 * 0x1C))
            for index in range(2):
                struct.pack_into("<2I", data, index * 0x1C, random_.choice(some), random_.choice(some))
            return Buffer(bytes(data))

        t.check("ai_unknown_find_index", random_.choice(some), globals={"unknown_004f55cc": two()})
        t.check("ai_unknown_find", random_.choice(some), globals={"unknown_004f55cc": two()})
        t.check("ai_unknown_clear_unknown4", random_.choice(some), globals={"unknown_004f55cc": two()})

        thing = bytearray(random_.randbytes(0x40))
        struct.pack_into("<I", thing, 0x28, random_.choice((0, 0, 1, random_.getrandbits(32))))
        struct.pack_into("<2I", thing, 0x2C, 0x10, 0)
        struct.pack_into("<I", thing, 0x34, 0x20)
        t.check("unknown28_get_element", Buffer(bytes(thing), pointers=(0x2C, 0x34)), random_.choice((0, 1, 2, 100)))

        t.check(
            "unknown_0047f054_is_recent",
            globals={
                "unknown_0047f054": random_.choice((NONE, now, (now - 1) & 0xFFFFFFFF, (now - 3) & 0xFFFFFFFF, (now - 4) & 0xFFFFFFFF, (now + 1) & 0xFFFFFFFF, 0, random_.getrandbits(32))),
                "game_time_globals": time(),
            },
        )

        # the tags: each with some things of 0x20 bytes after it
        count = random_.choice((0, 1, 3))
        tags = bytearray(random_.randbytes(TAGS * 16 + TAGS * (TAG + 3 * 0x20)))
        pointers = []
        for index in range(TAGS):
            at = TAGS * 16 + index * (TAG + 3 * 0x20)
            struct.pack_into("<I", tags, index * 16 + 8, at)
            struct.pack_into("<2I", tags, at + 0xC, count, at + TAG)
            pointers += [index * 16 + 8, at + 0x10]
            for one in range(3):
                struct.pack_into("<I", tags, at + TAG + one * 0x20, random_.choice((0, 1, 2, NONE)))
        thing = bytearray(random_.randbytes(0x6C))
        struct.pack_into("<I", thing, 0x68, random_.getrandbits(16) << 16 | random_.randrange(TAGS))
        t.check(
            "unknown68_find_tag_unknownC",
            Buffer(bytes(thing)),
            random_.choice((0, 1, 2, NONE, 3)),
            globals={"global_tag_instances": Buffer(bytes(tags), pointers=tuple(pointers))},
        )

        t.check("unknown_get_unknown4_byte", Buffer(random_.randbytes(8)), Buffer(random_.randbytes(4)))

        thing = bytearray(random_.randbytes(0x14 + 3 * 0x20))
        struct.pack_into("<2I", thing, 0xC, count, 0x14)
        for one in range(3):
            thing[0x14 + one * 0x20 + 0xA] = random_.choice((0, 1, 3, 0x80, 0xFF))
        t.check(
            "unknownC_find_with_flags",
            Buffer(bytes(thing), pointers=(0x10,)),
            random_.choice((0, 1, 2, 3, 0x80, 0xFF, 0x100, 0x101, NONE)),
        )

        t.check("unknown_001dd7a0_clear", Buffer(random_.randbytes(0x18)))

        # something with pairs of a key and a value after it
        pairs = random_.choice((0, 1, 4))
        thing = bytearray(random_.randbytes(0x38))
        struct.pack_into("<2I", thing, 0x30, 0x38, pairs)
        for _ in range(4):
            thing += struct.pack("<2I", random_.choice((0x2001, 0x2002, 0x2003, 0x2000, 0x12001, 0)), random_.getrandbits(32))
        a_thing = lambda: Buffer(bytes(thing), pointers=(0x30,))
        t.check("unknown30_find_2001", a_thing())
        t.check("unknown30_find_2002", a_thing())
        t.check("unknown30_find_2003", a_thing())

        # what unknown_00510c94 points to, with its things after it; or nothing
        def unknown() -> dict:
            if random_.random() < 0.15:
                return {"unknown_00510c94": 0}
            data = bytearray(random_.randbytes(0x428 + 4 * 0x14))
            struct.pack_into("<i", data, 0x420, random_.choice((0, 1, 4, -1)))
            struct.pack_into("<I", data, 0x424, 0x428)
            return {"unknown_00510c94": Buffer(bytes(data), pointers=(0x424,))}

        index = lambda: random_.choice((0, 1, 3, 4, NONE, 0x80000000))
        t.check("unknown_00510c94_get_unknown420", index(), globals=unknown())
        t.check("unknown_00510c94_get_unknown420_first", index(), globals=unknown())

        thing = bytearray(random_.randbytes(0xDC))
        thing[0xBC] = random_.choice((0, 1, 1, 0xFF))
        struct.pack_into(
            "<f", thing, 0xD8, random_.choice((0.0, 0.1, 0.09999999, 0.10000001, 1.0, -1.0, float("nan"), float("inf"), random_.uniform(0.0, 0.2)))
        )
        t.check("unknown_001ec3f0", Buffer(bytes(thing)), Buffer(random_.randbytes(8)))
