# source/networking/replication_entity_view.c

import random
import struct

from difftest import Buffer, Mock, Pointer, mock_address

NONE = 0xFFFFFFFF
COUNT = 0x400
# in one buffer: what keeps the things; what they are of, with its functions; the view; its functions; and the things
# that it has sent, each with its number and the next one
MANAGER, CLIENT, CLIENT_TABLE, VIEW, VIEW_TABLE, SENT, END = 0x0, 0x2048, 0x204C, 0x2090, 0x70D0, 0x70F4, 0x7114
VIEW_SIZE = 0x5040


def run(t) -> None:
    random_ = random.Random(425)

    def world(fill: float | None = None, sent: int | None = None) -> Buffer:
        if fill is None:
            fill = random_.choice((0.0, 0.05, 0.5, 1.0))
        data = bytearray(random_.randbytes(END))
        pointers = [MANAGER, CLIENT, VIEW, VIEW + 0x14]
        struct.pack_into("<I", data, MANAGER, CLIENT)
        struct.pack_into("<I", data, CLIENT, CLIENT_TABLE)
        struct.pack_into("<16I", data, CLIENT_TABLE, *(0x11 * (n + 1) for n in range(12)), mock_address(0), 0xEE, 0xFF, 0x111)
        struct.pack_into("<I", data, VIEW, VIEW_TABLE)
        struct.pack_into("<9I", data, VIEW_TABLE, *(0x11 * (n + 1) for n in range(8)), mock_address(1))
        index = random_.choice((0, 3, 14, 15, 31, 35))
        struct.pack_into("<I", data, VIEW + 0xC, index)
        struct.pack_into("<I", data, VIEW + 0x14, MANAGER)
        if sent is None:
            sent = random_.choice((0, 0, 1, 4))
        struct.pack_into("<I", data, VIEW + 0x18, SENT if sent else 0)
        if sent:
            pointers.append(VIEW + 0x18)
        for node in range(sent):
            struct.pack_into("<II", data, SENT + node * 8, random_.getrandbits(32), SENT + (node + 1) * 8 if node + 1 < sent else 0)
            if node + 1 < sent:
                pointers.append(SENT + node * 8 + 4)
        for place in range(COUNT):
            # the count of what still has a thing: with the bit of the view only, with others, or without it
            bit = 1 << (index & 15) if index & 31 < 16 else 0
            struct.pack_into("<H", data, MANAGER + 0x46 + place * 8, random_.choice((0, bit, bit, bit | 0x8000 >> (index & 7), random_.getrandbits(16))))
            # the thing in the view: of a place of its own or of another, in a state or in none
            struct.pack_into(
                "<IIHhIH",
                data,
                VIEW + 0x24 + place * 20,
                random_.choice((place, place, random_.randrange(COUNT))) | random_.getrandbits(4) << 28,
                random_.choice((0, 0, 1, random_.getrandbits(32))),
                random_.choice((0, 1, 2, 3, 3)) if random_.random() < fill else 0,
                random_.getrandbits(15),
                random_.getrandbits(32),
                random_.getrandbits(16),
            )
        return Buffer(bytes(data), pointers=tuple(pointers))

    def mocks() -> list:
        def unsent(call):
            # the view has one less of the things that it has sent: the next one is the first
            mu = t.harness.mu
            view = call.registers["ecx"]
            first = struct.unpack("<I", mu.mem_read(view + 0x18, 4))[0]
            mu.mem_write(view + 0x18, bytes(mu.mem_read(first + 4, 4)))
            return 0

        return [Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)), Mock(unsent, stack_arguments=2, registers=("ecx",))]

    # ---------- a view starts over

    for n in range(80):
        data = world()
        t.check("replication_entity_view_reset", Pointer(data, VIEW), extra=[data], mocks=mocks())

    # ---------- a view has more to say of a thing, and is told that one is being done with

    for n in range(600):
        data = world(1.0, 0)
        raw = bytearray(data.data)
        entity = random_.choice((0, 0x3FF, random_.randrange(COUNT))) | random_.getrandbits(22) << 10
        place = entity & 0x3FF
        struct.pack_into("<H", raw, VIEW + 0x24 + place * 20 + 8, random_.choice((0, 1, 1, 2, 3, 3)))
        struct.pack_into("<i", raw, VIEW + 0x5034, random_.choice((0, 1, 7)))
        data = Buffer(bytes(raw), pointers=data.pointers)
        t.check(
            "replication_entity_view_mark_entity",
            entity,
            Pointer(data, VIEW),
            random_.choice((0, 1, 0x80000000, random_.getrandbits(32))),
            extra=[data],
        )

        def state(harness, arguments):
            # the thing is in the state in the view, and the view is what is given back
            harness.mu.mem_write(arguments[0] + 0x24 + (arguments[1] & 0x3FF) * 20 + 8, struct.pack("<H", arguments[2]))
            return arguments[0], (harness.normalize(arguments[0]), arguments[1], arguments[2])

        t.check(
            "replication_entity_view_delete_entity",
            Pointer(data, VIEW),
            entity,
            extra=[data],
            externals={"replication_entity_view_set_entity_state": state},
        )

    # ---------- a view is set up, and the things that are in it

    for n in range(40):
        t.check("replication_entity_view_new", random_.getrandbits(32), random_.choice((0, 1, 14, 31, 32, 37)), Buffer(random_.randbytes(VIEW_SIZE)))
    for n in range(300):
        data = world(random_.choice((0.0, 0.002, 0.01, 0.5)), 0)
        t.check(
            "replication_entity_view_next_entity",
            random_.choice((NONE, 0, 0x3FE, 0x3FF, random_.randrange(COUNT) | random_.getrandbits(22) << 10)),
            Pointer(data, VIEW),
            extra=[data],
        )
    for n in range(600):
        data = world(1.0, 0)
        raw = bytearray(data.data)
        place = random_.choice((0, 0x3FF, random_.randrange(COUNT)))
        entity = struct.unpack_from("<I", raw, VIEW + 0x24 + place * 20)[0] if random_.random() < 0.7 else place | random_.getrandbits(22) << 10
        place = entity & 0x3FF
        raw[MANAGER + 0x44 + place * 8] = random_.choice((0, 2, 4, 4, 6, 0xFF, 0xFB))
        struct.pack_into("<H", raw, VIEW + 0x24 + place * 20 + 8, random_.choice((0, 1, 2, 3, 3, 3)))
        # the first of the things that are in the view: this one, or another
        if random_.random() < 0.4:
            struct.pack_into("<I", raw, VIEW + 0x5024, place)
        data = Buffer(bytes(raw), pointers=data.pointers)
        t.check("replication_entity_view_unknown_00099640", entity, Pointer(data, VIEW), extra=[data])
        t.check(
            "replication_entity_view_set_entity_state",
            Pointer(data, VIEW),
            entity,
            random_.choice((0, 0, 1, 2, 3, 3, 4)),
            extra=[data],
        )

    # ---------- what a view writes in a packet, and what it keeps of it

    # after the rest: the heap of the network, what gives its memory with its functions, how many times it was asked
    # and what it answers, the memory that it gives, three packets and six things said in them
    HEAP, ALLOCATOR, TABLE, ASKED, ANSWERS, GIVEN, PACKETS, UPDATES, END2 = (END + offset for offset in (0, 8, 0xC, 0x38, 0x3C, 0x40, 0x50, 0x80, 0xE0))

    def sending(data: Buffer, entity: int) -> Buffer:
        # and after it all, more memory for the heap to give: a piece of its own each time
        raw = bytearray(data.data) + bytearray(random_.randbytes(END2 - END + 0x200))
        pointers = [*(pointer for pointer in data.pointers if pointer != VIEW + 0x18), HEAP, ALLOCATOR, VIEW + 0x1C]
        struct.pack_into("<I", raw, CLIENT_TABLE, mock_address(4))
        struct.pack_into("<Ii", raw, HEAP, ALLOCATOR, random_.choice((0, 5)))
        struct.pack_into("<I", raw, ALLOCATOR, TABLE)
        struct.pack_into("<11I", raw, TABLE, 0x11, 0x22, 0x33, 0x44, 0x55, mock_address(2), 0x77, 0x88, 0x99, 0xAA, mock_address(3))
        struct.pack_into("<I4B", raw, ASKED, 0, *random_.choice(((1, 1, 1, 1), (1, 1, 1, 1), (0, 1, 1, 1), (0, 0, 1, 1), (1, 0, 0, 1))))
        # the packets that the view waits to hear of, one after the other; the one that it writes is one of them, or not
        waiting = random_.choice((0, 1, 2, 3))
        struct.pack_into("<I", raw, VIEW + 0x18, PACKETS if waiting else 0)
        if waiting:
            pointers.append(VIEW + 0x18)
        struct.pack_into("<I", raw, VIEW + 0x1C, PACKETS + random_.choice((0, 0, 2)) * 0x10)
        said = list(range(6))
        random_.shuffle(said)
        for packet in range(3):
            mine = [said.pop() for update in range(random_.choice((0, 1, 2)))]
            struct.pack_into("<4I", raw, PACKETS + packet * 0x10, random_.getrandbits(32), 0, UPDATES + mine[0] * 0x10 if mine else 0, PACKETS + (packet + 1) * 0x10 if packet + 1 < waiting else 0)
            if mine:
                pointers.append(PACKETS + packet * 0x10 + 8)
            if packet + 1 < waiting:
                pointers.append(PACKETS + packet * 0x10 + 0xC)
            for place, update in enumerate(mine):
                after = mine[place + 1] if place + 1 < len(mine) else None
                struct.pack_into("<4I", raw, UPDATES + update * 0x10, random_.choice((entity, entity, random_.getrandbits(32))), random_.getrandbits(32), 0, 0 if after is None else UPDATES + after * 0x10)
                if after is not None:
                    pointers.append(UPDATES + update * 0x10 + 0xC)
        return Buffer(bytes(raw), pointers=tuple(pointers))

    def sending_mocks(says: int = 1, wrote: int = 0, bits: int = 0) -> list:
        def gives(call):
            mu = t.harness.mu
            base = call.registers["ecx"] - ALLOCATOR
            asked = struct.unpack("<I", mu.mem_read(base + ASKED, 4))[0]
            mu.mem_write(base + ASKED, struct.pack("<I", asked + 1))
            return base + END2 + (asked & 0x1F) * 0x10 if mu.mem_read(base + ANSWERS + (asked & 3), 1)[0] else 0

        def writes(call):
            # what the things are of writes the thing: so many bits, and says which bits of how it is it wrote
            mu = t.harness.mu
            position = struct.unpack("<i", mu.mem_read(call.stack[3] + 0x10, 4))[0]
            mu.mem_write(call.stack[3] + 0x10, struct.pack("<i", position + bits))
            mu.mem_write(call.stack[5], struct.pack("<I", wrote))
            return says

        def writes_record(call):
            # where the bits go is in the stack
            return call.registers["ecx"], *call.stack[:5]

        return [
            *mocks(),
            Mock(gives, stack_arguments=3, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            Mock(writes, stack_arguments=6, registers=("ecx",), record=writes_record),
        ]

    def number(harness, arguments):
        # the number of the thing is written: fourteen bits
        position = struct.unpack("<i", harness.mu.mem_read(arguments[1] + 0x10, 4))[0]
        harness.mu.mem_write(arguments[1] + 0x10, struct.pack("<i", position + 14))
        return 0, (arguments[0], harness.normalize(arguments[1]))

    def at_time() -> dict:
        now = random_.getrandbits(32)
        while 0x20000000 <= now < 0x21000000:
            now = random_.getrandbits(32)
        return {
            "globals": {"unknown_00510548": bytes((random_.choice((0, 0, 1)),)), "unknown_0051054c": struct.pack("<I", now)},
            "externals": {"GetTickCount": lambda harness, arguments, now=now: now},
        }

    def a_stream() -> Buffer:
        size = random_.choice((4, 8, 16))
        position = random_.choice((0, 0, 5, size * 4, size * 8 - 20, size * 8 - 17, size * 8 - 3))
        kept = random_.choice((0, 1, 3))
        header = struct.pack("<I4iB3x7i", 0x34, size, 1, 1, position, 0, kept, *(random_.randrange(position + 1) for n in range(4)), 0, 0)
        return Buffer(header + bytes(size) + random_.randbytes(8), pointers=(0,))

    for n in range(700):
        setup = at_time()
        base = world(1.0, 0)
        raw = bytearray(base.data)
        place = random_.choice((0, 0x3FF, random_.randrange(COUNT)))
        entity = place | random_.getrandbits(4) << 28
        struct.pack_into("<IIH", raw, VIEW + 0x24 + place * 20, entity, random_.choice((0, 1, 0xF0, random_.getrandbits(32))), random_.choice((0, 1, 1, 2, 3, 3)))
        data = sending(Buffer(bytes(raw), pointers=base.pointers), entity)
        globals_ = dict(setup["globals"], unknown_004d87f8=Pointer(data, HEAP))
        externals = dict(setup["externals"], unknown_000b5650=number)
        wrote = random_.choice((0, 0, 1, 0xF0, random_.getrandbits(32)))
        made = sending_mocks(random_.choice((1, 1, 1, 0, 0x100)), wrote, random_.choice((0, 3, 40)))
        after = random_.choice((0, 0, 1, 15, 47, 64))
        t.check("replication_entity_view_sent_creation", entity, Pointer(data, VIEW), extra=[data], globals=globals_, externals=externals, mocks=made)
        t.check("replication_entity_view_sent_deletion", Pointer(data, VIEW), entity, extra=[data], globals=globals_, externals=externals, mocks=made)
        t.check(
            "replication_entity_view_sent_update",
            entity,
            Pointer(data, VIEW),
            random_.choice((0, 1, 0xF0, NONE, random_.getrandbits(32))),
            extra=[data],
            globals=globals_,
            externals=externals,
            mocks=made,
        )
        t.check(
            "replication_entity_view_write_deletion",
            place,
            a_stream(),
            Pointer(data, VIEW),
            after,
            extra=[data],
            globals=globals_,
            externals=externals,
            mocks=made,
        )
        t.check(
            "replication_entity_view_write_creation",
            a_stream(),
            Pointer(data, VIEW),
            place,
            random_.getrandbits(32),
            after,
            extra=[data],
            globals=globals_,
            externals=externals,
            mocks=made,
        )

    # ---------- the things that were made at once

    for n in range(700):
        setup = at_time()
        base = world(1.0, 0)
        raw = bytearray(base.data)
        count = random_.choice((0, 1, 2, 2, 3, 4, 4))
        places = random_.sample(range(COUNT), max(count, 1))
        entities = [place | random_.getrandbits(4) << 28 for place in places]
        for position, place in enumerate(places):
            struct.pack_into("<IIH", raw, VIEW + 0x24 + place * 20, entities[position], random_.choice((0, 1, random_.getrandbits(32))), random_.choice((0, 1, 1, 2, 3, 3)))
            struct.pack_into("<I", raw, MANAGER + 0x48 + place * 8, entities[position + 1] if position + 1 < len(places) else NONE)
        if count == 0:
            # no thing in the place
            struct.pack_into("<I", raw, VIEW + 0x24 + places[0] * 20, NONE)
        data = sending(Buffer(bytes(raw), pointers=base.pointers), entities[-1])
        globals_ = dict(setup["globals"], unknown_004d87f8=Pointer(data, HEAP))
        externals = dict(setup["externals"], unknown_000b5650=number)

        def writes(call, bits=random_.choice((0, 3, 20)), bad=random_.choice((NONE, NONE, NONE, entities[-1], entities[0]))):
            # what the things are of writes each thing, and says which bits it wrote; it says no for one of them
            mu = t.harness.mu
            position = struct.unpack("<i", mu.mem_read(call.stack[3] + 0x10, 4))[0]
            mu.mem_write(call.stack[3] + 0x10, struct.pack("<i", position + bits))
            mu.mem_write(call.stack[5], struct.pack("<I", call.stack[0] & 0x30))
            return call.stack[0] != bad

        made = sending_mocks()
        made[4] = Mock(writes, stack_arguments=6, registers=("ecx",), record=lambda call: (call.registers["ecx"], *call.stack[:5]))
        after = random_.choice((0, 0, 1, 15, 47, 64))
        t.check(
            "replication_entity_view_write_chain_creation",
            places[0],
            a_stream(),
            Pointer(data, VIEW),
            random_.getrandbits(32),
            after,
            extra=[data],
            globals=globals_,
            externals=externals,
            mocks=made,
        )
        t.check(
            "replication_entity_view_write_chain_deletion",
            places[0],
            a_stream(),
            Pointer(data, VIEW),
            after,
            extra=[data],
            globals=globals_,
            externals=externals,
            mocks=made,
        )
