# source/saved games/player_profile.c, and async_set_file_position of
# source/cseries/async.c

import random
import struct

from difftest import Buffer, Pointer

NONE = 0xFFFFFFFF
PROFILE_SIZE = 0x1E0
APPEARANCE = 0x118
RESULT_SIZE = 0x10C


def run(t) -> None:
    random_ = random.Random(59)

    def appearance(valid: bool = True) -> bytes:
        colors = [random_.choice((-1, 0, 10, 17)) for _ in range(4)]
        return struct.pack("<5b3B8s", *colors, random_.choice((-1, 0, 3)), random_.randrange(64), random_.randrange(32), random_.randrange(16), random_.randbytes(8))

    def profile(appearance_: bytes | None = None, unknown151: int | None = None) -> bytes:
        data = bytearray(random_.randbytes(PROFILE_SIZE))
        data[APPEARANCE : APPEARANCE + 16] = appearance(True) if appearance_ is None else appearance_
        data[0x151] = random_.randrange(3) if unknown151 is None else unknown151
        return bytes(data)

    # ---------- the looks of a player

    def with_byte(offset: int, value: int) -> bytes:
        data = bytearray(appearance())
        data[offset] = value & 0xFF
        return bytes(data)

    looks = [appearance() for _ in range(20)]
    for offset in range(4):
        looks += [with_byte(offset, value) for value in (-2, -1, 0, 17, 18, 19, 0x7F, -128)]
    looks += [with_byte(4, value) for value in (-2, -1, 0, 3, 4, 5, 0x7F, -128)]
    looks += [with_byte(5, value) for value in (0, 63, 64, 65, 0xFF)]
    looks += [with_byte(6, value) for value in (0, 31, 32, 33, 0xFF)]
    looks += [with_byte(7, value) for value in (0, 0xF, 0x10, 0x20, 0x40, 0x80, 0xFF)]
    for look in looks:
        t.check("player_appearance_valid", Buffer(look))

    # ---------- new profiles

    for _ in range(8):
        t.check("player_profile_set_defaults", Buffer(random_.randbytes(PROFILE_SIZE + 8)))
    for how in (0, 1, 2, NONE, 0x80000000, 0x101):
        for _ in range(3):
            t.check("player_profile_new", Buffer(random_.randbytes(PROFILE_SIZE + 8)), how)

    # ---------- reading

    def read(success: int, data: bytes | None):
        def handler(harness, arguments):
            file, buffer, size = arguments
            if data is not None:
                harness.mu.mem_write(buffer, data)
            # where it is read to is in the stack
            return success, (file, size)

        return handler

    bad_looks = [with_byte(0, 18), with_byte(4, 4), with_byte(5, 64), with_byte(7, 0x10)]
    for n in range(80):
        look = random_.choice(bad_looks) if n % 3 == 0 else None
        read_profile = profile(look, random_.choice((0, 1, 2, 3, 4, 0xFF)))
        success = random_.choice((1, 1, 0, 0x80))
        externals = {"saved_game_file_read": read(success, read_profile if n % 5 else None)}
        given = profile(random_.choice(bad_looks) if n % 4 == 0 else None, random_.choice((0, 2, 3, 0xFF)))
        t.check("player_profile_read", random_.getrandbits(32), Buffer(given), externals=externals)
        t.check("player_profile_load", Buffer(given), random_.getrandbits(32) if n % 4 else NONE, externals=externals)

    # ---------- writing

    for n in range(40):
        file = NONE if n % 5 == 4 else random_.getrandbits(32)
        result = random_.choice((0, 1, 1))
        t.check(
            "player_profile_write",
            file,
            Buffer(profile()),
            externals={"saved_game_file_write": lambda harness, arguments, result=result: result},
        )
        t.check(
            "player_profile_write_async",
            Buffer(profile()),
            file,
            Buffer(random_.randbytes(RESULT_SIZE)),
            externals={"saved_game_file_write_async": lambda harness, arguments, result=result: result},
        )

    # ---------- the colours of the players

    for count in (0, 1, 5, 18):
        for index in (0, 1, 4, 5, 17, 18, NONE, 0x80000000, 0x7FFFFFFF):
            for scenario in (0x80100000, 0):
                colors = struct.pack(f"<{3 * 18}f", *(random_.uniform(0, 1) for _ in range(3 * 18)))
                data = bytearray(random_.randbytes(0x168))
                struct.pack_into("<2I", data, 0x160, count, len(data))
                game_globals = Buffer(bytes(data) + colors, pointers=(0x164,))
                t.check(
                    "player_color_get",
                    Buffer(struct.pack("<3f", 7, 7, 7), format="<3f"),
                    index,
                    globals={"global_scenario": scenario, "global_game_globals": game_globals},
                )

    # ---------- four numbers of a profile

    for n in range(120):
        values = [random_.choice((NONE, 0, 1, 5, 5, 9, 0x7FFFFFFF, 0x80000001)) for _ in range(4)]
        if n % 6 == 0:
            values = [NONE] * 4
        real = {value: random_.random() < 0.7 for value in values}
        data = bytearray(profile())
        struct.pack_into("<4I", data, 0xEC, *values)
        t.check(
            "player_profile_get_unknownEC",
            Buffer(bytes(data)),
            Buffer(random_.randbytes(4)),
            Buffer(random_.randbytes(4)),
            externals={"unknown_0019c1f0": lambda harness, arguments, real=real: 0x1234 if real[arguments[0]] else 0},
        )

    # ---------- where a file is read from

    def set_pointer(result: int):
        def handler(harness, arguments):
            file, distance, high, method = arguments
            return result, (file, distance, bytes(harness.mu.mem_read(high, 4)), method)

        return handler

    for position in (0, 1, 0x800, 0x7FFFFFFF, 0x80000000, NONE):
        for result in (position, NONE, 0):
            t.check("async_set_file_position", 0x4321, position, externals={"SetFilePointer": set_pointer(result)})
