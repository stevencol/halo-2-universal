# source/networking/network_message_handler.c: the messages of the sessions

import random
import struct

from difftest import Buffer

from test_object_types_more import after

NONE = 0xFFFFFFFF
SESSION = 0x7460

SIMPLE = ('00094310', '00094330', '00094350', '00094610', '00094800', '00094380', '00094640', '00094680', '000946c0', '00094700', '00094780', '000947c0', '00094830', '00094870', '00094420', '00094440', '00094460', '000945d0', '000945f0', '00094740', '000948c0')


def run(t) -> None:
    random_ = random.Random(730)

    for n in range(120):
        session = bytearray(random_.randbytes(SESSION))
        struct.pack_into("<I", session, 0x741C, random_.choice((0, 2, 3, 4, 4, 5, 6, 7, 8, 9)))
        local = random_.randrange(4)
        struct.pack_into("<I", session, 0x72D8, local)
        struct.pack_into("<I", session, 0x40, random_.randrange(4))
        struct.pack_into("<I", session, 0x44, random_.choice((local, local, random_.randrange(4))))
        struct.pack_into("<I", session, 0x4980, random_.choice((4, 4, 0, 5)))

        def check(name: str, *arguments) -> None:
            # the session is after the buffers that are arguments; one time in five there is none
            there = after(*(one for one in arguments if isinstance(one, Buffer)))
            found = 0 if random_.random() < 0.2 else there
            answers = {}

            def an_answer(key: str, values, mask=None):
                value = random_.choice(values)

                def handler(harness, arguments_, value=value):
                    record = tuple(arguments_)
                    if mask is not None:
                        record = record[:mask] + (record[mask] & 0xFF,) + record[mask + 1 :]
                    return value, record

                answers[key] = handler

            def sent(harness, arguments_):
                data = bytearray(harness.mu.mem_read(arguments_[4], 12))
                data[2:4] = b"\\0\\0"
                return 1, (*arguments_[:4], bytes(data))

            an_answer("network_sessions_find", (found,))
            an_answer("network_session_channel_is_host", (0, 1, 1))
            an_answer("network_session_unknown_00058d90", (0, 1, 1))
            an_answer("network_session_find_peer_by_transport_address", (NONE, 0, 1, 2, 3))
            an_answer("network_session_enter_state8", (0,), mask=1)
            for other in OTHERS:
                an_answer(other, (0, 1))
            answers["network_message_gateway_send"] = sent
            t.check(name, *arguments, extra=[Buffer(bytes(session))], externals=answers)

        owner = lambda: Buffer(random_.randbytes(0x18))
        message = lambda: Buffer(random_.randbytes(0x20))
        for address in SIMPLE:
            check("network_message_handler_unknown_" + address, owner(), message(), random_.getrandbits(32))
        for address in ("000943d0", "00094270", "000942d0"):
            check("network_message_handler_unknown_" + address, owner(), message(), random_.getrandbits(32))
        check(
            "network_message_handler_unknown_00094100",
            Buffer(struct.pack("<h", random_.choice((2, 2, 2, 1))) + random_.randbytes(0x1E)),
            owner(),
            random_.getrandbits(32),
        )
        check("network_message_handler_unknown_00093f60", message(), owner(), random_.getrandbits(32))


OTHERS = (
    "network_session_unknown_0005e150",
    "network_session_unknown_0005e1a0",
    "network_session_unknown_0005e210",
    "network_session_membership_update_received",
    "network_session_parameters_update_received",
    "network_session_unknown_0005e340",
    "network_session_host_set_peer_properties",
    "network_session_host_change_leader",
    "network_session_host_leader_removes_peer",
    "network_session_host_add_peer_player",
    "network_session_host_remove_peer_player",
    "network_session_host_set_peer_player",
    "network_session_host_parameters_requested",
    "network_session_host_unknown_0005dea0",
    "network_session_unknown_0005e3f0",
    "network_session_unknown_0005e5b0",
    "network_session_unknown_0005e640",
    "network_session_unknown_0005ece0",
    "network_session_synchronize_time",
    "network_session_local_player_accepted",
    "network_session_peer_answered",
    "network_session_host_join_requested",
    "network_session_remove_peer_with_message_c",
    "network_session_disconnect",
)
