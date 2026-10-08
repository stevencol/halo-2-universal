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
                externals={"network_session_start": start(could), "unknown_00065770": lambda harness, arguments: 0},
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

    # ---------- a session is started

    for n in range(240):
        known = random_.choice((1, 1, 1, 0x80, 0))
        told = random_.choice((None, None, (1, random_.randbytes(0x24)), (0, random_.randbytes(0x24))))
        could = random_.choice((1, 1, 1, 0x80, 0, 0x100))
        random_bytes = random_.randbytes(8)

        def asks(harness, arguments, told=told):
            # what the console is known by can come while the session starts
            if told is not None:
                harness.mu.mem_write(harness.global_addresses["unknown_004cf792"], bytes((told[0],)))
                harness.mu.mem_write(harness.global_addresses["unknown_004cf793"], told[1])
            return 0

        def at_random(harness, arguments, random_bytes=random_bytes):
            harness.mu.mem_write(arguments[1], random_bytes[: min(arguments[0], 8)])
            return 0, (arguments[0], harness.normalize(arguments[1]))

        def comes_in(harness, arguments):
            # what the console is known by is in the stack
            return 0, (
                harness.normalize(arguments[0]),
                arguments[1],
                bytes(harness.mu.mem_read(arguments[2], 0x24)),
                *arguments[3:],
            )

        t.check(
            "network_session_start",
            Buffer(random_.randbytes(0x78B8 + 0x10)),
            random_.choice((0, 1, 1, 2, 2, 5)),
            random_.getrandbits(32),
            random_.choice((0, random_.getrandbits(32), random_.getrandbits(32))),
            random_.choice((0, random_.getrandbits(32), random_.getrandbits(32))),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.choice((NONE, NONE, 0, 1, 4, 0xFFFFFFFE, 0x80000000)),
            globals={
                "unknown_004d8b18": bytes((random_.choice((1, 1, 0x80, 0)),)),
                "unknown_004d8b19": bytes((random_.choice((1, 1, 0x80, 0)),)),
                "unknown_004cf792": bytes((known,)),
                "unknown_004cf793": random_.randbytes(0x24),
                "unknown_00510548": bytes((random_.choice((0, 0, 1, 0x80)),)),
                "unknown_0051054c": random_.randbytes(4),
                "global_language": struct.pack("<I", random_.choice((NONE, NONE, 0, 3))),
            },
            externals={
                "transport_security_get_address": asks,
                "network_session_create_identifier": lambda harness, arguments, could=could: could,
                # the fourth thing is not used: source/ does not give it
                "network_session_use_identifier": lambda harness, arguments, could=could: (
                    could,
                    (harness.normalize(arguments[0]), arguments[1], arguments[2], arguments[4]),
                ),
                "GetTickCount": lambda harness, arguments, value=random_.getrandbits(32): value,
                "XGetLanguage": lambda harness, arguments, value=random_.randrange(12): value,
                "transport_security_random_bytes": at_random,
                "network_session_add_peer": comes_in,
                "time": lambda harness, arguments, value=random_.getrandbits(31): value,
                "network_session_enter_state5": lambda harness, arguments: 0,
                "network_session_request_players": lambda harness, arguments: 0,
            },
        )

    # ---------- what a session is known by in the network

    ENTRIES = 0x80

    def known_session(has: int) -> Buffer:
        # what the session was given when it was set up goes after it
        data = bytearray(random_.randbytes(ENTRIES + 0x14 + 4 * 0x24))
        struct.pack_into("<I", data, 8, ENTRIES)
        struct.pack_into("<I", data, 0x10, random_.randrange(4))
        struct.pack_into("<I", data, 0x38, random_.randrange(8))
        data[0x24] = has
        return Buffer(bytes(data), pointers=(8,))

    def keys() -> GlobalData:
        data = bytearray(random_.randbytes(0x100))
        for index in range(8):
            data[index * 0x20] = random_.choice((1, 1, 0x80, 0))
        return GlobalData(bytes(data))

    def told() -> bytes:
        return bytes((random_.choice((1, 1, 0x80, 0)),))

    library = {"XNetQosListen": lambda harness, arguments: 0, "XNetUnregisterKey": lambda harness, arguments: 0}

    for n in range(60):
        t.check(
            "network_session_forget_identifier",
            known_session(random_.choice((1, 1, 0x80, 0))),
            globals={"unknown_004cf7d4": keys(), "unknown_004cf8d4": told()},
            externals=library,
        )
    for n in range(40):
        t.check(
            "network_session_set_identifier",
            known_session(random_.choice((1, 0))),
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(16)),
            random_.choice((0, 1, NONE, random_.getrandbits(32))),
            globals={"unknown_004cf7d4": keys(), "unknown_004cf8d4": told()},
            externals={**library, "network_observer_set_session": lambda harness, arguments: 0},
        )
    for n in range(80):
        t.check(
            "network_session_use_identifier",
            known_session(random_.choice((1, 1, 0x80, 0))),
            Buffer(random_.randbytes(16)),
            random_.choice((0, 1, random_.getrandbits(32))),
            random_.getrandbits(32),
            Buffer(random_.randbytes(8)),
            globals={"unknown_004cf7d4": keys(), "unknown_004cf8d4": told()},
            externals={
                **library,
                "network_observer_set_session": lambda harness, arguments: 0,
                "transport_security_set_key": lambda harness, arguments, could=random_.choice((1, 1, 0x80, 0, 0x100)): could,
            },
        )
    for n in range(120):
        made = random_.choice((None, None, 1, 0x80, 0))
        made_bytes = random_.randbytes(0x18)

        could = random_.choice((1, 1, 1, 0x80, 0, 0x100))

        def makes(harness, arguments, made=made, made_bytes=made_bytes, could=could):
            # only a byte of the last thing counts
            if made is not None:
                at = harness.global_addresses["unknown_004cf7d4"] + (arguments[1] & 7) * 0x20
                harness.mu.mem_write(at, bytes((made,)))
                harness.mu.mem_write(at + 8, made_bytes)
            return could, (arguments[0], arguments[1], arguments[2] & 0xFF)

        t.check(
            "network_session_create_identifier",
            known_session(random_.choice((1, 1, 0x80, 0))),
            random_.choice((0, 0, 1, random_.getrandbits(32))),
            random_.choice((0, 1, 2, 2, 2, 5, 0x102, 0x202)),
            globals={"unknown_004cf7d4": keys(), "unknown_004cf8d4": told()},
            externals={**library, "network_observer_set_session": lambda harness, arguments: 0, "transport_security_create_key": makes},
        )

    # ---------- a console comes into a session

    def clock() -> dict:
        return {
            "unknown_00510548": bytes((random_.choice((0, 0, 1, 0x80)),)),
            "unknown_0051054c": random_.randbytes(4),
        }

    def ticks():
        return lambda harness, arguments, value=random_.getrandbits(32): value

    for n in range(80):
        data = bytearray(random_.randbytes(0x78B8))
        data[0x765C] = random_.choice((0, 1, 0x80))
        t.check(
            "network_session_peer_state_new",
            random_.choice((0, 1, 1, 0x80, 0x100, 0x1FF)),
            Buffer(bytes(data)),
            random_.randrange(16),
            random_.choice((NONE, 0, 3, random_.getrandbits(32))),
            globals=clock(),
            externals={"GetTickCount": ticks()},
        )
    for n in range(80):
        data = bytearray(random_.randbytes(0x78B8))
        data[0x765C] = random_.choice((0, 1, 0x80))
        t.check(
            "network_session_add_peer",
            Buffer(bytes(data)),
            random_.randrange(17),
            Buffer(random_.randbytes(0x24)),
            random_.choice((0, 1, 1, 0x80, 0x100, 0x1FF)),
            random_.choice((NONE, 0, 3, random_.getrandbits(32))),
            random_.choice((Buffer(random_.randbytes(8)), Buffer(random_.randbytes(8)), 0)),
            globals=clock(),
            externals={"GetTickCount": ticks()},
        )

    # ---------- the players that ask to come into a session, and its fifth state

    MEMBERSHIP, PLAYER_MASK, PLAYER_ARRAY, PLAYER = 0x4C, 0x111C, 0x1120, 0x13C
    REQUESTS, REQUEST = 0x7668, 0x24

    def whole_session(full: bool = False) -> bytearray:
        data = bytearray(random_.randbytes(0x78B8))
        struct.pack_into("<I", data, STATE, random_.choice((0, 1, 3, 4, 5, 5, 6, 7, 8, 9)))
        struct.pack_into("<I", data, MEMBERSHIP, random_.choice((0, 0, 7, NONE)))
        struct.pack_into("<I", data, 0x54, random_.choice((0, 1, 2, 5, 16)))
        data[0x765C] = random_.choice((0, 1))
        for peer in range(16):
            data[0x72DC + peer * 0x14 + 1] = random_.choice((0, 0, 0, 1, 0x80))
        for request in range(16):
            data[REQUESTS + request * REQUEST] = 1 if full else random_.choice((0, 1, 1, 0x80))
        return data

    def known_by(data: bytearray) -> bytes:
        # what a player of the session is known by, most of the time
        player = random_.randrange(16)
        at = PLAYER_ARRAY + player * PLAYER
        if random_.randrange(5) == 0:
            return random_.randbytes(12)
        if random_.randrange(4) == 0:
            # only the last of the twelve bytes is not the same
            return bytes(data[at : at + 11]) + bytes((data[at + 11] ^ 0x40,))
        return bytes(data[at : at + 12])

    for n in range(200):
        data = whole_session()
        t.check("network_session_find_player", Buffer(bytes(data)), Buffer(known_by(data)))
    for n in range(40):
        t.check("network_session_reset_requests", Buffer(bytes(whole_session())), random_.choice((0, 1, 0x80, 0x100)))
    for n in range(80):
        t.check(
            "network_session_enter_state5",
            Buffer(bytes(whole_session())),
            globals=clock(),
            externals={"GetTickCount": ticks()},
        )
    for n in range(120):
        data = whole_session(full=n % 6 == 0)
        t.check(
            "network_session_request_player",
            Buffer(known_by(data)),
            Buffer(random_.randbytes(8)),
            Buffer(bytes(data)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals=clock(),
            externals={"GetTickCount": ticks()},
        )
    for n in range(120):
        data = whole_session(full=n % 6 == 0)
        if n % 3 == 0:
            # room for two more
            free = random_.sample(range(16), 2)
            for request in range(16):
                data[REQUESTS + request * REQUEST] = 0 if request in free else 1
        t.check(
            "network_session_request_players",
            Buffer(b"".join(known_by(data) for _ in range(4))),
            Buffer(random_.randbytes(8)),
            Buffer(bytes(data)),
            random_.choice((0, 1, 2, 3, 4, NONE)),
            random_.getrandbits(32),
            Buffer(random_.randbytes(16)),
            globals=clock(),
            externals={"GetTickCount": ticks()},
        )

    # ---------- a console of a session, the players of two sessions, and a console that is no more

    PEERS, PEER = 0x58, 0x10C

    for n in range(200):
        data = whole_session()
        peer = random_.randrange(16)
        address = bytes(data[PEERS + peer * PEER : PEERS + peer * PEER + 0x24])
        if n % 4 == 0:
            address = address[:0x23] + bytes((address[0x23] ^ 1,))
        if n % 7 == 0:
            address = random_.randbytes(0x24)
        t.check("network_session_find_peer", Buffer(bytes(data)), Buffer(address))

    for n in range(200):
        data = whole_session()
        other = whole_session()
        if n % 3:
            struct.pack_into("<I", data, STATE, random_.choice((3, 4, 5, 8)))
            struct.pack_into("<I", other, STATE, random_.choice((3, 4, 5, 8)))
        struct.pack_into("<I", other, MEMBERSHIP, random_.choice((0, 0, 0, NONE)))
        mask = random_.choice((0, 1, 0x8001, random_.getrandbits(16)))
        struct.pack_into("<I", data, PLAYER_MASK, mask)
        struct.pack_into("<I", other, PLAYER_MASK, random_.choice((0xFFFF, 0xFFFF, random_.getrandbits(16))))
        # the players of one are in the other, in another order; but for one, some of the time
        order = random_.sample(range(16), 16)
        for player in range(16):
            at, to = PLAYER_ARRAY + player * PLAYER, PLAYER_ARRAY + order[player] * PLAYER
            other[to : to + 12] = data[at : at + 12]
        if n % 4 == 0:
            other[PLAYER_ARRAY + random_.randrange(16) * PLAYER + 5] ^= 0x10
        t.check("network_session_has_players_of", Buffer(bytes(data)), Buffer(bytes(other)))

    CHANNELS = 0x78C0

    for n in range(120):
        data = whole_session() + bytearray(random_.randbytes(8 + 4 * 0x528))
        struct.pack_into("<I", data, 8, CHANNELS)
        struct.pack_into("<I", data, 0x10, random_.choice((0, 1, 2, 3, 7, 9)))
        for peer in range(16):
            at = 0x72DC + peer * 0x14
            data[at] = random_.choice((0, 1, 1))
            data[at + 2] = random_.choice((0, 1, 1))
            struct.pack_into("<I", data, at + 4, random_.choice((NONE, NONE, 0, 1, 2, 3)))
        t.check(
            "network_session_peer_state_delete",
            Buffer(bytes(data), pointers=(8,)),
            random_.randrange(16),
        )

    # ---------- more ways of finding a console of a session

    def with_peers() -> bytearray:
        data = whole_session()
        data[0x24] = random_.choice((0, 1, 1, 0x80))
        struct.pack_into("<I", data, 0x40, random_.choice((0, 1, 2, 3, NONE)))
        struct.pack_into("<I", data, LOCAL_PEER, random_.choice((0, 1, 2, 3, NONE)))
        for peer in range(16):
            at = 0x72DC + peer * 0x14
            data[at] = random_.choice((0, 1, 1, 0x80))
            struct.pack_into("<I", data, at + 4, random_.choice((NONE, 0, 1, 2, 3, 4)))
        return data

    for n in range(160):
        t.check(
            "network_session_find_peer_by_channel",
            Buffer(bytes(with_peers())),
            random_.choice((NONE, 0, 1, 2, 3, 4, 5)),
        )
    for n in range(240):
        channel = random_.choice((NONE, 0, 1, 2, 3, 4))
        t.check(
            "network_session_channel_is_host",
            Buffer(bytes(with_peers())),
            random_.getrandbits(32),
            externals={"network_observer_find_channel": lambda harness, arguments, channel=channel: channel},
        )
    for n in range(200):
        data = with_peers()
        peer = random_.randrange(16)
        at = PEERS + peer * PEER + 0xA
        six = bytes(data[at : at + 6])
        if n % 4 == 0:
            six = six[:5] + bytes((six[5] ^ 1,))
        if n % 7 == 0:
            six = random_.randbytes(6)
        t.check("network_session_find_peer_by_unknownA", Buffer(bytes(data)), Buffer(six))

    def known(data: bytearray):
        # what the console of the address is known by is that of a console of the session, most of the time
        peer = random_.randrange(4)
        address = bytes(data[PEERS + peer * PEER : PEERS + peer * PEER + 0x24])
        if random_.randrange(5) == 0:
            address = random_.randbytes(0x24)
        could = random_.choice((1, 1, 1, 0x80, 0, 0x100))

        def handler(harness, arguments):
            # where it goes is in the stack
            harness.mu.mem_write(arguments[5], address)
            return could, tuple(arguments[:5])

        return handler

    for n in range(200):
        data = with_peers()
        t.check(
            "network_session_find_peer_by_transport_address",
            Buffer(bytes(data)),
            Buffer(random_.randbytes(0x14)),
            externals={"transport_security_peer_address_from_address": known(data)},
        )
    for n in range(300):
        data = with_peers()
        t.check(
            "network_session_address_is_host",
            Buffer(bytes(data)),
            Buffer(random_.randbytes(0x14)),
            externals={
                "transport_security_peer_address_from_address": known(data),
                "network_session_matches_state1_address": lambda harness, arguments, same=random_.choice((0, 0, 0, 1, 0x80, 0x100)): same,
            },
        )

    # ---------- the address that a session keeps in its first state

    for n in range(240):
        data = whole_session()
        struct.pack_into("<I", data, STATE, random_.choice((1, 1, 1, 0, 2, 5)))
        size = random_.choice((4, 4, 4, 16, 6, 1, 0, 0xFFFF, 0x8004))
        address = bytearray(random_.randbytes(0x14))
        struct.pack_into("<H", address, 0x12, size)
        kept = bytearray(address)
        way = random_.randrange(6)
        if way == 0 and 0 < size <= 16:
            # the last byte that counts is not the same
            kept[size - 1] ^= 0x20
        elif way == 1 and size < 16:
            # the first byte that does not count is not the same
            kept[size] ^= 0x20
        elif way == 2:
            struct.pack_into("<H", kept, 0x12, random_.choice((4, 16, 6, 0)))
        elif way == 3:
            kept[0] ^= 1
        data[0x7448 : 0x7448 + 0x14] = kept
        t.check("network_session_matches_state1_address", Buffer(bytes(data)), Buffer(bytes(address)))

    # ---------- a console is told and taken out of a session

    def sends(harness, arguments):
        # what is sent is in the stack, and only a byte of the first thing counts
        return 0, (arguments[0] & 0xFF, *arguments[1:6], bytes(harness.mu.mem_read(arguments[6], arguments[5] & 0xFF)))

    for name in ("network_session_remove_peer_with_message_e", "network_session_remove_peer_with_message_d"):
        for n in range(120):
            data = whole_session() + bytearray(random_.randbytes(8 + 4 * 0x528))
            struct.pack_into("<I", data, 8, CHANNELS)
            struct.pack_into("<I", data, 0x10, random_.randrange(4))
            for peer in range(16):
                struct.pack_into("<I", data, 0x72DC + peer * 0x14 + 4, random_.randrange(4))
            for channel in range(4):
                struct.pack_into("<I", data, CHANNELS + channel * 0x528 + 0xA8, random_.choice((7, 7, 6, 0, 8)))
            t.check(
                name,
                random_.randrange(16),
                Buffer(bytes(data), pointers=(8,)),
                externals={"unknown_00075e80": sends, "network_session_remove_peer": lambda harness, arguments: 0},
            )

    # ---------- a console and its players go out of a session

    def consistent() -> bytearray:
        # a session whose players have consoles and places that are in it
        data = whole_session() + bytearray(random_.randbytes(8 + 4 * 0x528))
        struct.pack_into("<I", data, 8, CHANNELS)
        struct.pack_into("<I", data, 0x10, random_.randrange(4))
        struct.pack_into("<I", data, 0x54, random_.choice((1, 2, 3, 5, 16)))
        struct.pack_into("<I", data, 0x40, random_.choice((0, 1, 2, 3, NONE)))
        struct.pack_into("<I", data, LOCAL_PEER, random_.choice((0, 1, 2, 3, NONE)))
        struct.pack_into("<I", data, HOST_PEER, random_.choice((0, 1, 2, 3)))
        for player in range(16):
            at = PLAYER_ARRAY + player * PLAYER
            struct.pack_into("<II", data, at + 0xC, random_.randrange(16), random_.randrange(4))
        for peer in range(16):
            for place in range(4):
                struct.pack_into("<I", data, PEERS + peer * PEER + 0xFC + place * 4, random_.choice((NONE, NONE, random_.randrange(16))))
            struct.pack_into("<I", data, 0x72DC + peer * 0x14 + 4, random_.choice((NONE, 0, 1, 2, 3)))
        # some of the players that ask to come in are players of the session
        for request in range(16):
            if random_.randrange(2):
                at = PLAYER_ARRAY + random_.randrange(16) * PLAYER
                to = REQUESTS + request * REQUEST + 0xA
                data[to : to + 12] = data[at : at + 12]
        return data

    for n in range(160):
        data = consistent()
        to = REQUESTS + random_.randrange(16) * REQUEST + 0xA
        identifier = bytes(data[to : to + 12])
        if n % 4 == 0:
            identifier = identifier[:11] + bytes((identifier[11] ^ 2,))
        t.check(
            "network_session_find_request",
            Buffer(bytes(data), pointers=(8,)),
            Buffer(identifier),
            random_.choice((Buffer(random_.randbytes(4), places=(0,)), Buffer(random_.randbytes(4), places=(0,)), 0)),
        )
    for n in range(160):
        t.check(
            "network_session_remove_player",
            Buffer(bytes(consistent()), pointers=(8,)),
            random_.randrange(16),
        )
    for n in range(240):
        data = consistent()
        count = struct.unpack_from("<I", data, 0x54)[0]
        t.check(
            "network_session_remove_peer",
            Buffer(bytes(data), pointers=(8,)),
            random_.choice((random_.randrange(count), count - 1, 0)),
            externals={"network_session_stop_waiting_if_done": lambda harness, arguments: 0},
        )

    # ---------- the consoles that have not answered, and a player that comes in

    def waiting() -> bytearray:
        data = with_peers()
        struct.pack_into("<I", data, 0x54, random_.choice((0, 1, 3, 5, 16)))
        data[0x765C] = random_.choice((0, 1, 1, 1, 0x80))
        few = random_.choice((0, 0, 1, 4))
        for peer in range(16):
            at = 0x72DC + peer * 0x14
            data[at + 1] = random_.choice((0, 1, 1, 0x80))
            data[at + 2] = random_.choice((0, 1, 0x80)) if random_.randrange(8) < few else 0
        return data

    for n in range(240):
        t.check("network_session_stop_waiting_if_done", Buffer(bytes(waiting())))

    for n in range(400):
        data = waiting()
        answer = bytearray(random_.randbytes(0x10))
        if n % 5:
            answer[8:12] = data[0x4980:0x4984]
        if n % 7:
            answer[12:16] = data[0x497C:0x4980]
        channel = random_.choice((NONE, 0, 1, 2, 3, 4))
        if n % 2:
            # the console of the answer has not answered
            for peer in range(16):
                at = 0x72DC + peer * 0x14
                if data[at] and struct.unpack_from("<I", data, at + 4)[0] == channel:
                    data[at + 2] = 1
                    break
        t.check(
            "network_session_peer_answered",
            Buffer(bytes(data)),
            random_.getrandbits(32),
            Buffer(bytes(answer)),
            externals={"network_observer_find_channel": lambda harness, arguments, channel=channel: channel},
        )

    for n in range(160):
        data = consistent()
        to = REQUESTS + random_.randrange(16) * REQUEST + 0xA
        identifier = bytes(data[to : to + 12]) if n % 3 else random_.randbytes(12)
        t.check(
            "network_session_add_player",
            Buffer(identifier),
            Buffer(bytes(data), pointers=(8,)),
            random_.randrange(16),
            random_.randrange(16),
            random_.randrange(4),
        )
    for n in range(80):
        t.check(
            "network_session_remove_player_and_count",
            Buffer(bytes(consistent()), pointers=(8,)),
            random_.randrange(16),
        )

    # ---------- a player of a console comes into a session

    def room(data: bytearray) -> None:
        struct.pack_into("<I", data, 0x4994, random_.choice((16, 16, 16, 0, 4, 8, 32)))
        struct.pack_into("<I", data, 0x1118, random_.choice((0, 1, 4, 8, 15, 16)))
        for request in range(16):
            at = REQUESTS + request * REQUEST
            data[at] = random_.choice((0, 0, 1, 0x80))
            data[at + 1] = random_.choice((0, 0, 1, 0x80))

    for n in range(200):
        data = whole_session()
        room(data)
        t.check("network_session_free_player_places", Buffer(bytes(data)))

    OBJECT = CHANNELS + 4 * 0x528

    for n in range(400):
        data = consistent()
        room(data)
        struct.pack_into("<I", data, PLAYER_MASK, random_.choice((0, 1, 0xFF, 0x7FFF, 0xFFFF, random_.getrandbits(16))))
        # what says whether a player can come in goes after the rest, with its functions after it
        assert len(data) == OBJECT
        data += struct.pack("<5I", OBJECT + 4, 0x11, 0x22, 0x33, mock_address(0))
        pointers = [8, OBJECT]
        if n % 4:
            struct.pack_into("<I", data, 0x78A8, OBJECT)
            pointers.append(0x78A8)
        else:
            struct.pack_into("<I", data, 0x78A8, 0)
        peer, place = random_.randrange(16), random_.randrange(4)
        there = struct.unpack_from("<I", data, PEERS + peer * PEER + 0xFC + place * 4)[0]
        way = random_.randrange(6)
        if way == 0 and there != NONE:
            # the player that the console has there
            at = PLAYER_ARRAY + there * PLAYER
            identifier = bytes(data[at : at + 12])
        elif way == 1:
            # a player of the session
            at = PLAYER_ARRAY + random_.randrange(16) * PLAYER
            identifier = bytes(data[at : at + 12])
        elif way == 2:
            # a player that asks to come in
            at = REQUESTS + random_.randrange(16) * REQUEST + 0xA
            identifier = bytes(data[at : at + 12])
        else:
            identifier = random_.randbytes(12)
        t.check(
            "network_session_add_peer_player",
            Buffer(bytes(data), pointers=tuple(pointers)),
            peer,
            place,
            Buffer(identifier),
            mocks=[Mock(lambda call, can=random_.choice((1, 1, 1, 0x80, 0, 0x100)): can, stack_arguments=1, registers=("ecx",))],
        )

    # ---------- what has changed of a console of a session

    def text(length: int) -> bytes:
        # a text of characters of 16 bits that ends somewhere, or does not
        characters = [random_.randrange(1, 0x10000) for _ in range(length)]
        end = random_.choice((0, 1, length // 2, length - 1, length))
        if end < length:
            characters[end] = 0
        return struct.pack(f"<{length}H", *characters)

    for n in range(400):
        current = bytearray(text(16) + text(32) + random_.randbytes(0xC8 - 0x60))
        baseline = bytearray(current)
        for _ in range(random_.choice((0, 0, 1, 1, 2, 5))):
            way = random_.randrange(9)
            if way == 0:
                # a character of the first text, which is after its end or not
                baseline[random_.randrange(16) * 2] ^= 1
            elif way == 1:
                baseline[0x20 + random_.randrange(32) * 2] ^= 1
            elif way == 2:
                baseline[random_.choice((0x1E, 0x5E))] ^= 0x40
            else:
                # a byte of one of the other parts
                at = {3: 0x60, 4: 0x64, 5: 0x68 + random_.randrange(12), 6: 0x74 + random_.randrange(16), 7: 0x84 + random_.randrange(0x40), 8: 0xC4}[way]
                baseline[at + (random_.randrange(4) if way in (3, 4, 8) else 0)] ^= 0x10
        t.check(
            "network_session_peer_properties_get_update",
            Buffer(random_.randbytes(0xD8)),
            Buffer(bytes(current)),
            Buffer(bytes(baseline)) if n % 8 else 0,
        )

    # ---------- four things that a session is asked

    def maybe(size: int):
        return random_.choice((Buffer(random_.randbytes(size)), Buffer(random_.randbytes(size)), 0))

    for n in range(60):
        data = with_peers()
        # the place of the session is given back: its address is the same in the game and in source/
        t.check("network_session_get_identifier", Buffer(bytes(data)))
    for n in range(160):
        t.check(
            "network_session_get_security",
            Buffer(bytes(with_peers())),
            maybe(16),
            maybe(4),
            maybe(4),
            maybe(8),
        )
    for n in range(60):
        t.check("network_session_get_membership", maybe(4), Buffer(bytes(with_peers())), maybe(4))
    for n in range(160):
        t.check(
            "network_session_has_channel_peer",
            Buffer(bytes(with_peers())),
            random_.choice((NONE, 0, 1, 2, 3, 4, 5)),
        )

    # ---------- what the console that runs a session does with it

    for n in range(160):
        data = with_peers()
        local = struct.unpack_from("<I", data, LOCAL_PEER)[0]
        t.check(
            "network_session_host_remove_peer",
            Buffer(bytes(data)),
            random_.choice((local, random_.randrange(4), random_.randrange(16))),
            externals={"network_session_remove_peer_with_message_e": lambda harness, arguments: 0},
        )
    for n in range(160):
        data = with_peers()
        if n % 3 == 0:
            data[HOST_PEER : HOST_PEER + 4] = data[LOCAL_PEER : LOCAL_PEER + 4]
        t.check("network_session_host_set_local", Buffer(bytes(data)))
    for n in range(120):
        t.check(
            "network_session_host_set_player_unknownA8",
            random_.randrange(16),
            Buffer(bytes(with_peers())),
            Buffer(random_.randbytes(0x90)),
        )
    for state in STATES:
        t.check("network_session_in_last_three_states", session(state))
    for n in range(300):
        data = with_peers()
        struct.pack_into("<II", data, 0x4990, random_.choice((16, 16, 4, 0)), random_.choice((16, 16, 8, 0)))
        struct.pack_into("<I", data, 0x54, random_.choice((0, 1, 3, 4, 15, 16)))
        struct.pack_into("<I", data, 0x1118, random_.choice((0, 1, 7, 8, 15, 16)))
        t.check(
            "network_session_is_full_for",
            Buffer(bytes(data)),
            random_.choice((0, 1, 1, 2, 16, NONE)),
            random_.choice((0, 1, 1, 4, 16, NONE)),
        )
    for n in range(300):
        data = whole_session()
        room(data)
        count = random_.choice((0, 1, 2, 4, 4, 4))
        identifiers = b""
        for _ in range(4):
            # a player that asks to come in, or one that does not
            at = REQUESTS + random_.randrange(16) * REQUEST + 0xA
            identifiers += bytes(data[at : at + 12]) if random_.randrange(2) else random_.randbytes(12)
        t.check(
            "network_session_has_room_for_players",
            random_.choice((0, 0, 1, 0x80, 0x100)),
            Buffer(identifiers),
            Buffer(bytes(data)),
            count,
        )

    # ---------- ten more things that a session is asked

    def asked_session() -> Buffer:
        data = with_peers()
        for at in (0x49C4, 0x49FD, 0x4D10, 0x5DD8):
            data[at] = random_.choice((0, 1, 1, 0x80))
        struct.pack_into("<I", data, 0x4D0C, random_.choice((NONE, 0, 5, random_.getrandbits(32))))
        return Buffer(bytes(data))

    for name in (
        "network_session_get_unknown49c4",
        "network_session_get_unknown49c8",
        "network_session_get_unknown4a00",
        "network_session_get_chosen_peer",
        "network_session_get_random",
        "network_session_get_maximum_players",
    ):
        for n in range(60):
            t.check(name, asked_session())
    for n in range(160):
        t.check(
            "network_session_get_unknown4d08",
            asked_session(),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
        )
    for n in range(100):
        t.check("network_session_get_unknown5ddc", asked_session(), Buffer(random_.randbytes(0x48)))
    for n in range(80):
        t.check(
            "network_session_get_language",
            asked_session(),
            globals={"global_language": struct.pack("<I", random_.choice((NONE, NONE, 0, 3)))},
            externals={"XGetLanguage": lambda harness, arguments, value=random_.randrange(12): value},
        )
    for n in range(60):
        t.check(
            "network_session_host_unknown_0005aeb0",
            asked_session(),
            random_.getrandbits(32),
            # only a byte of the first thing counts: the game leaves the rest of the session's address in it
            externals={"unknown_000614a0": lambda harness, arguments: (0, (arguments[0] & 0xFF, *arguments[1:]))},
        )

    # ---------- the console that runs a session changes what the session knows

    def hosted() -> bytearray:
        data = with_peers()
        for at in (0x49C4, 0x49E8, 0x49FC, 0x49FD, 0x4998, 0x4F20, 0x5DD8):
            data[at] = random_.choice((0, 1, 1, 0x80))
        return data

    for n in range(160):
        data = hosted()
        thing = bytearray(random_.randbytes(0x44))
        if n % 3 == 0:
            thing[4:12] = data[0x1C:0x24]
        elif n % 3 == 1:
            thing[4:11] = data[0x1C:0x23]
        t.check(
            "network_session_unknown_0005c190",
            Buffer(bytes(thing)) if n % 9 else 0,
            Buffer(bytes(data)),
        )
    for n in range(100):
        t.check("network_session_host_set_unknown49f0", maybe(8), Buffer(bytes(hosted())))
    for n in range(80):
        t.check("network_session_host_clear_unknown49fc", Buffer(bytes(hosted())))
        t.check("network_session_host_clear_unknown49c4", Buffer(bytes(hosted())))
    for n in range(100):
        data = hosted()
        same = struct.unpack_from("<I", data, 0x49F8)[0]
        t.check("network_session_host_set_unknown49f8", Buffer(bytes(data)), random_.choice((same, same ^ 1, random_.getrandbits(32))))
    for n in range(120):
        data = hosted()
        thing = bytearray(data[0x49CC : 0x49CC + 0x1C])
        if n % 3:
            thing[random_.choice((0, 0x1B, random_.randrange(0x1C)))] ^= 4
        t.check("network_session_host_set_unknown49cc", Buffer(bytes(data)), Buffer(bytes(thing)))
    for n in range(120):
        t.check(
            "network_session_host_set_unknown4a00",
            maybe(0x308),
            Buffer(bytes(hosted())),
            externals={"unknown_00063340": lambda harness, arguments, right=random_.choice((1, 1, 1, 0x80, 0, 0x100)): right},
        )
    for n in range(160):
        data = hosted()
        thing = bytearray(random_.randbytes(0x44))
        if n % 4 == 0:
            thing[4:12] = data[0x1C:0x24]
        t.check(
            "network_session_host_set_unknown5ddc",
            Buffer(bytes(thing)) if n % 5 else 0,
            Buffer(bytes(data)),
        )
    for n in range(60):
        t.check("network_session_host_set_maximum_players", Buffer(bytes(hosted())), random_.getrandbits(32))
        t.check("network_session_host_set_random", Buffer(bytes(hosted())), random_.getrandbits(32), random_.getrandbits(32))
        t.check("network_session_host_set_unknown4da8", Buffer(bytes(hosted())), random_.getrandbits(32))
        t.check("network_session_host_set_chosen_peer", Buffer(bytes(hosted())), random_.choice((NONE, 0, 3, random_.getrandbits(32))))
    for n in range(100):
        there = n % 4 != 0
        t.check(
            "network_session_host_set_unknown4f24",
            Buffer(random_.randbytes(0x6C)) if there else 0,
            Buffer(bytes(hosted())),
            Buffer(random_.randbytes(0xE40)) if there or n % 8 else 0,
        )
    for n in range(100):
        t.check("network_session_host_set_unknown4999", maybe(8), Buffer(bytes(hosted())))

    # ---------- the address that a session keeps in its second state

    for name in ("network_session_matches_state2_address", "network_session_unknown_0005c990"):
        for n in range(240):
            data = whole_session()
            struct.pack_into("<I", data, STATE, random_.choice((2, 2, 2, 0, 1, 5)))
            size = random_.choice((4, 4, 4, 16, 6, 1, 0, 0xFFFF, 0x8004))
            address = bytearray(random_.randbytes(0x14))
            struct.pack_into("<H", address, 0x12, size)
            kept = bytearray(address)
            way = random_.randrange(6)
            if way == 0 and 0 < size <= 16:
                kept[size - 1] ^= 0x20
            elif way == 1 and size < 16:
                kept[size] ^= 0x20
            elif way == 2:
                struct.pack_into("<H", kept, 0x12, random_.choice((4, 16, 6, 0)))
            elif way == 3:
                kept[0] ^= 1
            data[0x7444 : 0x7444 + 0x14] = kept
            t.check(
                name,
                Buffer(bytes(address)),
                Buffer(bytes(data)),
                externals={"unknown_0005a520": lambda harness, arguments: 0},
            )

    # ---------- sending to the consoles of a session

    for n in range(20):
        t.check("network_session_reset_unknown761c", Buffer(bytes(whole_session())))

    def with_channels() -> bytearray:
        data = whole_session() + bytearray(random_.randbytes(8 + 4 * 0x528))
        struct.pack_into("<I", data, 8, CHANNELS)
        struct.pack_into("<I", data, 0x10, random_.randrange(4))
        for peer in range(16):
            struct.pack_into("<I", data, 0x72DC + peer * 0x14 + 4, random_.randrange(4))
        for channel in range(4):
            struct.pack_into("<I", data, CHANNELS + channel * 0x528 + 0xA8, random_.choice((7, 7, 6, 0, 8)))
        return data

    for n in range(240):
        t.check(
            "network_session_send_to_peer",
            random_.randrange(16),
            random_.choice((0, 1, 2, 2, 3, NONE)),
            Buffer(bytes(with_channels()), pointers=(8,)),
            random_.randrange(0x30),
            8,
            Buffer(random_.randbytes(8)),
            externals={"unknown_00075e80": sends},
        )
    for n in range(120):
        t.check(
            "network_session_send_to_peers",
            random_.choice((0, 1, 2, 2, 3)),
            Buffer(bytes(with_channels()), pointers=(8,)),
            random_.randrange(0x30),
            8,
            Buffer(random_.randbytes(8)),
            externals={"unknown_00075e80": sends},
        )

    # ---------- the players that ask to come in, again

    for n in range(160):
        data = whole_session()
        now = random_.getrandbits(32)
        for request in range(16):
            at = REQUESTS + request * REQUEST
            limit = random_.choice((NONE, 0, 1000, 1000, 30000, 0x80000000))
            ago = random_.choice((0, 1, 999, 1000, 1001, 29999, 30001, 0x7FFFFFFF, 0x80000001))
            struct.pack_into("<I", data, at + 0x1C, (now - ago) & 0xFFFFFFFF)
            struct.pack_into("<I", data, at + 0x20, limit)
        t.check(
            "network_session_requests_expire",
            Buffer(bytes(data)),
            globals={"unknown_00510548": bytes((random_.choice((0, 0, 1)),)), "unknown_0051054c": struct.pack("<I", now)},
            externals={"GetTickCount": lambda harness, arguments, now=now: now},
        )
    for n in range(160):
        data = whole_session()
        eight = random_.randbytes(8)
        for request in random_.sample(range(16), random_.choice((0, 1, 3, 16))):
            at = REQUESTS + request * REQUEST + 2
            data[at : at + 8] = eight if random_.randrange(4) else eight[:7] + bytes((eight[7] ^ 1,))
        t.check("network_session_requests_remove", Buffer(bytes(data)), Buffer(eight))

    # ---------- whether a console can come in

    for n in range(500):
        data = with_peers()
        room(data)
        struct.pack_into("<I", data, STATE, random_.choice((5, 5, 5, 5, 5, 6, 7, 8, 3, 0)))
        struct.pack_into("<I", data, 0x498C, random_.choice((0, 0, 1, 1, 2, NONE)))
        struct.pack_into("<I", data, 0x4990, random_.choice((16, 16, 2, 0)))
        struct.pack_into("<I", data, 0x54, random_.choice((1, 2, 3, 15, 16)))
        join = bytearray(random_.randbytes(0x1AC))
        struct.pack_into("<I", join, 0, random_.choice((0, 1, 1, 2, 4)))
        join[0x14C] = random_.choice((0, 0, 1, 0x80))
        for player in range(4):
            # a player that asks to come in, or one that does not
            at = REQUESTS + random_.randrange(16) * REQUEST + 0xA
            if random_.randrange(2):
                join[4 + player * 12 : 16 + player * 12] = data[at : at + 12]
        if n % 4 == 0:
            # the console is in the session already
            at = PEERS + random_.randrange(4) * PEER
            join[0x188 : 0x188 + 0x24] = data[at : at + 0x24]
        t.check("network_session_host_check_join", Buffer(bytes(data)), Buffer(bytes(join)))
