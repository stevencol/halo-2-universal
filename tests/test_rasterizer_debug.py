# source/rasterizer/rasterizer_debug.c
#
# The device of Direct3D is made up by the test: its push buffer is in the
# same memory, after it

import random
import struct

from difftest import Buffer

DEVICE_SIZE = 0x200
PUSH = 0x100
# where Direct3D makes room when there is none
SPACE = 0x180


def device(random_: random.Random) -> Buffer:
    data = bytearray(random_.randbytes(DEVICE_SIZE))
    push = PUSH + 8 * random_.randrange(4)
    # with room, at the end of it or past it
    limit = push + random_.choice((4, 8, 16, 0x40, 0, -8))
    flags = random_.choice((0, 0x800, 0x1000, 0x1800, 0xFFFFFFFF, 0xFFFFE7FF, random_.getrandbits(32)))
    struct.pack_into("<3I", data, 0, push, limit, flags)
    return Buffer(bytes(data), pointers=(0, 4))


def externals(device_: Buffer) -> dict:
    def make_space(harness, arguments):
        return device_.address + SPACE

    def lazy(harness, arguments):
        # Direct3D can write commands itself: the push buffer moves
        (push,) = struct.unpack("<I", harness.mu.mem_read(device_.address, 4))
        harness.mu.mem_write(push, struct.pack("<I", 0x5EED0001))
        harness.mu.mem_write(device_.address, struct.pack("<I", push + 4))
        return 0

    return {
        "D3D_MakeRequestedSpace": make_space,
        "D3D_LazySetState": lazy,
        "D3DDevice_SetRenderState_LineWidth": lambda harness, arguments: 0,
        "D3D_SetFence": lambda harness, arguments: 0x77,
    }


def globals_(random_: random.Random, device_: Buffer, type_: int) -> dict:
    return {
        "d3d_device": device_,
        "d3d_push_buffer_size": struct.pack("<I", random_.choice((0x80000, 0x20000, 0x12345))),
        "rasterizer_debug_primitive_type": struct.pack("<i", type_),
        "rasterizer_debug_immediate_active": bytes([random_.choice((0, 1, 0xFF))]),
    }


def run(t) -> None:
    random_ = random.Random(128)

    for name in ("rasterizer_debug_primitive_end", "rasterizer_debug_immediate_end"):
        for _ in range(150):
            device_ = device(random_)
            type_ = random_.choice((0, 0, 1, 2, 3, 4, 7, -1))
            t.check(name, globals=globals_(random_, device_, type_), externals=externals(device_))

    for _ in range(300):
        device_ = device(random_)
        type_ = random_.randrange(1, 5)
        current = random_.choice((0, 0, type_, 1, 2, 3, 4))
        t.check(
            "rasterizer_debug_primitive_begin",
            type_,
            globals=globals_(random_, device_, current),
            externals=externals(device_),
        )
