# source/networking/transport_qos.c

import random
import struct

from difftest import Buffer, Mock, mock_address
from test_data import DATA_ARRAY_SIZE, array, identifier, index, some_indices

NONE = 0xFFFFFFFF
MAXIMUM = 32
# a console of what the console's libraries keep of an attempt
INFO = 0x18
TARGETS = 4


def run(t) -> None:
    random_ = random.Random(393)

    def attempts(in_use: dict[int, int] | None = None) -> tuple:
        """The table of the attempts, and after it what the console's libraries
        keep of each: the attempts that are in use point at theirs."""
        if in_use is None:
            fill = random_.choice((0.0, 0.3, 0.7, 1.0))
            in_use = {n: identifier(random_) for n in range(MAXIMUM) if random_.random() < fill}
        data = array(random_, MAXIMUM, 8, in_use)
        table = bytearray(data.data)
        pointers = list(data.pointers)
        for n in in_use:
            kept = bytearray(random_.randbytes(8 + TARGETS * INFO))
            struct.pack_into("<II", kept, 0, TARGETS, random_.choice((0, 0, 1, TARGETS)))
            for target in range(TARGETS):
                at = 8 + target * INFO
                kept[at] = random_.choice((0, 1, 2, 3, 5, 7, 0x10, 0x12, 0x1A, 0x0B, 0x0F, 0x08, random_.getrandbits(8)))
                struct.pack_into("<H", kept, at + 2, random_.choice((0, 0, 1, 8)))
            struct.pack_into("<I", table, DATA_ARRAY_SIZE + n * 8 + 4, len(table))
            pointers.append(DATA_ARRAY_SIZE + n * 8 + 4)
            table += kept
        return Buffer(bytes(table), pointers=tuple(pointers)), in_use

    def on() -> bytes:
        # whether attempts can be made
        return bytes((random_.choice((1, 1, 1, 0, 0x80)),))

    # ---------- the table

    allocator = Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,))
    for n in range(20):
        memory = Buffer(random_.randbytes(0x400))
        give = n % 4 != 0
        t.check(
            "transport_qos_initialize",
            globals={
                "unknown_004cf8d4": bytes((random_.choice((0, 1)),)),
                "transport_qos_attempts": random_.randbytes(4),
                "heap_allocation": allocator,
            },
            mocks=[
                Mock(lambda call, memory=memory, give=give: memory.address if give else 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
            extra=[memory],
        )

    for n in range(120):
        table, in_use = attempts()
        t.check(
            "transport_qos_dispose",
            globals={"unknown_004cf8d4": on(), "transport_qos_attempts": table},
            externals={"transport_qos_delete": lambda harness, arguments: (0, (arguments[0],))},
        )

    # ---------- an attempt starts

    def started(harness, arguments, could=None):
        # the three lists of what the consoles are known by are in the stack: what they point at is what counts
        count = arguments[0] if arguments[0] < 0x80000000 else 0
        lists = tuple(
            tuple(harness.normalize(pointer) for pointer in struct.unpack(f"<{count}I", harness.mu.mem_read(arguments[n], 4 * count)))
            for n in (1, 2, 3)
        )
        # what the libraries keep of it goes where the last thing says, which is in the stack too
        harness.mu.mem_write(arguments[11], struct.pack("<I", 0x11223344))
        return could, (arguments[0], lists, *arguments[4:11])

    def service(harness, arguments, could=None):
        harness.mu.mem_write(arguments[2], struct.pack("<I", 0x55667788))
        return could, (arguments[0], arguments[1])

    def some() -> tuple:
        # with room, and with none
        return attempts({n: identifier(random_) for n in range(MAXIMUM)}) if random_.random() < 0.25 else attempts()

    for n in range(600):
        table, in_use = some()
        t.check(
            "transport_qos_new",
            random_.choice((NONE, NONE, 0, 5000)),
            random_.choice((0, 1, 1, 2, 5, 63, 64, 65, 100, NONE)),
            random_.choice((0, 1)),
            Buffer(random_.randbytes(101 * 0x3C)),
            globals={"unknown_004cf8d4": on(), "transport_qos_attempts": table},
            externals={
                "XNetQosLookup": lambda harness, arguments, could=random_.choice((0, 0, 0, 7)): started(harness, arguments, could),
                "XNetQosRelease": lambda harness, arguments: (0, (arguments[0],)),
            },
        )

    for n in range(300):
        table, in_use = some()
        t.check(
            "transport_qos_service_new",
            globals={"unknown_004cf8d4": on(), "transport_qos_attempts": table},
            externals={
                "XNetQosServiceLookup": lambda harness, arguments, could=random_.choice((0, 0, 0, 7)): service(harness, arguments, could),
                "XNetQosRelease": lambda harness, arguments: (0, (arguments[0],)),
            },
        )

    # ---------- an attempt is no more, and how it is going

    for n in range(60):
        table, in_use = attempts()
        for which in some_indices(random_, in_use, MAXIMUM):
            t.check(
                "transport_qos_delete",
                which,
                globals={"unknown_004cf8d4": on(), "transport_qos_attempts": table},
                externals={
                    "XNetQosRelease": lambda harness, arguments, could=random_.choice((0, 0, 3)): (could, (harness.normalize(arguments[0]),))
                },
            )
            t.check("transport_qos_is_complete", which, globals={"unknown_004cf8d4": on(), "transport_qos_attempts": table})
            t.check(
                "transport_qos_get_state",
                which,
                random_.randrange(TARGETS),
                globals={"unknown_004cf8d4": on(), "transport_qos_attempts": table},
            )
            t.check(
                "transport_qos_get_result",
                which,
                Buffer(random_.randbytes(0x20)),
                random_.randrange(TARGETS),
                globals={"unknown_004cf8d4": on(), "transport_qos_attempts": table},
            )
