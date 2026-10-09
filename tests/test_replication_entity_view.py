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
            externals={"unknown_00099690": state},
        )
