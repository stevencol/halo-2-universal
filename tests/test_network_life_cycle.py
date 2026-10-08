# source/networking/network_session.c and network_life_cycle.c

import random
import struct

from difftest import Buffer, GlobalRange

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
            globals={"unknown_00527334": life_cycle_state, "unknown_00527500": GlobalRange(4)},
            externals={
                "unknown_00070570": lambda harness, arguments, result=random_.getrandbits(32): (
                    result,
                    (harness.normalize(arguments[0]),),
                )
            },
        )
