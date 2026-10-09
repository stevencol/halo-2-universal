# source/networking/replication_entity_manager.c

import random
import struct

from difftest import Buffer, Mock, mock_address

NONE = 0xFFFFFFFF
COUNT = 0x400
VIEWS = 15
SIZE = 0x2048


def run(t) -> None:
    random_ = random.Random(416)

    def manager(fill: float | None = None, next_: int | None = None, chains: bool = False) -> Buffer:
        """What keeps the things, with some in use, and after it: what they are of, with its functions, and the views."""
        if fill is None:
            fill = random_.choice((0.0, 0.1, 0.5, 0.9, 0.99, 1.0))
        data = bytearray(random_.randbytes(SIZE))
        pointers = [0, SIZE]
        struct.pack_into("<I", data, 0, SIZE)
        mask = 0
        for view in range(VIEWS):
            if random_.random() < 0.6:
                struct.pack_into("<I", data, 4 + view * 4, SIZE + 0x80 + view * 0x10)
                pointers.append(4 + view * 4)
                if random_.random() < 0.7:
                    mask |= 1 << view
            else:
                struct.pack_into("<I", data, 4 + view * 4, 0)
        struct.pack_into("<I", data, 0x40, random_.choice((0, mask, mask)))
        for index in range(COUNT):
            used = random_.random() < fill
            flags = (random_.getrandbits(8) & ~1) | used
            if chains:
                # the first of some, and what it has after it: a few places on, and then no more
                flags = used | random_.choice((0, 0, 8, 8, 0x10))
                after = random_.choice((NONE, NONE, (index + random_.randrange(1, 4)) | random_.getrandbits(4) << 28))
                if after != NONE and (after & 0x3FF) >= COUNT - 1 or index >= COUNT - 4:
                    after = NONE
            else:
                after = random_.getrandbits(32)
            struct.pack_into("<BBhI", data, 0x44 + index * 8, flags, random_.getrandbits(8), random_.choice((0, 0, 1, -1)), after)
        if chains:
            # no thing is after itself: what a thing has after it is in a place after its own, so it ends
            pass
        struct.pack_into("<i", data, 0x2044, random_.choice((0, 0, 1, 0x200, 0x3FF, 0x400, random_.randrange(COUNT))) if next_ is None else next_)
        tail = struct.pack("<I", SIZE + 4) + struct.pack("<16I", 0x11, 0x22, mock_address(3), *(0x11 * (n + 4) for n in range(7)), mock_address(0), 0xCC, mock_address(1), 0xEE, 0xFF, mock_address(2))
        tail += bytes(0x80 - len(tail)) + random_.randbytes(VIEWS * 0x10)
        return Buffer(bytes(data) + tail, pointers=tuple(pointers))

    view_stubs = {
        "unknown_00097fe0": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
        "unknown_00099690": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1], arguments[2])),
        "unknown_000995c0": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
    }

    def client_mocks() -> list:
        return [Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)), Mock(lambda call: 0, stack_arguments=1, registers=("ecx",))]

    def an_entity() -> int:
        return random_.choice((0, 1, 0x3FF, random_.randrange(COUNT))) | random_.getrandbits(4) << 28

    # ---------- set up, and with no things

    for n in range(40):
        t.check("replication_entity_manager_new", manager(), externals=view_stubs)
        t.check("replication_entity_manager_reset", manager(), externals=view_stubs)

    # ---------- a place for a thing, and the thing of a place

    for n in range(400):
        t.check("replication_entity_manager_allocate_index", manager())
    for fill, next_ in ((1.0, 0), (1.0, 0x400), (1.0, 0x3FF), (0.0, 0x3FF), (0.0, 0x400), (0.0, 0)):
        t.check("replication_entity_manager_allocate_index", manager(fill, next_))
    for n in range(60):
        # one place that is not in use, before or after the next one
        data = manager(1.0, random_.choice((0, 5, 0x200, 0x3FF, 0x400)))
        free = random_.choice((0, 4, 5, 6, 0x1FF, 0x200, 0x3FE, 0x3FF))
        raw = bytearray(data.data)
        raw[0x44 + free * 8] &= ~1 & 0xFF
        t.check("replication_entity_manager_allocate_index", Buffer(bytes(raw), pointers=data.pointers))
    for n in range(300):
        t.check(
            "replication_entity_manager_create_entity",
            random_.choice((0, 1, 0x3FF, random_.randrange(COUNT))),
            manager(),
            externals=view_stubs,
        )

    # ---------- some things made at once

    for n in range(500):
        count = random_.choice((0, 1, 2, 3, 4, 4))
        t.check(
            "replication_entity_manager_create_entities",
            manager(random_.choice((0.0, 0.5, 0.99, 0.999, 1.0))),
            count,
            Buffer(random_.randbytes(16)),
            externals=view_stubs,
        )
    for free_count in (0, 1, 2, 3, 4, 5):
        for count in (1, 2, 3, 4):
            data = manager(1.0)
            raw = bytearray(data.data)
            for free in random_.sample(range(COUNT), free_count):
                raw[0x44 + free * 8] &= ~1 & 0xFF
            t.check(
                "replication_entity_manager_create_entities",
                Buffer(bytes(raw), pointers=data.pointers),
                count,
                Buffer(random_.randbytes(16)),
                externals=view_stubs,
            )

    # ---------- a thing is done with

    for n in range(400):
        t.check("replication_entity_manager_delete_entity", manager(), an_entity(), externals=view_stubs, mocks=client_mocks())
        t.check("replication_entity_manager_delete_entity_now", manager(), an_entity(), externals=view_stubs, mocks=client_mocks())

    # ---------- the things that were made at once

    for n in range(300):
        data = manager(chains=True)
        t.check("replication_entity_manager_get_chain", random_.choice((NONE, an_entity(), an_entity())), Buffer(bytes(0x1000)), data)
    for n in range(300):
        data = manager(random_.choice((0.02, 0.1, 0.5)), chains=True)
        raw = data.data
        # a thing that some thing has after it, mostly, or one that is the first
        wanted = [struct.unpack_from("<I", raw, 0x48 + index * 8)[0] for index in range(COUNT) if raw[0x44 + index * 8] & 1]
        wanted = [entity for entity in wanted if entity != NONE]
        firsts = [raw[0x45 + index * 8] << 28 | index for index in range(COUNT) if raw[0x44 + index * 8] & 9 == 9]
        entity = random_.choice((an_entity(), NONE, *(random_.choice(wanted) for place in range(3) if wanted), *(random_.choice(firsts) for place in range(2) if firsts)))
        t.check("replication_entity_manager_find_chain", data, Buffer(bytes(0x1000)), entity)

    # ---------- some things are done with at once

    def a_chain(data: Buffer, count: int, counted: int | None = None) -> tuple:
        """So many things that are one after the other, in places of their own."""
        raw = bytearray(data.data)
        places = random_.sample(range(COUNT), count)
        entities = [place | random_.getrandbits(4) << 28 for place in places]
        for position, place in enumerate(places):
            struct.pack_into("<I", raw, 0x48 + place * 8, entities[position + 1] if position + 1 < count else NONE)
        if count and counted is not None:
            struct.pack_into("<h", raw, 0x46 + places[0] * 8, counted)
        return Buffer(bytes(raw), pointers=data.pointers), entities

    for n in range(500):
        data, entities = a_chain(manager(), random_.choice((1, 1, 2, 3, 4)), random_.choice((0, 0, 1, -1)))
        t.check(
            "replication_entity_manager_delete_chain",
            random_.choice((entities[0], entities[0], entities[0], entities[-1], NONE)),
            data,
            externals=view_stubs,
            mocks=client_mocks(),
        )
    for n in range(400):
        count = random_.choice((0, 1, 2, 3, 4, 8))
        data, entities = a_chain(manager(), count)
        t.check(
            "replication_entity_manager_delete_entities_now",
            data,
            count,
            Buffer(struct.pack(f"<{count + 1}I", *entities, random_.getrandbits(32))),
            externals=view_stubs,
            mocks=client_mocks(),
        )

    # ---------- one more view

    for n in range(120):
        gives = {
            "unknown_000980d0": lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1]), arguments[2])),
        }

        def has(call, kind=random_.choice((0, 1, 2))):
            # what the things are of has something of some things
            return (0, 0x1234, (call.stack[0] & 1) * 0x77)[kind]

        t.check(
            "replication_entity_manager_add_view",
            Buffer(random_.randbytes(0x20)),
            manager(random_.choice((0.0, 0.02, 0.1, 1.0))),
            random_.randrange(VIEWS),
            externals=dict(view_stubs, **gives),
            mocks=[*client_mocks(), Mock(has, stack_arguments=1, registers=("ecx",))],
        )

    # ---------- things that another console made

    for n in range(400):
        count = random_.choice((0, 1, 2, 3, 4))
        entities = [place | random_.getrandbits(4) << 28 for place in random_.sample(range(COUNT), count)]
        things = Buffer(random_.randbytes(count * 0x10 + 4))
        t.check(
            "replication_entity_manager_create_entities_remote",
            Buffer(random_.randbytes(count * 4 + 4)),
            manager(),
            count,
            Buffer(struct.pack(f"<{count + 1}I", *entities, random_.getrandbits(32))),
            Buffer(random_.randbytes(count * 4 + 4)),
            things,
            mocks=[
                *client_mocks(),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=5, registers=("ecx",)),
            ],
        )
