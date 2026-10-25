# two functions of the way to the main menu (guided by halo2-decompiled): an
# animation and those that follow it asked for, and the textures of the
# stages of a shader

import random
import struct

from difftest import Buffer

NONE = 0xFFFFFFFF
ANIMATION = 0x6C


def run(t) -> None:
    random_ = random.Random(854)

    for n in range(400):
        # a graph with four animations: a parent at 0x30 and a next at 0x32 each
        graph = bytearray(random_.randbytes(0xC0 + 4 * ANIMATION + 4 * 0x14))
        struct.pack_into("<I", graph, 0x30, 0xC0)
        struct.pack_into("<I", graph, 0xB0, 0xC0 + 4 * ANIMATION)
        for resource in range(4):
            graph[0xC0 + 4 * ANIMATION + resource * 0x14 + 0x10] = random_.choice((0, 1))
        struct.pack_into("<i", graph, 0x4C, 0)
        for animation in range(4):
            at = 0xC0 + animation * ANIMATION
            later = [other for other in range(animation + 1, 4)]
            graph[at + 0x16] = random_.choice((0, 0x10))
            struct.pack_into("<i", graph, at + 0x24, random_.randrange(4))
            struct.pack_into("<hh", graph, at + 0x30, random_.choice((-1, -1, 0)), random_.choice(later + [-1, -1]) if later else -1)
        flags = bytes((random_.choice((0, 1)), random_.choice((0, 1))))
        other = bytearray(graph)
        for animation in range(4):
            other[0xC0 + animation * ANIMATION + 0x16] ^= 0x10
        other_buffer = Buffer(bytes(other), pointers=(0x30, 0xB0))
        t.check(
            "havok_unknown_001dd9d0",
            Buffer(bytes(graph), pointers=(0x30, 0xB0)),
            random_.choice((0xFFFF, 0, 1, 2, 3)) << 16 | random_.choice((0xFFFF, 0)),
            globals={"animation_cache_initialized": flags[:1], "animation_cache_opened": flags[1:]},
            extra=[other_buffer],
            externals={
                "animation_cache_predict": lambda harness, arguments: (0, (arguments[0], arguments[1] & 0xFF)),
                "unknown4C_get_tag": lambda harness, arguments, other_buffer=other_buffer: (other_buffer.address, (arguments[0] & 0xFFFF,)),
            },
        )

        # the state: values (12 bytes) and entries (4 bytes) through two pointers
        state = bytearray(random_.randbytes(0x320 + 4 * 0x40))
        holder = len(state)
        state += bytearray(random_.randbytes(0x40))
        values = len(state)
        state += bytearray(random_.randbytes(4 * 12))
        entries = len(state)
        state += bytearray(random_.randbytes(8 * 4))
        struct.pack_into("<II", state, 0x0C, holder, holder + 0x10)
        struct.pack_into("<I", state, holder + 0x08, values)
        struct.pack_into("<I", state, holder + 0x10 + 0x24, entries)
        for value in range(4):
            struct.pack_into("<iii", state, values + value * 12, random_.choice((-1, 5)), random_.randrange(-100, 100), random_.getrandbits(31))
        for entry in range(8):
            state[entries + entry * 4] = random_.randrange(4)
            state[entries + entry * 4 + 3] = random_.randrange(4)
        globals_ = bytearray(random_.randbytes(0x68))
        t.check(
            "rasterizer_unknown_00016f60",
            Buffer(bytes(state), pointers=(0x0C, 0x10, holder + 0x08, holder + 0x10 + 0x24)),
            Buffer(struct.pack("<H", random_.randrange(4) | random_.randrange(5) << 9)),
            globals={"unknown_00485a80": Buffer(bytes(globals_))},
        )
