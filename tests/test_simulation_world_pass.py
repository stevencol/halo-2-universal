# source/networking/simulation_world.c: what the players and the other things do is passed on to the views

import hashlib
import random
import struct

from difftest import Buffer

WORLD = 0x8FC + 16 * 0x90
VIEW = 0x40
DATA = 0x5098 + 0x18 + 32 * 0x34
DATUM = 0x21C


def a_machine(random_) -> bytes:
    return bytes((random_.choice((1, 2)),)) * 5 + bytes((random_.choice((1, 2, 3)),))


def run(t) -> None:
    random_ = random.Random(588)

    def packs(*where):
        def stand_in(harness, arguments):
            packed, actions = (arguments[index] for index in where)
            given = bytes(harness.mu.mem_read(actions, 0x5C))
            harness.mu.mem_write(packed, (hashlib.sha256(given).digest() * 2)[:0x34])
            return 0, (harness.normalize(actions), arguments[2])

        return stand_in

    for n in range(300):
        # the thing that is going, then some views, each with what it has at 0x8
        count = random_.choice((0, 1, 3, 6))
        data = bytearray(random_.randbytes(WORLD + count * (VIEW + DATA)))
        pointers = []
        for index in range(15):
            struct.pack_into("<I", data, 0x40 + index * 4, 0)
        struct.pack_into("<i", data, 0x3C, count)
        for index in range(count):
            at = WORLD + index * (VIEW + DATA)
            struct.pack_into("<I", data, 0x40 + index * 4, at)
            struct.pack_into("<H", data, at + 2, random_.choice((4, 4, 4, 2, 3)))
            struct.pack_into("<I", data, at + 8, at + VIEW)
            data[at + 0x14 : at + 0x1A] = a_machine(random_)
            pointers += [0x40 + index * 4, at + 8]
        for index in range(16):
            player = 0x7C + index * 0x88
            struct.pack_into("<i", data, player, random_.choice((-1, index, index, 15 - index)))
            struct.pack_into("<I", data, player + 4, random_.getrandbits(16) << 16 | random_.randrange(4))
            data[player + 0x18 : player + 0x1E] = a_machine(random_)
            other = 0x8FC + index * 0x90
            struct.pack_into("<i", data, other, random_.choice((-1, index, index, 15 - index)))
        datums = bytearray(random_.randbytes(4 * DATUM))
        for index in range(4):
            struct.pack_into("<i", datums, index * DATUM + 0x2C, random_.choice((-1, 7, 7)))
        array = bytearray(random_.randbytes(0x4C))
        struct.pack_into("<I", array, 0x44, 0)
        players = Buffer(bytes(array) + bytes(datums), pointers=(0x44,))
        struct.pack_into("<I", array, 0x44, 0x4C)
        players = Buffer(bytes(array) + bytes(datums), pointers=(0x44,))
        mask = random_.choice((0, 1, 0x8001, 0xFFFF, random_.getrandbits(16), random_.getrandbits(32)))

        t.check(
            "simulation_world_pass_player_actions",
            Buffer(random_.randbytes(16 * 0x5C)),
            Buffer(bytes(data), pointers=tuple(pointers)),
            mask,
            globals={"player_data": players},
            externals={"unknown_000823d0": packs(0, 1)},
        )
        t.check(
            "simulation_world_pass_other_actions",
            Buffer(random_.randbytes(16 * 0x7C)),
            Buffer(bytes(data), pointers=tuple(pointers)),
            mask,
            externals={"unknown_000824d0": packs(1, 0)},
        )
