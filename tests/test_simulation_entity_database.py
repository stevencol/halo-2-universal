# source/networking/simulation_entity_database.c

import random
import struct

from difftest import Buffer, Mock, Pointer, mock_address

NONE = 0xFFFFFFFF
COUNT = 0x400
ENTITY = 0x20
# in one buffer: what the things are; the kinds that there are, four kinds and their functions; the heap of the
# network, and what gives its memory with its functions
DATABASE, TYPES, KINDS, KIND_TABLE, HEAP, ALLOCATOR, TABLE, END = 0x0, 0x8014, 0x8028, 0x8038, 0x808C, 0x8094, 0x809C, 0x80C8
KIND_COUNT = 4


def run(t) -> None:
    random_ = random.Random(420)

    def world(fill: float | None = None, initialized: int | None = None) -> Buffer:
        if fill is None:
            fill = random_.choice((0.0, 0.02, 0.3, 1.0))
        data = bytearray(random_.randbytes(END))
        pointers = [0x10, HEAP, ALLOCATOR]
        data[4] = random_.choice((0, 1, 1, 0x80)) if initialized is None else initialized
        # what keeps the things is not looked at here: a number
        struct.pack_into("<II", data, 0xC, random_.getrandbits(32), TYPES)
        for kind in range(KIND_COUNT):
            struct.pack_into("<I", data, TYPES + 4 + kind * 4, KINDS + kind * 4)
            struct.pack_into("<I", data, KINDS + kind * 4, KIND_TABLE)
            pointers += [TYPES + 4 + kind * 4, KINDS + kind * 4]
        struct.pack_into("<21I", data, KIND_TABLE, *(0x11 * (n + 1) for n in range(20)), mock_address(2))
        struct.pack_into("<Ii", data, HEAP, ALLOCATOR, random_.choice((0, 5, 5000)))
        struct.pack_into("<I", data, ALLOCATOR, TABLE)
        struct.pack_into("<11I", data, TABLE, mock_address(0), mock_address(1), *(0x11 * (n + 3) for n in range(9)))
        for index in range(COUNT):
            there = random_.random() < fill
            struct.pack_into(
                "<IhBBIIIIII",
                data,
                0x14 + index * ENTITY,
                (index | random_.getrandbits(4) << 28) if there else NONE,
                random_.randrange(KIND_COUNT),
                random_.choice((0, 1, 1, 0x80)),
                random_.getrandbits(8),
                random_.getrandbits(32),
                random_.choice((0, 0, 1, random_.getrandbits(32))),
                random_.getrandbits(8),
                # what was said of it is not looked at here: numbers
                random_.choice((0, 0x1000 + index)),
                random_.getrandbits(8),
                random_.choice((0, 0x2000 + index)),
            )
        return Buffer(bytes(data), pointers=tuple(pointers))

    def heap_mocks() -> list:
        return [
            Mock(lambda call: 0, stack_arguments=2, registers=("ecx",)),
            # where the size goes is in the stack
            Mock(lambda call, known=random_.choice((0, 1)): known, stack_arguments=2, registers=("ecx",), record=lambda call: (call.registers["ecx"], call.stack[0])),
        ]

    # ---------- there are no things

    for n in range(60):
        data = world()
        t.check(
            "simulation_entity_database_reset",
            data,
            globals={"unknown_004d87f8": Pointer(data, HEAP)},
            mocks=heap_mocks(),
        )

    # ---------- a thing is no more, and one is set

    for n in range(300):
        data = world(random_.choice((0.5, 1.0)))
        t.check(
            "simulation_entity_database_free_entity",
            random_.choice((0, 0x3FF, random_.randrange(COUNT))) | random_.getrandbits(22) << 10,
            data,
            globals={"unknown_004d87f8": Pointer(data, HEAP)},
            mocks=heap_mocks(),
        )
        t.check(
            "simulation_entity_database_set_entity",
            random_.choice((0, 0x3FF, random_.randrange(COUNT))) | random_.getrandbits(22) << 10,
            world(),
            random_.choice((0, 1, 0xFFFF, random_.getrandbits(32))),
            *(random_.getrandbits(32) for value in range(4)),
        )

    # ---------- what has changed is told of

    for n in range(150):
        data = world()

        def update(call, kind=random_.choice((0, 1, 2, 2, 3))):
            # the kind says what of the thing is to be told: nothing, the same, some of it, or it says no
            mu = t.harness.mu
            flags = struct.unpack("<I", mu.mem_read(call.stack[1], 4))[0]
            if kind == 0:
                mu.mem_write(call.stack[1], bytes(4))
            elif kind == 2:
                mu.mem_write(call.stack[1], struct.pack("<I", flags & 0x0F0F0F0F))
            return (1, 1, 1, flags & 1)[kind]

        def update_record(call):
            # where the bits go is in the stack
            return call.registers["ecx"], call.stack[0], bytes(t.harness.mu.mem_read(call.stack[1], 4)), call.stack[2], call.stack[3]

        t.check(
            "simulation_entity_database_flush_updates",
            data,
            externals={"replication_entity_manager_mark_entity": lambda harness, arguments: (0, tuple(arguments))},
            mocks=[*heap_mocks(), Mock(update, stack_arguments=4, registers=("ecx",), record=update_record)],
        )
