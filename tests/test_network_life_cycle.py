# source/networking/network_session.c and network_life_cycle.c

import random
import struct

from difftest import Buffer, GlobalData, GlobalRange, Mock, mock_address

NONE = 0xFFFFFFFF
SESSION_SIZE = 0x7424
STATE = 0x741C
LOCAL_PEER = 0x72D8
HOST_PEER = 0x50

STATES = (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, NONE, 0x103)


def run(t) -> None:
    random_ = random.Random(286)

    def session(state: int, **fields: int) -> Buffer:
        data = bytearray(random_.randbytes(SESSION_SIZE))
        struct.pack_into("<I", data, STATE, state & 0xFFFFFFFF)
        for offset, value in fields.items():
            struct.pack_into("<I", data, int(offset[1:], 16), value & 0xFFFFFFFF)
        return Buffer(bytes(data))

    def peers(same: bool) -> dict:
        local = random_.choice((0, 3, NONE))
        return {f"x{LOCAL_PEER:x}": local, f"x{HOST_PEER:x}": local if same else local ^ 1}

    # ---------- a session

    for state in STATES:
        t.check("network_session_unknown_00058d20", session(state))
        t.check("network_session_established", session(state))
        for same in (True, False):
            t.check("network_session_local_peer_is_host", session(state, **peers(same)))
        for byte in (0, 1, 0x80):
            data = bytearray(session(state).data)
            data[0x7420] = byte
            t.check("network_session_unknown_00058d90", Buffer(bytes(data)))

    # ---------- the network game

    for there in (1, 0, 0x80):
        for life_cycle_state in (0, 1, 5, 6, 7, 9):
            life_cycle = {"unknown_00527330": bytes((there,)), "unknown_00527334": life_cycle_state}
            t.check("network_life_cycle_in_state5", globals=life_cycle)
            t.check("network_life_cycle_get_state", globals=life_cycle)

        for state in STATES:
            for same in (True, False):
                t.check(
                    "network_life_cycle_local_peer_is_host",
                    globals={"unknown_00527330": bytes((there,)), "network_life_cycle_session": session(state, **peers(same))},
                )
            for says in (1, 0, 0x100):
                t.check(
                    "network_life_cycle_unknown_00059330",
                    globals={"unknown_00527330": bytes((there,)), "network_life_cycle_session": session(state)},
                    externals={"network_session_unknown_00058d90": lambda harness, arguments, says=says: says},
                )
            for name, other in (
                ("network_life_cycle_get_session", "network_life_cycle_session"),
                ("network_life_cycle_get_other_session", "network_life_cycle_other_session"),
            ):
                for place in (Buffer(random_.randbytes(4)), 0):
                    t.check(
                        name,
                        place,
                        globals={
                            "unknown_00527330": bytes((there,)),
                            "network_life_cycle_session": session(state if other.endswith("cycle_session") else 0),
                            "network_life_cycle_other_session": session(0 if other.endswith("cycle_session") else state),
                        },
                    )

    for state in STATES:
        for value in (0, 1, 0x100):
            t.check(
                "network_life_cycle_unknown_00059470",
                globals={"network_life_cycle_session": session(state, x18=value)},
            )
        t.check(
            "network_life_cycle_unknown_00059500",
            globals={"network_life_cycle_session": session(state, x49f8=random_.getrandbits(32) | 2)},
        )

    for first in (0, 1, 2, 3):
        t.check(
            "network_life_cycle_unknown_000594f0",
            globals={"unknown_00527e78": first, "unknown_00527fa8": random_.getrandbits(32) | 0x10},
        )

    for life_cycle_state in (0, 5, 6, 7, 8, NONE):
        t.check(
            "network_life_cycle_unknown_00059570",
            globals={"unknown_00527334": life_cycle_state, "unknown_00527500": GlobalRange(0x10)},
            externals={
                "unknown_00070570": lambda harness, arguments, result=random_.getrandbits(32): (
                    result,
                    (harness.normalize(arguments[0]),),
                )
            },
        )

    # ---------- things that are done with the sessions

    def plain(*names: str, **results: int) -> dict:
        return {
            **{name: (lambda harness, arguments: 0) for name in names},
            **{name: (lambda harness, arguments, result=result: result) for name, result in results.items()},
        }

    for started in (1, 0, 0x100):
        t.check("network_life_cycle_unknown_000591e0", externals=plain(network_life_cycle_unknown_00059890=started))
        t.check("network_life_cycle_unknown_000591f0", externals=plain(network_life_cycle_unknown_00059890=started))

    def holder(a_session: Buffer, offset: int) -> tuple:
        """Something that has a session at an offset: the session comes after it in the same buffer."""
        data = bytearray(random_.randbytes(offset + 4)) + a_session.data
        struct.pack_into("<I", data, offset, offset + 4)
        return Buffer(bytes(data), pointers=(offset,))

    for there in (1, 0):
        for life_cycle_state in (5, 4, 6):
            for other in (0, 1):
                for state in STATES:
                    for same in (True, False):
                        t.check(
                            "network_life_cycle_unknown_00059280",
                            globals={
                                "unknown_00527330": bytes((there,)),
                                "unknown_00527334": life_cycle_state,
                                "unknown_005274fc": other,
                                "unknown_00527400": holder(session(state, **peers(same)), 0x30),
                                "unknown_005274e1": bytes((random_.choice((0, 0x80)),)),
                            },
                        )

    for state in STATES:
        for flag in (1, 0, 0x100, 0x80):
            for name, other in (
                ("network_life_cycle_unknown_00059360", "network_life_cycle_session"),
                ("network_life_cycle_unknown_000593a0", "network_life_cycle_other_session"),
            ):
                t.check(
                    name,
                    flag,
                    globals={
                        "network_life_cycle_session": session(state if other.endswith("cycle_session") else 0),
                        "network_life_cycle_other_session": session(0 if other.endswith("cycle_session") else state),
                    },
                    externals=plain("network_life_cycle_unknown_000598f0", "unknown_0005a520", "unknown_0005a400"),
                )
        for same in (True, False):
            t.check(
                "network_life_cycle_unknown_00059530",
                globals={"network_life_cycle_session": session(state, **peers(same))},
                externals=plain("unknown_0005b570"),
            )
            for life_cycle_state in (0, 1, 2, 7, NONE):
                for says in (1, 0, 0x100):
                    t.check(
                        "network_life_cycle_unknown_00059610",
                        globals={
                            "unknown_00527334": life_cycle_state,
                            "network_life_cycle_session": session(state, **peers(same)),
                        },
                        externals=plain(unknown_0005b1e0=says),
                    )
        for life_cycle_state in (7, 6, 0):
            for has in (1, 0, 0x80):
                data = bytearray(session(state, x49ac=random_.getrandbits(32) | 4).data)
                data[0x49A8] = has
                t.check(
                    "network_life_cycle_unknown_00059590",
                    globals={
                        "unknown_00527334": life_cycle_state,
                        "unknown_00527fa0": holder(Buffer(bytes(data)), 0x38),
                    },
                )

    for life_cycle_state in (6, 5, 7, 0):
        for unknown in (1, 0):
            # what unknown_00527500 points to, with a session after it
            thing = holder(Buffer(random_.randbytes(0x10)), 0x30)

            def a_thing(harness, arguments):
                return 0, (arguments[0],)

            t.check(
                "network_life_cycle_unknown_000595e0",
                globals={
                    "unknown_00527334": life_cycle_state,
                    "unknown_00510518": unknown,
                    "unknown_0051051c": bytes((0x40,)),
                    "unknown_0051051d": bytes((0x40,)),
                    "unknown_00527500+0x8": thing,
                },
                externals={"unknown_0005a6e0": a_thing},
            )

    # ---------- the network game stops

    def a_handler() -> Buffer:
        # what does a state: its functions come after the pointer to them
        return Buffer(struct.pack("<4I", 4, 0x55555555, mock_address(0), mock_address(1)), pointers=(0,))

    for life_cycle_state in range(10):
        for state, other_state in ((3, 3), (0, 3), (3, 0), (0, 0)):
            none, doing = a_handler(), a_handler()
            handlers = {"network_life_cycle_handlers": none}
            if life_cycle_state:
                handlers[f"network_life_cycle_handlers+{life_cycle_state * 4:#x}"] = doing
            t.check(
                "network_life_cycle_end",
                globals={
                    "unknown_00527334": life_cycle_state,
                    "network_life_cycle_session": session(state),
                    "network_life_cycle_other_session": session(other_state),
                    "unknown_0052737c": bytes((0x40,)),
                    "unknown_0052737d": bytes((0x40,)),
                    **handlers,
                },
                externals=plain("network_life_cycle_unknown_000598f0", "unknown_0005a520"),
                mocks=[
                    Mock(lambda call: 0, stack_arguments=3, registers=("ecx",)),
                    Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                ],
            )

    for first, second in ((3, 0), (2, 3), (2, 2), (0, 3), (1, 3)):
        for life_cycle_state in (6, 7, 5, 0):
            for says in (2, 4, 0, 3, 5):
                for result in (1, 0, 0x100):
                    t.check(
                        "network_life_cycle_unknown_000594a0",
                        random_.getrandbits(32),
                        random_.getrandbits(32),
                        random_.getrandbits(32),
                        globals={
                            "unknown_00527e78": first,
                            "unknown_00527fa8": second,
                            "unknown_00527334": life_cycle_state,
                            "unknown_00527500": GlobalRange(0x10),
                        },
                        externals={
                            "unknown_00070570": lambda harness, arguments, says=says: (says, (harness.normalize(arguments[0]),)),
                            "unknown_000705f0": lambda harness, arguments, result=result: (
                                result,
                                (harness.normalize(arguments[0]), *arguments[1:]),
                            ),
                        },
                    )

    for there in (1, 0):
        for life_cycle_state in range(11):
            for state, other_state in ((3, 3), (0, 3), (3, 0), (0, 0)):
                for unknown, thing, says in ((5, 0x80345000, 0), (NONE, 0x80345000, 0), (5, 0, 0), (5, 0x80345000, 1)):
                    for place in (Buffer(random_.randbytes(4)), 0):
                        t.check(
                            "network_life_cycle_get_current_session",
                            place,
                            globals={
                                "unknown_00527330": bytes((there,)),
                                "unknown_00527334": life_cycle_state,
                                "network_life_cycle_session": session(state),
                                "network_life_cycle_other_session": session(other_state),
                            },
                            externals=plain(unknown_000641f0=unknown, unknown_00192e60=thing, unknown_00193470=says),
                        )

    # ---------- which session

    for there in (1, 0, 0x80):
        for state, other_state in ((3, 3), (0, 3), (3, 0), (0, 0)):
            for place in (Buffer(random_.randbytes(4)), 0):
                sessions = {
                    "unknown_00527330": bytes((there,)),
                    "network_life_cycle_session": session(state),
                    "network_life_cycle_other_session": session(other_state),
                }
                t.check("network_life_cycle_get_any_session", place, globals=sessions)
                for life_cycle_state in range(11):
                    t.check(
                        "network_life_cycle_get_state_session",
                        place,
                        globals={**sessions, "unknown_00527334": life_cycle_state},
                    )
        for state in STATES:
            for value in (1, 0, 2):
                for place in (Buffer(random_.randbytes(4)), 0):
                    t.check(
                        "network_life_cycle_unknown_00059840",
                        place,
                        globals={"unknown_00527330": bytes((there,)), "network_life_cycle_session": session(state, x18=value)},
                    )

    for unknown in (1, 0):
        for same in ("same", "first half", "second half", "no"):
            identifier = random_.randbytes(8)
            # the session has what it is known by: with none the game reads at no address
            data = bytearray(session(3).data)
            data[0x24] = random_.choice((1, 0x80))
            data[0x1C:0x24] = identifier
            if same == "first half":
                data[0x1C] ^= 1
            elif same == "second half":
                data[0x23] ^= 0x80
            elif same == "no":
                data[0x1C:0x24] = random_.randbytes(8)
            t.check(
                "network_life_cycle_unknown_000598f0",
                Buffer(bytes(data)),
                globals={
                    "unknown_00510518": unknown,
                    "unknown_00510528": identifier,
                    "unknown_0051051c": bytes((0x40,)),
                    "unknown_0051051d": bytes((0x40,)),
                },
            )

    # ---------- what is asked of the network game

    for value in (0, 1, 0x3FE, 0x3FF, 0x400, NONE, 0x80000000, 0x7FFFFFFF):
        t.check("network_life_cycle_request_new2", Buffer(random_.randbytes(0x1C)), value)
        t.check("network_life_cycle_request_new3", Buffer(random_.randbytes(0x1C)), value)
    t.check("network_life_cycle_request_new4", Buffer(random_.randbytes(0x1C)))

    # ---------- starting the session

    values = (0, 1, 15, 16, 63, 64, 0x3FF, 0x400, NONE, 0x80000000, 0x7FFFFFFF)
    for n in range(40):
        t.check(
            "network_life_cycle_request_new1",
            random_.choice(values),
            random_.choice(values),
            random_.choice(values),
            random_.choice(values),
            Buffer(random_.randbytes(0x1C)),
        )

    def start(could: int):
        def handler(harness, arguments):
            # the two numbers that it is given a place for are in the stack
            return could, (*arguments[:8], bytes(harness.mu.mem_read(arguments[8], 8)), arguments[9])

        return handler

    for state in (0, 0, 3, 8):
        for could in (1, 0, 0x100):
            t.check(
                "network_life_cycle_unknown_00059890",
                random_.getrandbits(32),
                random_.getrandbits(32),
                random_.getrandbits(32),
                globals={"network_life_cycle_session": session(state)},
                externals={"unknown_00059bd0": start(could), "unknown_00065770": lambda harness, arguments: 0},
            )

    # ---------- a session is set up

    for index in (0, 1, 3):
        t.check(
            "network_session_initialize",
            Buffer(random_.randbytes(0x78B0 + 0x10)),
            Buffer(random_.randbytes(4 * 0x24)),
            Buffer(random_.randbytes(4 * 4)),
            index,
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
        )

    # ---------- two things that are asked

    def asked(byte_at: int = -1):
        def handler(harness, arguments):
            return 0, tuple(
                (argument & 0xFF) if n == byte_at else harness.normalize(argument) for n, argument in enumerate(arguments)
            )

        return handler

    for unknown in (1, 0):
        flags = {
            "unknown_00510518": unknown,
            "unknown_0051051c": bytes((0x40,)),
            "unknown_0051051d": bytes((0x40,)),
            "unknown_005273f8": GlobalRange(8),
        }
        for flag in (1, 0, 0x180):
            t.check(
                "network_life_cycle_unknown_00059200",
                random_.getrandbits(32),
                random_.getrandbits(32),
                random_.getrandbits(32),
                flag,
                globals=flags,
                # the yes or the no comes as a byte in a place of four
                externals={"unknown_0006f200": asked(0)},
            )
        t.check(
            "network_life_cycle_unknown_00059230",
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals=flags,
            externals={"unknown_0006f3a0": asked()},
        )

    # ---------- each frame

    def with_places(result: int = 0):
        return lambda harness, arguments: (result, tuple(harness.normalize(argument) for argument in arguments))

    for there in (1, 0, 0x80):
        for watched in (1, 0):
            for has in (0x80345000, 0, 1):
                t.check(
                    "network_life_cycle_update",
                    globals={
                        "unknown_00527330": bytes((there,)),
                        "unknown_00527334": GlobalData(struct.pack("<I", random_.randint(0, 9))),
                        "unknown_0047708c": GlobalRange(0xA0),
                        "unknown_0047712c": GlobalRange(0xA0),
                        "unknown_00527360": 0x80346000,
                        "unknown_00510518": watched,
                        "unknown_00510528": GlobalRange(8),
                        "unknown_0051051c": bytes((0x40,)),
                        "unknown_0051051d": bytes((0x40,)),
                    },
                    externals={
                        "unknown_0006dec0": with_places(),
                        "unknown_00073040": with_places(),
                        "unknown_000737b0": with_places(),
                        "unknown_00075800": with_places(has),
                    },
                )
