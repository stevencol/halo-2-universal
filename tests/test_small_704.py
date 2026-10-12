# whether two teams are enemies in multiplayer; and room is made in the world of the physics

import random
import struct

from difftest import Buffer, Mock, mock_address

from test_object_types_more import after

KINDS = 8


def run(t) -> None:
    random_ = random.Random(704)

    for n in range(200):
        globals_ = bytearray(random_.randbytes(0xC9C))
        kind = random_.randrange(KINDS)
        struct.pack_into("<I", globals_, 0xC14, kind)
        engine_globals = Buffer(bytes(globals_))
        # what runs the kind of game: where its functions are, and after it the 28 of them
        engine = bytearray(random_.randbytes(8 + 28 * 4))
        struct.pack_into("<I", engine, 0, 8)
        struct.pack_into("<I", engine, 8 + 27 * 4, mock_address(0))
        kinds = bytearray(KINDS * 4)
        if random_.random() < 0.75:
            struct.pack_into("<I", kinds, kind * 4, after(engine_globals))
        answer = random_.choice((0, 1))
        t.check(
            "game_engine_teams_are_enemies",
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={"unknown_004e9ae8": engine_globals, "unknown_0055e4d0": bytes(kinds)},
            extra=[Buffer(bytes(engine), pointers=(0,))],
            mocks=[
                Mock(
                    lambda call, answer=answer: answer,
                    stack_arguments=2,
                    registers=("ecx",),
                    record=lambda call: (call.registers["ecx"], call.stack[0] & 0xFFFF, call.stack[1] & 0xFFFF),
                )
            ],
        )

        # each time that something is taken out, the world has one less; in the end it always can
        could = [random_.choice((0, 0, 1)) for _ in range(12)]

        def room(harness, arguments, could=could):
            calls = sum(1 for call in harness._calls if call[0] == "unknown_001c4040")
            done = could[calls] if calls < len(could) else 1
            if done:
                address = harness.global_addresses["unknown_0047f050"]
                count = struct.unpack("<i", harness.mu.mem_read(address, 4))[0]
                harness.mu.mem_write(address, struct.pack("<i", count - 1))
            return done, (arguments[0] & 0xFF, *arguments[1:])

        t.check(
            "havok_unknown_001c51c0",
            random_.getrandbits(32),
            globals={"unknown_0047f050": random_.choice((0, 0x34E, 0x34F, 0x352, 0x355, 0xFFFFFFFF))},
            externals={"unknown_001c4040": room},
        )
