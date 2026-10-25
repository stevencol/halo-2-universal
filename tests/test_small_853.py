# three functions of the way to the main menu (guided by halo2-decompiled):
# how much a biped crouches in a tick, a forward and up made perpendicular,
# and the colors of a player

import random
import struct

from difftest import Buffer, f32

from test_units import COUNT, OBJECT, some_objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12


def run(t) -> None:
    random_ = random.Random(853)

    def salted(index: int) -> int:
        return random_.getrandbits(16) << 16 | index

    for n in range(400):
        table = some_objects(random_)
        objects = bytearray(table.data) + bytearray(random_.randbytes(0x200))
        for index in range(COUNT):
            at = FIRST + index * OBJECT
            struct.pack_into("<I", objects, at, salted(0))
            struct.pack_into("<I", objects, at + 0x12C, random_.choice((NONE, salted(0))))
            struct.pack_into("<I", objects, at + 0x134, random_.getrandbits(32))
            struct.pack_into("<f", objects, at + 0x2C4, random_.choice((0.0, 0.5, 0.99, 1.0)))
            objects[at + 0x3DC] = random_.choice((0, 6, 6))
            objects[at + 0x3F8] = random_.choice((0, 1))
            struct.pack_into("<H", objects, at + 0x348, random_.getrandbits(16))
        tags = bytearray(random_.randbytes(0x10 + 0x260))
        struct.pack_into("<I", tags, 8, 0x10)
        struct.pack_into("<f", tags, 0x10 + 0x250, random_.choice((0.5, 2.0, 30.0)))
        actors = bytearray(random_.randbytes(0x50 + 0x888))
        struct.pack_into("<I", actors, 0x44, 0x50)
        struct.pack_into("<I", actors, 0x50 + 0x858, random_.choice((NONE, 3)))
        time = bytearray(random_.randbytes(0x24))
        struct.pack_into("<hf", time, 2, 30, 1 / 30)
        speed = random_.choice((0.25, 4.0))

        def actor(harness, arguments, speed=speed):
            harness.mu.mem_write(arguments[1], struct.pack("<f", speed))
            return (1, (arguments[0],))

        t.check(
            "biped_unknown_000dd7d0",
            salted(random_.randrange(COUNT)),
            globals={
                "object_header_data": Buffer(bytes(objects), pointers=table.pointers),
                "global_tag_instances": Buffer(bytes(tags), pointers=(8,)),
                "actor_data": Buffer(bytes(actors), pointers=(0x44,)),
                "game_time_globals": Buffer(bytes(time)),
            },
            externals={"actor_unknown_0025aad0": actor},
        )

        table = some_objects(random_)
        forward = random_.choice(([0.0, 0.0, 0.0], [random_.uniform(-1, 1) for _ in range(3)]))
        up = random_.choice((list(forward), [random_.uniform(-1, 1) for _ in range(3)]))
        t.check(
            "object_unknown_000b91d0",
            salted(random_.randrange(COUNT)),
            Buffer(struct.pack("<3f", *forward)),
            Buffer(struct.pack("<3f", *up)),
            globals={"object_header_data": table},
            externals={"generate_up_vector3d": lambda harness, arguments: (arguments[1], tuple(arguments))},
        )

        colors = [random_.randbytes(12) for _ in range(20)]

        def color(harness, arguments, colors=colors):
            harness.mu.mem_write(arguments[0], colors[arguments[1] % 20])
            return (arguments[0], (arguments[1],))

        def team(harness, arguments, colors=colors):
            harness.mu.mem_write(arguments[0], colors[(arguments[1] & 0xFFFF) % 20])
            return (arguments[0], (arguments[1] & 0xFFFF,))

        t.check(
            "game_engine_player_colors",
            Buffer(bytes(random_.randrange(18) for _ in range(4))),
            Buffer(random_.randbytes(48)),
            random_.choice((0xFFFF, 0, 3)) | random_.getrandbits(16) << 16,
            random_.choice((0, 1)),
            globals={"player_color_default": Buffer(random_.randbytes(12))},
            externals={"player_color_get": color, "game_engine_unknown_0007f720": team},
        )
