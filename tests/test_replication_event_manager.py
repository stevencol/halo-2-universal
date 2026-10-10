# source/networking/replication_event_manager.c

import random
import struct

from difftest import Buffer, Mock, Pointer, mock_address

NONE = 0xFFFFFFFF
VIEWS = 15
EVENT = 0x2C
# in one buffer: the heap of the network, what gives its memory with its functions, how many times it was asked and
# what it answers, the memory that it gives, what the things are of with its functions, what keeps the things, its
# things, the views, and a thing that is not in the list
HEAP, ALLOCATOR, TABLE, ASKED, ANSWERS, GIVEN, CLIENT, CLIENT_TABLE, MANAGER, NODES, VIEW_DATA, LOOSE, END = (
    0x0,
    0x8,
    0x10,
    0x3C,
    0x40,
    0x44,
    0x70,
    0x74,
    0x84,
    0xD4,
    0x1B0,
    0x408,
    0x434,
)
VIEW = 0x28


def run(t) -> None:
    random_ = random.Random(419)

    def world(count: int | None = None, answers=(1, 1, 1, 1)) -> tuple:
        """The buffer, and where its things are."""
        if count is None:
            count = random_.choice((0, 1, 2, 3, 5))
        data = bytearray(random_.randbytes(END))
        pointers = [HEAP, ALLOCATOR, CLIENT, MANAGER + 4]
        struct.pack_into("<Ii", data, HEAP, ALLOCATOR, random_.choice((0, 1, 7, 100)))
        struct.pack_into("<I", data, ALLOCATOR, TABLE)
        struct.pack_into("<11I", data, TABLE, mock_address(0), mock_address(1), 0x33, 0x44, 0x55, mock_address(2), 0x77, 0x88, 0x99, 0xAA, mock_address(3))
        struct.pack_into("<I4B", data, ASKED, 0, *answers)
        struct.pack_into("<I", data, CLIENT, CLIENT_TABLE)
        struct.pack_into("<4I", data, CLIENT_TABLE, 0x11, 0x22, 0x33, mock_address(4))
        struct.pack_into("<I", data, MANAGER + 4, CLIENT)
        for view in range(VIEWS):
            if random_.random() < 0.5:
                struct.pack_into("<I", data, MANAGER + 0xC + view * 4, VIEW_DATA + view * VIEW)
                pointers.append(MANAGER + 0xC + view * 4)
            else:
                struct.pack_into("<I", data, MANAGER + 0xC + view * 4, 0)
            struct.pack_into("<I", data, VIEW_DATA + view * VIEW + 0xC, random_.choice((view, view, 31, random_.getrandbits(32))))
        struct.pack_into("<i", data, MANAGER + 0x48, random_.choice((count, count, 0, 9)))
        nodes = [NODES + node * EVENT for node in range(count)]
        struct.pack_into("<I", data, MANAGER + 0x4C, nodes[0] if nodes else 0)
        if nodes:
            pointers.append(MANAGER + 0x4C)
        for place, node in enumerate([*nodes, LOOSE]):
            # what a thing has more is not looked at here: a number
            struct.pack_into("<I", data, node + 0x18, random_.choice((0, 0, 0x12345678, random_.getrandbits(32) | 1)))
            last = place >= len(nodes) - 1
            struct.pack_into("<I", data, node + 0x28, 0 if last else nodes[place + 1])
            if not last:
                pointers.append(node + 0x28)
        if random_.random() < 0.5:
            # the thing that is not in the list has one of the list after it, or itself
            struct.pack_into("<I", data, LOOSE + 0x28, random_.choice((*nodes, LOOSE)))
            pointers.append(LOOSE + 0x28)
        return Buffer(bytes(data), pointers=tuple(pointers)), nodes

    def mocks() -> list:
        def gives(call):
            # what gives the memory gives it or not, as its answers say, one after the other
            mu = t.harness.mu
            base = call.registers["ecx"] - ALLOCATOR
            asked = struct.unpack("<I", mu.mem_read(base + ASKED, 4))[0]
            mu.mem_write(base + ASKED, struct.pack("<I", asked + 1))
            return base + GIVEN if mu.mem_read(base + ANSWERS + (asked & 3), 1)[0] else 0

        return [
            Mock(lambda call: 0, stack_arguments=2, registers=("ecx",)),
            # where the size goes is in the stack
            Mock(lambda call, known=random_.choice((0, 1)): known, stack_arguments=2, registers=("ecx",), record=lambda call: (call.registers["ecx"], call.stack[0])),
            Mock(gives, stack_arguments=3, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
        ]

    def at_time() -> dict:
        now = random_.getrandbits(32)
        while 0x20000000 <= now < 0x21000000:
            now = random_.getrandbits(32)
        return {
            "globals": {
                "unknown_00510548": bytes((random_.choice((0, 0, 1)),)),
                "unknown_0051054c": struct.pack("<I", now),
            },
            "externals": {"GetTickCount": lambda harness, arguments, now=now: now},
        }

    # ---------- something is done with each view

    for n in range(60):
        data, nodes = world()
        t.check(
            "replication_event_manager_reset_views",
            Pointer(data, MANAGER),
            extra=[data],
            externals={"replication_event_view_reset": lambda harness, arguments: (0, (harness.normalize(arguments[0]),))},
        )

    # ---------- a new thing that happens

    for n in range(300):
        data, nodes = world(answers=random_.choice(((1, 1, 1, 1), (1, 0, 0, 0), (0, 1, 0, 0), (0, 0, 1, 1), (0, 0, 0, 0))))
        t.check(
            "replication_event_manager_new_event",
            Pointer(data, MANAGER),
            extra=[data],
            globals={"unknown_004d87f8": Pointer(data, HEAP)},
            mocks=mocks(),
        )

    # ---------- a thing is taken from the list, and let go of

    for n in range(400):
        data, nodes = world()
        event = random_.choice((*nodes, *nodes, LOOSE, None))
        t.check(
            "replication_event_manager_remove_event",
            0 if event is None else Pointer(data, event),
            Pointer(data, MANAGER),
            extra=[data],
            globals={"unknown_004d87f8": Pointer(data, HEAP)},
            mocks=mocks(),
        )
    for n in range(300):
        data, nodes = world()
        t.check(
            "replication_event_free",
            Pointer(data, random_.choice((*nodes, LOOSE))),
            random_.choice((0, 1, 1, 2, 3, random_.getrandbits(32))),
            extra=[data],
            globals={"unknown_004d87f8": Pointer(data, HEAP)},
            mocks=mocks(),
        )

    # ---------- a thing is to be told of

    for n in range(500):
        setup = at_time()
        data, nodes = world()
        raw = data.data
        there = sum(1 << view for view in range(VIEWS) if struct.unpack_from("<I", raw, MANAGER + 0xC + view * 4)[0])
        mask = random_.choice((0, there, ~there & 0x7FFF, 0xFFFF8000, random_.getrandbits(15), random_.getrandbits(32), 1 << random_.randrange(VIEWS)))
        t.check(
            "replication_event_manager_send_event",
            Pointer(data, random_.choice((*nodes, *nodes, LOOSE))),
            Pointer(data, MANAGER),
            mask,
            extra=[data],
            globals=dict(setup["globals"], unknown_004d87f8=Pointer(data, HEAP)),
            externals=setup["externals"],
            mocks=mocks(),
        )

    # ---------- a thing that happens has something more

    for n in range(300):
        data, nodes = world(answers=random_.choice(((1, 1, 1, 1), (0, 1, 0, 0), (0, 0, 1, 1), (0, 0, 0, 0))))
        size = random_.choice((0, 1, 3, 4, 7, 0x20, EVENT))
        t.check(
            "replication_event_set_data",
            size,
            Pointer(data, random_.choice((*nodes, LOOSE))),
            Buffer(random_.randbytes(size + 4)),
            extra=[data],
            globals={"unknown_004d87f8": Pointer(data, HEAP)},
            mocks=mocks(),
        )
