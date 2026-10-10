# source/networking/simulation_entity_database.c

import random
import struct

from difftest import Buffer, Mock, Pointer, mock_address

NONE = 0xFFFFFFFF
COUNT = 0x400
ENTITY = 0x20
# in one buffer: what the things are; the kinds that there are, four kinds and their functions; the heap of the
# network, and what gives its memory with its functions
DATABASE, TYPES, KINDS, KIND_TABLE, HEAP, ALLOCATOR, TABLE, END = 0x0, 0x8014, 0x8098, 0x80A8, 0x8110, 0x8118, 0x8120, 0x8454
# and after them: how many times what gives the memory was asked, what it answers, and the memory that it gives
ASKED, ANSWERS, GIVEN = 0x814C, 0x8150, 0x8154
# how a thing is, to copy from; and the memory that is given every other time
SOURCE, GIVEN2 = 0x8254, 0x8354
# the kinds that a packet can say
ALL_KINDS = 32
KIND_COUNT = 4


def run(t) -> None:
    random_ = random.Random(420)

    def world(fill: float | None = None, initialized: int | None = None, manager: float | None = None) -> Buffer:
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
        for kind in range(KIND_COUNT, ALL_KINDS):
            # the rest of the kinds: there are some, which are the first ones again
            if random_.random() < 0.6:
                struct.pack_into("<I", data, TYPES + 4 + kind * 4, KINDS + (kind % KIND_COUNT) * 4)
                pointers.append(TYPES + 4 + kind * 4)
            else:
                struct.pack_into("<I", data, TYPES + 4 + kind * 4, 0)
        struct.pack_into("<21I", data, KIND_TABLE, *(0x11 * (n + 1) for n in range(20)), mock_address(2))
        struct.pack_into("<5I", data, KIND_TABLE + 0x54, *(mock_address(n) for n in range(14, 19)))
        struct.pack_into("<I", data, KIND_TABLE + 0x3C, mock_address(19))
        struct.pack_into("<I", data, KIND_TABLE + 0x34, mock_address(20))
        struct.pack_into("<3I", data, KIND_TABLE + 8, mock_address(3), mock_address(4), mock_address(5))
        struct.pack_into("<2I", data, KIND_TABLE + 0x48, mock_address(6), mock_address(7))
        struct.pack_into("<I4B", data, ASKED, 0, *random_.choice(((1, 1, 1, 1), (1, 1, 1, 1), (0, 1, 0, 0), (0, 0, 1, 1), (0, 0, 0, 0))))
        struct.pack_into("<Ii", data, HEAP, ALLOCATOR, random_.choice((0, 5, 5000)))
        struct.pack_into("<I", data, ALLOCATOR, TABLE)
        struct.pack_into("<11I", data, TABLE, mock_address(0), mock_address(1), 0x33, 0x44, 0x55, mock_address(8), 0x77, 0x88, 0x99, 0xAA, mock_address(9))
        for index in range(COUNT):
            there = random_.random() < fill
            struct.pack_into(
                "<IhBBIIIIII",
                data,
                0x14 + index * ENTITY,
                # its number: its place, with the count of its place or with any other bits
                (index | random_.choice((random_.getrandbits(4) << 28, random_.getrandbits(4) << 28, random_.getrandbits(21) << 10))) if there else NONE,
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
        struct.pack_into("<3I", data, KIND_TABLE + 0x14, mock_address(10), 0x77, 0x88)
        struct.pack_into("<I", data, KIND_TABLE + 0x28, mock_address(11))
        struct.pack_into("<I", data, KIND_TABLE + 0x30, mock_address(12))
        struct.pack_into("<I", data, KIND_TABLE + 0x38, mock_address(13))
        if manager is not None:
            # what keeps the things, after the rest: with no views, and so many of its places in use
            struct.pack_into("<I", data, 0xC, END)
            pointers.append(0xC)
            made = bytearray(random_.randbytes(0x2048))
            made[0:0x44] = bytes(0x44)
            for index in range(COUNT):
                made[0x44 + index * 8] = (made[0x44 + index * 8] & ~1) | (random_.random() < manager)
            struct.pack_into("<i", made, 0x2044, random_.randrange(COUNT))
            data += made
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

    # ---------- a thing is there, what a new console is told of it, and memory for a thing

    def all_mocks(state_size: int, creation_size: int, bit_count: int, says: int = 1) -> list:
        def builds(call):
            # the kind builds how the thing is, where it is told to: in the stack, or a number that is not looked at
            if call.stack[2] and call.stack[3] < 0x2000 or call.stack[3] > 0x10000:
                pass
            if 0 < call.stack[2] <= 0x400 and call.stack[3] > 0x100000:
                t.harness.mu.mem_write(call.stack[3], bytes((0xA5,)) * min(call.stack[2], 8))
            return 0

        def builds_record(call):
            return call.registers["ecx"], call.stack[0], call.stack[1], call.stack[2]

        def update(call):
            # the kind takes a bit away, and says so or not; where the bits are and how the thing is are in the stack
            mu = t.harness.mu
            flags = struct.unpack("<I", mu.mem_read(call.stack[1], 4))[0]
            mu.mem_write(call.stack[1], struct.pack("<I", flags & ~2))
            return says

        def update_record(call):
            mu = t.harness.mu
            state = bytes(mu.mem_read(call.stack[3], min(call.stack[2], 0x400))) if call.stack[3] > 0x100000 and 0 < call.stack[2] <= 0x400 else None
            return call.registers["ecx"], call.stack[0], bytes(mu.mem_read(call.stack[1], 4)), call.stack[2], state

        def gives(call):
            mu = t.harness.mu
            base = call.registers["ecx"] - ALLOCATOR
            asked = struct.unpack("<I", mu.mem_read(base + ASKED, 4))[0]
            mu.mem_write(base + ASKED, struct.pack("<I", asked + 1))
            return base + (GIVEN, GIVEN2)[asked & 1] if mu.mem_read(base + ANSWERS + (asked & 3), 1)[0] else 0

        return [
            *heap_mocks(),
            Mock(update, stack_arguments=4, registers=("ecx",), record=update_record),
            Mock(lambda call: state_size, stack_arguments=0, registers=("ecx",)),
            Mock(lambda call: creation_size, stack_arguments=0, registers=("ecx",)),
            Mock(lambda call: bit_count, stack_arguments=0, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=3, registers=("ecx",)),
            Mock(builds, stack_arguments=4, registers=("ecx",), record=builds_record),
            Mock(gives, stack_arguments=3, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
        ]

    for n in range(300):
        sizes = (random_.choice((0, 1, 7, 0x40, 0x100)), random_.choice((0, 0, NONE, 5, 0x40, 0x100)))
        bit_count = random_.choice((0, 1, 5, 16, 31, 32, 37))
        says = random_.choice((0, 1, 1, 0x100))
        entity = random_.choice((0, 0x3FF, random_.randrange(COUNT))) | random_.getrandbits(22) << 10
        t.check("simulation_entity_database_activate_entity", entity, world(), mocks=all_mocks(*sizes, bit_count, says))
        t.check("simulation_entity_database_get_initial_update", world(), entity, mocks=all_mocks(*sizes, bit_count, says))
        for name in ("simulation_entity_database_allocate_creation_data", "simulation_entity_database_allocate_state_data"):
            data = world()
            t.check(
                name,
                data,
                random_.randrange(KIND_COUNT),
                Buffer(random_.randbytes(4)),
                Buffer(random_.randbytes(4)),
                globals={"unknown_004d87f8": Pointer(data, HEAP)},
                mocks=all_mocks(*sizes, bit_count, says),
            )

    # ---------- a new thing

    for n in range(400):
        data = world(manager=random_.choice((0.0, 0.5, 0.999, 1.0, 1.0)))
        t.check(
            "simulation_entity_database_create_entity",
            data,
            random_.randrange(KIND_COUNT),
            globals={"unknown_004d87f8": Pointer(data, HEAP)},
            mocks=all_mocks(random_.choice((0, 1, 7, 0x40, 0x100)), random_.choice((0, 0, NONE, 5, 0x40, 0x100)), 0),
        )

    # ---------- the functions that only the table has

    def a_packet() -> Buffer:
        size = random_.choice((1, 4, 8, 0x40))
        position = random_.choice((0, 0, 3, 8, size * 8 - 6, size * 8 - 5, size * 8 - 4, size * 8 - 1, size * 8, size * 8 + 2))
        header = struct.pack("<I4iB3x7i", 0x34, size, 1, 1, max(position, 0), 0, *(random_.getrandbits(8) for n in range(7)))
        return Buffer(header + bytes(size) + random_.randbytes(8), pointers=(0,))

    for n in range(500):
        entity = random_.choice((0, 0x3FF, random_.randrange(COUNT))) | random_.getrandbits(22) << 10
        flags = random_.choice((0, 1, 6, 0xFF, random_.getrandbits(32)))
        all_flags = random_.choice((0, 1, 0xFF, 0xF0, NONE))

        def encodes(call, how=random_.choice((0, 0, 0, 1, 2, 3, 4)), says=random_.choice((1, 1, 1, 0, 0x100))):
            # the kind says which bits it did: the ones that it was asked for, fewer, none, or others
            # or it says nothing of that, and what was there is what there is
            if how < 4:
                done = (call.stack[1], call.stack[1] & 0xFFFFFFFE, 0, call.stack[1] ^ 0x10)[how]
                t.harness.mu.mem_write(call.stack[2], struct.pack("<I", done))
            return says

        mocks = [
            *all_mocks(0x40, 0x40, 5),
            Mock(lambda call: all_flags, stack_arguments=0, registers=("ecx",)),
            Mock(lambda call, value=random_.getrandbits(32): value, stack_arguments=4, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=4, registers=("ecx",)),
            # the first is a byte: the low one of what is in the stack
            Mock(encodes, stack_arguments=8, registers=("ecx",), record=lambda call: (call.registers["ecx"], call.stack[0] & 0xFF, *call.stack[1:])),
        ]
        data = world()
        t.check(
            "simulation_entity_database_unknown_0008a580",
            data,
            entity,
            *(random_.getrandbits(32) for value in range(3)),
            mocks=mocks,
        )
        t.check(
            "simulation_entity_database_read_update",
            data,
            entity,
            flags,
            random_.getrandbits(32),
            a_packet(),
            random_.getrandbits(32),
            Buffer(random_.randbytes(4)),
            mocks=mocks,
        )
        t.check(
            "simulation_entity_database_write_creation",
            data,
            entity,
            flags,
            random_.getrandbits(32),
            a_packet(),
            random_.getrandbits(32),
            Buffer(random_.randbytes(4)),
            mocks=mocks,
        )

    # ---------- some new things at once

    def table_mocks(sizes=(0x40, 0x40), says: int = 1, value: int = 0) -> list:
        """All the functions that there are, in their places."""
        return [
            *all_mocks(*sizes, 5),
            Mock(lambda call: 0xFF, stack_arguments=0, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=4, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=4, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=8, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            Mock(lambda call: says, stack_arguments=6, registers=("ecx",)),
            Mock(lambda call: value, stack_arguments=4, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            Mock(lambda call: says, stack_arguments=1, registers=("ecx",)),
        ]

    for n in range(400):
        data = world(manager=random_.choice((0.0, 0.5, 0.995, 0.999, 1.0)))
        count = random_.choice((0, 1, 2, 3, 4, 4))
        t.check(
            "simulation_entity_database_create_entities",
            Buffer(random_.randbytes(16)),
            data,
            count,
            Buffer(struct.pack("<4I", *(random_.randrange(KIND_COUNT) for kind in range(4)))),
            globals={"unknown_004d87f8": Pointer(data, HEAP)},
            mocks=table_mocks((random_.choice((0, 1, 0x40, 0x100)), random_.choice((0, NONE, 5, 0x100)))),
        )

    # ---------- more of the functions that only the table has

    for n in range(300):
        entity = random_.choice((0, 0x3FF, random_.randrange(COUNT))) | random_.getrandbits(22) << 10
        place = 0x14 + (entity & 0x3FF) * ENTITY
        # what the kind says is a byte: any that is not nothing is yes
        says = random_.choice((0, 1, 1, 0x80, 2, 0x100))
        t.check(
            "simulation_entity_database_created_remote",
            world(),
            entity,
            random_.randrange(KIND_COUNT),
            random_.getrandbits(32),
            random_.choice((2, 2, 2, 0, 1, 3)),
            Buffer(random_.randbytes(0x20)),
            mocks=table_mocks(says=says),
        )
        # how the thing is has a place: the memory that the heap gives, here
        data = world()
        raw = bytearray(data.data)
        size = random_.choice((0, 1, 3, 4, 0x40, 0x100))
        struct.pack_into("<II", raw, place + 0x18, size, GIVEN)
        update = struct.pack("<II", random_.getrandbits(32), 8) + random_.randbytes(size + 4)
        t.check(
            "simulation_entity_database_set_state",
            Buffer(bytes(raw), pointers=(*data.pointers, place + 0x1C)),
            entity,
            random_.getrandbits(32),
            random_.getrandbits(32),
            Buffer(update, pointers=(4,)),
            mocks=table_mocks(value=random_.getrandbits(32)),
        )
        raw = bytearray(world().data)
        raw[place + 7] = random_.choice((0, 1, 0x7F, 0x80, 0xFF))
        t.check(
            "simulation_entity_database_unknown_0008aca0",
            Buffer(bytes(raw), pointers=data.pointers),
            entity,
            random_.getrandbits(32),
            Buffer(random_.randbytes(4)),
        )
        t.check("simulation_entity_database_deleting", world(), entity, mocks=table_mocks())
        t.check("simulation_entity_database_adopt", world(), entity, mocks=table_mocks(says=says))
        data = world(random_.choice((0.5, 1.0)))
        t.check(
            "simulation_entity_database_deleted",
            data,
            entity,
            globals={"unknown_004d87f8": Pointer(data, HEAP)},
            mocks=table_mocks(),
        )
    for n in range(60):
        t.check("simulation_entity_database_adopted", world(), mocks=table_mocks())

    # ---------- a thing is read from a packet

    def reading_mocks(sizes, all_flags: int, says: tuple, how: int) -> list:
        made = table_mocks(sizes)

        def decodes(call):
            # the kind reads some bits: all that there are, one of them, none, or one that there is not
            done = (all_flags, all_flags & 0x10, 0, all_flags ^ 0x100)[how]
            t.harness.mu.mem_write(call.stack[1], struct.pack("<I", done))
            return says[2]

        def decodes_record(call):
            # where the bits go is in the stack; the first is a byte
            return call.registers["ecx"], call.stack[0] & 0xFF, bytes(t.harness.mu.mem_read(call.stack[1], 4)), *call.stack[2:]

        made[7] = Mock(lambda call: says[1], stack_arguments=4, registers=("ecx",))
        made[10] = Mock(lambda call: all_flags, stack_arguments=0, registers=("ecx",))
        return [
            *made,
            Mock(decodes, stack_arguments=5, registers=("ecx",), record=decodes_record),
            Mock(lambda call: says[0], stack_arguments=3, registers=("ecx",)),
        ]

    def a_stream(state: int) -> Buffer:
        size = random_.choice((1, 8, 8))
        header = struct.pack("<I4iB3x7i", 0x34, size, 1, state, random_.choice((0, 0, 3, size * 8 - 5, size * 8)), 0, *(random_.getrandbits(8) for n in range(7)))
        return Buffer(header + random_.randbytes(size) + random_.randbytes(8), pointers=(0,))

    for n in range(700):
        sizes = (random_.choice((0, 1, 0x40, 0x100)), random_.choice((0, 0, NONE, 5, 0x100)))
        says = tuple(random_.choice((1, 1, 1, 1, 0, 0x100)) for value in range(3))
        mocks = reading_mocks(sizes, random_.choice((0, 0xFF, 0xFF, 0x1F0, NONE)), says, random_.choice((0, 0, 1, 2, 3)))
        data = world()
        t.check(
            "simulation_entity_database_decode_creation",
            data,
            random_.getrandbits(32),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            random_.getrandbits(32),
            Buffer(struct.pack("<i", random_.choice((0, 1, 2)))),
            Buffer(random_.randbytes(0x28)),
            a_stream(3),
            globals={"unknown_004d87f8": Pointer(data, HEAP)},
            mocks=mocks,
        )
        # a thing that there is, mostly, with how it is in a place
        raw = bytearray(data.data)
        place = 0x14 + random_.randrange(COUNT) * ENTITY
        entity = (place - 0x14) // ENTITY | random_.getrandbits(4) << 28
        there = random_.choice((entity, entity, entity, entity ^ 0x10000000, NONE))
        struct.pack_into("<I", raw, place, there)
        struct.pack_into("<II", raw, place + 0x18, random_.choice((0, 1, 3, 0x40, 0x100)), SOURCE)
        t.check(
            "simulation_entity_database_decode_update",
            Buffer(bytes(raw), pointers=(*data.pointers, place + 0x1C)),
            random_.choice((entity, entity, entity, NONE)),
            Buffer(random_.randbytes(4)),
            random_.getrandbits(32),
            Buffer(struct.pack("<i", random_.choice((0, 1, 2)))),
            Buffer(random_.randbytes(0x20)),
            a_stream(random_.choice((4, 4, 3, 1))),
            globals={"unknown_004d87f8": Pointer(data, HEAP)},
            mocks=mocks,
        )

    # ---------- the heap gives memory

    for n in range(100):
        data = world()
        t.check(
            "network_heap_allocate_block",
            random_.choice((0, 1, 0x40, 0x100)),
            extra=[data],
            globals={"unknown_004d87f8": Pointer(data, HEAP)},
            mocks=table_mocks(),
        )
