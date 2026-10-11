# source/networking/network_blob.c

import random
import struct
import zlib

from difftest import PHYSICAL_ADDRESS, Buffer, Memory, Mock, mock_address

NONE = 0xFFFFFFFF


def run(t) -> None:
    random_ = random.Random(491)

    for n in range(3000):
        size = random_.choice((0x10, 0x10, 0x11, 0x40, 0x123))
        blob = bytearray(random_.randbytes(size))
        version = random_.choice((1, 5, 0x80000000))
        said = random_.choice((size, size, size, size, size + 1, size - 1))
        struct.pack_into("<2I", blob, 4, random_.choice((version, version, version, version - 1, version + 1)), said)
        # the checksum that the game makes is the common one, not turned over at the end
        good = zlib.crc32(bytes(blob[4:])) ^ 0xFFFFFFFF
        struct.pack_into("<I", blob, 0, random_.choice((good, good, good, good ^ 1)))
        t.check("network_blob_checksum", Buffer(bytes(blob)), size)
        # what has the block, with its functions after it: of them, only the version is looked at
        definition = bytearray(random_.randbytes(0x40))
        struct.pack_into("<I", definition, 0, 0x30)
        struct.pack_into("<H", definition, 8, random_.choice((4, 4, 1, 3)))
        struct.pack_into("<I", definition, 0x28, random_.choice((size, size, size, size + 8, size - 1, 0x1000)))
        struct.pack_into("<I", definition, 0x38, version)
        t.check(
            "network_blob_verify",
            random_.choice((size, size, size, size, 0xF, 8, 0)),
            Buffer(bytes(blob)),
            Buffer(bytes(definition), pointers=(0,)),
        )

    # ---------- what has a block reads it and writes it by a task

    def a_blob(state=None, functions=None) -> Buffer:
        """What has a block, with its functions after it if it has them: each is there or not."""
        data = bytearray(random_.randbytes(0x50))
        has = random_.random() < 0.85 if functions is None else functions
        struct.pack_into("<I", data, 0, 0x30 if has else 0)
        struct.pack_into(
            "<4Hh",
            data,
            4,
            random_.getrandbits(16),
            random_.choice((0, 0, 1, 2, 3, random_.getrandbits(16))),
            random_.choice((0, 1, 4, 4, 2)),
            random_.choice((0, 0, 0, 1, 2, 3)) if state is None else state,
            random_.choice((0, 1, 2, 7)),
        )
        struct.pack_into("<I", data, 0x20, random_.choice((NONE, 0, 3, 5)))
        for index in range(3):
            struct.pack_into("<I", data, 0x40 + index * 4, mock_address(index) if random_.random() < 0.8 else 0)
        return Buffer(bytes(data), pointers=(0,) if has else ())

    def tasks() -> dict:
        started = random_.choice((0, 3, 7, 7, NONE))
        normal = lambda harness, arguments, started=started: (
            started,
            (harness.normalize(arguments[0]), arguments[1], arguments[2]),
        )
        return {"network_blob_task_start_read": normal, "network_blob_task_start_write": normal}

    for n in range(3000):
        table = bytearray(random_.randbytes(32 * 0x14))
        for index in range(32):
            struct.pack_into("<I", table, index * 0x14 + 4, random_.choice((NONE, NONE, 0x10005, index)))
        t.check(
            "network_blob_get_progress",
            a_blob(),
            Buffer(random_.randbytes(4)),
            globals={"unknown_004d8c28": bytes(table)},
            externals={
                "unknown_000b49a0": lambda harness, arguments, known=random_.choice((0, 1)): (
                    known,
                    (arguments[0], harness.normalize(arguments[1])),
                ),
            },
        )
        for name in ("network_blob_start_read", "network_blob_start_write"):
            t.check(
                name,
                a_blob(),
                random_.getrandbits(32),
                random_.getrandbits(32),
                random_.getrandbits(32),
                random_.getrandbits(32),
                random_.getrandbits(16) | random_.choice((0, 0xABCD0000)),
                externals=tasks(),
            )
        says = [random_.choice((0, 1, 1, 0x100)) for index in range(3)]
        t.check(
            "network_blob_task_finished",
            a_blob(state=random_.choice((1, 1, 2, 2, 0, 3))),
            random_.choice((1, 1, 1, 6, 7, 7, 0, 5, 0x10001, NONE)),
            globals={"unknown_0055e700": random_.getrandbits(32)},
            externals=tasks(),
            mocks=[
                Mock(
                    lambda call, said=said: said,
                    stack_arguments=1,
                    record=lambda call: (t.harness.normalize(call.stack[0]),),
                )
                for said in says
            ],
        )
        t.check(
            "network_blob_memory_stolen",
            random_.getrandbits(32),
            a_blob(functions=False),
            externals={
                "network_blob_task_end": lambda harness, arguments: (0, tuple(arguments)),
                "texture_cache_return_memory": lambda harness, arguments: (0, (arguments[0],)),
            },
        )

    # ---------- what has a block takes its memory

    from test_async import calls_so_far

    REGISTERS = PHYSICAL_ADDRESS + 0x100000
    TAKEN = PHYSICAL_ADDRESS + 0x200000

    for n in range(400):
        blob = bytearray(random_.randbytes(0x30))
        struct.pack_into("<I", blob, 0x24, random_.choice((0, 0, 0, 0, 0x80123000)))
        struct.pack_into("<i", blob, 0x2C, random_.choice((0x40, 0x1000, 0x10, 0xF, 0, 0x7FFFFFFF, -1, -0x80000000)))
        # what the cache has, of which only its count of pages is looked at; and the device, with its registers
        cache = bytearray(random_.randbytes(0x40))
        struct.pack_into("<i", cache, 0x30, random_.choice((8, 8, 8, 0, -1)))
        device = bytearray(random_.randbytes(0x940))
        struct.pack_into("<I", device, 0x934, REGISTERS)
        same = random_.getrandbits(32)
        # how many times the cache has no memory before it has it: not for so long that it gives up
        fails = random_.choice((0, 0, 1, 5, 89, 90, 91, 95))

        def steal(harness, arguments, fails=fails):
            taken = calls_so_far(harness, "texture_cache_steal_memory_now") >= fails
            return (TAKEN if taken else 0), (arguments[0], harness.normalize(arguments[1]), arguments[2], harness.normalize(arguments[3]))

        t.check(
            "network_blob_take_memory",
            Buffer(bytes(blob)),
            random_.choice((Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4)), 0)),
            random_.choice((Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4)), 0)),
            globals={
                "texture_cache_globals": random_.randbytes(0x10) + bytes(4) + random_.randbytes(0x24),
                "texture_cache_globals+0x10": Buffer(bytes(cache)),
                "d3d_device": Buffer(bytes(device)),
            },
            memory={REGISTERS + 0x3240: Memory(struct.pack("<II", same, random_.choice((same, same ^ 1))))},
            externals={
                "texture_cache_steal_memory_now": steal,
                "texture_cache_idle": lambda harness, arguments: (0, ()),
                "D3DDevice_KickOff": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "SwitchToThread": lambda harness, arguments: (0, ()),
            },
        )
