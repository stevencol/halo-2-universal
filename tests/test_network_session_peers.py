# source/networking/network_session.c: what is asked of all the peers of a session

import random
import struct

from difftest import Buffer

PEER_COUNT, PEERS, PEER, MAXIMUM_PEERS = 0x54, 0x58, 0x10C, 16


def run(t) -> None:
    random_ = random.Random(536)

    for n in range(3000):
        session = bytearray(random_.randbytes(PEERS + MAXIMUM_PEERS * PEER))
        struct.pack_into("<I", session, PEER_COUNT, random_.choice((0, 1, 2, 4, 15, 16, random_.randrange(17))))
        for index in range(MAXIMUM_PEERS):
            struct.pack_into("<I", session, PEERS + index * PEER + 0x88, random_.choice((0, 1, 2, 3, 3, 3, 4, 8, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF)))
        t.check(
            "network_session_peers_unknown88_above_2",
            Buffer(bytes(session)),
            random_.choice((Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4)), 0)),
        )
