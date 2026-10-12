# fourteen small functions of the tables of the minds

import random
import struct

from difftest import Buffer

from test_small_709 import COUNT, OBJECT, objects

NONE = 0xFFFFFFFF
FIRST = 0x4C + COUNT * 12
JOINTS, JOINT = 3, 0xBC
PROPS, PROP = 3, 0xC4
REFS, REF = 3, 0x3C
SQUADS, SQUAD = 3, 0x98
GROUPS, GROUP = 4, 0x38
SCRIPTS, SCRIPT = 3, 0xD4


def run(t) -> None:
    random_ = random.Random(727)

    for n in range(200):
        number = random_.getrandbits(32)
        some = lambda: random_.choice((number, number, NONE, random_.getrandbits(32)))

        def table(count: int, size: int, fill=None) -> Buffer:
            data = bytearray(random_.randbytes(0x4C + count * size))
            struct.pack_into("<I", data, 0x44, 0x4C)
            for index in range(count):
                if fill:
                    fill(data, 0x4C + index * size, index)
            return Buffer(bytes(data), pointers=(0x44,))

        def a_joint(data, at, index) -> None:
            struct.pack_into("<h", data, at + 2, random_.choice((1, 1, 0, 2)))
            struct.pack_into("<I", data, at + 4, random_.choice((NONE, random_.getrandbits(32))))
            struct.pack_into("<h", data, at + 0x7C, random_.choice((0, 1, 2, 3)))
            for offset in (0x80, 0x84, 0x88):
                struct.pack_into("<I", data, at + offset, some())
            if random_.random() < 0.5:
                data[at + 0x80] = random_.choice((0, 1))
            data[at + 0x81] = random_.choice((0, 1))
            struct.pack_into("<h", data, at + 0x94, random_.choice((1, 1, 0, 2)))

        def joints() -> Buffer:
            return table(JOINTS, JOINT, a_joint)

        def state(allow_none: bool = False) -> Buffer:
            data = bytearray(random_.randbytes(0x28))
            data[0xC] = random_.choice((0, 1, 1))
            named = random_.getrandbits(16) << 16 | random_.randrange(JOINTS)
            struct.pack_into("<I", data, 0x10, NONE if allow_none and random_.random() < 0.25 else named)
            for offset in (0x1C, 0x20, 0x24):
                struct.pack_into("<I", data, offset, some())
            if random_.random() < 0.7:
                struct.pack_into("<H", data, 0x1C, random_.choice((0, 1, 0x11, 0xFFFF, 0xFFFE)))
            return Buffer(bytes(data))

        answers = lambda: {"unknown_0046fbe4": random_.getrandbits(32), "unknown_0046fbe8": random_.getrandbits(32)}
        actor_index = random_.getrandbits(32)

        for name in ("joint_unknown_001b0d10", "joint_unknown_001adce0"):
            t.check(name, actor_index, state(), random_.getrandbits(32), globals={"joint_state_data": joints(), **answers()})
        for name in (
            "joint_unknown_001b68b0",
            "joint_unknown_001b8460",
            "joint_unknown_00257cb0",
            "joint_unknown_001bb530",
            "joint_unknown_001bdfd0",
        ):
            t.check(name, actor_index, state(), number, globals={"joint_state_data": joints()})
        t.check("joint_unknown_001b4f10", actor_index, state(True), number, globals={"joint_state_data": joints()})
        t.check("joint_unknown_001b68f0", actor_index, state(), globals={"joint_state_data": joints()})

        # the props, each with its ring
        def a_prop(data, at, index) -> None:
            data[at + 0x52] = random_.randrange(5)
            data[at + 0x53] = random_.randrange(6)
            struct.pack_into("<I", data, at + 0x54, some())
            if random_.random() < 0.5:
                struct.pack_into("<I", data, at + 0x3E + random_.randrange(5) * 4, number)

        prop_index = random_.getrandbits(16) << 16 | random_.randrange(PROPS)
        nothing = random_.choice((NONE, NONE, number))
        t.check(
            "prop_unknown_001b6010",
            prop_index,
            globals={"prop_data": table(PROPS, PROP, a_prop), "unknown_00471004": nothing},
        )
        wanted = random_.choice((number, number, random_.getrandbits(32)))
        t.check(
            "prop_unknown_001b6070",
            prop_index,
            random_.getrandbits(16) << 16 | wanted & 0xFFFF,
            random_.getrandbits(16) << 16 | wanted >> 16,
            globals={"prop_data": table(PROPS, PROP, a_prop)},
        )

        # the groups of squads: each is of one with a bigger number, or of none
        group_salts = [random_.getrandbits(16) for _ in range(GROUPS)]
        a_group_index = lambda index: group_salts[index] << 16 | index

        def a_group(data, at, index) -> None:
            above = NONE if index + 1 >= GROUPS or random_.random() < 0.3 else a_group_index(random_.randrange(index + 1, GROUPS))
            struct.pack_into("<I", data, at + 0x10, above)

        def a_squad(data, at, index) -> None:
            struct.pack_into("<I", data, at + 0x6C, random_.choice((NONE, a_group_index(random_.randrange(GROUPS)), a_group_index(0))))

        t.check(
            "squad_is_in_group",
            random_.getrandbits(16) << 16 | random_.randrange(SQUADS),
            a_group_index(random_.randrange(GROUPS)),
            globals={"squad_data": table(SQUADS, SQUAD, a_squad), "squad_group_data": table(GROUPS, GROUP, a_group)},
        )

        def a_ref(data, at, index) -> None:
            struct.pack_into("<h", data, at + 0x18, random_.choice((7, 7, 7, 6)))
            struct.pack_into("<I", data, at + 0x20, random_.getrandbits(16) << 16 | random_.randrange(COUNT))
            struct.pack_into("<h", data, at + 0x24, random_.choice((0, 1, 2, 3)))

        def some_objects() -> Buffer:
            objects_ = objects(random_)
            data = bytearray(objects_.data)
            for one in range(COUNT):
                at = FIRST + one * OBJECT
                data[at + 0x19] = random_.choice((0, 1, 2, 3, 0xFF, 0xFC))
                data[at + 0xAA] = random_.choice((0, 0, 0, 1))
                data[at + 0x10A] = random_.choice((4, 4, 0xFF, 0, 0xFB))
            return Buffer(bytes(data), pointers=objects_.pointers)

        t.check(
            "prop_ref_unknown_00256790",
            random_.getrandbits(16) << 16 | random_.randrange(REFS),
            globals={"prop_ref_data": table(REFS, REF, a_ref), "object_header_data": some_objects()},
        )

        def a_script(data, at, index) -> None:
            data[at + 0xAC] = random_.choice((0, 0, 1))
            struct.pack_into("<I", data, at + 0xB0, random_.choice((NONE, NONE, random_.getrandbits(32))))

        t.check(
            "command_script_unknown_00259e70",
            random_.getrandbits(16) << 16 | random_.randrange(SCRIPTS),
            globals={"command_script_data": table(SCRIPTS, SCRIPT, a_script)},
        )
