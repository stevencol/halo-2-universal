# source/objects/object_types.c: the type of a tag, and the functions that give something back

import random
import struct

from difftest import SCRATCH_ADDRESS, Buffer, Mock, mock_address

from test_units import COUNT, some_objects

TYPE = 0x84 + 5 * 4
TYPES = 13
GROUPS = tuple(0x61000000 + index for index in range(TYPES))


def some_types(random_, base: int, offset: int) -> bytes:
    """What the thirteen types are, for a buffer at an address: each with its group, a function at a place or none, and the types that it is of."""
    definitions = bytearray(random_.randbytes(TYPES * TYPE))
    for index in range(TYPES):
        at = index * TYPE
        struct.pack_into("<I", definitions, at + 4, GROUPS[index])
        struct.pack_into("<I", definitions, at + offset, random_.choice((0, mock_address(index % 4), mock_address(index % 4))))
        chain = random_.sample(range(TYPES), random_.choice((0, 1, 2, 4)))
        for place in range(5):
            struct.pack_into("<I", definitions, at + 0x84 + place * 4, base + chain[place] * TYPE if place < len(chain) else 0)
    return bytes(definitions)


def after(*buffers: Buffer) -> int:
    """Where the harness puts the buffer that comes after these."""
    at = SCRATCH_ADDRESS
    for buffer in buffers:
        at += (len(buffer.data) + 64 + 15) & ~15
    return at


def run(t) -> None:
    random_ = random.Random(602)

    for n in range(1200):
        # the type of a tag: the tags are a buffer of the global, the types one that is no argument
        tags = bytearray(random_.randbytes(8 * 16))
        for index in range(8):
            struct.pack_into("<I", tags, index * 16, random_.choice((*GROUPS, 0x12345678)))
        tags = Buffer(bytes(tags))
        base = after(tags)
        t.check(
            "object_type_from_tag",
            random_.choice((0, 3, 7, 0xABCD0005)),
            globals={
                "global_tag_instances": tags,
                "object_type_definitions": struct.pack("<13I", *(base + index * TYPE for index in range(TYPES))),
            },
            extra=[Buffer(some_types(random_, base, 0x28))],
        )

        # something that starts with a tag of one of the types
        tag = random_.randrange(8)
        placement = Buffer(struct.pack("<I", tag) + random_.randbytes(0x20))
        tags_data = bytearray(tags.data)
        struct.pack_into("<I", tags_data, tag * 16, random_.choice(GROUPS))
        tags2 = Buffer(bytes(tags_data))
        base = after(placement, tags2)
        t.check(
            "object_types_call_unknown28",
            placement,
            globals={
                "global_tag_instances": tags2,
                "object_type_definitions": struct.pack("<13I", *(base + index * TYPE for index in range(TYPES))),
            },
            extra=[Buffer(some_types(random_, base, 0x28))],
            mocks=[Mock(lambda call: 0, stack_arguments=1) for index in range(4)],
        )

        table = some_objects(random_)
        an_object = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        says = lambda: [Mock(lambda call, value=random_.choice((0, 0, 1, 0x100)): value, stack_arguments=3) for index in range(4)]
        base = after(table)
        for name, offset in (("object_types_call_unknown2C", 0x2C), ("object_types_call_unknown48", 0x48)):
            t.check(
                name,
                an_object,
                random_.getrandbits(32),
                random_.getrandbits(32),
                globals={
                    "object_header_data": table,
                    "object_type_definitions": struct.pack("<13I", *(base + index * TYPE for index in range(TYPES))),
                },
                extra=[Buffer(some_types(random_, base, offset))],
                mocks=says(),
            )
        t.check(
            "object_types_call_unknown68",
            an_object,
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={
                "object_header_data": table,
                "object_type_definitions": struct.pack("<13I", *(base + index * TYPE for index in range(TYPES))),
            },
            extra=[Buffer(some_types(random_, base, 0x68))],
            mocks=[Mock(lambda call: 0, stack_arguments=4) for index in range(4)],
        )

        # one that is given where to put a real and a byte: the buffers of the arguments come first
        real, flag = Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4))
        base = after(real, flag, table)

        def puts(call, value=None):
            return value

        t.check(
            "object_types_call_unknown4C",
            real,
            flag,
            an_object,
            random_.getrandbits(32),
            globals={
                "object_header_data": table,
                "object_type_definitions": struct.pack("<13I", *(base + index * TYPE for index in range(TYPES))),
            },
            extra=[Buffer(some_types(random_, base, 0x4C))],
            mocks=[Mock(lambda call, value=random_.choice((0, 0, 1, 0x100)): value, stack_arguments=4) for index in range(4)],
        )


def run_603(t) -> None:
    random_ = random.Random(603)

    for n in range(1200):
        table = some_objects(random_)
        an_object = random_.getrandbits(16) << 16 | random_.randrange(COUNT)
        base = after(table)
        globals_ = {
            "object_header_data": table,
            "object_type_definitions": struct.pack("<13I", *(base + index * TYPE for index in range(TYPES))),
        }
        t.check(
            "object_types_call_unknown64",
            an_object,
            globals=globals_,
            extra=[Buffer(some_types(random_, base, 0x64))],
            mocks=[Mock(lambda call, value=random_.choice((0, 0, 1, 0x100)): value, stack_arguments=1) for index in range(4)],
        )
        t.check(
            "object_types_call_unknown6C",
            an_object,
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals=globals_,
            extra=[Buffer(some_types(random_, base, 0x6C))],
            mocks=[Mock(lambda call: 0, stack_arguments=4) for index in range(4)],
        )


_run_602 = run


def run(t) -> None:
    _run_602(t)
    run_603(t)
