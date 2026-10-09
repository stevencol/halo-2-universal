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
                    externals=plain("network_life_cycle_unknown_000598f0", "network_session_disconnect", "network_session_leave"),
                )
        for same in (True, False):
            t.check(
                "network_life_cycle_unknown_00059530",
                globals={"network_life_cycle_session": session(state, **peers(same))},
                externals=plain("network_session_leader_clear_unknown49f8"),
            )
            for life_cycle_state in (0, 1, 2, 7, NONE):
                for says in (1, 0, 0x100):
                    t.check(
                        "network_life_cycle_unknown_00059610",
                        globals={
                            "unknown_00527334": life_cycle_state,
                            "network_life_cycle_session": session(state, **peers(same)),
                        },
                        externals=plain(network_session_leader_unknown_0005b1e0=says),
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
                externals={"network_session_request_unknown49fc": a_thing},
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
                externals=plain("network_life_cycle_unknown_000598f0", "network_session_disconnect"),
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
            externals={"network_session_enter_state7": lambda harness, arguments: (0, (arguments[0] & 0xFF, *arguments[1:]))},
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
            externals={"network_session_roster_valid": lambda harness, arguments, right=random_.choice((1, 1, 1, 0x80, 0, 0x100)): right},
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
                externals={"network_session_disconnect": lambda harness, arguments: 0},
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

    # ---------- the groups and the players of the thing of 0x4A00

    def roster(right: bool = True) -> bytearray:
        data = bytearray(random_.randbytes(0x308))
        groups = random_.choice((0, 1, 2, 4, 16))
        players = random_.choice((0, 1, 2, 5, 16))
        struct.pack_into("<i", data, 0, groups)
        struct.pack_into("<i", data, 0x184, players)
        for player in range(16):
            struct.pack_into("<i", data, 0x248 + player * 4, random_.choice((-1, 0, 5, 50, 0x7F)))
            struct.pack_into("<i", data, 0x288 + player * 4, random_.choice((-1, 0, 1000, 0x3FFFFFFF)))
            struct.pack_into("<i", data, 0x2C8 + player * 4, random_.randrange(max(groups, 1)))
        if not right:
            way = random_.randrange(8)
            player = random_.randrange(16)
            if way == 0:
                struct.pack_into("<i", data, 0, random_.choice((-1, 17, 0x7FFFFFFF)))
            elif way == 1:
                struct.pack_into("<i", data, 0x184, random_.choice((-1, 17, -0x80000000)))
            elif way == 2:
                struct.pack_into("<i", data, 0x248 + player * 4, random_.choice((-2, 0x80, 1000)))
            elif way == 3:
                struct.pack_into("<i", data, 0x288 + player * 4, random_.choice((-2, 0x40000000)))
            elif way == 4:
                struct.pack_into("<i", data, 0x2C8 + player * 4, random_.choice((-1, groups, 16)))
            else:
                # the last player is not right
                last = max(players - 1, 0)
                struct.pack_into("<i", data, 0x2C8 + last * 4, groups)
        return data

    for n in range(400):
        t.check("network_session_roster_valid", Buffer(bytes(roster(right=n % 2 == 0))))

    def one_of(data: bytearray, at: int, size: int) -> bytes:
        # one of sixteen things of a roster, most of the time; two of them can be the same
        index = random_.randrange(16)
        thing = bytes(data[at + index * size : at + (index + 1) * size])
        if random_.randrange(4) == 0:
            other = random_.randrange(16)
            data[at + other * size : at + (other + 1) * size] = thing
        if random_.randrange(5) == 0:
            thing = thing[:-1] + bytes((thing[-1] ^ 8,))
        return thing

    for n in range(240):
        data = roster()
        identifier = one_of(data, 0x188, 12)
        t.check("network_session_roster_find_player", Buffer(bytes(data)), Buffer(identifier))
    for n in range(240):
        data = roster()
        thing = one_of(data, 0x84, 12)
        t.check("network_session_roster_find_group_by_unknown84", Buffer(bytes(data)), Buffer(thing))
    for n in range(240):
        data = roster()
        thing = one_of(data, 4, 8)
        t.check("network_session_roster_find_group_by_unknown4", Buffer(bytes(data)), Buffer(thing))
    for n in range(240):
        t.check(
            "network_session_roster_group_maximum_unknown248",
            Buffer(bytes(roster())),
            random_.choice((0, 0, 1, 2, 3, 15, NONE)),
        )
    for n in range(240):
        data = roster()
        identifiers = b"".join(
            one_of(data, 0x188, 12) if random_.randrange(4) == 0 else random_.randbytes(12) for _ in range(4)
        )
        t.check(
            "network_session_roster_has_any_player",
            Buffer(identifiers),
            Buffer(bytes(data)),
            random_.choice((0, 1, 2, 4, 4, NONE)),
        )

    # ---------- the bits of a number, and two players

    for value in (0, 1, 0x80000000, 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA, 0x00010000, 0x0000FFFF, 0xFFFF0000):
        t.check("network_count_bits", value)
    for n in range(100):
        t.check("network_count_bits", random_.getrandbits(32) & random_.getrandbits(32) if n % 2 else random_.getrandbits(32))

    for n in range(300):
        identifier = bytearray(random_.randbytes(12))
        other = bytearray(identifier)
        way = random_.randrange(7)
        if way == 0:
            other[random_.randrange(8)] ^= 1 << random_.randrange(8)
        elif way == 1:
            # two of the bits that count
            other[8] ^= random_.choice((1, 2, 3))
        elif way == 2:
            # bits that do not count
            other[8] ^= random_.choice((4, 0x80))
            other[9 + random_.randrange(3)] ^= 0xFF
        t.check(
            "network_player_identifier_equal",
            Buffer(bytes(identifier)) if n % 11 else 0,
            Buffer(bytes(other)) if n % 13 else 0,
            random_.choice((0, 1, 1, 0x80, 0x100)),
        )

    # ---------- two things that the session of the network game is asked

    for name in ("network_life_cycle_session_is_open", "network_life_cycle_unknown_00063f00"):
        for n in range(400):
            data = bytearray(random_.randbytes(0x78B8))
            struct.pack_into("<I", data, STATE, random_.choice((3, 4, 5, 5, 5, 8, 0, 1, 2, 9)))
            struct.pack_into("<I", data, 0x498C, random_.choice((0, 0, 0, 0, 1, NONE)))
            struct.pack_into("<I", data, 0x4994, random_.choice((16, 16, 8, 0)))
            struct.pack_into("<I", data, 0x1118, random_.choice((0, 3, 7, 8, 15, 16, NONE)))
            struct.pack_into("<H", data, 0x5DD0, random_.choice((0xFFFF, 0xFFFF, 0xFFFF, 0, 5)))
            struct.pack_into("<I", data, 0x5DD2, random_.getrandbits(16))
            struct.pack_into("<I", data, 0x4980, random_.choice((1, 1, 1, 0, 2)))
            local = random_.choice((0, 1, 3, NONE))
            struct.pack_into("<I", data, LOCAL_PEER, local)
            struct.pack_into("<I", data, HOST_PEER, random_.choice((local, local, local ^ 1)))
            t.check(
                name,
                globals={
                    "unknown_00527330": bytes((random_.choice((1, 1, 1, 1, 0x80, 0)),)),
                    "unknown_00527334": random_.choice((1, 2, 3, 4, 5, 5, 5, 6, 7, 8, 9, 10)),
                    "network_life_cycle_session": Buffer(bytes(data)),
                },
            )

    # ---------- a session goes into its third, fourth, sixth and tenth states

    nothing = lambda harness, arguments: 0  # noqa: E731

    for n in range(30):
        t.check(
            "network_session_enter_state3",
            Buffer(bytes(whole_session())),
            globals=clock(),
            externals={"GetTickCount": ticks(), "network_session_state3_update": nothing},
        )
        t.check(
            "network_session_enter_state4",
            Buffer(bytes(whole_session())),
            globals=clock(),
            externals={"GetTickCount": ticks(), "network_session_state4_update": nothing},
        )
        t.check(
            "network_session_enter_state6",
            Buffer(bytes(with_channels()), pointers=(8,)),
            externals={"unknown_00075e80": sends},
        )
        t.check("network_session_enter_state10", Buffer(bytes(whole_session())), random_.getrandbits(32))

    for name, at, limit_name, other in (
        ("network_session_unknown_000618d0", 0x7488, "unknown_004cf4a4", "network_session_unknown_000623e0"),
        ("network_session_unknown_00061910", 0x7420, "unknown_004cf4ac", "network_session_state4_update"),
    ):
        for n in range(120):
            data = whole_session()
            now = random_.getrandbits(32)
            limit = random_.choice((0, 1000, 1000, 30000, 0x7FFFFFFF, 0xFFFFFFFF))
            ago = random_.choice((0, 1, 999, 1000, 1001, 29999, 30001, 0x7FFFFFFF, 0x80000001, 0xFFFFFFFF))
            struct.pack_into("<I", data, at, (now - ago) & 0xFFFFFFFF)
            t.check(
                name,
                Buffer(bytes(data)),
                globals={
                    "unknown_00510548": bytes((random_.choice((0, 0, 1)),)),
                    "unknown_0051054c": struct.pack("<I", now),
                    limit_name: struct.pack("<I", limit),
                },
                externals={"GetTickCount": lambda harness, arguments, now=now: now, "network_session_disconnect": nothing, other: nothing},
            )

    for n in range(160):
        data = whole_session()
        peer = random_.randrange(16)
        struct.pack_into("<I", data, 0x742C, random_.choice((peer, peer, random_.randrange(16), NONE)))
        t.check(
            "network_session_unknown_00061950",
            peer,
            Buffer(bytes(data)),
            globals=clock(),
            externals={"GetTickCount": ticks()},
        )

    for n in range(160):
        t.check(
            "network_session_get_channel_peer",
            Buffer(bytes(with_peers())),
            random_.choice((NONE, 0, 1, 2, 3, 4, 5)),
        )

    # ---------- something comes from the console that runs a session

    for name, other in (("network_session_unknown_0005e150", "network_session_host_lost"), ("network_session_unknown_0005e1a0", None)):
        for n in range(60):
            externals = {
                "network_session_address_is_host": lambda harness, arguments, value=random_.choice((1, 1, 1, 0x80, 0, 0x100)): value,
                "network_session_matches_state1_address": lambda harness, arguments, value=random_.choice((1, 0x80, 0, 0, 0x100)): value,
                "network_session_disconnect": nothing,
            }
            if other:
                externals[other] = nothing
            t.check(name, Buffer(random_.randbytes(0x14)), Buffer(bytes(whole_session())), externals=externals)

    # ---------- six things that come from other consoles

    def does(harness, arguments):
        # only a byte of the second thing counts
        return 1, (harness.normalize(arguments[0]), arguments[1] & 0xFF, *arguments[2:])

    def which_channel():
        return lambda harness, arguments, channel=random_.choice((NONE, 0, 1, 2, 3, 4)): channel

    for n in range(80):
        t.check(
            "network_session_host_unknown_0005bec0",
            Buffer(bytes(with_peers())),
            externals={"network_session_host_set_unknown49a8": does},
        )
    for n in range(240):
        message = bytearray(random_.randbytes(0x40))
        struct.pack_into("<I", message, 0x10, random_.choice((0, 1, 1, 2, NONE)))
        t.check(
            "network_session_host_unknown_0005dea0",
            Buffer(bytes(message)),
            Buffer(bytes(with_peers())),
            random_.getrandbits(32),
            externals={"network_observer_find_channel": which_channel(), "network_session_host_set_unknown49a8": does},
        )
    for n in range(240):
        t.check(
            "network_session_unknown_0005e5b0",
            Buffer(bytes(with_peers())),
            random_.getrandbits(32),
            externals={
                "network_observer_find_channel": which_channel(),
                "network_session_host_lost": nothing,
                "network_session_leave": nothing,
            },
        )
    for n in range(240):
        data = with_peers()
        if n % 4:
            struct.pack_into("<I", data, STATE, 8)
        t.check(
            "network_session_unknown_0005e640",
            Buffer(bytes(data)),
            random_.getrandbits(32),
            externals={"network_observer_find_channel": which_channel()},
        )
    for n in range(300):
        data = consistent()
        struct.pack_into("<I", data, LOCAL_PEER, random_.randrange(16))
        message = bytearray(random_.randbytes(0x18))
        struct.pack_into("<I", message, 8, random_.choice((0, 1, 2, 3, 3, 4, NONE, 0x80000000)))
        t.check(
            "network_session_local_player_accepted",
            Buffer(bytes(data), pointers=(8,)),
            Buffer(bytes(message)),
            random_.getrandbits(32),
            externals={
                "network_session_channel_is_host": lambda harness, arguments, value=random_.choice((1, 1, 1, 0x80, 0, 0x100)): value
            },
        )
    for n in range(300):
        data = consistent()
        for peer in range(16):
            data[0x72DC + peer * 0x14] = random_.choice((0, 1, 1, 0x80))
        message = bytearray(random_.randbytes(0x18))
        struct.pack_into("<I", message, 8, random_.choice((0, 1, 2, 3, 3, 4, NONE, 0x80000000)))
        t.check(
            "network_session_host_remove_peer_player",
            Buffer(bytes(data), pointers=(8,)),
            random_.getrandbits(32),
            Buffer(bytes(message)),
            externals={"network_observer_find_channel": which_channel()},
        )

    # ---------- the console that leads a session asks for a change

    def sends_all(harness, arguments):
        # all of what is sent, which is in the stack
        return 0, (arguments[0] & 0xFF, *arguments[1:6], bytes(harness.mu.mem_read(arguments[6], arguments[5] & 0xFFFF)))

    def led() -> Buffer:
        data = with_channels()
        local = random_.randrange(16)
        struct.pack_into("<I", data, LOCAL_PEER, local)
        struct.pack_into("<I", data, HOST_PEER, random_.choice((local, local, local, local ^ 1)))
        struct.pack_into("<I", data, 0x40, random_.randrange(16))
        struct.pack_into("<I", data, STATE, random_.choice((3, 3, 4, 5, 6, 8, 3, 0, 2, 9)))
        for peer in range(16):
            data[0x72DC + peer * 0x14 + 1] = random_.choice((0, 1, 1, 0x80))
        return Buffer(bytes(data), pointers=(8,))

    for name, has_value in (
        ("network_session_leader_unknown_0005b1e0", True),
        ("network_session_leader_set_unknown49a4", True),
        ("network_session_leader_set_unknown49c4", False),
        ("network_session_leader_set_unknown49c8", True),
        ("network_session_leader_clear_unknown49f8", False),
        ("network_session_leader_set_unknown4dac", True),
    ):
        for n in range(80):
            arguments = (led(), random_.getrandbits(32)) if has_value else (led(),)
            t.check(name, *arguments, externals={"unknown_00075e80": sends_all, "network_session_host_set_unknown4980": nothing})

    # ---------- four more changes that the console that leads a session asks for

    for name in (
        "network_session_leader_set_unknown5dd0",
        "network_session_leader_set_language",
        "network_session_leader_set_unknown498c",
    ):
        for n in range(80):
            t.check(name, led(), random_.getrandbits(32), externals={"unknown_00075e80": sends_all})
    for n in range(80):
        t.check(
            "network_session_leader_set_unknown49a1",
            led(),
            Buffer(random_.randbytes(4)),
            externals={"unknown_00075e80": sends_all},
        )

    # ---------- three bigger changes that the console that leads a session asks for

    for n in range(160):
        there = n % 4 != 0
        t.check(
            "network_session_leader_set_roster",
            Buffer(bytes(roster(right=n % 5 != 0))) if there else 0,
            led(),
            externals={"unknown_00075e80": sends_all},
        )
    for n in range(120):
        length = random_.choice((0, 5, 0x7E, 0x7F, 0x80, 0x90))
        text_ = bytes(random_.randrange(1, 256) for _ in range(length)) + bytes(1) + random_.randbytes(0x100 - length)
        t.check(
            "network_session_leader_set_map",
            led(),
            Buffer(text_),
            random_.getrandbits(32),
            random_.getrandbits(32),
            externals={"unknown_00075e80": sends_all},
        )
    for n in range(120):
        thing = bytearray(random_.randbytes(0x130))
        if n % 3 == 0:
            struct.pack_into("<I", thing, 0x44, 0)
        t.check(
            "network_session_leader_set_unknown4db0",
            Buffer(bytes(thing)) if n % 5 else 0,
            led(),
            externals={"unknown_00075e80": sends_all},
        )

    # ---------- two more things that are asked of the console that runs a session

    for n in range(200):
        t.check(
            "network_session_unknown_0005bf10",
            led(),
            random_.choice((0, 1, 0x80, 0x100, 0x17F)),
            random_.getrandbits(32),
            random_.choice((0, 1, 1, 2, NONE)),
            Buffer(random_.randbytes(12)),
            externals={"unknown_00075e80": sends_all, "network_session_host_set_unknown49a8": does},
        )
    for n in range(200):
        session_ = led()
        thing = bytearray(random_.randbytes(0x44))
        if n % 4 == 0:
            thing[4:12] = session_.data[0x1C:0x24]
        t.check(
            "network_session_leader_set_unknown5ddc",
            session_,
            Buffer(bytes(thing)) if n % 5 else 0,
            externals={"unknown_00075e80": sends_all},
        )

    # ---------- leaving a session, and two more

    for n in range(240):
        data = waiting()
        same = struct.unpack_from("<I", data, 0x4980)[0]
        t.check(
            "network_session_host_set_unknown4980",
            Buffer(bytes(data)),
            random_.choice((same, same ^ 1, random_.getrandbits(32))),
            globals=clock(),
            externals={"GetTickCount": ticks()},
        )

    for n in range(400):
        data = with_channels()
        struct.pack_into("<I", data, STATE, random_.choice((0, 1, 2, 3, 4, 5, 5, 6, 7, 7, 8, 9, 9, 10, 10)))
        struct.pack_into("<I", data, 0x54, random_.choice((0, 1, 3, 16)))
        data[0x48] = random_.choice((0, 0, 0, 0, 1, 0x80))
        data[0x7420] = random_.choice((0, 0, 0, 1))
        for peer in range(16):
            data[0x72DC + peer * 0x14 + 1] = random_.choice((0, 1, 1, 0x80))
        t.check(
            "network_session_leave",
            Buffer(bytes(data), pointers=(8,)),
            random_.choice((0, 0, 1, 1, 0x80, 0x100)),
            globals=clock(),
            externals={
                "GetTickCount": ticks(),
                "network_session_state4_update": nothing,
                "unknown_00075e80": sends_all,
                "network_session_enter_state2": nothing,
                "network_session_enter_state7": lambda harness, arguments: (0, (arguments[0] & 0xFF, *arguments[1:])),
                "network_session_disconnect": nothing,
            },
        )

    for n in range(120):
        t.check("network_session_request_unknown49fc", led(), externals={"unknown_00075e80": sends_all})

    # ---------- a session is disconnected, and its seventh state

    for n in range(160):
        data = with_channels()
        struct.pack_into("<I", data, STATE, random_.choice((0, 1, 3, 5, 7, 8, 9, 10)))
        struct.pack_into("<I", data, 0x38, random_.randrange(8))
        data[0x24] = random_.choice((0, 1, 1, 0x80))
        for peer in range(16):
            at = 0x72DC + peer * 0x14
            data[at] = random_.choice((0, 1, 1, 0x80))
            struct.pack_into("<I", data, at + 4, random_.choice((NONE, 0, 1, 2, 3)))
        assert len(data) == OBJECT
        data += struct.pack("<5I", OBJECT + 4, 0x11, 0x22, mock_address(0), 0x44)
        pointers = [8, OBJECT]
        if n % 4:
            struct.pack_into("<I", data, 0x78A8, OBJECT)
            pointers.append(0x78A8)
        else:
            struct.pack_into("<I", data, 0x78A8, 0)
        t.check(
            "network_session_disconnect",
            Buffer(bytes(data), pointers=tuple(pointers)),
            globals={"unknown_004cf7d4": keys(), "unknown_004cf8d4": told()},
            externals={**library, "network_session_leave": nothing},
            mocks=[Mock(lambda call: 0, stack_arguments=0, registers=("ecx",))],
        )

    for n in range(240):
        data = whole_session()
        count = random_.choice((0, 1, 2, 2, 5, 16))
        struct.pack_into("<I", data, 0x54, count)
        struct.pack_into("<I", data, LOCAL_PEER, random_.randrange(16))
        t.check(
            "network_session_enter_state7",
            random_.choice((0, 1, 1, 0x80, 0x100)),
            Buffer(bytes(data)),
            random_.choice((NONE, NONE, random_.randrange(16))),
            globals=clock(),
            externals={"GetTickCount": ticks(), "network_session_leave": nothing},
        )

    for n in range(400):
        data = with_peers()
        struct.pack_into("<I", data, STATE, random_.choice((0, 1, 2, 3, 3, 3, 3, 4, 5, 6, 7, 8, 9, 10)))
        struct.pack_into("<I", data, MEMBERSHIP, random_.choice((0, 0, 0, 7, NONE)))
        struct.pack_into("<I", data, 0x54, random_.choice((0, 1, 2, 2, 5, 16)))
        struct.pack_into("<I", data, 0x40, random_.randrange(16))
        data[0x7420] = random_.choice((0, 0, 1, 0x80))
        t.check(
            "network_session_host_lost",
            Buffer(bytes(data)),
            externals={
                "network_session_disconnect": nothing,
                "network_observer_channel_unknown_00075e40": nothing,
                "network_session_enter_state9": nothing,
            },
        )

    # ---------- what a session does again and again in two states, and its ninth state

    def waits(data: bytearray, at: int, limit_name: str, never: bool = False) -> dict:
        now = random_.getrandbits(32)
        limit = random_.choice((0, 1000, 1000, 30000, 0x7FFFFFFF, 0xFFFFFFFF))
        ago = random_.choice((0, 1, 999, 1000, 1001, 29999, 30001, 0x7FFFFFFF, 0x80000001, 0xFFFFFFFF))
        struct.pack_into("<I", data, at, NONE if never else (now - ago) & 0xFFFFFFFF)
        return {
            "globals": {
                "unknown_00510548": bytes((random_.choice((0, 0, 1)),)),
                "unknown_0051054c": struct.pack("<I", now),
                limit_name: struct.pack("<I", limit),
            },
            "tick": lambda harness, arguments, now=now: now,
        }

    for n in range(200):
        data = with_channels()
        struct.pack_into("<I", data, 0x40, random_.randrange(16))
        for peer in range(16):
            data[0x72DC + peer * 0x14 + 1] = random_.choice((0, 1, 1, 0x80))
        setup = waits(data, 0x7424, "unknown_004cf4b0", never=n % 5 == 0)
        t.check(
            "network_session_state4_update",
            Buffer(bytes(data), pointers=(8,)),
            globals=setup["globals"],
            externals={"GetTickCount": setup["tick"], "unknown_00075e80": sends_all},
        )

    def sends_to_address(harness, arguments):
        # what is sent is in the stack
        return 0, (arguments[0], harness.normalize(arguments[1]), arguments[2], arguments[3], bytes(harness.mu.mem_read(arguments[4], arguments[3] & 0xFF)))

    for n in range(200):
        data = whole_session()
        setup = waits(data, 0x748C, "unknown_004cf4a0")
        t.check(
            "network_session_unknown_000623e0",
            Buffer(bytes(data)),
            globals=setup["globals"],
            externals={"GetTickCount": setup["tick"], "unknown_0007b140": sends_to_address},
        )

    def fills(harness, arguments):
        # where it goes is in the stack; what was there has to be zeros
        before = bytes(harness.mu.mem_read(arguments[1], 0xC0))
        harness.mu.mem_write(arguments[1], bytes(range(1, 0xC1)))
        return 0x1234, (harness.normalize(arguments[0]), before)

    for n in range(60):
        data = whole_session()
        struct.pack_into("<I", data, LOCAL_PEER, random_.randrange(16))
        struct.pack_into("<I", data, 0x40, random_.randrange(16))
        t.check(
            "network_session_enter_state9",
            Buffer(bytes(data)),
            globals=clock(),
            externals={"GetTickCount": ticks(), "network_session_election_new": fills},
        )

    # ---------- the second state of a session, and what it does again and again in the third

    for n in range(60):
        data = with_channels()
        for peer in range(16):
            at = 0x72DC + peer * 0x14
            data[at] = random_.choice((0, 1, 1, 0x80))
            struct.pack_into("<I", data, at + 4, random_.choice((NONE, 0, 1, 2, 3)))
        t.check(
            "network_session_enter_state2",
            Buffer(bytes(data), pointers=(8,)),
            globals=clock(),
            externals={"GetTickCount": ticks(), "network_session_unknown_000623e0": nothing},
        )

    for n in range(400):
        data = with_channels()
        struct.pack_into("<I", data, 0x40, random_.randrange(16))
        struct.pack_into("<I", data, STATE, random_.choice((3, 3, 3, 3, 4, 7, 8, 5)))
        data[0x7420] = random_.choice((0, 0, 1))
        for channel in range(4):
            struct.pack_into("<I", data, CHANNELS + channel * 0x528 + 0xA8, random_.choice((7, 7, 7, 6)))
        now = random_.getrandbits(32)
        globals_ = {
            "unknown_00510548": bytes((random_.choice((0, 0, 1)),)),
            "unknown_0051054c": struct.pack("<I", now),
        }
        for at, name in ((0x7654, "unknown_004cf4d8"), (0x7658, "unknown_004cf4dc")):
            ago = random_.choice((0, 999, 1000, 1001, 30001, 0x80000001))
            struct.pack_into("<I", data, at, random_.choice((NONE, NONE, (now - ago) & 0xFFFFFFFF, (now - ago) & 0xFFFFFFFF)))
            globals_[name] = struct.pack("<I", random_.choice((0, 1000, 1000, 30000, 0xFFFFFFFF)))
        t.check(
            "network_session_state3_update",
            Buffer(bytes(data), pointers=(8,)),
            globals=globals_,
            externals={"GetTickCount": lambda harness, arguments, now=now: now, "unknown_00075e80": sends_all},
        )

    # ---------- a thing that the console that runs a session sets

    for n in range(600):
        data = whole_session()
        host = random_.randrange(4)
        struct.pack_into("<I", data, HOST_PEER, host)
        data[0x49A8] = random_.choice((0, 1, 1, 0x80))
        struct.pack_into(
            "<iii",
            data,
            0x49AC,
            random_.choice((0, 1, 5, 9)),
            random_.choice((0, 0, 1, 2)),
            random_.choice((-1, 0, 5, 6, 9)),
        )
        t.check(
            "network_session_host_set_unknown49a8",
            Buffer(bytes(data)),
            random_.choice((0, 1, 1, 0x80, 0x100)),
            random_.choice((0, 1, 5, 9, 0xFFFFFFFF)),
            random_.choice((0, 1, 1, 2)),
            random_.choice((host, host, host ^ 1)),
            Buffer(random_.randbytes(12)),
        )

    # ---------- what a session has in its ninth state

    for n in range(60):
        data = whole_session()
        struct.pack_into("<I", data, LOCAL_PEER, random_.randrange(16))
        struct.pack_into("<I", data, 0x40, random_.randrange(16))
        t.check("network_session_election_new", Buffer(bytes(data)), Buffer(random_.randbytes(0xC0)))

    def election():
        data = bytearray(random_.randbytes(0xC0))
        count = random_.choice((0, 1, 2, 5, 15, 16))
        struct.pack_into("<I", data, 0x54, count)
        address = bytearray(random_.randbytes(0x24))
        way = random_.randrange(4)
        if way and count:
            # one of the consoles that are known of; two of them can have the same six bytes
            index = random_.randrange(count)
            address[0xA:0x10] = data[0x58 + index * 6 : 0x5E + index * 6]
            if way == 2:
                other = random_.randrange(16)
                data[0x58 + other * 6 : 0x5E + other * 6] = address[0xA:0x10]
            if way == 3:
                address[0xF] ^= 1
        return Buffer(bytes(data)), Buffer(bytes(address))

    for n in range(300):
        election_, address = election()
        t.check("network_session_election_find", election_, address)
    for name in ("network_session_election_add_unknownB8", "network_session_election_add_unknownBC"):
        for n in range(300):
            election_, address = election()
            t.check(name, address, election_)

    # ---------- two more of what a session has in its ninth state

    for n in range(400):
        election_, address = election()
        local = bytes(election_.data[0x24:0x48])
        way = random_.randrange(5)
        if way == 0:
            first, second = Buffer(local), address
        elif way == 1:
            first, second = address, Buffer(local)
        elif way == 2:
            first, second = Buffer(local), 0
        elif way == 3:
            first, second = Buffer(local[:0x23] + bytes((local[0x23] ^ 1,))), address
        else:
            first, second = Buffer(local), Buffer(local)
        t.check("network_session_election_add_other", first, election_, second)
    for n in range(400):
        election_, address = election()
        t.check(
            "network_session_election_set_unknownBC",
            election_,
            address,
            random_.choice((0, 1, 1, 0x80, 0x100)),
        )

    # ---------- the map of the session of the network game

    def game_session() -> dict:
        data = bytearray(random_.randbytes(0x78B8))
        struct.pack_into("<I", data, STATE, random_.choice((3, 4, 5, 5, 5, 8, 0, 1, 2, 9)))
        struct.pack_into("<I", data, 0x4980, random_.choice((1, 1, 1, 0, 2)))
        local = random_.choice((0, 1, 3, NONE))
        struct.pack_into("<I", data, LOCAL_PEER, local)
        struct.pack_into("<I", data, HOST_PEER, random_.choice((local, local, local ^ 1)))
        return {
            "unknown_00527330": bytes((random_.choice((1, 1, 1, 1, 0x80, 0)),)),
            "network_life_cycle_session": Buffer(bytes(data)),
        }

    def says(values=(1, 1, 1, 0x80, 0, 0x100)):
        return lambda harness, arguments, value=random_.choice(values): (
            value,
            tuple(harness.normalize(argument) for argument in arguments),
        )

    for n in range(200):
        t.check(
            "network_life_cycle_set_map",
            random_.getrandbits(32),
            random_.getrandbits(32),
            Buffer(b"scenarios\\multi\\lockout\\lockout\0"),
            globals=game_session(),
            externals={"network_session_leader_set_map": says()},
        )
    for n in range(200):
        t.check(
            "network_life_cycle_set_unknown49a4",
            random_.getrandbits(32),
            globals=game_session(),
            externals={"network_session_leader_set_unknown49a4": says()},
        )
    for n in range(300):
        t.check(
            "network_life_cycle_set_map_and_unknown49a8",
            random_.getrandbits(32),
            random_.getrandbits(32),
            Buffer(b"scenarios\\multi\\lockout\\lockout\0"),
            globals=game_session(),
            externals={
                "network_session_leader_set_map": says(),
                # only a byte of the second thing counts
                "network_session_unknown_0005bf10": lambda harness, arguments, value=random_.choice((1, 1, 0, 0x100)): (
                    value,
                    (harness.normalize(arguments[0]), arguments[1] & 0xFF, *arguments[2:]),
                ),
            },
        )

    # ---------- a console is taken out with another message

    for n in range(300):
        data = consistent()
        for peer in range(16):
            data[0x72DC + peer * 0x14] = random_.choice((0, 1, 1))
        struct.pack_into("<I", data, STATE, random_.choice((3, 5, 5, 5, 6, 7, 8, 9, 1)))
        struct.pack_into("<I", data, 0x742C, random_.randrange(16))
        count = struct.unpack_from("<I", data, 0x54)[0]
        assert len(data) == OBJECT
        data += struct.pack("<5I", OBJECT + 4, 0x11, mock_address(0), 0x33, 0x44)
        pointers = [8, OBJECT]
        if n % 4:
            struct.pack_into("<I", data, 0x78A8, OBJECT)
            pointers.append(0x78A8)
        else:
            struct.pack_into("<I", data, 0x78A8, 0)
        t.check(
            "network_session_remove_peer_with_message_c",
            Buffer(bytes(data), pointers=tuple(pointers)),
            random_.randrange(count),
            globals=clock(),
            externals={"GetTickCount": ticks(), "unknown_00075e80": sends_all},
            # the two numbers that it is given are in the stack: only that it is told is compared
            mocks=[Mock(lambda call: 0, stack_arguments=1, registers=("ecx",), record=lambda call: ())],
        )

    # ---------- what changed of a console is put in it

    for n in range(300):
        update = bytearray(random_.randbytes(0xD8))
        update[2:0x22] = text(16)
        update[0x22:0x62] = text(32)
        for at in (0, 0x62, 0x6C, 0x8C, 0xD0):
            update[at] = random_.choice((0, 1, 1, 0x80))
        t.check(
            "network_session_peer_properties_apply_update",
            Buffer(random_.randbytes(0xC8)),
            Buffer(bytes(update)),
        )

    # ---------- something that comes in the seventh state

    for n in range(400):
        data = with_peers()
        struct.pack_into("<I", data, STATE, random_.choice((7, 7, 7, 7, 8, 5)))
        data[0x7430] = random_.choice((1, 1, 1, 0x80, 0))
        count = random_.choice((1, 3, 16))
        struct.pack_into("<I", data, 0x54, count)
        said = random_.randrange(count)
        struct.pack_into("<I", data, 0x742C, random_.choice((said, said, said, said ^ 1)))
        message = bytearray(random_.randbytes(0x30))
        message[8:0x2C] = data[PEERS + said * PEER : PEERS + said * PEER + 0x24]
        struct.pack_into("<H", message, 0x2C, random_.choice((said, said, said, said, count, 0xFFFF)))
        if n % 5 == 0:
            message[8 + random_.randrange(0x24)] ^= 1
        t.check(
            "network_session_unknown_0005e340",
            Buffer(bytes(data)),
            random_.getrandbits(32),
            Buffer(bytes(message)),
            externals={"network_observer_find_channel": which_channel()},
        )

    # ---------- four more things that a session does with what comes

    for n in range(400):
        data = with_peers()
        struct.pack_into("<I", data, STATE, random_.choice((9, 9, 9, 9, 3, 8)))
        struct.pack_into("<I", data, 0x7430 + 0x54, random_.choice((0, 1, 5, 16)))
        peer = random_.choice((NONE, 0, 1, 2, 3, 5))
        if peer != NONE and random_.randrange(2):
            # the console is one of those that are known of
            at = 0x7430 + 0x58 + random_.randrange(16) * 6
            data[at : at + 6] = data[PEERS + peer * PEER + 0xA : PEERS + peer * PEER + 0x10]
        # which console the address is of is said by what the console's library says that it is known by
        known_by = random_.randbytes(0x24) if peer == NONE else bytes(data[PEERS + peer * PEER : PEERS + peer * PEER + 0x24])

        def of_peer(harness, arguments, known_by=known_by):
            harness.mu.mem_write(arguments[5], known_by)
            return 1, tuple(arguments[:5])

        t.check(
            "network_session_unknown_0005ece0",
            Buffer(bytes(data)),
            Buffer(random_.randbytes(0x14)),
            globals=clock(),
            externals={"GetTickCount": ticks(), "transport_security_peer_address_from_address": of_peer},
        )

    for name in ("network_session_host_change_leader", "network_session_host_leader_removes_peer"):
        for n in range(400):
            data = with_peers()
            struct.pack_into("<I", data, STATE, random_.choice((5, 5, 5, 6, 7, 8, 3, 0)))
            struct.pack_into("<I", data, 0x54, random_.choice((2, 4, 16)))
            struct.pack_into("<I", data, MEMBERSHIP, 0)
            leader = random_.randrange(4)
            struct.pack_into("<I", data, HOST_PEER, leader)
            struct.pack_into("<I", data, LOCAL_PEER, random_.choice((leader ^ 1, leader ^ 1, leader ^ 1, leader)))
            for peer in range(16):
                at = 0x72DC + peer * 0x14
                data[at] = 1
                struct.pack_into("<I", data, at + 4, peer)
            message = bytearray(random_.randbytes(0x30))
            said = random_.randrange(4)
            if n % 5:
                message[8:0x2C] = data[PEERS + said * PEER : PEERS + said * PEER + 0x24]
            t.check(
                name,
                Buffer(bytes(data)),
                random_.getrandbits(32),
                Buffer(bytes(message)),
                externals={
                    "network_observer_find_channel": lambda harness, arguments, channel=random_.choice((leader, leader, leader, leader ^ 1, NONE)): channel,
                    "network_session_remove_peer_with_message_e": nothing,
                },
            )

    for n in range(400):
        data = with_peers()
        data[0x7420] = random_.choice((0, 0, 1))
        for peer in range(16):
            struct.pack_into("<I", data, PEERS + peer * PEER + 0xF8, random_.choice((0, 0, 1, 4)))
        t.check(
            "network_session_unknown_0005f3c0",
            Buffer(bytes(data)),
            random_.choice((NONE, 0, 1, 2, 3, 4)),
            random_.choice((0, 0, 1, 0x80, 0x100)),
        )

    # ---------- three things that a console says of itself and of its players

    def hosted_with_channels() -> tuple:
        # a session that this console runs, most of the time; each console has the channel of its number
        data = consistent()
        struct.pack_into("<I", data, STATE, random_.choice((5, 5, 5, 6, 7, 8, 3, 0)))
        struct.pack_into("<I", data, 0x54, random_.choice((4, 16, 16)))
        struct.pack_into("<I", data, MEMBERSHIP, random_.choice((0, 7, NONE)))
        struct.pack_into("<I", data, LOCAL_PEER, random_.choice((0, 0, 1, 2, 3, NONE)))
        for peer in range(16):
            at = 0x72DC + peer * 0x14
            data[at] = 1
            data[at + 1] = random_.choice((0, 1, 1, 0x80))
            struct.pack_into("<I", data, at + 4, peer)
        return data, random_.choice((0, 1, 2, 3, 3, NONE))

    def from_channel(channel: int):
        return lambda harness, arguments: channel

    for n in range(400):
        data, channel = hosted_with_channels()
        message = bytearray(random_.randbytes(0xD0))
        if channel != NONE:
            at = PEERS + channel * PEER
            data[at + 0x24] = random_.choice((0, 0, 1, 0x80))
            if n % 3:
                message[8:0xD0] = data[at + 0x28 : at + 0xF0]
            if n % 3 == 2:
                # one byte of it is another
                message[8 + random_.randrange(0xC8)] ^= 0x10
        t.check(
            "network_session_host_set_peer_properties",
            Buffer(bytes(data), pointers=(8,)),
            random_.getrandbits(32),
            Buffer(bytes(message)),
            externals={"network_observer_find_channel": from_channel(channel)},
        )

    for n in range(400):
        data, channel = hosted_with_channels()
        message = bytearray(random_.randbytes(0xB0))
        struct.pack_into("<I", message, 8, random_.choice((0, 1, 2, 3)))

        def comes_in(harness, arguments, player=random_.choice((NONE, NONE, random_.randrange(16)))):
            # what the player is known by is in what comes
            return player, (harness.normalize(arguments[0]), arguments[1], arguments[2], bytes(harness.mu.mem_read(arguments[3], 12)))

        t.check(
            "network_session_host_add_peer_player",
            Buffer(bytes(data), pointers=(8,)),
            random_.getrandbits(32),
            Buffer(bytes(message)),
            externals={
                "network_observer_find_channel": from_channel(channel),
                "network_session_add_peer_player": comes_in,
                "unknown_00075e80": sends_all,
            },
        )

    for n in range(500):
        data, channel = hosted_with_channels()
        message = bytearray(random_.randbytes(0xA4))
        place = random_.choice((0, 1, 2, 3, 3, 4, NONE, 0x80000000))
        struct.pack_into("<I", message, 8, place)
        if channel != NONE and place < 4:
            if n % 4 == 0:
                struct.pack_into("<I", data, PEERS + channel * PEER + 0xFC + place * 4, NONE)
            player = struct.unpack_from("<I", data, PEERS + channel * PEER + 0xFC + place * 4)[0]
            if player != NONE:
                # each of the three things is what the player has already, or not
                at = PLAYER_ARRAY + player * PLAYER
                if random_.randrange(3):
                    message[0xC:0x10] = data[at + 0x14 : at + 0x18]
                if random_.randrange(3):
                    message[0x10:0xA0] = data[at + 0x18 : at + 0xA8]
                    if random_.randrange(4) == 0:
                        message[0x10 + random_.randrange(0x90)] ^= 0x10
                if random_.randrange(3):
                    message[0xA0:0xA4] = data[at + 0x138 : at + 0x13C]
        t.check(
            "network_session_host_set_peer_player",
            Buffer(bytes(data), pointers=(8,)),
            random_.getrandbits(32),
            Buffer(bytes(message)),
            externals={"network_observer_find_channel": from_channel(channel)},
        )

    # ---------- three more things that come: of the time, of a console, and by a channel

    for n in range(500):
        data = with_peers()
        data[0x7420] = random_.choice((0, 0, 1))
        message = bytearray(random_.randbytes(0x1C))
        struct.pack_into("<H", message, 0x18, random_.choice((0, 0, 1, 1, 2, 0x100, 0x101)))
        if n % 5 == 0:
            # times that are near each other
            base = random_.getrandbits(32)
            for at in (8, 0xC, 0x10, 0x14):
                struct.pack_into("<I", message, at, (base + random_.randrange(-50, 50)) & 0xFFFFFFFF)
        t.check(
            "network_session_synchronize_time",
            Buffer(bytes(data)),
            Buffer(random_.randbytes(0x14)),
            Buffer(bytes(message)),
            globals=clock(),
            externals={
                "GetTickCount": ticks(),
                "transport_security_peer_address_from_address": known(data),
                "network_session_matches_state1_address": lambda harness, arguments, same=random_.choice((0, 0, 1, 1, 0x80, 0x100)): same,
                "unknown_0007b140": sends_to_address,
            },
        )

    def leaves(harness, arguments):
        return 0, (harness.normalize(arguments[0]), arguments[1])

    for n in range(500):
        data = consistent()
        data[0x7420] = random_.choice((0, 0, 1))
        struct.pack_into("<I", data, 0x40, random_.choice((0, 1, 2, 3, 3, NONE)))
        for peer in range(16):
            data[0x72DC + peer * 0x14 + 1] = random_.choice((0, 1, 1, 0x80))
        message = bytearray(random_.randbytes(0x30))
        said = random_.choice((0, 0, 1, 2, 3, 5, 15, 16, 0x7FFF, 0x8000, 0xFFFF))
        struct.pack_into("<H", message, 0x2C, said)
        if said < 16 and n % 5:
            message[8:0x2C] = data[PEERS + said * PEER : PEERS + said * PEER + 0x24]
            if n % 5 == 1:
                message[8 + random_.randrange(0x24)] ^= 4
        t.check(
            "network_session_unknown_0005e210",
            Buffer(bytes(data), pointers=(8,)),
            Buffer(bytes(message)),
            externals={"network_session_leave": leaves, "unknown_00075e80": sends_all},
        )

    for n in range(500):
        data = consistent()
        struct.pack_into("<I", data, 0x40, random_.randrange(16))
        for peer in range(16):
            data[0x72DC + peer * 0x14] = random_.choice((0, 1, 1, 0x80))
        t.check(
            "network_session_unknown_0005e6b0",
            Buffer(bytes(data), pointers=(8,)),
            random_.getrandbits(32),
            externals={"network_observer_find_channel": which_channel(), "unknown_00075e80": sends_all},
        )

    # ---------- six things that this console says of itself, of its players and of the session it leads

    def local_session() -> bytearray:
        # this console is one of the session, and so is the one that runs it
        data = consistent()
        struct.pack_into("<I", data, STATE, random_.choice((5, 5, 6, 7, 8, 3, 3, 4, 4, 0, 1, 9)))
        struct.pack_into("<I", data, 0x54, random_.choice((4, 16, 16)))
        struct.pack_into("<I", data, MEMBERSHIP, random_.choice((0, 7, NONE)))
        struct.pack_into("<I", data, LOCAL_PEER, random_.randrange(16))
        struct.pack_into("<I", data, 0x40, random_.randrange(16))
        for peer in range(16):
            data[0x72DC + peer * 0x14 + 1] = random_.choice((0, 1, 1, 0x80))
        return data

    for n in range(300):
        t.check(
            "network_session_set_local_peer_properties",
            Buffer(random_.randbytes(0xC8)),
            Buffer(bytes(local_session()), pointers=(8,)),
            externals={"unknown_00075e80": sends_all},
        )

    for n in range(400):

        def comes_in_here(harness, arguments, player=random_.choice((NONE, random_.randrange(16), random_.randrange(16)))):
            return player, (harness.normalize(arguments[0]), arguments[1], arguments[2], bytes(harness.mu.mem_read(arguments[3], 12)))

        t.check(
            "network_session_add_local_player",
            Buffer(bytes(local_session()), pointers=(8,)),
            Buffer(random_.randbytes(0x90)),
            Buffer(random_.randbytes(12)),
            random_.randrange(4),
            random_.getrandbits(32),
            random_.getrandbits(32),
            externals={"network_session_add_peer_player": comes_in_here, "unknown_00075e80": sends_all},
        )

    for n in range(400):
        t.check(
            "network_session_set_local_player",
            Buffer(bytes(local_session()), pointers=(8,)),
            Buffer(random_.randbytes(0x90)),
            random_.getrandbits(32),
            random_.randrange(4),
            random_.getrandbits(32),
            externals={"unknown_00075e80": sends_all},
        )

    for n in range(300):
        t.check(
            "network_session_remove_local_player",
            Buffer(bytes(local_session()), pointers=(8,)),
            random_.randrange(4),
            externals={
                "network_session_remove_player": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                "unknown_00075e80": sends_all,
            },
        )

    for name in ("network_session_change_leader", "network_session_leader_remove_peer"):
        for n in range(400):
            data = local_session()
            local = struct.unpack_from("<I", data, LOCAL_PEER)[0]
            struct.pack_into("<I", data, HOST_PEER, local if n % 4 else local ^ 1)
            said = random_.choice((local, random_.randrange(4), random_.randrange(16), random_.randrange(16)))
            known_as = bytes(data[PEERS + said * PEER : PEERS + said * PEER + 0x24])
            if n % 6 == 0:
                known_as = random_.randbytes(0x24)
            t.check(
                name,
                Buffer(bytes(data), pointers=(8,)),
                Buffer(known_as),
                externals={
                    "network_session_remove_peer_with_message_e": lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1]))),
                    "unknown_00075e80": sends_all,
                },
            )
