# source/objects/objects.c

import math
import random
import struct

from difftest import Buffer, Mock, Pointer, mock_address
from test_data import DATA_ARRAY_SIZE, NONE, identifier, index

HEADER_SIZE = 0xC
OBJECT_SIZE = 0x200
NODE_MATRICES_OFFSET = 0x120
MATRIX_SIZE = 0x34
NODE_COUNT = 4


def world(random_: random.Random, object_count: int, maximum_count: int, havok_count: int = 0) -> tuple[Buffer, list[int]]:
    """The data array of the object headers, with its objects after it in the buffer, and the index of each object.

    An object can be attached to an object before it; each object has the
    matrices of some nodes.
    """
    headers_offset = DATA_ARRAY_SIZE
    bits_offset = headers_offset + maximum_count * HEADER_SIZE
    objects_offset = (bits_offset + ((maximum_count + 31) >> 5) * 4 + 15) & ~15
    places = sorted(random_.sample(range(maximum_count), object_count))

    array = bytearray(DATA_ARRAY_SIZE)
    array[0:7] = b"object\0"
    struct.pack_into(
        "<2iBBHIIiiihHII",
        array,
        0x20,
        maximum_count,
        HEADER_SIZE,
        0,
        1,
        0,
        0x64407440,
        0,
        0,
        max(places, default=-1) + 1,
        object_count,
        -0x7F00,
        0,
        headers_offset,
        bits_offset,
    )

    headers = bytearray(random_.randbytes(maximum_count * HEADER_SIZE))
    for n in range(maximum_count):
        struct.pack_into("<h", headers, n * HEADER_SIZE, 0)
    bits = bytearray(objects_offset - bits_offset)
    objects = bytearray()
    indices: list[int] = []
    pointers = [0x44, 0x48]
    for number, place in enumerate(places):
        identifier_ = identifier(random_)
        indices.append(index(place, identifier_))
        # the header: identifier, flags, type, cluster, size of the data, the data
        struct.pack_into(
            "<hBBhhI",
            headers,
            place * HEADER_SIZE,
            identifier_,
            random_.getrandbits(8),
            random_.randrange(0, 12),
            random_.randrange(0, 50),
            OBJECT_SIZE,
            objects_offset + number * OBJECT_SIZE,
        )
        pointers.append(headers_offset + place * HEADER_SIZE + 8)
        bits[place >> 3] |= 1 << (place & 7)

        datum = bytearray(random_.randbytes(OBJECT_SIZE))
        parent = random_.choice(indices[:-1]) if indices[:-1] and random_.random() < 0.6 else NONE
        struct.pack_into("<ib", datum, 0x14, parent, random_.randrange(0, NODE_COUNT))
        struct.pack_into("<9f", datum, 0x64, *(random_.uniform(-40, 40) for _ in range(9)))
        struct.pack_into("<h", datum, 0x116, NODE_MATRICES_OFFSET)
        if havok_count:
            # the salt of the index is not looked at
            component = NONE if random_.random() < 0.25 else random_.getrandbits(16) << 16 | random_.randrange(havok_count)
            struct.pack_into("<I", datum, 0xB4, component & 0xFFFFFFFF)
        for node in range(NODE_COUNT):
            matrix = [random_.choice((1.0, 1.0, 0.5, 2.5)), *(random_.uniform(-1, 1) for _ in range(9))]
            matrix += [random_.uniform(-40, 40) for _ in range(3)]
            struct.pack_into("<13f", datum, NODE_MATRICES_OFFSET + node * MATRIX_SIZE, *matrix)
        objects += datum

    return Buffer(bytes(array) + bytes(headers) + bytes(bits) + bytes(objects), pointers=tuple(pointers)), indices


def run(t) -> None:
    random_ = random.Random(8)

    for _ in range(40):
        maximum_count = random_.choice((4, 20, 64))
        object_count = random_.randint(1, maximum_count)
        data, indices = world(random_, object_count, maximum_count)
        globals_ = {"object_header_data": data}

        for object_index in random_.sample(indices, min(len(indices), 8)):
            t.check("object_wake", object_index, globals=globals_)
            t.check("object_get_world_matrix", object_index, Buffer(random_.randbytes(0x34), format="<13f"), globals=globals_)
            t.check("object_get_location", object_index, Buffer(bytes(8)), globals=globals_)
            t.check("object_get_ultimate_parent", object_index, globals=globals_)
            t.check("object_get_origin", object_index, Buffer(struct.pack("<3f", 9, 9, 9), format="<3f"), globals=globals_)
            for asked in ((True, True), (True, False), (False, True)):
                t.check(
                    "object_get_orientation",
                    object_index,
                    *(Buffer(struct.pack("<3f", 9, 9, 9), format="<3f") if ask else 0 for ask in asked),
                    globals=globals_,
                )
            for type_mask in (0xFFFFFFFF, 0, 1, 0x555, 0xAAA, random_.getrandbits(12)):
                t.check("object_try_and_get_and_verify_type", object_index, type_mask, globals=globals_)
                t.check("object_get_and_verify_type", object_index, type_mask, globals=globals_)
            t.check("object_get_first_unflagged_ancestor", object_index, globals=globals_)
            # the salt is not looked at: the absolute index alone, which can be 0
            t.check("object_get_first_unflagged_ancestor", object_index & 0xFFFF, globals=globals_)

        # an iterator, from the start and from any header
        for absolute_index in (0, 1, maximum_count - 1, maximum_count, *random_.sample(range(maximum_count), 3)):
            for type_mask, header_flags in ((0xFFFFFFFF, 0), (0x0F3, 0), (0xFFFFFFFF, 1), (0xFFF, 0x21), (0, 0)):
                iterator = struct.pack("<IBBhi", type_mask, header_flags, 0xCC, absolute_index, 0x55555555)
                t.check("object_iterator_next", Buffer(iterator), globals=globals_)

        # objects that are not there
        for object_index in (NONE, 0, index(maximum_count + 3, 5), indices[0] ^ 0x10000, indices[-1] ^ 0x7FFF0000):
            t.check("object_try_and_get_and_verify_type", object_index, 0xFFFFFFFF, globals=globals_)
            t.check("object_get_and_verify_type", object_index, 0xFFFFFFFF, globals=globals_)
        t.check("object_get_first_unflagged_ancestor", NONE, globals=globals_)
        t.check("object_get_ultimate_parent", NONE, globals=globals_)

    # ---------- a new iterator

    for type_mask in (0, 1, 0xFFFFFFFF, 0x0F3, 0x80000000):
        for header_flags in (0, 1, 0x21, 0xFF, 0x100, 0xABCDEF21):
            t.check("object_iterator_new", Buffer(random_.randbytes(0x14)), type_mask, header_flags)

    # ---------- the center of mass

    HAVOK_COMPONENT_SIZE = 0xA0
    RIGID_BODY_SIZE = 0x60
    RIGID_BODIES = 3

    INVERSE_MASSES = [
        struct.pack("<f", value)
        for value in (0.0, -0.0, 1.0, 0.5, 2.0, 0.25, 0.999, 1.001, 1e-30, 1e30, math.inf, -math.inf, -2.0, -0.5, 3.0)
    ] + [bytes.fromhex("0000c07f"), bytes.fromhex("0100807f"), bytes.fromhex("01000000")]

    def havok_components(
        count: int, fixed: bool = False, methods: bool = False, transforms: bool = False, islands: bool = False
    ) -> Buffer:
        """The data array of the havok components: only where its data is
        (0x44) is looked at. Each component has three rigid bodies, each of
        them one of Havok and what moves that."""
        components = 0x50
        bodies = components + count * HAVOK_COMPONENT_SIZE
        havok = bodies + count * RIGID_BODIES * RIGID_BODY_SIZE
        # Havok's rigid body has its simulation island after 0x40
        body_size = 0x50 if islands else 0x40
        motions = havok + count * RIGID_BODIES * body_size
        # what moves a rigid body has its transform after 0x80
        motion_size = 0xC0 if transforms else 0x80
        data = bytearray(random_.randbytes(motions + count * RIGID_BODIES * motion_size))
        pointers = [0x44]
        struct.pack_into("<I", data, 0x44, components)
        if islands:
            # two simulation islands in the header of the array, one awake
            # (its 0x29) and one asleep
            data[0x29] = random_.choice((1, 0xFF))
            data[0x10 + 0x29] = 0
        for c in range(count):
            component = components + c * HAVOK_COMPONENT_SIZE
            rigid_body_count = random_.choice((0, 1, 2, 3, 3, -1, 0x100))
            if rigid_body_count == 0x100:
                # past 0x7F a signed index is negative: it is none of the
                # many rigid bodies
                data[component + 0x18] = random_.choice((0, 1, 2, 0x80, 0xFF))
            else:
                data[component + 0x18] = random_.choice((0, 1, 2, 2, 3, 0x7F, 0x80, 0xFF))
            struct.pack_into("<Ii", data, component + 0x70, bodies + c * RIGID_BODIES * RIGID_BODY_SIZE, rigid_body_count)
            if transforms:
                # whether the rigid bodies are from the component's transform
                flags = random_.getrandbits(32) & ~0x40 | random_.choice((0, 0x40))
                struct.pack_into("<I", data, component + 4, flags)
            elif islands:
                # whether the rigid bodies are not made to turn
                flags = random_.getrandbits(32) & ~2 | random_.choice((0, 0, 2))
                struct.pack_into("<I", data, component + 4, flags)
            pointers.append(component + 0x70)
            for r in range(RIGID_BODIES):
                n = c * RIGID_BODIES + r
                struct.pack_into("<I", data, bodies + n * RIGID_BODY_SIZE + 0x40, havok + n * body_size)
                struct.pack_into("<I", data, havok + n * body_size + 0x3C, motions + n * motion_size)
                pointers += [bodies + n * RIGID_BODY_SIZE + 0x40, havok + n * body_size + 0x3C]
                struct.pack_into("<3f", data, motions + n * motion_size + 0x70, *(random_.uniform(-40, 40) for _ in range(3)))
                if fixed:
                    # the byte after Havok's rigid body is whether it is
                    # fixed, and what moves it has its velocities
                    data[havok + n * body_size + 0x40] = random_.choice((0, 0, 1, 0xFF))
                    struct.pack_into("<3f", data, motions + n * motion_size + 0x40, *(random_.uniform(-9, 9) for _ in range(3)))
                    struct.pack_into("<3f", data, motions + n * motion_size + 0x50, *(random_.uniform(-9, 9) for _ in range(3)))
                if islands:
                    data[havok + n * body_size + 0x40] = random_.choice((0, 0, 0, 1, 0xFF))
                    island = random_.choice((None, 0, 0x10))
                    struct.pack_into("<I", data, havok + n * body_size + 0x44, 0 if island is None else island)
                    if island is not None:
                        pointers.append(havok + n * body_size + 0x44)
                if methods:
                    # the methods, at the end; the inverse of the mass; and
                    # what the test's get_type gives
                    struct.pack_into("<I", data, motions + n * motion_size, len(data))
                    pointers.append(motions + n * motion_size)
                    data[motions + n * motion_size + 0x2C : motions + n * motion_size + 0x30] = random_.choice(INVERSE_MASSES)
                    struct.pack_into("<I", data, motions + n * motion_size + 0x7C, random_.choice((6, 6, 0, 1, 5, 7, 0x106, 0xFFFFFFFF)))
        if methods:
            data += bytes(0x18) + struct.pack("<I", mock_address(0)) + bytes(8) + struct.pack("<I", mock_address(1))
            data += bytes(0x28) + struct.pack("<3I", mock_address(2), mock_address(3), mock_address(4))
        return Buffer(bytes(data), pointers=tuple(pointers))

    for _ in range(40):
        maximum_count = random_.choice((4, 20, 64))
        object_count = random_.randint(1, maximum_count)
        # past 0xFF the index of a component has more than 8 bits
        havok_count = random_.choice((1, 2, 3, 4, 5, 6, 300))
        data, indices = world(random_, object_count, maximum_count, havok_count)
        globals_ = {"object_header_data": data, "havok_component_data": havok_components(havok_count)}
        for object_index in random_.sample(indices, min(len(indices), 10)):
            # compared as bytes, to tell 0 from -0
            t.check("object_get_center_of_mass", object_index, Buffer(random_.randbytes(12)), globals=globals_)

    # ---------- the velocities

    COMPONENTS = 0x50

    for _ in range(40):
        maximum_count = random_.choice((4, 20, 64))
        object_count = random_.randint(1, maximum_count)
        havok_count = random_.choice((1, 2, 3, 4, 5, 6, 300))
        data, indices = world(random_, object_count, maximum_count, havok_count)
        havok = havok_components(havok_count, fixed=True)
        globals_ = {"object_header_data": data, "havok_component_data": havok}
        for object_index in random_.sample(indices, min(len(indices), 10)):
            for asked in ((True, True), (True, False), (False, True), (False, False)):
                # compared as bytes, to tell 0 from -0
                t.check(
                    "object_get_velocities",
                    object_index,
                    *(Buffer(random_.randbytes(12)) if ask else 0 for ask in asked),
                    globals=globals_,
                )
        for _ in range(10):
            component = random_.randrange(havok_count)
            for name in ("havok_component_get_rigid_body_linear_velocity", "havok_component_get_rigid_body_angular_velocity"):
                t.check(
                    name,
                    Pointer(havok, COMPONENTS + component * HAVOK_COMPONENT_SIZE),
                    random_.randrange(RIGID_BODIES),
                    Buffer(random_.randbytes(12)),
                    extra=[havok],
                )

    # ---------- the rotation, the mass and the kind of motion of a rigid body

    def get_type(call) -> int:
        (type_,) = struct.unpack("<I", t.harness.mu.mem_read(call.registers["ecx"] + 0x7C, 4))
        return type_

    def get_rotation(call) -> int:
        # what moves the body has the matrix after its methods, as Havok's
        # three columns of four reals
        t.harness.mu.mem_write(call.stack[0], bytes(t.harness.mu.mem_read(call.registers["ecx"] + 4, 0x30)))
        return 0

    # the matrix is on the stack, which is not the same in both functions
    methods = [
        Mock(get_type, registers=("ecx",)),
        Mock(get_rotation, stack_arguments=1, registers=("ecx",), record=lambda call: call.registers["ecx"]),
    ]
    for _ in range(30):
        havok_count = random_.randint(1, 6)
        havok = havok_components(havok_count, methods=True)
        for _ in range(10):
            component = Pointer(havok, COMPONENTS + random_.randrange(havok_count) * HAVOK_COMPONENT_SIZE)
            rigid_body_index = random_.randrange(RIGID_BODIES)
            t.check("havok_component_get_rigid_body_mass", component, rigid_body_index, extra=[havok])
            t.check("havok_component_rigid_body_is_keyframed", component, rigid_body_index, extra=[havok], mocks=methods)
            # compared as bytes, to tell 0 from -0
            t.check(
                "havok_component_get_rigid_body_rotation",
                component,
                rigid_body_index,
                Buffer(random_.randbytes(0x24)),
                extra=[havok],
                mocks=methods,
            )

    # ---------- where a rigid body is

    def set_mul_eq(harness, arguments):
        """Stands in for Havok's product of two transforms: what the test
        leaves is the bits of the first with those of the second."""
        transform, other = arguments
        a = bytes(harness.mu.mem_read(transform, 0x40))
        b = bytes(harness.mu.mem_read(other, 0x40))
        harness.mu.mem_write(transform, bytes(x ^ y for x, y in zip(a, b)))
        # the first is on the stack, which is not the same in both functions
        return 0, (harness.normalize(other),)

    for _ in range(30):
        havok_count = random_.randint(1, 6)
        havok = havok_components(havok_count, transforms=True)
        for _ in range(10):
            component = Pointer(havok, COMPONENTS + random_.randrange(havok_count) * HAVOK_COMPONENT_SIZE)
            rigid_body_index = random_.randrange(RIGID_BODIES)
            # compared as bytes, to tell 0 from -0
            for name, size in (("havok_component_get_rigid_body_matrix", 0x34), ("havok_component_get_rigid_body_position", 12)):
                t.check(
                    name,
                    component,
                    rigid_body_index,
                    Buffer(random_.randbytes(size)),
                    extra=[havok],
                    externals={"hkTransform_setMulEq": set_mul_eq},
                )

    # ---------- the velocities of a rigid body that are set, and that of a point

    def is_active(harness, arguments):
        """Stands in for Havok's: awake if its simulation island says so."""
        rigid_body, active = arguments
        (island,) = struct.unpack("<I", harness.mu.mem_read(rigid_body + 0x44, 4))
        value = harness.mu.mem_read(island + 0x29, 1) if island else b"\0"
        harness.mu.mem_write(active, bytes(value))
        # where it is left is on the stack, which is not the same in both
        return active, (harness.normalize(rigid_body),)

    def vector4(address: int) -> bytes:
        return bytes(t.harness.mu.mem_read(address, 16))

    def get_point_velocity(call) -> int:
        # the bits of the point with those of the motion
        point, velocity = call.stack
        motion = vector4(call.registers["ecx"] + 0x40)
        t.harness.mu.mem_write(velocity, bytes(x ^ y for x, y in zip(vector4(point), motion)))
        return 0

    velocity_methods = [
        Mock(get_type, registers=("ecx",)),
        Mock(get_rotation, stack_arguments=1, registers=("ecx",), record=lambda call: call.registers["ecx"]),
        # the vectors are on the stack: what is compared is what they have
        Mock(lambda call: 0, stack_arguments=1, registers=("ecx",), record=lambda call: (call.registers["ecx"], vector4(call.stack[0]))),
        Mock(lambda call: 0, stack_arguments=1, registers=("ecx",), record=lambda call: (call.registers["ecx"], vector4(call.stack[0]))),
        Mock(get_point_velocity, stack_arguments=2, registers=("ecx",), record=lambda call: (call.registers["ecx"], vector4(call.stack[0]))),
    ]
    activation = {"hkEntity_isActive": is_active, "hkEntity_activate": lambda harness, arguments: 0}

    def vector() -> Buffer:
        return Buffer(struct.pack("<3f", *(random_.uniform(-9, 9) for _ in range(3))))

    for _ in range(30):
        havok_count = random_.randint(1, 6)
        havok = havok_components(havok_count, methods=True, islands=True)
        for _ in range(10):
            component = Pointer(havok, COMPONENTS + random_.randrange(havok_count) * HAVOK_COMPONENT_SIZE)
            rigid_body_index = random_.randrange(RIGID_BODIES)
            for name in ("havok_component_set_rigid_body_linear_velocity", "havok_component_set_rigid_body_angular_velocity"):
                t.check(name, component, rigid_body_index, vector(), extra=[havok], mocks=velocity_methods, externals=activation)
            # compared as bytes, to tell 0 from -0
            t.check(
                "havok_component_get_rigid_body_point_velocity",
                component,
                rigid_body_index,
                vector(),
                Buffer(random_.randbytes(12)),
                extra=[havok],
                mocks=velocity_methods,
            )
