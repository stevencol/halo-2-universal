# source/sound/sound_manager.c and source/sound/sound_classes.c
#
# The tags of a map that the functions read are the test's: a sound, the
# playback parameters of the sounds, and the classes of sounds that the
# globals tag tells of

import random
import struct
import zlib

from difftest import Buffer, GlobalRange, Memory, Mock, Pointer, mock_address
from difftest import external_address as harness_external_address
from test_async import calls_so_far
from test_data import array as data_array
from test_data import identifier

NONE = 0xFFFFFFFF
PLAYBACK_SIZE = 0x38
CLASS_SIZE = 0x5C
SOURCE_SIZE = 0x98
LISTENER_SIZE = 0x48
LISTENERS = 0x88
# more than 256: all of the low half of an index is where a tag is
TAGS = 0x140
PLAYBACKS = 6
CLASSES = 5
# angles that are not the same if they are divided by 65535 as if they are
# multiplied by its inverse
ROUNDED_ANGLES = [113, 226, 452, 685, 904, 1370, 1808, 2083, 2521, 2740, 3453, 3616, 4166, 4329, 4879, 5042]


def fade_bits(source_index: int) -> int:
    """What the stand-in of sound_source_fade_decibels gives: the bits of some decibels."""
    return struct.unpack("<I", struct.pack("<f", -((source_index ^ source_index >> 16) % 257) / 4.0))[0]


def reals(random_: random.Random, count: int) -> bytes:
    return struct.pack(f"<{count}f", *(random_.choice((0.0, 1.0, random_.uniform(-50, 50), random_.uniform(0, 3))) for _ in range(count)))


def playbacks(random_: random.Random) -> bytes:
    data = bytearray()
    # one more, before the first: an index can be negative
    for _ in range(PLAYBACKS + 1):
        playback = bytearray(reals(random_, PLAYBACK_SIZE // 4))
        struct.pack_into("<2h", playback, 0x18, random_.randint(-1200, 1200), random_.randint(-1200, 1200))
        data += playback
    return bytes(data)


def sound_data(random_: random.Random, flags: int | None = None) -> bytes:
    data = bytearray(random_.randbytes(0x20))
    struct.pack_into(
        "<Hbxxxh",
        data,
        0,
        random_.getrandbits(16) if flags is None else flags,
        random_.randrange(CLASSES),
        random_.randrange(-1, PLAYBACKS),
    )
    return bytes(data)


def sound(random_: random.Random, flags: int | None = None) -> Buffer:
    return Buffer(sound_data(random_, flags))


def world(random_: random.Random, sound_index: int, sound_: bytes) -> dict:
    """The globals that the functions read, with the sound at an index of the
    tags. All is in one buffer, whose pointers are to itself."""
    classes_index = random_.choice([index for index in range(TAGS) if index != sound_index])
    data = bytearray(random_.randbytes(TAGS * 0x10))
    pointers = []

    def add(part: bytes) -> int:
        offset = len(data)
        data.extend(part)
        return offset

    def point(offset: int, to: int) -> None:
        struct.pack_into("<I", data, offset, to)
        pointers.append(offset)

    point(sound_index * 0x10 + 8, add(sound_))
    classes = add(struct.pack("<2I", CLASSES, 0))
    point(classes_index * 0x10 + 8, classes)
    point(classes + 4, add(reals(random_, CLASSES * CLASS_SIZE // 4)))
    # the reference to the tag of the classes: only the low half of the index
    # is where the tag is
    reference = add(struct.pack("<2I", random_.getrandbits(32), random_.getrandbits(16) << 16 | classes_index))
    game_globals = add(random_.randbytes(0xC0) + struct.pack("<2I", random_.choice((1, 1, 3)), 0))
    point(game_globals + 0xC4, reference)
    gestalt = add(struct.pack("<2I", PLAYBACKS, 0) + random_.randbytes(8))
    point(gestalt + 4, add(playbacks(random_)) + PLAYBACK_SIZE)
    buffer = Buffer(bytes(data), pointers=tuple(pointers))
    return {
        "global_tag_instances": buffer,
        "global_game_globals": Pointer(buffer, game_globals),
        "global_sound_cache_file_gestalt": Pointer(buffer, gestalt),
    }


def source(random_: random.Random, flags: int, angle: int | None = None) -> Buffer:
    data = bytearray(random_.randbytes(SOURCE_SIZE))
    struct.pack_into("<H", data, 0, flags)
    data[0x34:0x3C] = reals(random_, 2)
    for offset in (0x3C, 0x3E):
        value = random_.choice((0, 1, 0x7FFF, 0x8000, 0xFFFE, 0xFFFF, random_.getrandbits(16))) if angle is None else angle
        struct.pack_into("<H", data, offset, value)
    data[0x40:0x44] = reals(random_, 1)
    return Buffer(bytes(data))


def run(t) -> None:
    random_ = random.Random(45)

    # ---------- a class of sounds

    for bit in range(16):
        for flags, internal_flags in ((1 << bit, 0), (0, 1 << bit), (0xFFFF ^ 1 << bit, 0), (0, 0xFFFF ^ 1 << bit)):
            data = bytearray(random_.randbytes(CLASS_SIZE))
            struct.pack_into("<2H", data, 8, flags, internal_flags)
            t.check("sound_class_unknown_00124f90", Buffer(bytes(data)))
    for flags, internal_flags in ((0, 0), (0xFFF5, 0xF7FF), (0xFFFF, 0xFFFF)):
        data = bytearray(random_.randbytes(CLASS_SIZE))
        struct.pack_into("<2H", data, 8, flags, internal_flags)
        t.check("sound_class_unknown_00124f90", Buffer(bytes(data)))

    for class_index in (0, 1, 4, 0x7FFF, 0xFFFF, 0x8000):
        t.check("sound_class_get", class_index | random_.getrandbits(16) << 16, globals=world(random_, 0, sound_data(random_)))

    # ---------- what a source says, or the sound

    names = (
        "sound_get_minimum_distance",
        "sound_get_maximum_distance",
        "sound_get_inner_cone_angle",
        "sound_get_outer_cone_angle",
        "sound_get_outer_cone_gain",
    )
    for name in names:
        # each flag of the source and of the sound, alone and missing
        cases = [(1 << bit, 0) for bit in range(16)] + [(0, 1 << bit) for bit in range(16)]
        cases += [(0xFFFF ^ 1 << bit, 0xFFFF) for bit in range(8)] + [(0, 0xFFFF ^ 1 << bit) for bit in range(16)]
        cases += [(random_.getrandbits(16), random_.getrandbits(16)) for _ in range(40)]
        for source_flags, sound_flags in cases:
            sound_index = random_.randrange(TAGS)
            t.check(
                name,
                source(random_, source_flags),
                # only the low half of the index is where the tag is
                random_.getrandbits(16) << 16 | sound_index,
                globals=world(random_, sound_index, sound_data(random_, sound_flags)),
            )
    # the angles that a source can say
    for name, flags in (("sound_get_inner_cone_angle", 0x10), ("sound_get_outer_cone_angle", 0x20)):
        for angle in list(range(0, 0x10000, 0x111)) + [1, 2, 0x7FFF, 0x8000, 0x8001, 0xFFFD, 0xFFFE, 0xFFFF] + ROUNDED_ANGLES:
            t.check(name, source(random_, flags, angle), 3, globals=world(random_, 3, sound_data(random_)))

    cases = [(1 << bit, 0) for bit in range(16)] + [(0, 1 << bit) for bit in range(16)]
    cases += [(0, 0xFFFF), (0xFFCF, 0xFFFF), (0xFFCF, 0xEFFF), (0xFFEF, 0xFFFF), (0xFFDF, 0xFFFF)]
    cases += [(random_.getrandbits(16), random_.getrandbits(16)) for _ in range(40)]
    for source_flags, sound_flags in cases:
        sound_index = random_.randrange(TAGS)
        t.check(
            "sound_unknown_00125060",
            source(random_, source_flags),
            random_.getrandbits(16) << 16 | sound_index,
            globals=world(random_, sound_index, sound_data(random_, sound_flags)),
        )

    # ---------- at random

    seeds = (0, 1, 0xFFFFFFFF, 0x80000000, 0x12345678)
    for n in range(80):
        seed = seeds[n] if n < len(seeds) else random_.getrandbits(32)
        globals_ = world(random_, 0, sound_data(random_))
        t.check(
            "sound_definition_random_gain",
            sound(random_),
            globals={**globals_, "global_random_seed_reference": Buffer(struct.pack("<2I", random_.getrandbits(32), seed))},
        )
        t.check("sound_definition_random_pitch", sound(random_), Buffer(struct.pack("<I", seed)), globals=globals_)

    # ---------- who hears

    for n in range(60):
        players = [NONE if random_.random() < 0.6 else random_.getrandbits(32) for _ in range(4)]
        if n == 0:
            players = [NONE] * 4
        data = bytearray(random_.randbytes(LISTENERS + 4 * LISTENER_SIZE))
        for index in range(4):
            data[LISTENERS + index * LISTENER_SIZE + 6] = random_.choice((0, 1, 1, 0x80))
        t.check(
            "sound_first_local_listener",
            globals={
                "players_globals": Buffer(random_.randbytes(0xC) + struct.pack("<4I", *players)),
                "game_sound_globals": Buffer(bytes(data)),
            },
        )

    # ---------- setting up and taking down

    for count in (0, 1, 2, 5, 0x80, 0xFFFFFFFF):
        for value in (0, 0xFF, 1, 0xA5):
            size = ((count + 31) >> 5) * 4 & 0xFF
            # the count of 0xFFFFFFFF is -1: no bits
            data = random_.randbytes(max(size, 4) + 8)
            t.check("bit_vector_fill", Buffer(data), count if count != 0x80 or value else 0x80, value | random_.getrandbits(24) << 8)

    STATE = 0x80100000
    STATE_SIZE = 0x14000
    ALLOCATION = 0x479864

    def shell_memory(allocated: int) -> bytes:
        return struct.pack("<4I", random_.getrandbits(32), STATE, allocated, random_.getrandbits(32))

    for maximum_count, size in ((1, 4), (32, 0x10), (33, 0x1C), (0x180, 0xBC), (0x80, 0xD4), (0, 8), (64, 1)):
        for allocated in (0, 0x1290, 0x1291, 0x1292, 0x1293, 0x345):
            t.check(
                "shell_memory_data_new",
                Buffer(random_.choice((b"sound sources", b"x", b"a name that is longer than the array keeps")) + b"\0"),
                maximum_count,
                size,
                globals={"shell_memory_globals": shell_memory(allocated), "lruv_blocks_allocation": ALLOCATION},
                memory={STATE: Memory(random_.randbytes(STATE_SIZE))},
            )

    def answers(*values: int) -> dict:
        names = ("sound_playback_controllers_initialize", "looping_sounds_initialize", "sound_effects_initialize", "sound_voices_initialize", "unknown_0021e4b0")

        def configuration(harness, arguments):
            return bytes(harness.mu.mem_read(arguments[0], 14))

        def last(harness, arguments, value=values[4]):
            # leaves something where the vectors of bits are, and around them
            harness.mu.mem_write(sound_globals(harness) + 0x58, noise)
            return value, (configuration(harness, arguments),)

        result = {}
        for name, value in zip(names, values):
            if name == "unknown_0021e4b0":
                result[name] = last
            elif name == "sound_voices_initialize":
                result[name] = lambda harness, arguments, value=value: (value, (configuration(harness, arguments),))
            else:
                result[name] = lambda harness, arguments, value=value: value
        return result

    noise = bytes(b | 1 for b in random_.randbytes(0x30))

    def sound_globals(harness) -> int:
        return struct.unpack("<I", harness.mu.mem_read(harness.global_addresses["game_sound_globals"], 4))[0]

    def console_sound(harness, arguments):
        # what the globals are when the sound of the console is set up, and
        # it leaves something in them
        address = sound_globals(harness)
        record = address, bytes(harness.mu.mem_read(address, 0x20C))
        harness.mu.mem_write(address + 0x7C, noise[:4])
        return 0, record

    cases = [(1, 1, 1, 1, 1)] * 6 + [tuple(int(n != index) for n in range(5)) for index in range(5)]
    cases += [(0x80, 0x100 | 1, 2, 0xFF00 | 1, 0xFF)]
    for n, values in enumerate(cases):
        for count in (0, 0, 1, 3, 0xFFFFFFFF):
            t.check(
                "sound_initialize",
                globals={
                    "sound_initialize_count": count,
                    "shell_memory_globals": shell_memory(random_.choice((0x1290, 0x1291, 0x2003, 0x10, 0))),
                    "lruv_blocks_allocation": ALLOCATION,
                    "game_sound_globals": random_.getrandbits(32),
                    "sound_channels": random_.getrandbits(32),
                    "sound_sources": random_.getrandbits(32),
                },
                memory={STATE: Memory(random_.randbytes(STATE_SIZE))},
                externals={**answers(*values), "sound_cache_initialize": console_sound},
            )

    def array(valid: int) -> Buffer:
        data = bytearray(random_.randbytes(0x4C))
        data[0x29] = valid
        return Buffer(bytes(data))

    for n in range(60):
        pointer = lambda: random_.choice((0, random_.getrandbits(32) | 1))
        t.check(
            "sound_dispose",
            globals={
                "sound_initialize_count": random_.choice((1, 1, 1, 2, 0, 5)),
                "sound_effects": random_.choice((0, array(1))),
                "sound_voices": random_.choice((0, array(1), array(1))),
                "looping_sounds": random_.choice((0, array(1))),
                "unknown_0051ebd8": pointer(),
                "unknown_0051ebdc": pointer(),
                "game_sound_globals": pointer(),
                "sound_sources": random_.choice((0, array(1), array(1))),
                "sound_channels": pointer(),
            },
            externals={"sound_cache_dispose": lambda harness, arguments: 0, "sound_console_dispose": lambda harness, arguments: 0},
        )

    # ---------- a map is loaded

    GLOBALS_SIZE = 0x20C

    def sound_state(initialized=1, unknown79=1, map_loaded=1, disabled=0, channels=0) -> Buffer:
        data = bytearray(random_.randbytes(GLOBALS_SIZE))
        data[0x78:0x7C] = bytes((initialized, unknown79, map_loaded, disabled))
        struct.pack_into("<h", data, 0x208, channels)
        return Buffer(bytes(data))

    def console() -> Buffer:
        return Buffer(random_.randbytes(0x2AA0))

    for n in range(40):
        # the tags, what the globals tag says of the sound, and the globals tag
        gestalt_index = random_.randrange(TAGS)
        data = bytearray(random_.randbytes(TAGS * 0x10))
        element = len(data)
        data += random_.randbytes(0x20) + struct.pack("<I", random_.getrandbits(16) << 16 | gestalt_index)
        game_globals = len(data)
        data += random_.randbytes(0xC0) + struct.pack("<2I", random_.choice((1, 2)), element)
        tags = Buffer(bytes(data), pointers=(game_globals + 0xC4,))
        game = bytearray(random_.randbytes(0x1200))
        struct.pack_into("<I", game, 8, random_.choice((1, 2, 3, 3, 4, 0)))
        t.check(
            "sound_initialize_for_new_map",
            globals={
                "global_tag_instances": tags,
                "global_game_globals": Pointer(tags, game_globals),
                "global_sound_cache_file_gestalt": random_.getrandbits(32),
                "game_sound_globals": sound_state(map_loaded=n % 2),
                "unknown_0051ebe4": console(),
                "main_game_globals": Buffer(bytes(game)),
                "unknown_0047f0e0": random_.getrandbits(32),
            },
            externals={
                "GetTickCount": lambda harness, arguments, value=random_.getrandbits(32): value,
                "sound_load_environment": lambda harness, arguments: 0,
                "unknown_00191270": lambda harness, arguments: 0,
            },
        )

    # ---------- the sounds that play

    DATUM = 0xBC

    def sources(count: int = 12) -> Buffer:
        """The array of the sounds that play: what the functions read of it,
        and its elements. Which are in use is told by the stand-in."""
        data = bytearray(random_.randbytes(0x4C + count * DATUM))
        struct.pack_into("<I", data, 0x24, DATUM)
        struct.pack_into("<I", data, 0x44, 0x4C)
        for index in range(count):
            data[0x4C + index * DATUM + 0x1B] = random_.choice((0, 1, 1, 2, 0x11, 0xF1, 0x21, 0xF, 9, 0x19))
            # no reals that are not numbers where the place of the sound is
            data[0x4C + index * DATUM + 0x24 : 0x4C + index * DATUM + 0x30] = reals(random_, 3)
        return Buffer(bytes(data), pointers=(0x44,))

    def next_absolute_index(in_use: list):
        def handler(harness, arguments):
            data, absolute_index = arguments
            absolute_index -= (1 << 32) if absolute_index & 0x80000000 else 0
            following = [index for index in in_use if index >= absolute_index]
            return (following[0] if following and absolute_index >= 0 else NONE), (data, absolute_index)

        return handler

    def used() -> list:
        return sorted(random_.sample(range(12), random_.choice((0, 1, 3, 12))))

    # the time that the game waits: what the clock says when the sounds
    # start to fade, and each time it is looked at after
    clocks = (
        (1000, 1100, 1299, 1300),
        (1000, 1301),
        (1000, 1000, 1000, 1300),
        (0, 299, 300),
        (0, 300),
        # a real cannot tell 16777217 from 16777216
        (16777217, 16777515, 16777516),
        (16777217, 16777515, 16777517),
        (16777217, 16777516),
        (0x7FFFFFF0, 0x7FFFFFFF, 0x80000000, 0x80000100, 0x80000200),
        # the start is taken with a sign
        (0x80000010, 0x80000020),
        (0xFFFFFF00, 0xFFFFFF10, 20, 40, 50),
        (0xFFFFFF00, 0xFFFFFFFF),
    )

    def clock(times: tuple):
        return lambda harness, arguments: times[min(calls_so_far(harness, "GetTickCount"), len(times) - 1)]

    states = [(1, 1, 1, 0)] * 14 + [(0, 1, 1, 0), (1, 0, 1, 0), (1, 1, 0, 0), (1, 1, 1, 1), (0x80, 2, 4, 0), (1, 1, 1, 0x80)]
    for n in range(60):
        in_use = used()
        array = lambda: random_.choice((0, Buffer(random_.randbytes(0x4C))))
        t.check(
            "sound_dispose_from_old_map",
            globals={
                "game_sound_globals": sound_state(*states[n % len(states)]),
                "sound_sources": sources(),
                "looping_sounds": array(),
                "unknown_0051ebd8": array(),
                "global_sound_cache_file_gestalt": random_.getrandbits(32),
                "unknown_0051ebe4": console(),
            },
            externals={
                "GetTickCount": clock(clocks[n % len(clocks)]),
                "data_next_absolute_index": next_absolute_index(in_use),
                "sound_source_fade_decibels": lambda harness, arguments: (fade_bits(arguments[0]), tuple(arguments)),
                "sound_idle": lambda harness, arguments: 0,
                "sound_reset": lambda harness, arguments: 0,
                "data_delete_all": lambda harness, arguments: 0,
                "unknown_00191270": lambda harness, arguments: 0,
            },
        )

    # ---------- the structure bsp changes

    LEAVES = 9
    for n in range(60):
        in_use = used()
        state_data = bytearray(sound_state(*states[n % len(states)]).data)
        for index in range(4):
            listener = LISTENERS + index * LISTENER_SIZE
            state_data[listener + 6] = random_.choice((0, 1, 1, 0x80))
            state_data[listener + 0x30 : listener + 0x3C] = reals(random_, 3)
        structure_bsp = bytearray(random_.randbytes(0x140))
        struct.pack_into("<2I", structure_bsp, 0x2C, LEAVES, len(structure_bsp))
        structure_bsp += random_.randbytes(LEAVES * 8)
        leaves = [random_.choice((NONE, random_.randrange(LEAVES), random_.randrange(LEAVES))) for _ in range(4)]

        def location(harness, arguments):
            where, point = arguments
            index = calls_so_far(harness, "scenario_location_from_point")
            harness.mu.mem_write(where, struct.pack("<2I", 0x1000 + index, 0x2000 + index))
            return 0, (bytes(harness.mu.mem_read(point, 12)),)

        def test_point(harness, arguments, leaves=leaves):
            bsp, node, point = arguments
            return leaves[calls_so_far(harness, "bsp3d_test_point") % 4], (bsp, node, bytes(harness.mu.mem_read(point, 12)))

        t.check(
            "sound_initialize_for_new_structure_bsp",
            globals={
                "game_sound_globals": Buffer(bytes(state_data)),
                "sound_sources": sources(),
                "global_structure_bsp": Buffer(bytes(structure_bsp), pointers=(0x30,)),
                "global_bsp3d": random_.getrandbits(32),
            },
            externals={
                "data_next_absolute_index": next_absolute_index(in_use),
                "scenario_location_from_point": location,
                "bsp3d_test_point": test_point,
                "sound_voices_initialize_for_new_structure_bsp": lambda harness, arguments: 0,
                "looping_sounds_initialize_for_new_structure_bsp": lambda harness, arguments: 0,
            },
        )

    # ---------- whether there is sound, and stopping it

    for initialized in (0, 1, 0x80):
        for unknown79 in (0, 1, 2):
            for map_loaded in (0, 1, 0x40):
                t.check("sound_available", globals={"game_sound_globals": sound_state(initialized, unknown79, map_loaded, random_.getrandbits(8))})

    channel = lambda harness, arguments: (0, (arguments[0] & 0xFFFF,))
    for enable in (0, 1, 2, NONE, 0x100, 0x80000000):
        for disabled in (0, 1, 0x80):
            for channels in (0, 1, 5, -1):
                t.check(
                    "sound_enable",
                    enable,
                    globals={"game_sound_globals": sound_state(disabled=disabled, channels=channels)},
                    externals={
                        "sound_channel_pause": channel,
                        "sound_channel_resume": channel,
                        "sound_playback_controllers_unknown_00219d90": lambda harness, arguments: (0, (arguments[0] & 0xFF,)),
                        "sound_console_streams_resume": lambda harness, arguments: 0,
                        "sound_console_streams_pause": lambda harness, arguments: 0,
                        "GetTickCount": lambda harness, arguments, value=random_.getrandbits(32): value,
                    },
                )

    # ---------- giving the sound some time

    idle_names = ("sound_console_update", "sound_update_time", "sound_channels_update", "sound_channels_update_sources", "sound_console_commit", "sound_cache_idle")

    def flags_then(harness, arguments):
        # the two flags that are set while the sound is given time
        address = struct.unpack("<I", harness.mu.mem_read(harness.global_addresses["game_sound_globals"], 4))[0]
        return 0, (bytes(harness.mu.mem_read(address + 0x7C, 2)),)

    idle_cases = []
    for seconds_per_tick, deltas in ((1 / 30, (0, 33, 34, 1000, -1, -1000, 0x7FFFFFFF)), (1 / 60, (16, 17)), (0.0, (0, 1)), (1.0, (999, 1000, 1001))):
        for delta in deltas:
            idle_cases.append((seconds_per_tick, delta, 1, 1, 0, (1, 1, 1)))
    idle_cases += [
        (1 / 30, 500, 0, 1, 0, (1, 1, 1)),
        (1 / 30, 500, 1, 0, 0, (1, 1, 1)),
        (1 / 30, 500, 1, 0x80, 0, (1, 1, 1)),
        (1 / 30, 500, 1, 1, 1, (1, 1, 1)),
        (1 / 30, 500, 1, 1, 0x80, (1, 1, 1)),
        (1 / 30, 500, 1, 1, 0, (0, 1, 1)),
        (1 / 30, 500, 1, 1, 0, (1, 0, 1)),
        (1 / 30, 500, 1, 1, 0, (1, 1, 0)),
    ]
    for seconds_per_tick, delta, has_game, active, busy, available in idle_cases:
        start = random_.choice((0, 1000, 0x7FFFFF00, 0xFFFFFF00, random_.getrandbits(32)))
        state_data = bytearray(sound_state(*available).data)
        state_data[0x7C] = random_.getrandbits(8)
        state_data[0x7D] = busy
        struct.pack_into("<I", state_data, 0x80, start)
        game = bytearray(random_.randbytes(0x1200))
        game[0x1120] = active
        t.check(
            "sound_idle",
            globals={
                "game_sound_globals": Buffer(bytes(state_data)),
                "main_game_globals": Buffer(bytes(game)) if has_game else 0,
                "game_time_globals": Buffer(struct.pack("<BBhfI", 1, 0, 30, seconds_per_tick, random_.getrandbits(32))),
            },
            externals={
                "GetTickCount": lambda harness, arguments, value=(start + delta) & 0xFFFFFFFF: value,
                **{name: flags_then for name in idle_names},
            },
        )

    # ---------- the tags of the sounds of a map

    PITCH_RANGES, PERMUTATIONS, CHUNKS = 8, 20, 40

    def sound_tags(sound_index: int, pitch_range_count: int = 3, permutation_count: int | None = None):
        """The tags, a sound and what the sounds share, in one buffer; where
        the sound says its pitch ranges start, and the shared tag."""
        data = bytearray(random_.randbytes(TAGS * 0x10))
        pointers = []

        def add(part: bytes) -> int:
            offset = len(data)
            data.extend(part)
            return offset

        def point(offset: int, to: int) -> None:
            struct.pack_into("<I", data, offset, to)
            pointers.append(offset)

        first_pitch_range = random_.randrange(2, PITCH_RANGES - 2)
        sound_ = bytearray(random_.randbytes(0x20))
        struct.pack_into("<hB", sound_, 8, first_pitch_range, pitch_range_count)
        point(sound_index * 0x10 + 8, add(bytes(sound_)))
        gestalt = add(random_.randbytes(0x60))
        ranges = bytearray()
        for index in range(PITCH_RANGES):
            count = random_.choice((0, 2)) if permutation_count is None else permutation_count
            ranges += random_.randbytes(8) + struct.pack("<2h", random_.randrange(2, PERMUTATIONS - 2), count)
        point(gestalt + 0x24, add(bytes(ranges)))
        permutations = bytearray()
        for index in range(PERMUTATIONS):
            permutations += random_.randbytes(0xC) + struct.pack("<2h", random_.randrange(2, CHUNKS - 2), random_.randrange(1, 4))
        point(gestalt + 0x2C, add(bytes(permutations)))
        point(gestalt + 0x44, add(random_.randbytes(CHUNKS * 0xC)))
        buffer = Buffer(bytes(data), pointers=tuple(pointers))
        return buffer, gestalt

    def request(result: int):
        return lambda harness, arguments, result=result: (result, tuple(arguments))

    for n in range(120):
        sound_index = random_.randrange(TAGS)
        tags, gestalt = sound_tags(sound_index)
        source_data = bytearray(random_.randbytes(DATUM))
        struct.pack_into("<I", source_data, 0xC, random_.getrandbits(16) << 16 | sound_index)
        struct.pack_into("<bbh", source_data, 0x9C, random_.randint(-2, 1), random_.randint(-2, 1), random_.randint(-2, 1))
        t.check(
            "sound_source_request_data",
            Buffer(bytes(source_data)),
            globals={"global_tag_instances": tags, "global_sound_cache_file_gestalt": Pointer(tags, gestalt)},
            externals={
                "sound_cache_chunk_request": request(n % 4 | random_.choice((0, 0, 0xFFFFFFFC, 4))),
                "sound_source_hold_chunk": lambda harness, arguments: 0,
            },
        )

    for n in range(80):
        sound_index = random_.randrange(TAGS)
        tags, gestalt = sound_tags(sound_index, random_.choice((0, 1, 3, 0x80)), None if n % 2 else 2)
        t.check(
            "sound_predict",
            NONE if n % 10 == 9 else random_.getrandbits(16) << 16 | sound_index,
            globals={"global_tag_instances": tags, "global_sound_cache_file_gestalt": Pointer(tags, gestalt)},
            externals={"sound_cache_chunk_request": request(random_.getrandbits(32))},
        )

    for n in range(30):
        count = 6
        index = random_.randrange(count)
        data = bytearray(random_.randbytes(0x4C + count * 0x10))
        struct.pack_into("<I", data, 0x44, 0x4C)
        data[0x4C + index * 0x10 + 4] = random_.choice((0, 1, 2, 0x80, 0xFF))
        t.check(
            "sound_unknown_00125ef0",
            Buffer(random_.randbytes(8) + struct.pack("<I", random_.getrandbits(16) << 16 | index)),
            globals={"unknown_00502104": Buffer(bytes(data), pointers=(0x44,))},
        )

    # ---------- the sound of the places of a map

    for n in range(40):
        environment_index = random_.randrange(TAGS)
        data = bytearray(random_.randbytes(TAGS * 0x10))
        pointers = []
        struct.pack_into("<I", data, environment_index * 0x10 + 8, len(data))
        pointers.append(environment_index * 0x10 + 8)
        data += random_.randbytes(0x60)
        element = len(data)
        index = NONE if n % 4 == 3 else random_.getrandbits(16) << 16 | environment_index
        data += random_.randbytes(0x14) + struct.pack("<I", index) + random_.randbytes(0x10)
        game_globals = len(data)
        data += random_.randbytes(0xC0) + struct.pack("<2I", random_.choice((1, 2)), element)
        pointers.append(game_globals + 0xC4)
        tags = Buffer(bytes(data), pointers=tuple(pointers))

        def set_environment(harness, arguments):
            return 0, tuple(bytes(harness.mu.mem_read(argument, 8)) for argument in arguments)

        t.check(
            "sound_load_environment",
            globals={
                "global_tag_instances": tags,
                "global_game_globals": Pointer(tags, game_globals),
                "game_sound_globals": sound_state(),
            },
            externals={"sound_console_set_environment": set_environment},
        )

    # ---------- updating each channel

    random_ = random.Random(139)

    def channels_globals(count: int) -> Buffer:
        data = bytearray(random_.randbytes(GLOBALS_SIZE))
        struct.pack_into("<h", data, 0x208, count)
        return Buffer(bytes(data))

    def update_channel(harness, arguments):
        """Stands in for 0x0012a450; the channel 2 takes one away from the
        count, to see that it is looked at again."""
        (channel_index,) = arguments
        if channel_index & 0xFFFF == 2:
            (globals_address,) = struct.unpack("<I", harness.mu.mem_read(harness.global_addresses["game_sound_globals"], 4))
            (count,) = struct.unpack("<h", harness.mu.mem_read(globals_address + 0x208, 2))
            harness.mu.mem_write(globals_address + 0x208, struct.pack("<H", (count - 1) & 0xFFFF))
        return 0, (channel_index & 0xFFFF,)

    for count in (0, 1, 2, 3, 4, 7, 24, -1, -0x8000):
        t.check(
            "sound_channels_update",
            globals={"game_sound_globals": channels_globals(count)},
            externals={"sound_channel_update": update_channel},
        )

    # ---------- pausing a channel

    random_ = random.Random(147)
    CHANNELS = 8
    # past 0xFF the index of a source has more than 8 bits
    SOURCES = 260

    def pause_world():
        """The channels, the sound sources, the tags of their sounds and the
        sound classes that sound_class_get gives."""
        channels = bytearray(random_.randbytes(CHANNELS * 0x24))
        for n in range(CHANNELS):
            source = random_.choice((-1, 0, 1, 2, 3, 4, 5, 0x100, 0x101, 0x103)) & 0xFFFFFFFF
            struct.pack_into("<I", channels, n * 0x24, source | (random_.getrandbits(16) << 16 if source != 0xFFFFFFFF else 0))
            channels[n * 0x24 + 9] = random_.choice((0, 0, 1, 0xFF))
            struct.pack_into("<h", channels, n * 0x24 + 0xC, random_.choice((-1, 0, 1, 2, 3)))
        sources = bytearray(0x50) + bytearray(random_.randbytes(SOURCES * 0xBC))
        struct.pack_into("<I", sources, 0x44, 0x50)
        tags = bytearray(random_.randbytes(SOURCES * 0x10)) + bytearray(SOURCES * 0x10)
        pointers = []
        for n in range(SOURCES):
            struct.pack_into("<I", sources, 0x50 + n * 0xBC + 0xC, random_.getrandbits(16) << 16 | n)
            definition = SOURCES * 0x10 + n * 0x10
            struct.pack_into("<I", tags, n * 0x10 + 8, definition)
            pointers.append(n * 0x10 + 8)
            tags[definition + 2] = random_.choice((0, 1, 5, 0x35, 0x36, 0x7F, 0x80, 0xFF))
        classes = Buffer(bytes(random_.randbytes(0x36 * 0x5C)))
        return (
            Buffer(bytes(channels)),
            Buffer(bytes(sources), pointers=(0x44,)),
            Buffer(bytes(tags), pointers=tuple(pointers)),
            classes,
        )

    for _ in range(40):
        channels, sources, tags, classes = pause_world()

        def class_get(harness, arguments, classes=classes):
            index = arguments[0] & 0xFFFF
            return classes.address + index * 0x5C, (index,)

        def voice(harness, arguments):
            return 0, (harness.normalize(arguments[0]),)

        options = {
            "globals": {
                "sound_channels": channels,
                "sound_sources": sources,
                "global_tag_instances": tags,
                "unknown_0051ebe4": Buffer(random_.randbytes(4 * 0x34 + 0x10)),
            },
            "extra": [classes],
            "externals": {"sound_class_get": class_get, "unknown_002ae750": voice},
        }
        for channel_index in range(CHANNELS):
            t.check("sound_channel_pausable", channel_index, **options)
            t.check("sound_channel_pause", channel_index, **options)

    # ---------- a channel goes on

    random_ = random.Random(148)
    CHUNKS = 40

    def resume_world():
        channels, sources, tags, classes = pause_world()
        data = bytearray(channels.data)
        # the permutations, after the channels in the same buffer
        permutations = len(data)
        data += random_.randbytes(4 * 0x10)
        for n in range(4):
            struct.pack_into("<h", data, permutations + n * 0x10 + 0xC, random_.randrange(0, 20))
        pointers = []
        for n in range(CHANNELS):
            data[n * 0x24 + 9] = random_.choice((0, 1, 1, 0x80))
            struct.pack_into("<f", data, n * 0x24 + 0x10, random_.uniform(0, 5))
            struct.pack_into("<hh", data, n * 0x24 + 0x18, random_.randrange(0, 10), random_.randrange(0, 10))
            for field in (0x1C, 0x20):
                if random_.random() < 0.7:
                    struct.pack_into("<I", data, n * 0x24 + field, permutations + random_.randrange(4) * 0x10)
                    pointers.append(n * 0x24 + field)
                else:
                    struct.pack_into("<I", data, n * 0x24 + field, 0)
        gestalt = bytearray(random_.randbytes(0x48)) + bytearray(random_.randbytes(CHUNKS * 12))
        struct.pack_into("<iI", gestalt, 0x40, CHUNKS, 0x48)
        # the kinds of the sounds, which can be negative
        definitions = bytearray(tags.data)
        for n in range(SOURCES):
            at = SOURCES * 0x10 + n * 0x10
            definitions[at + 3 : at + 6] = bytes(random_.choice((0, 1, 2, 3, 0x7F, 0x80, 0xFF)) for _ in range(3))
        return (
            Buffer(bytes(data), pointers=tuple(pointers)),
            sources,
            Buffer(bytes(definitions), pointers=tags.pointers),
            Buffer(bytes(gestalt), pointers=(0x44,)),
        )

    def resume_externals():
        def played(harness, arguments):
            unknown4, unknown5, unknown3, seconds = arguments
            return (unknown4 * 7 + unknown5 * 3 + unknown3 + seconds) & 0xFFFFFF, (unknown4, unknown5, unknown3, seconds)

        def chunk_bytes(harness, arguments):
            permutation, count = arguments
            return (count * 0x100 + 3) & 0xFFFFFFFF, (harness.normalize(permutation), count & 0xFFFF)

        def restart(harness, arguments):
            voice, offset = arguments
            return 0, (voice & 0xFFFF, offset)

        def chunk(harness, arguments):
            voice, chunk_ = arguments
            return 0, (harness.normalize(voice), harness.normalize(chunk_))

        return {
            "sound_seconds_to_bytes": played,
            "sound_permutation_chunks_size": chunk_bytes,
            "sound_console_voice_stop": restart,
            "unknown_002ae500": chunk,
        }

    for _ in range(40):
        channels, sources, tags, gestalt = resume_world()
        options = {
            "globals": {
                "sound_channels": channels,
                "sound_sources": sources,
                "global_tag_instances": tags,
                "global_sound_cache_file_gestalt": gestalt,
                "unknown_0051ebe4": Buffer(random_.randbytes(4 * 0x34 + 0x10)),
            },
            "externals": resume_externals(),
        }
        for channel_index in range(CHANNELS):
            t.check("sound_channel_resume", channel_index, **options)

    # ---------- a channel that plays, each time

    random_ = random.Random(149)

    def voices() -> Buffer:
        data = bytearray(random_.randbytes(4 * 0x34 + 0x10))
        for n in range(4):
            voice = n * 0x34 + 0xC
            data[voice + 1] = random_.getrandbits(8)
            struct.pack_into("<i", data, voice + 8, random_.choice((0, -0x11F5, 0x1000, -0x2000, random_.randint(-0x8000, 0x8000))))
        return Buffer(bytes(data))

    for voice_index in range(4):
        for kind in (0, 1, 2, -1, 0x80, 0x7F):
            for _ in range(5):
                t.check("sound_voice_playback_rate", voice_index, kind, globals={"unknown_0051ebe4": voices()})
    for chunk in (-1, 0, 1, 2, 5, 6, 7, 0x7FFF):
        for count in (0, 1, 6, 7, -1):
            permutation = Buffer(random_.randbytes(0xC) + struct.pack("<hh", random_.randrange(20), count))
            t.check("sound_permutation_next_chunk", permutation, chunk)
        t.check("sound_permutation_next_chunk", 0, chunk)

    def update_externals():
        def release(harness, arguments):
            return 0, (harness.normalize(arguments[0]),)

        def request(harness, arguments):
            sound_index, chunk, unknown = arguments
            (value,) = struct.unpack("<I", harness.mu.mem_read(chunk, 4))
            return value & 3, (sound_index, harness.normalize(chunk), unknown)

        return {"sound_unknown_00125ef0": release, "sound_cache_chunk_request": request}

    for _ in range(40):
        channels, sources, tags, gestalt = resume_world()
        data = bytearray(random_.randbytes(GLOBALS_SIZE))
        struct.pack_into("<f", data, 0x1E0, random_.choice((1 / 30, 0.5, 0.0)))
        options = {
            "globals": {
                "sound_channels": channels,
                "sound_sources": sources,
                "global_tag_instances": tags,
                "global_sound_cache_file_gestalt": gestalt,
                "unknown_0051ebe4": voices(),
                "game_sound_globals": Buffer(bytes(data)),
            },
            "externals": update_externals(),
        }
        for channel_index in range(CHANNELS):
            t.check("sound_channel_update", channel_index, **options)

    # ---------- how loud a sound is, in decibels

    random_ = random.Random(150)

    def real_bits(value: float) -> int:
        return struct.unpack("<I", struct.pack("<f", value))[0]

    def bits_real(value: int) -> float:
        return struct.unpack("<f", struct.pack("<I", value & 0xFFFFFFFF))[0]

    for decibels in (-100.0, -64.0, -63.99, -40.0, -20.0, -6.0, -0.5, 0.0, -0.0, 0.5, 30.0, float("nan"), float("-inf")):
        t.check("sound_decibels_to_gain", real_bits(decibels))
    for _ in range(100):
        t.check("sound_decibels_to_gain", real_bits(random_.uniform(-70, 5)))

    def decibels_externals():
        def most(harness, arguments):
            (sound,) = arguments
            value = harness.mu.mem_read(sound + 0xB, 1)[0]
            return real_bits(-value / 3.0 + 4.0), (harness.normalize(sound),)

        def least(harness, arguments):
            (sound,) = arguments
            value = harness.mu.mem_read(sound + 0xB, 1)[0]
            return real_bits(-value / 2.0 - 10.0), (harness.normalize(sound),)

        def to_decibels(harness, arguments):
            (gain,) = arguments
            return real_bits(bits_real(gain) * 7.0 - 3.0), (gain & 0xFFFFFFFF,)

        def class_decibels(harness, arguments):
            (sound_class,) = arguments
            sound_class = (sound_class & 0xFFFF) - (0x10000 if sound_class & 0x8000 else 0)
            return real_bits(sound_class * 0.25), (sound_class,)

        return {
            "sound_definition_get_maximum_decibels": most,
            "sound_definition_get_minimum_decibels": least,
            "sound_gain_to_decibels": to_decibels,
            "sound_class_get_decibels": class_decibels,
        }

    externals = decibels_externals()
    for _ in range(200):
        definition = bytearray(random_.randbytes(0x10))
        definition[2] = random_.choice((0, 1, 5, 0x35, 0x80, 0xFF))
        t.check(
            "sound_definition_decibels",
            Buffer(bytes(definition)),
            real_bits(random_.choice((0.0, -3.0, 6.5, random_.uniform(-20, 20)))),
            real_bits(random_.choice((0.0, 1.0, 0.5, random_.uniform(0, 1), -0.5, 1.5))),
            externals=externals,
        )

    # ---------- what the sound does each frame

    random_ = random.Random(151)
    UPDATE_NAMES = (
        "sound_console_update",
        "sound_playback_controllers_unknown_00219d90",
        "sound_console_streams_resume",
        "sound_update_time",
        "unknown_00127400",
        "sound_playback_controllers_update",
        "looping_sounds_update",
        "sound_channels_update",
        "sound_sources_update",
        "sound_sources_get_channels",
        "sound_channels_update_sources",
        "sound_console_commit",
    )

    def frame_externals(first_player: int, unload: bool) -> dict:
        def stack_bytes(harness, address, size=8):
            return bytes(harness.mu.mem_read(address, size))

        def sound_globals(harness) -> int:
            return struct.unpack("<I", harness.mu.mem_read(harness.global_addresses["game_sound_globals"], 4))[0]

        def first_update(harness, arguments):
            # what the frame is marked as; the map can be let go meanwhile
            globals_ = sound_globals(harness)
            marked = harness.mu.mem_read(globals_ + 0x7D, 1)[0]
            if unload:
                harness.mu.mem_write(globals_ + 0x7A, b"\0")
            return 0, (marked,)

        def tables(harness, arguments):
            first, second, listeners = arguments
            # what it is given, and then something of its own in the tables
            record = (stack_bytes(harness, first, 0x100), stack_bytes(harness, second, 0x200), stack_bytes(harness, listeners, 4))
            harness.mu.mem_write(first, bytes(range(0x100)))
            harness.mu.mem_write(second, bytes(x ^ 0xA5 for x in range(0x100)) * 2)
            return 0, record

        externals = {name: (lambda harness, arguments: 0) for name in UPDATE_NAMES}
        externals.update(
            {
                "sound_channel_resume": lambda harness, arguments: (0, (arguments[0] & 0xFFFF,)),
                "GetTickCount": lambda harness, arguments: 0x12345,
                "sound_console_get_state": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "sound_classes_update": lambda harness, arguments: (0, (arguments[0] & 0xFFFFFFFF,)),
                "sound_console_update": first_update,
                "sound_channels_build_queries": tables,
                "unknown_0014dde0": lambda harness, arguments: first_player,
                "sound_listener_compute_queries": lambda harness, arguments: (0, (arguments[0], stack_bytes(harness, arguments[1], 4))),
                "sound_channels_unknown_00129380": lambda harness, arguments: (0, (stack_bytes(harness, arguments[0], 16), stack_bytes(harness, arguments[1], 16))),
                "lruv_wrap_ticks": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            }
        )
        return externals

    for _ in range(120):
        data = bytearray(random_.randbytes(GLOBALS_SIZE))
        data[0x78:0x7C] = bytes(random_.choice((1, 1, 1, 0)) for _ in range(3)) + bytes((random_.choice((0, 1)),))
        data[0x7E] = random_.choice((0, 1))
        struct.pack_into("<h", data, 0x208, random_.choice((0, 1, 3)))
        struct.pack_into("<f", data, 0x1E0, random_.uniform(0, 0.1))
        players = bytearray(random_.randbytes(0xC)) + struct.pack(
            "<4i", *(random_.choice((-1, -1, 0, 1, 7)) for _ in range(4))
        )
        cache = bytearray(random_.randbytes(0x70))
        struct.pack_into("<i", cache, 0x38, random_.choice((0x7FFFFFFF, 0x7FFFFFFE, 5)))
        t.check(
            "sound_update",
            globals={
                "game_sound_globals": Buffer(bytes(data)),
                "game_time_globals": Buffer(bytes((random_.choice((0, 1)), random_.choice((0, 1)))) + random_.randbytes(10)),
                "players_globals": Buffer(bytes(players)),
                "sound_lruv_cache": Buffer(bytes(cache)),
            },
            externals=frame_externals(random_.choice((-1, 0, 1, 2, 3)), random_.random() < 0.2),
        )

    # ---------- slots, a flag of a voice, and a fraction

    random_ = random.Random(152)
    for _ in range(150):
        data = bytearray(random_.randbytes(GLOBALS_SIZE))
        struct.pack_into("<h", data, 0x20A, random_.choice((0, 1, 5, 33, 64, -1)))
        if random_.random() < 0.3:
            data[0x58:0x60] = bytes(8)
        t.check("sound_slot_new", globals={"game_sound_globals": Buffer(bytes(data))})
        for slot in (random_.randrange(64), -1, 0, 31, 32, 63):
            t.check("sound_slot_delete", slot, globals={"game_sound_globals": Buffer(bytes(data))})

    for _ in range(20):
        channels, sources, tags, classes = pause_world()
        for channel_index in range(CHANNELS):
            t.check(
                "sound_channel_set_voice_flag3",
                channel_index,
                globals={"sound_channels": channels, "unknown_0051ebe4": Buffer(random_.randbytes(4 * 0x34 + 0x10))},
            )

    def as_bits(value: float) -> int:
        return struct.unpack("<I", struct.pack("<f", value))[0]

    for minimum, maximum in ((0.0, 1.0), (1.0, 0.0), (2.0, 2.0), (2.0, 2.00005), (-1.0, 3.0), (5.0, float("nan")), (0.0, 0.0001)):
        for value in (minimum, maximum, -2.0, 0.5, 4.0, 2.0, float("nan")):
            for above in (0, 1):
                t.check("sound_interpolation_fraction", as_bits(minimum), as_bits(maximum), as_bits(value), above)
    # and values that a division and a product by the inverse round apart
    for _ in range(300):
        minimum = random_.uniform(-10, 10)
        maximum = minimum + random_.choice((1.0, -1.0)) * random_.uniform(0.001, 20)
        value = random_.uniform(min(minimum, maximum), max(minimum, maximum))
        t.check("sound_interpolation_fraction", as_bits(minimum), as_bits(maximum), as_bits(value), random_.randrange(2))

    # ---------- something of a sound, and the index at 0x1F8

    random_ = random.Random(153)
    for _ in range(200):
        data = bytearray(random_.randbytes(GLOBALS_SIZE))
        current = random_.choice((-1, 0, 1, 2))
        struct.pack_into("<iii", data, 0x1F8, current, random_.getrandbits(32) - (1 << 31), random_.choice((-1, 0, 1, 2)))
        t.check(
            "sound_set_unknown1f8",
            random_.choice((-1, 0, 1, 2, current)),
            globals={"game_sound_globals": Buffer(bytes(data))},
        )

    for _ in range(300):
        # the tags of the sounds, and the pitch ranges after the gestalt
        definitions = bytearray(random_.randbytes(TAGS * 0x10))
        pointers = []
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        for n in range(TAGS):
            struct.pack_into("<I", tags, n * 0x10 + 8, TAGS * 0x10 + n * 0x10)
            pointers.append(n * 0x10 + 8)
            at = n * 0x10
            definitions[at + 4] = random_.choice((0, 1, 2, 2, 3))
            definitions[at + 5] = random_.choice((0, 1, 1, 2))
            struct.pack_into("<h", definitions, at + 8, random_.randrange(8))
            definitions[at + 0xA] = random_.choice((0, 1, 3))
        gestalt = bytearray(random_.randbytes(0x48)) + bytearray(random_.randbytes(8 * 12))
        struct.pack_into("<iI", gestalt, 0x20, 8, 0x48)
        for n in range(8):
            struct.pack_into("<h", gestalt, 0x48 + n * 12 + 0xA, random_.choice((0, 0, 1, 5)))
        t.check(
            "sound_definition_unknown_00126bd0",
            random_.getrandbits(16) << 16 | random_.randrange(TAGS),
            globals={
                "global_tag_instances": Buffer(bytes(tags + definitions), pointers=tuple(pointers)),
                "global_sound_cache_file_gestalt": Buffer(bytes(gestalt), pointers=(0x24,)),
                "unknown_0047f0e4": struct.pack("<i", random_.choice((0, 1))),
            },
        )

    # ---------- a sound stops, all are let go, and one is let go

    random_ = random.Random(154)
    PLAYING = 12
    CONTROLLERS = 6

    def playing(callbacks: tuple = (1, 1)) -> tuple:
        """The array of the sounds that play, with what tells whether an
        index is of one of them, and who is told of each: none, or a table
        after the array whose two functions are the test's. Gives too the
        index of each sound."""
        data = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        pointers = [0x44]
        struct.pack_into("<I", data, 0x24, DATUM)
        struct.pack_into("<i", data, 0x38, random_.choice((PLAYING, PLAYING, PLAYING - 3, 1, 0)))
        struct.pack_into("<I", data, 0x44, 0x4C)
        table = len(data)
        data += random_.randbytes(0x10) + struct.pack(
            "<2I", mock_address(0) if callbacks[0] else 0, mock_address(1) if callbacks[1] else 0
        )
        indices = []
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            identifier = random_.choice((0, 1, 0x7FFF, 0x8001, 0xFFFF, random_.getrandbits(16)))
            struct.pack_into("<H", data, at, identifier)
            data[at + 2] = random_.choice((0, 0, 0, 1, 0x80))
            data[at + 3] = random_.choice((0xFF, 0xFF, 0, 1, 0x7F))
            if random_.random() < 0.7:
                struct.pack_into("<I", data, at + 0x14, table)
                pointers.append(at + 0x14)
            else:
                struct.pack_into("<I", data, at + 0x14, 0)
            struct.pack_into(
                "<I", data, at + 0xA8, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(CONTROLLERS)))
            )
            struct.pack_into("<H", data, at + 0xAC, random_.choice((0xFFFF, 0, 3, 0x8000, 0xFFFE)))
            indices.append(identifier << 16 | index)
        return Buffer(bytes(data), pointers=tuple(pointers)), indices

    def told() -> list:
        return [Mock(lambda call: 0, stack_arguments=3), Mock(lambda call: 0, stack_arguments=2)]

    for n in range(60):
        buffer, indices = playing((random_.random() < 0.8, random_.random() < 0.8))
        others = [NONE, 0, PLAYING, 0x10000 | PLAYING, 0xFFFF0000 | random_.randrange(PLAYING)]
        # the index of another sound, with the identifier of this one
        others += [index ^ 0x10000 for index in random_.sample(indices, 3)]
        for source_index in indices + others:
            t.check(
                "sound_source_stop",
                source_index,
                globals={"sound_sources": Buffer(buffer.data, pointers=buffer.pointers)},
                externals={"sound_source_fade_decibels": lambda harness, arguments: (fade_bits(arguments[0]), tuple(arguments))},
                mocks=told(),
            )

    for n in range(80):
        buffer, indices = playing()
        in_use = used()
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        state[0x78] = random_.choice((1, 1, 1, 1, 0, 0x80))
        t.check(
            "sound_stop_all",
            globals={"game_sound_globals": Buffer(bytes(state)), "sound_sources": buffer},
            externals={
                "sound_effects_delete_all": lambda harness, arguments: 0,
                "data_next_absolute_index": next_absolute_index(in_use),
                "sound_source_delete": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    for n in range(60):
        buffer, indices = playing((random_.random() < 0.8, random_.random() < 0.8))
        for source_index in indices:
            t.check(
                "sound_source_delete",
                source_index,
                random_.choice((10, 0, random_.getrandbits(32))),
                globals={"sound_sources": Buffer(buffer.data, pointers=buffer.pointers)},
                externals={
                    "sound_channel_delete": lambda harness, arguments: (0, (arguments[0] & 0xFFFF,)),
                    "sound_source_release_chunk": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                    "sound_source_free": lambda harness, arguments: (0, tuple(arguments)),
                },
                mocks=told(),
            )

    for n in range(60):
        buffer, indices = playing()
        controllers = bytearray(random_.randbytes(0x4C + CONTROLLERS * 0x1C))
        struct.pack_into("<I", controllers, 0x44, 0x4C)
        for index in range(CONTROLLERS):
            at = 0x4C + index * 0x1C
            controllers[at + 3] = random_.choice((0, 1, 0x80, 0x81, 0xFF, random_.getrandbits(8)))
            struct.pack_into("<I", controllers, at + 4, random_.choice((NONE, NONE, 0, random_.getrandbits(32))))
        for source_index in indices:
            t.check(
                "sound_source_free",
                source_index,
                globals={
                    "sound_sources": Buffer(buffer.data, pointers=buffer.pointers),
                    "unknown_0051ebd8": Buffer(bytes(controllers), pointers=(0x44,)),
                },
                externals={
                    "sound_playback_controller_release": lambda harness, arguments: (0, tuple(arguments)),
                    "datum_delete": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                },
            )

    # ---------- a sound and a channel let go of what they have

    random_ = random.Random(155)
    COUNTED = 8
    PIECES = 30

    def counted() -> Buffer:
        """The array that counts who is at each piece of the samples."""
        data = bytearray(random_.randbytes(0x4C + COUNTED * 0x10))
        struct.pack_into("<I", data, 0x44, 0x4C)
        for index in range(COUNTED):
            data[0x4C + index * 0x10 + 4] = random_.choice((0, 1, 2, 0x80, 0xFF))
        return Buffer(bytes(data), pointers=(0x44,))

    def recordings() -> tuple:
        """What the sounds of a map share, with 4 pitch ranges, 12 recordings
        and their pieces after it in the buffer. Gives where the recordings
        are in it."""
        data = bytearray(random_.randbytes(0x48))
        pitch_ranges = len(data)
        data += random_.randbytes(4 * 0xC)
        permutations = len(data)
        data += random_.randbytes(12 * 0x10)
        chunks = len(data)
        data += random_.randbytes(PIECES * 0xC)
        for n in range(4):
            struct.pack_into("<h", data, pitch_ranges + n * 0xC + 8, random_.randrange(0, 6))
        for n in range(12):
            struct.pack_into("<h", data, permutations + n * 0x10 + 0xC, random_.randrange(0, 12))
        for n in range(PIECES):
            struct.pack_into("<I", data, chunks + n * 0xC + 8, random_.getrandbits(16) << 16 | random_.randrange(COUNTED))
        struct.pack_into("<I", data, 0x24, pitch_ranges)
        struct.pack_into("<I", data, 0x2C, permutations)
        struct.pack_into("<I", data, 0x44, chunks)
        return bytes(data), permutations

    def playing_sounds() -> tuple:
        """The sounds that play, with the tags of their sounds."""
        sources = bytearray(0x50) + bytearray(random_.randbytes(SOURCES * 0xBC))
        struct.pack_into("<I", sources, 0x44, 0x50)
        tags = bytearray(random_.randbytes(SOURCES * 0x10)) + bytearray(random_.randbytes(SOURCES * 0x10))
        pointers = []
        for n in range(SOURCES):
            at = 0x50 + n * 0xBC
            sources[at + 4] = random_.choice((0, 2, 2, 3, 0xFF, 0xFD, random_.getrandbits(8)))
            struct.pack_into("<I", sources, at + 0xC, random_.getrandbits(16) << 16 | n)
            sources[at + 0x9C] = random_.choice((0, 1, 2))
            sources[at + 0x9D] = random_.choice((0, 1, 3, 5))
            struct.pack_into("<h", sources, at + 0x9E, random_.randrange(0, 6))
            definition = SOURCES * 0x10 + n * 0x10
            struct.pack_into("<I", tags, n * 0x10 + 8, definition)
            pointers.append(n * 0x10 + 8)
            struct.pack_into("<h", tags, definition + 8, random_.choice((0, 1)))
        return Buffer(bytes(sources), pointers=(0x44,)), Buffer(bytes(tags), pointers=tuple(pointers))

    for _ in range(60):
        sources, tags = playing_sounds()
        gestalt, _permutations = recordings()
        world_ = {
            "global_tag_instances": tags,
            "global_sound_cache_file_gestalt": Buffer(gestalt, pointers=(0x24, 0x2C, 0x44)),
            "unknown_00502104": counted(),
        }
        for n in range(SOURCES):
            t.check(
                "sound_source_release_chunk",
                Buffer(sources.data[0x50 + n * 0xBC : 0x50 + (n + 1) * 0xBC]),
                globals=world_,
            )
            t.check(
                "sound_source_set_sound",
                random_.getrandbits(16) << 16 | n,
                random_.getrandbits(32),
                random_.getrandbits(8),
                random_.getrandbits(8),
                random_.getrandbits(16),
                globals={**world_, "sound_sources": Buffer(sources.data, pointers=sources.pointers)},
            )

    for _ in range(120):
        gestalt, permutations = recordings()
        # the channels, after what the sounds share, in the same buffer
        data = bytearray(gestalt)
        channels = len(data)
        data += random_.randbytes(CHANNELS * 0x24)
        pointers = [0x24, 0x2C, 0x44]
        for n in range(CHANNELS):
            at = channels + n * 0x24
            struct.pack_into("<I", data, at + 4, random_.choice((NONE, NONE, 0, random_.getrandbits(32))))
            struct.pack_into("<h", data, at + 0xC, random_.choice((-1, 0, 1, 2, 3, 31, 32, 63, 127)))
            struct.pack_into("<hh", data, at + 0x18, random_.randrange(0, 10), random_.randrange(0, 10))
            for field in (0x1C, 0x20):
                if random_.random() < 0.6:
                    struct.pack_into("<I", data, at + field, permutations + random_.randrange(12) * 0x10)
                    pointers.append(at + field)
                else:
                    struct.pack_into("<I", data, at + field, 0)
        buffer = Buffer(bytes(data), pointers=tuple(pointers))
        for channel_index in range(CHANNELS):
            t.check(
                "sound_channel_delete",
                channel_index,
                globals={
                    "global_sound_cache_file_gestalt": buffer,
                    "sound_channels": Pointer(buffer, channels),
                    "unknown_00502104": counted(),
                    "game_sound_globals": Buffer(random_.randbytes(GLOBALS_SIZE)),
                    "unknown_0051ebe4": Buffer(random_.randbytes(128 * 0x34 + 0x10)),
                },
                externals={
                    "unknown_002ae4b0": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                    "sound_voice_release": lambda harness, arguments: (0, tuple(arguments)),
                },
            )

    for _ in range(150):
        controllers = bytearray(random_.randbytes(0x4C + CONTROLLERS * 0x1C))
        struct.pack_into("<I", controllers, 0x44, 0x4C)
        for index in range(CONTROLLERS):
            controllers[0x4C + index * 0x1C + 2] = random_.choice((0, 1, 2, 0x80, 0x81, 0x82, 0xFF, 0x7F))
        for controller_index in [NONE] + [random_.getrandbits(16) << 16 | index for index in range(CONTROLLERS)]:
            t.check(
                "sound_playback_controller_release",
                controller_index,
                globals={"unknown_0051ebd8": Buffer(bytes(controllers), pointers=(0x44,))},
                externals={"datum_delete": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1]))},
            )

    for _ in range(80):
        buffer, indices = playing()
        data = bytearray(buffer.data)
        # the first function of the table of who is told: there is one, or none
        struct.pack_into("<I", data, 0x4C + PLAYING * DATUM, random_.choice((0, 1, random_.getrandbits(32))))
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            data[at + 4] = random_.choice((0, 0, 0, 1, 0x20, 0xDE, 0x21, random_.getrandbits(8)))
            struct.pack_into("<i", data, at + 0x8C, random_.choice((-5, 0, 99, 100, 101, 0x7FFFFFFF, -0x80000000)))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        state[0x7B] = random_.choice((0, 0, 1, 0x80))
        struct.pack_into("<i", state, 0x84, random_.choice((100, 100, 0, -1)))
        for source_index in indices:
            t.check(
                "sound_source_unknown_00127fc0",
                source_index,
                globals={"sound_sources": Buffer(bytes(data), pointers=buffer.pointers), "game_sound_globals": Buffer(bytes(state))},
                externals={
                    "sound_source_unknown_00127f10": lambda harness, arguments: (
                        arguments[0] & 1,
                        (arguments[0], harness.normalize(arguments[1]), harness.normalize(arguments[2])),
                    )
                },
            )

    for _ in range(300):
        said = bytearray(random_.randbytes(0x48))
        said[2] = random_.choice((0, 0, 1, 1, 2, 0xFF, 0x80))
        t.check(
            "sound_source_get_position",
            Buffer(bytes(said)),
            random_.randrange(4),
            Buffer(random_.randbytes(12)),
            globals={"game_sound_globals": Buffer(random_.randbytes(GLOBALS_SIZE))},
        )

    # ---------- a sound voice is let go, who is told is asked, and the sounds are stopped

    random_ = random.Random(156)
    VOICES = 6

    for _ in range(150):
        voices_ = bytearray(random_.randbytes(0x4C + VOICES * 0x6C))
        struct.pack_into("<I", voices_, 0x44, 0x4C)
        for index in range(VOICES):
            at = 0x4C + index * 0x6C
            struct.pack_into("<h", voices_, at + 2, random_.choice((-1, 0, 5, 31, 32, 63)))
            struct.pack_into("<h", voices_, at + 0xA, random_.choice((1, 1, 2, 0, -1, 0x100)))
        state = random_.randbytes(GLOBALS_SIZE)
        for voice_index in [NONE] + [random_.getrandbits(16) << 16 | index for index in range(VOICES)]:
            t.check(
                "sound_voice_release",
                voice_index,
                globals={"sound_voices": Buffer(bytes(voices_), pointers=(0x44,)), "game_sound_globals": Buffer(state)},
                externals={"datum_delete": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1]))},
            )

    def classed(classes: tuple, kinds: tuple) -> tuple:
        """The sounds that play and the tags of their sounds, with the
        classes of the sounds and the kinds of those that play taken from
        some, and the classes of sounds that sound_class_get gives."""
        sources, tags = playing_sounds()
        sources_ = bytearray(sources.data)
        tags_ = bytearray(tags.data)
        for n in range(SOURCES):
            sources_[0x50 + n * 0xBC + 2] = random_.choice(kinds)
            tags_[SOURCES * 0x10 + n * 0x10 + 2] = random_.choice(classes)
        table = bytearray(random_.randbytes(0x100 * 0x5C))
        for n in range(0x100):
            table[n * 0x5C + 0x4C] = random_.choice((0, 1, 1, 2))
        return (
            Buffer(bytes(sources_), pointers=sources.pointers),
            Buffer(bytes(tags_), pointers=tags.pointers),
            Buffer(bytes(table)),
        )

    def class_of(table: Buffer):
        def handler(harness, arguments):
            index = arguments[0] & 0xFFFF
            # a class is a signed number: those under 0 are given as the last ones
            return table.address + (index & 0xFF) * 0x5C, (index,)

        return handler

    for _ in range(12):
        sources, tags, table = classed((0, 1, 5, 0x20, 0x26, 0x35), (0, 0, 1, 2, 3, 4, 5, 6, 0x21, 0x3F, 0xFF))
        for n in random_.sample(range(SOURCES), 60):
            answer = random_.choice((0, 0, 1, 0x100, 0xFF))
            for told_ in (True, True, False):
                t.check(
                    "sound_source_unknown_00127f10",
                    random_.getrandbits(16) << 16 | n,
                    Buffer(struct.pack("<I", mock_address(0)) + random_.randbytes(0x14)) if told_ else 0,
                    Buffer(random_.randbytes(4)),
                    globals={"sound_sources": Buffer(sources.data, pointers=sources.pointers), "global_tag_instances": tags},
                    extra=[table],
                    externals={"sound_class_get": class_of(table)},
                    mocks=[Mock(lambda call, answer=answer: answer, stack_arguments=4)],
                )

    for n in range(160):
        sources, tags, table = classed((0, 5, 0x20, 0x20, 0x26, 0x26, 0x35), (0, 1, 2, 3, 4, 5, 0x21, 0xFF))
        in_use = sorted(random_.sample(range(SOURCES), random_.choice((0, 1, 8, 30))))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        state[0x78] = random_.choice((1, 1, 1, 1, 0, 0x80))
        state[0x7B] = random_.choice((0, 1, 0x80))
        t.check(
            "sound_reset",
            random_.choice((0, 0x20, 0x40, 0x60, 0x20, 0x60, random_.getrandbits(32))),
            globals={
                "game_sound_globals": Buffer(bytes(state)),
                "sound_sources": sources,
                "global_tag_instances": tags,
                "unknown_004ed288": Buffer(random_.randbytes(0x30)),
            },
            extra=[table],
            externals={
                "sound_effects_delete_all": lambda harness, arguments: 0,
                "data_next_absolute_index": next_absolute_index(in_use),
                "sound_source_delete": lambda harness, arguments: (0, tuple(arguments)),
                "sound_class_get": class_of(table),
                "sound_idle": lambda harness, arguments: 0,
                "sound_console_voices_stop": lambda harness, arguments: 0,
                "GetTickCount": lambda harness, arguments: 0x89ABCDEF,
                "looping_sounds_delete_all": lambda harness, arguments: 0,
            },
        )

    # ---------- small functions of other files of the sound

    random_ = random.Random(157)

    for _ in range(40):
        cache = bytearray(random_.randbytes(0x70))
        struct.pack_into("<i", cache, 0x38, random_.choice((0x7FFFFFFF, 0x7FFFFFFE, 5, 0, -1)))
        t.check(
            "sound_cache_idle",
            globals={"sound_lruv_cache": Buffer(bytes(cache))},
            externals={"lruv_wrap_ticks": lambda harness, arguments: (0, (harness.normalize(arguments[0]),))},
        )

    for _ in range(200):
        gestalt, _permutations = recordings()
        permutation = bytearray(random_.randbytes(0x10))
        struct.pack_into("<h", permutation, 0xC, random_.randrange(0, PIECES - 12))
        t.check(
            "sound_permutation_chunks_size",
            Buffer(bytes(permutation)),
            random_.choice((0, 1, 2, 5, 12, -1, -0x80000000)),
            globals={"global_sound_cache_file_gestalt": Buffer(gestalt, pointers=(0x24, 0x2C, 0x44))},
        )

    for _ in range(60):
        # what the sounds share: 4 promotions after it, and 8 rules of each
        data = bytearray(random_.randbytes(0x60))
        pointers = [0x34, 0x4C]
        struct.pack_into("<I", data, 0x34, len(data))
        data += random_.randbytes(6 * 0x34)
        promotions = len(data)
        struct.pack_into("<I", data, 0x4C, promotions)
        data += random_.randbytes(4 * 0x1C)
        for n in range(4):
            struct.pack_into("<I", data, promotions + n * 0x1C + 4, len(data))
            pointers.append(promotions + n * 0x1C + 4)
            for _rule in range(8):
                data += struct.pack("<h", random_.choice((-1, -1, 0, 3, 0x7FFF, -2))) + random_.randbytes(0xE)
        shared = Buffer(bytes(data), pointers=tuple(pointers))
        for _sound in range(12):
            definition = bytearray(random_.randbytes(0x14))
            definition[0xC] = random_.choice((0xFF, 0, 1, 2, 3))
            definition[0xD] = random_.choice((0xFF, 0, 1, 2, 5))
            t.check(
                "sound_definition_promotion_rule",
                random_.randrange(8),
                Buffer(bytes(definition)),
                globals={"global_sound_cache_file_gestalt": shared},
            )
            t.check(
                "sound_definition_get_custom_playback",
                Buffer(bytes(definition)),
                globals={"global_sound_cache_file_gestalt": shared},
            )

    for _ in range(40):
        controllers = bytearray(random_.randbytes(0x4C + CONTROLLERS * 0x1C))
        struct.pack_into("<I", controllers, 0x44, 0x4C)
        for controller_index in [NONE] + [random_.getrandbits(16) << 16 | index for index in range(CONTROLLERS)]:
            t.check(
                "sound_playback_controller_get",
                controller_index,
                globals={
                    "unknown_0051ebd8": Buffer(bytes(controllers), pointers=(0x44,)),
                    "unknown_0051ebdc": Buffer(random_.randbytes(0x1C)),
                },
            )
    for _ in range(300):
        controller = bytearray(random_.randbytes(0x1C))
        struct.pack_into("<I", controller, 4, random_.choice((NONE, 0, random_.getrandbits(32))))
        controller[3] = random_.choice((0, 1, 0x7F, 0x80, 0xFF, random_.getrandbits(8)))
        t.check("sound_playback_controller_add_count", Buffer(bytes(controller)), random_.choice((1, 0xFF, 0, 2, 0x7F, 0x80)))

    def new_array(memory):
        def handler(harness, arguments):
            name, maximum_count, size = arguments
            text = bytes(harness.mu.mem_read(name, 16)).split(b"\0")[0]
            return (memory.address if memory else 0), (text, maximum_count, size)

        return handler

    def made(harness, arguments):
        return 0, (harness.normalize(arguments[0]),)

    for name, variable in (
        ("looping_sounds_initialize", "looping_sounds"),
        ("sound_effects_initialize", "sound_effects"),
        ("sound_voices_initialize", "sound_voices"),
    ):
        for give in (True, True, False):
            for _ in range(6):
                memory = Buffer(random_.randbytes(0x4C)) if give else None
                configuration = random_.randbytes(0xC) + struct.pack("<h", random_.choice((51, 0, 1, -1, 0x7FFF)))
                t.check(
                    name,
                    *((Buffer(configuration),) if name == "sound_voices_initialize" else ()),
                    globals={variable: random_.getrandbits(32)},
                    extra=[memory] if memory else [],
                    externals={"shell_memory_data_new": new_array(memory), "data_delete_all": made},
                )

    for _ in range(60):
        effects = bytearray(random_.randbytes(0x4C + 16 * 0x14))
        struct.pack_into("<I", effects, 0x44, 0x4C)
        for index in range(16):
            struct.pack_into("<I", effects, 0x4C + index * 0x14 + 8, random_.choice((NONE, 0, random_.getrandbits(32))))
        for index in range(16):
            t.check(
                "sound_effect_delete",
                random_.getrandbits(16) << 16 | index,
                globals={"sound_effects": Buffer(bytes(effects), pointers=(0x44,))},
                externals={
                    "sound_playback_controller_release": lambda harness, arguments: (0, tuple(arguments)),
                    "datum_delete": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                },
            )

    for _ in range(20):
        for name in ("sound_console_streams_pause", "sound_console_streams_resume"):
            t.check(
                name,
                globals={"unknown_0051ebe4": Buffer(random_.randbytes(0x2AA0))},
                externals={"unknown_0037b73f": lambda harness, arguments: (0, tuple(arguments))},
            )

    # ---------- more small functions of other files of the sound

    random_ = random.Random(158)

    for _ in range(200):
        # the cache of the sounds, with its array of blocks after it
        cache = bytearray(random_.randbytes(0x70)) + bytearray(random_.randbytes(0x4C + COUNTED * 0x18))
        struct.pack_into("<i", cache, 0x34, random_.choice((0, 1, 9, 12)))
        struct.pack_into("<I", cache, 0x64, 0x70)
        struct.pack_into("<I", cache, 0x70 + 0x44, 0x70 + 0x4C)
        for index in range(COUNTED):
            struct.pack_into("<i", cache, 0x70 + 0x4C + index * 0x18 + 8, random_.randrange(0, 0x2000))
        uses = bytearray(counted().data)
        for index in range(COUNTED):
            uses[0x4C + index * 0x10 + 5] = random_.choice((0, 1, 0xFE, 0xFF, 0x80))
        chunk = random_.randbytes(8) + struct.pack("<I", random_.getrandbits(16) << 16 | random_.randrange(COUNTED))
        t.check(
            "sound_cache_chunk_get_address",
            Buffer(chunk),
            globals={
                "unknown_00502104": Buffer(bytes(uses), pointers=(0x44,)),
                "sound_lruv_cache": Buffer(bytes(cache), pointers=(0x64, 0x70 + 0x44)),
                "sound_cache_base": random_.choice((0, 0x80000000, random_.getrandbits(32))),
            },
        )

    for _ in range(30):
        sources, tags = playing_sounds()
        gestalt, _permutations = recordings()
        for n in random_.sample(range(SOURCES), 20):
            t.check(
                "sound_permutation_unknown6",
                random_.getrandbits(16) << 16 | n,
                random_.randrange(3),
                random_.randrange(6),
                globals={
                    "global_tag_instances": tags,
                    "global_sound_cache_file_gestalt": Buffer(gestalt, pointers=(0x24, 0x2C, 0x44)),
                },
            )

    for bit in range(16):
        for sound_flags in (1 << bit, 0xFFFF ^ 1 << bit):
            for _ in range(6):
                sound_index = random_.randrange(TAGS)
                t.check(
                    "sound_definition_get_maximum_distance",
                    random_.getrandbits(16) << 16 | sound_index,
                    globals=world(random_, sound_index, sound_data(random_, sound_flags)),
                )

    for _ in range(150):
        definition = sound_data(random_)
        globals_ = world(random_, 0, sound_data(random_))
        for name in ("sound_pitch_range_set_unknown4", "sound_pitch_range_set_unknown5"):
            t.check(name, Buffer(random_.randbytes(0xC)), Buffer(definition), random_.getrandbits(8), globals=globals_)
        for name in ("sound_pitch_range_get_unknown4", "sound_pitch_range_get_unknown5"):
            pitch_range = bytearray(random_.randbytes(0xC))
            pitch_range[4:6] = bytes(random_.choice((0, 1, 0x7F, 0x80, 0xFF)) for _ in range(2))
            t.check(name, Buffer(bytes(pitch_range)), Buffer(definition), globals=globals_)

    for _ in range(60):
        looping = bytearray(random_.randbytes(0x4C + 6 * 0xD4))
        struct.pack_into("<I", looping, 0x44, 0x4C)
        controllers = bytearray(random_.randbytes(0x4C + CONTROLLERS * 0x1C))
        struct.pack_into("<I", controllers, 0x44, 0x4C)
        for index in range(CONTROLLERS):
            controllers[0x4C + index * 0x1C + 2] = random_.choice((0, 1, 2, 0x80, 0x81, 0x82, 0xFF))
        for index in range(6):
            struct.pack_into(
                "<I",
                looping,
                0x4C + index * 0xD4 + 0x54,
                random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(CONTROLLERS))),
            )
        for index in range(6):
            t.check(
                "looping_sound_delete",
                random_.getrandbits(16) << 16 | index,
                globals={
                    "looping_sounds": Buffer(bytes(looping), pointers=(0x44,)),
                    "unknown_0051ebd8": Buffer(bytes(controllers), pointers=(0x44,)),
                },
                externals={"datum_delete": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1]))},
            )

    for _ in range(120):
        start = random_.choice((0x1234, NONE, 0))
        voices_ = bytearray(random_.randbytes(0x4C + VOICES * 0x6C))
        struct.pack_into("<I", voices_, 0x44, 0x4C)
        for index in range(VOICES):
            struct.pack_into("<I", voices_, 0x4C + index * 0x6C + 0x44, random_.choice((NONE, start, start, 7)))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        struct.pack_into("<I", state, 0x80, start)
        for index in range(VOICES):
            t.check(
                "sound_voice_unknown_0021dff0",
                random_.getrandbits(16) << 16 | index,
                Buffer(random_.randbytes(0x48)),
                Buffer(random_.randbytes(4)),
                globals={"sound_voices": Buffer(bytes(voices_), pointers=(0x44,)), "game_sound_globals": Buffer(bytes(state))},
            )

    for _ in range(150):
        table = bytearray(random_.randbytes(0x100 * 0x5C))
        for n in range(0x100):
            struct.pack_into("<f", table, n * 0x5C + 0x10, random_.choice((0.0, 1.0, 1.0, 2.5, -0.0, float("nan"))))
            struct.pack_into(
                "<f", table, n * 0x5C + 0x50, random_.choice((0.0, 0.00005, 0.0001, 0.00011, 0.00009999, 1.0, 1.00005, float("nan")))
            )
        table_ = Buffer(bytes(table))
        for _pair in range(8):
            first = random_.randrange(12)
            t.check(
                "sound_classes_match",
                first,
                random_.choice((first, random_.randrange(12), random_.randrange(12))),
                extra=[table_],
                externals={"sound_class_get": class_of(table_)},
            )

    # ---------- the decibels of a sound, and looking for a sound playback controller

    random_ = random.Random(159)

    for _ in range(200):
        definition = bytearray(sound_data(random_))
        definition[0xB] = random_.randrange(4)
        globals_ = world(random_, 0, sound_data(random_))
        scales = struct.pack("<3I", random_.getrandbits(32), random_.getrandbits(32), random_.getrandbits(32)) + struct.pack("<I", 0x10)
        scales += reals(random_, 4 * 0x14 // 4)
        globals_["global_sound_cache_file_gestalt"] = Buffer(scales, pointers=(0xC,))
        for name in ("sound_definition_get_minimum_decibels", "sound_definition_get_maximum_decibels"):
            t.check(name, Buffer(bytes(definition)), globals=globals_)
        permutation = bytearray(random_.randbytes(0x10))
        struct.pack_into("<H", permutation, 6, random_.choice((0xFFFF, 0, 5, 0x8000, 0xFFFE)))
        t.check("sound_permutation_get_unknown6", Buffer(bytes(permutation)), Buffer(bytes(definition)), globals=globals_)

    def controllers_(owners: tuple) -> Buffer:
        data = bytearray(random_.randbytes(0x4C + CONTROLLERS * 0x1C))
        struct.pack_into("<I", data, 0x24, 0x1C)
        struct.pack_into("<i", data, 0x38, random_.choice((CONTROLLERS, CONTROLLERS, CONTROLLERS - 2, 0)))
        struct.pack_into("<I", data, 0x44, 0x4C)
        for index in range(CONTROLLERS):
            at = 0x4C + index * 0x1C
            struct.pack_into("<H", data, at, random_.choice((0, 1, 0x7FFF, 0x8001, random_.getrandbits(16))))
            data[at + 2] = random_.choice((0, 1, 0x7F, 0x80, 0xFE, 0xFF))
            struct.pack_into("<I", data, at + 4, random_.choice(owners))
        return Buffer(bytes(data), pointers=(0x44,))

    OWNERS = (NONE, 0, 1, 2, 0xE1740001)
    for _ in range(60):
        buffer = controllers_(OWNERS)
        identifiers = [struct.unpack_from("<H", buffer.data, 0x4C + index * 0x1C)[0] for index in range(CONTROLLERS)]
        indices = [NONE, CONTROLLERS, 0x10000 | CONTROLLERS] + [identifiers[index] << 16 | index for index in range(CONTROLLERS)]
        indices += [(identifiers[index] ^ 1) << 16 | index for index in range(CONTROLLERS)]
        for controller_index in indices:
            t.check(
                "sound_playback_controller_try_and_get",
                controller_index,
                globals={
                    "unknown_0051ebd8": Buffer(buffer.data, pointers=buffer.pointers),
                    "unknown_0051ebdc": Buffer(random_.randbytes(0x1C)),
                },
            )

    for n in range(240):
        state = sound_state(*states[n % len(states)])
        in_use = sorted(random_.sample(range(CONTROLLERS), random_.choice((0, 1, 3, CONTROLLERS))))
        owner = random_.choice(OWNERS)
        t.check(
            "sound_playback_controller_find",
            owner,
            globals={
                "game_sound_globals": state,
                "unknown_0051ebd8": controllers_(OWNERS),
                "unknown_0051ebdc": Buffer(random_.randbytes(4) + struct.pack("<I", owner) + random_.randbytes(0x14)),
            },
            externals={"data_next_absolute_index": next_absolute_index(in_use)},
        )
        found = random_.choice((NONE, NONE, random_.getrandbits(16) << 16 | random_.randrange(CONTROLLERS)))
        t.check(
            "sound_playback_controller_acquire",
            owner,
            globals={"game_sound_globals": sound_state(*states[n % len(states)]), "unknown_0051ebd8": controllers_(OWNERS)},
            externals={"sound_playback_controller_find": lambda harness, arguments, found=found: (found, tuple(arguments))},
        )

    # ---------- bytes in some seconds, the bits of a pitch range, and a new sound playback controller

    random_ = random.Random(160)

    for unknown4 in range(3):
        for unknown5 in range(5):
            for unknown3 in range(3):
                for seconds in (0.0, 1.0, 0.5, 2.0, 1.0 / 3.0, 59.99, -1.0, *(random_.uniform(0, 30) for _ in range(12))):
                    t.check("sound_seconds_to_bytes", unknown4, unknown5, unknown3, as_bits(seconds))

    for _ in range(300):
        definition = sound_data(random_)
        globals_ = world(random_, 0, sound_data(random_))
        bits = random_.randbytes(0x40) + bytes(0x10)
        shared = bytearray(random_.randbytes(0x40))
        struct.pack_into("<I", shared, 0x3C, len(shared))
        globals_["global_sound_cache_file_gestalt"] = Buffer(bytes(shared) + bits, pointers=(0x3C,))
        pitch_range = bytearray(random_.randbytes(0xC))
        struct.pack_into("<H", pitch_range, 6, random_.randrange(0, 0x40 * 8))
        struct.pack_into("<h", pitch_range, 0xA, random_.choice((0, 1, 2, 3, 8, 9, 16, 17, 24, 25, 32, -1, -9)))
        t.check("sound_pitch_range_get_played_bits", Buffer(bytes(pitch_range)), Buffer(definition), globals=globals_)
        t.check(
            "sound_pitch_range_set_played_bits",
            Buffer(bytes(pitch_range)),
            Buffer(definition),
            random_.getrandbits(32),
            globals=globals_,
        )

    for _ in range(200):
        controller = bytearray(random_.randbytes(0x1C))
        controller[2] = random_.choice((0, 1, 0x7F, 0x80, 0xFF, 0x55))
        t.check(
            "sound_playback_controller_initialize",
            Buffer(bytes(controller)),
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.choice((0, 1, 0x80, 0x100)),
        )

    for n in range(240):
        owner = random_.choice(OWNERS)
        found = random_.choice((NONE, NONE, random_.getrandbits(16) << 16 | random_.randrange(CONTROLLERS)))
        made = random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(CONTROLLERS)))
        state = bytearray(sound_state(*states[n % len(states)]).data)
        state[0x7F] = random_.choice((0, 1, 2, 3, 0xFF))
        buffer = controllers_(OWNERS)
        data = bytearray(buffer.data)
        for index in range(CONTROLLERS):
            data[0x4C + index * 0x1C + 2] = random_.choice((0, 0, 0x80, 1, 0x7F, 0xFF, 0x81))
        t.check(
            "sound_playback_controller_new",
            owner,
            globals={
                "game_sound_globals": Buffer(bytes(state)),
                "unknown_0051ebd8": Buffer(bytes(data), pointers=buffer.pointers),
                "unknown_0051ebdc": Buffer(random_.randbytes(0x1C)),
            },
            externals={
                "sound_playback_controller_find": lambda harness, arguments, found=found: (found, tuple(arguments)),
                "datum_new": lambda harness, arguments, made=made: (made, (harness.normalize(arguments[0]),)),
                "unknown_001462b0": lambda harness, arguments: 0x13579BDF,
                "sound_playback_controller_initialize": lambda harness, arguments: (
                    0,
                    (harness.normalize(arguments[0]), arguments[1], arguments[2], arguments[3] & 0xFF),
                ),
            },
        )
        buffer = controllers_(OWNERS)
        found = random_.choice((NONE, *(random_.getrandbits(16) << 16 | index for index in range(CONTROLLERS))))
        if found != NONE and random_.random() < 0.6:
            # the index that was found is of a controller
            found = struct.unpack_from("<H", buffer.data, 0x4C + (found & 0xFFFF) * 0x1C)[0] << 16 | found & 0xFFFF
        t.check(
            "sound_playback_controller_unknown_00219c30",
            owner,
            random_.getrandbits(32),
            globals={
                "game_sound_globals": Buffer(bytes(state)),
                "unknown_0051ebd8": buffer,
                "unknown_0051ebdc": Buffer(random_.randbytes(0x1C)),
            },
            externals={
                "sound_playback_controller_find": lambda harness, arguments, found=found: (found, tuple(arguments)),
                "sound_playback_controller_unknown_00219ed0": lambda harness, arguments: (
                    0,
                    (arguments[0] & 0xFF, harness.normalize(arguments[1]), arguments[2]),
                ),
            },
        )

    # ---------- a sound playback controller takes a flag, and three of the sound effects

    random_ = random.Random(161)

    for _ in range(300):
        controller = bytearray(random_.randbytes(0x1C))
        struct.pack_into("<I", controller, 4, random_.choice((NONE, 0, random_.getrandbits(32))))
        controller[2] = random_.choice((0, 1, 0x7F, 0x80, 0xFF, 0x55))
        said = bytearray(random_.randbytes(0x48))
        said[3] = random_.choice((1, 1, 0x11, 0xF1, 0, 2, 0x10))
        players = random_.choice(((), (0,), (1, 3), (0, 1, 2, 3)))

        def first_player(harness, arguments, players=players):
            return players[0] if players else NONE

        def next_player(harness, arguments, players=players):
            following = [player for player in players if player > arguments[0]]
            return (following[0] if following else NONE), tuple(arguments)

        def listener_real(harness, arguments):
            return real_bits(arguments[0] * 1.5 + 0.25), (arguments[0], bytes(harness.mu.mem_read(arguments[1], 12)))

        t.check(
            "sound_playback_controller_unknown_00219ed0",
            random_.choice((0, 1)),
            Buffer(bytes(controller)),
            Buffer(bytes(said)),
            externals={"unknown_0014dde0": first_player, "unknown_0014de10": next_player, "sound_listener_doppler_pitch": listener_real},
        )

    EFFECTS = 16

    def effects_() -> Buffer:
        data = bytearray(random_.randbytes(0x4C + EFFECTS * 0x14))
        struct.pack_into("<I", data, 0x44, 0x4C)
        for index in range(EFFECTS):
            at = 0x4C + index * 0x14
            data[at + 3] = random_.choice((0, 1, 1, 1, 2, 3, 0xFF))
            data[at + 4] = random_.choice((0, 0, 8, 0x10, 0x18, 0xE7, 0xFF, 2))
            struct.pack_into("<I", data, at + 0xC, random_.getrandbits(16) << 16 | random_.randrange(PLAYING))
        return Buffer(bytes(data), pointers=(0x44,))

    def effect_update(harness, arguments):
        # what the effect is when the function is called
        effects_address = struct.unpack("<I", harness.mu.mem_read(harness.global_addresses["sound_effects"], 4))[0]
        elements = struct.unpack("<I", harness.mu.mem_read(effects_address + 0x44, 4))[0]
        return 0, (tuple(arguments), bytes(harness.mu.mem_read(elements + (arguments[0] & 0xFFFF) * 0x14, 0x14)))

    for _ in range(40):
        buffer, indices = playing()
        for index in range(EFFECTS):
            effect_index = random_.getrandbits(16) << 16 | index
            t.check(
                "sound_effect_unknown_0021d360",
                effect_index,
                globals={"sound_effects": effects_()},
                externals={"sound_effect_start_sounds": effect_update},
            )
            mark = random_.choice((0, 0x20, 0xFF, 0xDF))

            def asked(harness, arguments, mark=mark):
                source_index, callbacks, flags = arguments
                record = source_index, harness.normalize(callbacks), bytes(harness.mu.mem_read(flags, 2))
                harness.mu.mem_write(flags, bytes((mark,)))
                return source_index & 1 | (source_index >> 3 & 1) << 8, record

            t.check(
                "sound_effect_unknown_0021d5a0",
                effect_index,
                globals={"sound_effects": effects_(), "sound_sources": Buffer(buffer.data, pointers=buffer.pointers)},
                externals={"sound_source_unknown_00127f10": asked},
            )

    for _ in range(60):
        buffer, indices = playing((random_.random() < 0.8, 1))
        data = bytearray(buffer.data)
        table = 0x4C + PLAYING * DATUM
        pointers = [0x44]
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            struct.pack_into("<I", data, at + 0x84, random_.getrandbits(16) << 16 | random_.randrange(EFFECTS))
            if random_.random() < 0.7:
                struct.pack_into("<I", data, at + 0x88, table)
                pointers.append(at + 0x88)
            else:
                struct.pack_into("<I", data, at + 0x88, 0)
        for index in range(PLAYING):
            t.check(
                "sound_effect_source_deleted",
                random_.getrandbits(32),
                random_.getrandbits(16) << 16 | index,
                random_.choice((10, 2, 2, 0, 1, 3)),
                globals={"sound_effects": effects_(), "sound_sources": Buffer(bytes(data), pointers=tuple(pointers))},
                externals={"sound_effect_start_sounds": effect_update},
                mocks=told(),
            )

    # ---------- numbers for the console's sound

    random_ = random.Random(162)

    gains = [0.0, 1.0, 0.5, -1.0, 2.0, float("nan"), 1e-4, 6.3e-4, 6.31e-4, 6.3095e-4, 6.3096e-4, 1e-30, 0.999999, 1.0000001]
    gains += [random_.uniform(0, 1.2) for _ in range(200)] + [10 ** random_.uniform(-4, 0) for _ in range(200)]
    for gain in gains:
        t.check("sound_gain_to_decibels", as_bits(gain))

    for _ in range(300):
        count = random_.randrange(8)
        mixbins = bytearray(random_.randbytes(0x44))
        struct.pack_into("<i", mixbins, 0x40, count)
        decibels = random_.choice((0.0, -64.0, -32.0, 1.0, -65.0, float("nan"), random_.uniform(-70, 5), random_.uniform(-64, 0)))
        t.check("sound_mixbins_add", Buffer(bytes(mixbins)), random_.getrandbits(32), as_bits(decibels))

    frequencies = [29.999, 30.0, 30.001, 8000.0, 8000.001, 0.0, -5.0, float("nan"), 1000.0, 4000.0, 7999.5]
    frequencies += [random_.uniform(30, 8000) for _ in range(300)]
    for frequency in frequencies:
        t.check("sound_frequency_to_filter_coefficient", as_bits(frequency))

    values = [0.0, 22.5, 22.4999, 23.0, 100.0, -1.0, -6.0, -6.0206, -6.03, 6.0, 1.0, -20.0, -90.0, -150.0]
    values += [random_.uniform(-12, 30) for _ in range(300)]
    for value in values:
        t.check("sound_unknown_0021e2d0", as_bits(value))
    values = [0.0, 1.0, -1.0, 12.0, 13.9, 14.0, 20.0, 100.0, -6.0, -20.0, -100.0, 6.0, 0.001, -0.001, 250.0]
    values += [random_.uniform(-40, 20) for _ in range(300)]
    for value in values:
        t.check("sound_unknown_0021e330", as_bits(value))

    # ---------- decibels between two others, and those of a sound that fades

    random_ = random.Random(163)

    def some_decibels() -> float:
        return random_.choice((0.0, -64.0, -6.0, -70.0, 3.0, random_.uniform(-70, 5), random_.uniform(-64, 0)))

    def some_fraction() -> float:
        return random_.choice((0.0, 1.0, 0.5, 0.25, -0.5, 1.5, random_.uniform(0, 1), random_.uniform(-0.2, 1.2)))

    for kind in (0, 1, 2, 3, 4, NONE, 0x80000001):
        for _ in range(150):
            t.check("sound_decibels_interpolate", kind, as_bits(some_decibels()), as_bits(some_decibels()), as_bits(some_fraction()))
            t.check("sound_decibels_fade_in", kind, as_bits(some_decibels()), as_bits(some_fraction()))
    for kind in (1, 2):
        t.check("sound_decibels_fade_in", kind, as_bits(-3.0), as_bits(float("nan")))

    for _ in range(80):
        buffer, indices = playing()
        now = random_.choice((0, 100, 1000, 0x7FFF0000))
        data = bytearray(buffer.data)
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            data[at + 4] = random_.choice((0x80, 0x80, 0x80, 0xFF, 0x7F, 0))
            struct.pack_into("<h", data, at + 0xAE, random_.choice((0, 0, 1, 1, 2, -1)))
            struct.pack_into("<f", data, at + 0xB0, some_decibels())
            for field in (0xB4, 0xB8):
                struct.pack_into(
                    "<i", data, at + field, random_.choice((-1, 0, 300, now, now + 150, now - 150, now + 300, now - 300, now + 1))
                )
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        struct.pack_into("<i", state, 0x84, now)
        for source_index in indices:
            t.check(
                "sound_source_fade_decibels",
                source_index,
                globals={"sound_sources": Buffer(bytes(data), pointers=buffer.pointers), "game_sound_globals": Buffer(bytes(state))},
            )

    # ---------- how long a recording lasts

    random_ = random.Random(164)

    def lengths_world() -> tuple:
        """What the sounds of a map share for the length of a recording: how
        6 sounds are played, 4 scales, 5 parameters of pitch ranges, 6 pitch
        ranges and 12 recordings. Gives where the pitch ranges are."""
        data = bytearray(random_.randbytes(0x48))
        pointers = []

        def block(offset: int, content: bytes) -> int:
            struct.pack_into("<I", data, offset, len(data))
            pointers.append(offset)
            at = len(data)
            data.extend(content)
            return at

        playbacks_ = bytearray(random_.randbytes(6 * 0x38))
        for n in range(6):
            struct.pack_into("<2h", playbacks_, n * 0x38 + 0x18, random_.randint(-1200, 1200), random_.randint(-1200, 1200))
        block(4, bytes(playbacks_))
        scales_ = bytearray(random_.randbytes(4 * 0x14))
        for n in range(4):
            struct.pack_into("<2h", scales_, n * 0x14 + 8, random_.randint(-600, 600), random_.randint(-600, 600))
        block(0xC, bytes(scales_))
        parameters = bytearray(random_.randbytes(5 * 0xA))
        for n in range(5):
            struct.pack_into("<h", parameters, n * 0xA, random_.choice((0, 0, 100, -100, random_.randint(-2400, 2400))))
        block(0x1C, bytes(parameters))
        pitch_ranges = bytearray(random_.randbytes(6 * 0xC))
        for n in range(6):
            struct.pack_into("<h", pitch_ranges, n * 0xC + 2, random_.randrange(5))
            struct.pack_into("<h", pitch_ranges, n * 0xC + 8, random_.randrange(6))
        ranges_at = block(0x24, bytes(pitch_ranges))
        permutations_ = bytearray(random_.randbytes(12 * 0x10))
        for n in range(12):
            struct.pack_into("<i", permutations_, n * 0x10 + 8, random_.choice((0, 1, 22050, 44100, 1000000, random_.randrange(1 << 24))))
        block(0x2C, bytes(permutations_))
        return Buffer(bytes(data), pointers=tuple(pointers)), ranges_at

    def length_sound() -> bytes:
        definition = bytearray(random_.randbytes(0x10))
        definition[3:6] = bytes((random_.randrange(3), random_.randrange(3), random_.randrange(5)))
        struct.pack_into("<2h", definition, 6, random_.randrange(6), random_.randrange(3))
        definition[0xB] = random_.randrange(4)
        return bytes(definition)

    for _ in range(300):
        shared, ranges_at = lengths_world()
        t.check(
            "sound_permutation_get_maximum_milliseconds",
            Buffer(length_sound()),
            Buffer(shared.data[ranges_at + random_.randrange(6) * 0xC :][:0xC]),
            random_.choice((0, 1, 22050, 1000000, random_.randrange(1 << 24))),
            globals={"global_sound_cache_file_gestalt": shared},
        )
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        sound_index = random_.randrange(TAGS)
        struct.pack_into("<I", tags, sound_index * 0x10 + 8, len(tags))
        t.check(
            "sound_get_maximum_milliseconds",
            random_.getrandbits(16) << 16 | sound_index,
            random_.randrange(3),
            random_.randrange(6),
            as_bits(random_.choice((0.0, 100.0, -1200.0, 1200.0, random_.uniform(-2400, 2400)))),
            globals={
                "global_sound_cache_file_gestalt": shared,
                "global_tag_instances": Buffer(bytes(tags) + length_sound(), pointers=(sound_index * 0x10 + 8,)),
            },
        )

    # ---------- a number at random, two of the looping sounds, and a byte of a pitch range

    random_ = random.Random(165)

    for _ in range(300):
        t.check(
            "sound_unknown_00219880",
            Buffer(reals(random_, 4)),
            Buffer(struct.pack("<I", random_.choice((0, 1, 0xFFFFFFFF, random_.getrandbits(32))))),
            as_bits(random_.choice((0.0, 1.0, 0.5, random_.uniform(0, 1), random_.uniform(-1, 2)))),
        )

    for current in range(7):
        for following in range(7):
            something = bytearray(random_.randbytes(0x10))
            something[0xC:0xE] = bytes((current, following))
            t.check("looping_sound_unknown_0021a080", Buffer(bytes(something)))
            for _ in range(6):
                definition = bytearray(random_.randbytes(0x50))
                for offset in (0x14, 0x1C, 0x24, 0x2C, 0x34, 0x40, 0x48):
                    struct.pack_into("<I", definition, offset + 4, random_.choice((NONE, NONE, 0, random_.getrandbits(32))))
                t.check("looping_sound_unknown_0021a050", Buffer(bytes(something)), Buffer(bytes(definition)))

    for _ in range(200):
        gestalt, _permutations = recordings()
        definition = bytearray(sound_data(random_))
        struct.pack_into("<h", definition, 8, random_.randrange(2))
        globals_ = world(random_, 0, sound_data(random_))
        shared = bytearray(gestalt)
        # the bytes that the pitch ranges keep
        for n in range(4):
            shared[0x48 + n * 0xC + 4] = random_.choice((0, 1, 0x7F, 0x80, 0xFF))
        globals_["global_sound_cache_file_gestalt"] = Buffer(bytes(shared), pointers=(0x24, 0x2C, 0x44))
        t.check(
            "sound_unknown_0021cc30",
            Buffer(bytes((random_.choice((0, 0, 0, 0x10, 0x20, 0x30, 0xCF, 0xFF)),)) + random_.randbytes(3)),
            Buffer(bytes(definition)),
            random_.randrange(3),
            globals=globals_,
        )

    # ---------- a sound starts, and four more

    random_ = random.Random(166)

    for n in range(200):
        looked = random_.choice((1, 1, 1, 0, 0x100))
        refused = random_.choice((0, 0, 1, 0x80000000))
        made = random_.choice((NONE, random_.getrandbits(32), 0x12340005))

        def look(harness, arguments, looked=looked):
            request_, sound_index_, first, failure = arguments
            harness.mu.mem_write(first, struct.pack("<I", 0xA1B2C3D4))
            return looked, (harness.normalize(request_), sound_index_, harness.normalize(failure))

        def refuse(harness, arguments, refused=refused):
            sound_index_, second = arguments
            harness.mu.mem_write(second, struct.pack("<I", 0x0F1E2D3C))
            return refused, (sound_index_,)

        said = Buffer(random_.randbytes(0x90))
        t.check(
            "sound_source_new",
            random_.getrandbits(32),
            random_.choice((0, Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4)))),
            said,
            globals={"game_sound_globals": sound_state(*states[n % len(states)])},
            externals={
                "sound_start_test": look,
                "sound_promotion_update": refuse,
                "sound_source_create": lambda harness, arguments, made=made: (
                    made,
                    (arguments[0], arguments[1], harness.normalize(arguments[2]), arguments[3]),
                ),
            },
        )

    for _ in range(40):
        sources, tags = playing_sounds()
        gestalt, _permutations = recordings()
        shared = Buffer(gestalt, pointers=(0x24, 0x2C, 0x44))

        def request(harness, arguments):
            sound_index, chunk, kind = arguments
            return 0, (sound_index, harness.normalize(chunk), kind)

        for n in random_.sample(range(SOURCES), 30):
            t.check(
                "sound_source_hold_chunk",
                Buffer(sources.data[0x50 + n * 0xBC : 0x50 + (n + 1) * 0xBC]),
                globals={"global_tag_instances": tags, "global_sound_cache_file_gestalt": shared},
                externals={"sound_cache_chunk_request": request},
            )

    for _ in range(150):
        count = random_.choice((0, 1, 5, CHANNELS, -1))
        data = bytearray(random_.randbytes(CHANNELS * 0x24))
        for n in range(CHANNELS):
            struct.pack_into("<I", data, n * 0x24, random_.choice((NONE, 0, random_.getrandbits(32))))
            data[n * 0x24 + 9] = random_.choice((0, 0, 0, 1, 0x80))
            struct.pack_into("<hh", data, n * 0x24 + 0xC, random_.choice((-1, 0, 5, 127)), random_.choice((-1, -1, 0, 9, 63)))
        t.check(
            "sound_channels_unknown_00129380",
            Buffer(random_.randbytes(0x200)),
            Buffer(random_.randbytes(0x100)),
            globals={"game_sound_globals": sound_state(channels=count), "sound_channels": Buffer(bytes(data))},
            externals={"sound_channel_update_slot": lambda harness, arguments: (0, tuple(arguments))},
        )

    for _ in range(40):
        t.check(
            "sound_console_set_environment",
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(0x30)),
            globals={"unknown_0051ebe4": Buffer(random_.randbytes(0x2AC0))},
            externals={"sound_console_set_environment_handler": lambda harness, arguments: (0, tuple(arguments))},
        )

    for _ in range(120):
        gains_ = bytearray(random_.randbytes(0x36 * 0x10))
        for n in range(0x36):
            struct.pack_into(
                "<3f",
                gains_,
                n * 0x10,
                random_.uniform(-64, 0),
                random_.uniform(-64, 0),
                random_.choice((0.0, 0.5, 1.0, 1.0 / 30.0, random_.uniform(0, 3))),
            )
        seconds = random_.choice((0.0, -1.0, 1.0 / 30.0, 0.5, 1.0, float("nan"), random_.uniform(0, 2)))
        t.check("sound_classes_update", as_bits(seconds), globals={"sound_class_gains": Buffer(bytes(gains_))})

    # ---------- two sounds fade, a distance, a piece for a channel, and how much is heard at a distance

    random_ = random.Random(167)

    for _ in range(60):
        buffer, indices = playing()
        for _pair in range(8):
            t.check(
                "sound_sources_cross_fade",
                random_.choice((NONE, *indices)),
                random_.choice((NONE, *indices)),
                random_.choice((0, 1, 2, 0xFFFF)),
                as_bits(random_.choice((0.0, 0.5, 1.0, 0.3, 2.5, 0.0005))),
                globals={"sound_sources": Buffer(buffer.data, pointers=buffer.pointers)},
                externals={"sound_source_fade_decibels": lambda harness, arguments: (fade_bits(arguments[0]), tuple(arguments))},
            )

    for _ in range(600):
        said = bytearray(random_.randbytes(0x48))
        said[2] = random_.choice((0, 0, 1, 2, 0xFF))
        said[3] = random_.choice((0, 1, 1, 1, 0x11, 0xF1, 2, 0xF, 0xF0, 0x21))
        said[0xC:0x18] = reals(random_, 3)
        struct.pack_into("<f", said, 0x30, random_.choice((0.0, 1.0, 3.0, 50.0, -1.0)))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        for index in range(4):
            position = LISTENERS + index * LISTENER_SIZE + 0x30
            state[position : position + 12] = reals(random_, 3)
            if random_.random() < 0.3:
                # at the height of the sound, or a little over
                height = struct.unpack_from("<f", said, 0x14)[0] + random_.choice((0.0, 0.5, 2.0))
                struct.pack_into("<f", state, position + 8, height)
        t.check(
            "sound_source_unknown_00127e20",
            Buffer(bytes(said)),
            random_.randrange(4),
            globals={"game_sound_globals": Buffer(bytes(state))},
        )

    def piece_request(harness, arguments):
        sound_index, chunk, kind = arguments
        return 0, (sound_index, harness.normalize(chunk), kind)

    for _ in range(120):
        gestalt, permutations = recordings()
        data = bytearray(gestalt)
        channels = len(data)
        data += random_.randbytes(CHANNELS * 0x24)
        pointers = [0x24, 0x2C, 0x44]
        for n in range(CHANNELS):
            at = channels + n * 0x24
            struct.pack_into("<h", data, at + 0xC, random_.choice((0, 1, 2, 3, 31)))
            struct.pack_into("<hh", data, at + 0x18, random_.randrange(0, 10), random_.randrange(0, 10))
            for field in (0x1C, 0x20):
                if random_.random() < 0.6:
                    struct.pack_into("<I", data, at + field, permutations + random_.randrange(12) * 0x10)
                    pointers.append(at + field)
                else:
                    struct.pack_into("<I", data, at + field, 0)
        buffer = Buffer(bytes(data), pointers=tuple(pointers))
        for channel_index in range(CHANNELS):
            recording = bytearray(random_.randbytes(0x10))
            struct.pack_into("<h", recording, 0xC, random_.randrange(0, 12))
            t.check(
                "sound_channel_queue_chunk",
                channel_index,
                Buffer(bytes(recording)),
                random_.randrange(0, 10),
                globals={
                    "global_sound_cache_file_gestalt": buffer,
                    "sound_channels": Pointer(buffer, channels),
                    "unknown_00502104": counted(),
                    "unknown_0051ebe4": Buffer(random_.randbytes(32 * 0x34 + 0x10)),
                },
                externals={
                    "unknown_002ae500": lambda harness, arguments: (
                        0,
                        (harness.normalize(arguments[0]), harness.normalize(arguments[1])),
                    ),
                    "sound_cache_chunk_request": piece_request,
                },
            )

    for bit in range(16):
        for flags, sound_flags in ((1 << bit, 0), (0, 1 << bit), (0xFFFF ^ 1 << bit, 0xFFFF), (0, 0xFFFF ^ 1 << bit)):
            for _ in range(5):
                sound_index = random_.randrange(TAGS)
                said = bytearray(source(random_, flags).data)
                if random_.random() < 0.3:
                    struct.pack_into("<f", said, 0x34, random_.choice((0.0, 0.0005, 0.001, 0.002, -1.0)))
                t.check(
                    "sound_source_distance_gain",
                    random_.getrandbits(16) << 16 | sound_index,
                    Buffer(bytes(said)),
                    as_bits(random_.choice((0.0, 0.5, 1.0, 2.0, 10.0, 60.0, random_.uniform(0, 60)))),
                    globals=world(random_, sound_index, sound_data(random_, sound_flags)),
                )

    # ---------- what is done with all the looping sounds, the sound effects and the sound playback controllers

    random_ = random.Random(168)

    def some_in_use(maximum_count: int) -> dict:
        count = random_.choice((0, 1, maximum_count // 2, maximum_count))
        return {n: identifier(random_) for n in random_.sample(range(maximum_count), count)}

    def patched(buffer: Buffer, size: int, count: int, patch) -> Buffer:
        """A data array with something of each of its elements changed."""
        data = bytearray(buffer.data)
        for n in range(count):
            patch(data, 0x4C + n * size)
        return Buffer(bytes(data), pointers=buffer.pointers)

    def controller_patch(data: bytearray, at: int) -> None:
        data[at + 2] = random_.choice((0, 1, 2, 0x80, 0x81, 0xFF))
        data[at + 3] = random_.choice((0, 0x80, 1, 0x81, 0x7F, 0xFF))
        struct.pack_into("<I", data, at + 4, random_.choice((NONE, 0, random_.getrandbits(32))))

    def all_controllers() -> Buffer:
        return patched(data_array(random_, CONTROLLERS, 0x1C, some_in_use(CONTROLLERS)), 0x1C, CONTROLLERS, controller_patch)

    for _ in range(80):
        t.check(
            "sound_playback_controllers_update",
            globals={"unknown_0051ebd8": all_controllers(), "unknown_0051ebdc": Buffer(random_.randbytes(0x1C))},
        )
        for enable in (0, 1, 0x80):
            t.check(
                "sound_playback_controllers_unknown_00219d90",
                enable,
                globals={"unknown_0051ebd8": all_controllers(), "unknown_0051ebdc": Buffer(random_.randbytes(0x1C))},
                externals={"sound_console_unknown_0021f430": lambda harness, arguments: (0, tuple(arguments))},
            )

    def looping_patch(data: bytearray, at: int) -> None:
        data[at + 0xF] = random_.choice((0, 1, 1, 0x11, 0xF1, 2))
        data[at + 0x18 : at + 0x24] = reals(random_, 3)
        struct.pack_into("<I", data, at + 0x54, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(CONTROLLERS))))

    def all_looping() -> Buffer:
        return patched(data_array(random_, 12, 0xD4, some_in_use(12)), 0xD4, 12, looping_patch)

    def placed(harness, arguments):
        where, point = arguments
        index = calls_so_far(harness, "scenario_location_from_point")
        harness.mu.mem_write(where, struct.pack("<2I", 0x1000 + index, 0x2000 + index))
        return 0, (harness.normalize(where), bytes(harness.mu.mem_read(point, 12)))

    for _ in range(80):
        t.check(
            "looping_sounds_initialize_for_new_structure_bsp",
            globals={"looping_sounds": all_looping()},
            externals={"scenario_location_from_point": placed},
        )
        t.check(
            "looping_sounds_delete_all",
            globals={"looping_sounds": random_.choice((0, all_looping(), all_looping()))},
            externals={"looping_sound_delete": lambda harness, arguments: (0, tuple(arguments))},
        )

    def effect_patch(data: bytearray, at: int) -> None:
        struct.pack_into("<I", data, at + 8, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(CONTROLLERS))))

    for _ in range(80):
        t.check(
            "sound_effects_delete_all",
            globals={
                "sound_effects": patched(data_array(random_, EFFECTS, 0x14, some_in_use(EFFECTS)), 0x14, EFFECTS, effect_patch),
                "unknown_0051ebd8": all_controllers(),
            },
            externals={"datum_delete": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1]))},
        )

    # ---------- the ring of a track of a looping sound, the sound effects each time, and two of the console's sound

    random_ = random.Random(169)

    def ring_world() -> tuple:
        """The looping sounds (6, with the first sound of each of 3 tracks)
        and the sounds that play, in rings by what each says is next."""
        looping = bytearray(random_.randbytes(0x4C + 6 * 0xD4))
        struct.pack_into("<I", looping, 0x44, 0x4C)
        playing_ = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        struct.pack_into("<I", playing_, 0x44, 0x4C)
        indices = [random_.getrandbits(16) << 16 | n for n in range(PLAYING)]
        order = random_.sample(range(PLAYING), PLAYING)
        rings = [order[0:5], order[5:6], order[6:9], order[9:12]]
        for ring in rings:
            for position, n in enumerate(ring):
                at = 0x4C + n * DATUM
                following = indices[ring[(position + 1) % len(ring)]]
                if random_.random() < 0.05:
                    following = NONE
                struct.pack_into("<I", playing_, at + 0x6C, following)
                struct.pack_into("<I", playing_, at + 0xC, random_.choice((1, 2, 0xE0010003)))
                playing_[at + 0x9C] = random_.choice((0, 1, 0xFF))
        for n in range(6):
            for track in range(3):
                first = random_.choice((NONE, indices[random_.choice(random_.choice(rings))]))
                struct.pack_into("<I", looping, 0x4C + n * 0xD4 + 0x88 + track * 0x14, first)
        return Buffer(bytes(looping), pointers=(0x44,)), Buffer(bytes(playing_), pointers=(0x44,)), indices

    for _ in range(80):
        looping, playing_, indices = ring_world()
        for _case in range(8):
            globals_ = {
                "looping_sounds": Buffer(looping.data, pointers=looping.pointers),
                "sound_sources": Buffer(playing_.data, pointers=playing_.pointers),
            }
            t.check(
                "looping_sound_find_source",
                random_.getrandbits(16) << 16 | random_.randrange(6),
                random_.randrange(3),
                random_.choice((1, 2, 0xE0010003, 7)),
                random_.choice((NONE, NONE, 0, 1)),
                globals=globals_,
            )
            t.check(
                "looping_sound_add_source",
                random_.getrandbits(16) << 16 | random_.randrange(6),
                random_.randrange(3),
                random_.choice(indices),
                globals=globals_,
            )

    def update_patch(data: bytearray, at: int) -> None:
        data[at + 4] = random_.choice((0, 0, 1, 1, 0xFE, 0xFF))
        struct.pack_into("<I", data, at + 8, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(CONTROLLERS))))

    for _ in range(100):
        t.check(
            "sound_effects_update",
            globals={
                "sound_effects": patched(data_array(random_, EFFECTS, 0x14, some_in_use(EFFECTS)), 0x14, EFFECTS, update_patch),
                "unknown_0051ebd8": all_controllers(),
            },
            externals={
                "datum_delete": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                "sound_effect_unknown_0021d5a0": lambda harness, arguments: (
                    (0, 1, 0x100, 0)[arguments[0] & 3],
                    tuple(arguments),
                ),
                "sound_effect_start_sounds": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    def console_voices(count: int) -> bytearray:
        data = bytearray(random_.randbytes(0x2AC0))
        struct.pack_into("<h", data, 4, count)
        for n in range(8):
            at = 0xC + n * 0x34
            data[at] = random_.choice((0xFF, 0, 5, 31, 32, 63))
            data[at + 1] = random_.getrandbits(5) << 3 | random_.choice((0, 0, 1, 2, 3))
            struct.pack_into("<I", data, at + 0x28, random_.choice((NONE, 1, 2, 0x30001)))
        return data

    for _ in range(150):
        console_ = console_voices(random_.choice((0, 1, 4, 8, -1)))
        struct.pack_into("<h", console_, 8, random_.choice((0, 1, 32, 33, 64)))
        t.check(
            "sound_console_get_state",
            Buffer(random_.randbytes(GLOBALS_SIZE)),
            globals={"unknown_0051ebe4": Buffer(bytes(console_))},
        )
        t.check(
            "sound_console_unknown_0021f430",
            random_.choice((NONE, 1, 2, 0x30001, 5)),
            globals={"unknown_0051ebe4": Buffer(bytes(console_))},
            externals={
                "unknown_0037b822": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037b657": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- a ring fades out, what a looping sound says, and looking for a looping sound

    random_ = random.Random(170)

    for _ in range(100):
        looping, playing_, indices = ring_world()
        data = bytearray(playing_.data)
        for n in range(PLAYING):
            at = 0x4C + n * DATUM
            data[at + 2] = random_.choice((2, 2, 2, 0, 1, 3))
            data[at + 4] = random_.choice((0, 0x80, 0x7F, 0xFF))
        for first in (NONE, *random_.sample(indices, 4)):
            t.check(
                "sound_source_ring_fade_out",
                first,
                random_.choice((0, 1, 2, 0xFFFF)),
                as_bits(random_.choice((0.0, 0.5, 1.0, 2.5))),
                globals={"sound_sources": Buffer(bytes(data), pointers=playing_.pointers)},
                externals={"sound_source_fade_decibels": lambda harness, arguments: (fade_bits(arguments[0]), tuple(arguments))},
            )

    def said_patch(data: bytearray, at: int) -> None:
        number = random_.choice((1, 2, 0x10001, random_.getrandbits(32)))
        struct.pack_into("<2I", data, at + 4, number, random_.choice((number, number | 0x6000, number | 0x6000, 5)))
        data[at + 0xF] = random_.getrandbits(8)
        struct.pack_into("<I", data, at + 0x54, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(CONTROLLERS))))

    for _ in range(100):
        in_use = some_in_use(12)
        looping_ = patched(data_array(random_, 12, 0xD4, in_use), 0xD4, 12, said_patch)
        globals_ = {
            "looping_sounds": looping_,
            "unknown_0051ebd8": all_controllers(),
            "unknown_0051ebdc": Buffer(random_.randbytes(0x1C)),
        }
        candidates = [NONE, 12, random_.randrange(12)] + [(in_use[n] & 0xFFFF) << 16 | n for n in in_use]
        for looping_sound_index in random_.sample(candidates, min(len(candidates), 6)):
            t.check(
                "looping_sound_get_source",
                looping_sound_index,
                random_.getrandbits(32),
                random_.getrandbits(32),
                Buffer(random_.randbytes(0x48)),
                globals=globals_,
            )
        for _pair in range(4):
            t.check(
                "looping_sound_find",
                random_.choice((1, 2, 0x10001)),
                random_.choice((1, 2, 0x10001, 0x6001, 0x6002, 0x16001, 5)),
                globals={"looping_sounds": looping_},
            )

    # ---------- the sound voices when the structure bsp changes, a voice of the console starts, and two clusters

    random_ = random.Random(171)

    for _ in range(80):
        # the game has the code of scenario_location_from_point in the function
        structure_bsp = bytearray(random_.randbytes(0x140))
        struct.pack_into("<2I", structure_bsp, 0x2C, LEAVES, len(structure_bsp))
        structure_bsp += random_.randbytes(LEAVES * 8)

        def leaf_of(harness, arguments):
            bsp, node, point = arguments
            leaf = (NONE, 0, 3, LEAVES - 1)[calls_so_far(harness, "bsp3d_test_point") % 4]
            return leaf, (bsp, node, bytes(harness.mu.mem_read(point, 12)))

        t.check(
            "sound_voices_initialize_for_new_structure_bsp",
            globals={
                "sound_voices": data_array(random_, 8, 0x6C, some_in_use(8)),
                "global_structure_bsp_index": struct.pack("<h", random_.choice((0, 1, 5, -1))),
                "global_structure_bsp": Buffer(bytes(structure_bsp), pointers=(0x30,)),
                "global_bsp3d": random_.getrandbits(32),
            },
            externals={"bsp3d_test_point": leaf_of},
        )

    def console_call(*pointers: int):
        """A stand-in of a function of the console: of its arguments, those
        at some places are addresses."""

        def handler(harness, arguments):
            return 0, tuple(harness.normalize(value) if n in pointers else value for n, value in enumerate(arguments))

        return handler

    for _ in range(200):
        console_ = console_voices(8)
        something = bytearray(random_.randbytes(0x30))
        struct.pack_into("<h", something, 0x24, random_.choice((-1, -1, 0, 3, 0x105)))
        struct.pack_into("<I", something, 0x2C, random_.choice((NONE, 0, random_.getrandbits(32))))
        t.check(
            "sound_console_voice_start",
            Buffer(bytes(something)),
            random_.choice((0, Buffer(random_.randbytes(0x20)))),
            random_.randrange(8),
            globals={"unknown_0051ebe4": Buffer(bytes(console_))},
            externals={
                "unknown_0037b822": console_call(),
                "unknown_002ae7a0": console_call(0),
                "unknown_0037c705": console_call(),
                "sound_console_voice_apply_effects": console_call(1),
                "sound_console_voice_set_parameters": console_call(0),
            },
        )

    for _ in range(600):
        said = bytearray(random_.randbytes(0x48))
        said[0] = random_.choice((0, 0, 0, 1, 0xFE, 0xFF))
        said[3] = random_.choice((1, 1, 1, 0x11, 0xF1, 0, 2))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        for index in range(4):
            listener = LISTENERS + index * LISTENER_SIZE
            struct.pack_into("<ih", state, listener, random_.choice((5, 6, 7)), random_.choice((-1, 1, 2, 3)))
        struct.pack_into("<ih", said, 0x28, random_.choice((5, 6, 7, 8)), random_.choice((-1, 1, 2, 3, 3)))
        measure = random_.choice((0.0, 1.0, 100.0, 255.9, 256.0, 300.0, float("nan")))

        def between(harness, arguments, measure=measure):
            return real_bits(measure), (arguments[0], arguments[1], harness.normalize(arguments[2]))

        t.check(
            "sound_source_unknown_00221da0",
            random_.choice((NONE, 0, 1, 2, 3, 0, 1, 2, 3)),
            Buffer(bytes(said)),
            as_bits(random_.choice((1.0, 1.0, 0.5, 2.0, 0.0))),
            globals={"game_sound_globals": Buffer(bytes(state)), "global_structure_bsp": Buffer(random_.randbytes(0x40))},
            externals={"unknown_00249d60": between},
        )

    # ---------- what a tag says of a sound effect, a new one, and what the game says of its sound

    random_ = random.Random(172)

    def collection(count: int, elements: bool, unknown8: int) -> Buffer:
        data = bytearray(random_.randbytes(0x34)) + random_.randbytes(8) + struct.pack("<i", unknown8) + random_.randbytes(8)
        struct.pack_into("<iI", data, 0x2C, count, 0x34 if elements else 0)
        return Buffer(bytes(data), pointers=(0x30,) if elements else ())

    for _ in range(400):
        first = random_.choice((None, collection(random_.choice((0, 1, 3, -1)), random_.random() < 0.8, random_.choice((0, 1, 5, -1)))))
        second = random_.choice((None, collection(random_.choice((0, 1)), True, 1)))
        number = random_.choice((NONE, 0, 7))
        if first is None and number == NONE and second is not None:
            # the game reads from nothing here
            number = 7
        t.check(
            "sound_effect_definition_get",
            random_.choice((NONE, random_.getrandbits(32))),
            number,
            Buffer(random_.randbytes(4)),
            extra=[buffer for buffer in (first, second) if buffer],
            externals={
                "unknown_0018d090": lambda harness, arguments, first=first: (first.address if first else 0, tuple(arguments)),
                "unknown_0018d170": lambda harness, arguments, second=second: (second.address if second else 0, tuple(arguments)),
            },
        )

    for _ in range(200):
        said_ = Buffer(random_.randbytes(8) + struct.pack("<i", random_.choice((0, 1, 1, 9))) + random_.randbytes(8))
        found = random_.choice((1, 1, 1, 0))
        made = random_.choice((NONE, *(random_.getrandbits(16) << 16 | index for index in range(EFFECTS))))

        def get(harness, arguments, said_=said_, found=found):
            tag_index, unknown, definition = arguments
            harness.mu.mem_write(definition, struct.pack("<I", said_.address))
            return found, (tag_index, unknown)

        t.check(
            "sound_effect_new",
            random_.getrandbits(32),
            as_bits(random_.uniform(-10, 10)),
            random_.getrandbits(16),
            globals={"sound_effects": effects_()},
            extra=[said_],
            externals={
                "sound_effect_definition_get": get,
                "datum_new": lambda harness, arguments, made=made: (made, (harness.normalize(arguments[0]),)),
                "sound_effect_start_sounds": effect_update,
            },
        )

    for _ in range(60):
        buffer, indices = playing()
        sound_index = random_.randrange(TAGS)
        data = bytearray(buffer.data)
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            struct.pack_into("<I", data, at + 0xC, random_.getrandbits(16) << 16 | sound_index)
            struct.pack_into("<H", data, at + 0x18, random_.choice((0, 0x100, 0x104, 0x108, 0x10C, 0xFFFF, random_.getrandbits(16))))
            data[at + 0x18 + 0x34 : at + 0x18 + 0x3C] = reals(random_, 2)
        globals_ = world(random_, sound_index, sound_data(random_))
        globals_["sound_sources"] = Buffer(bytes(data), pointers=buffer.pointers)
        for _effect in range(6):
            effects = bytearray(effects_().data)
            globals_["sound_effects"] = Buffer(bytes(effects), pointers=(0x44,))
            t.check(
                "sound_effect_get_source",
                random_.getrandbits(16) << 16 | random_.randrange(EFFECTS),
                Buffer(random_.randbytes(0x48)),
                globals=globals_,
                externals={"unknown_0018cbc0": lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1])))},
            )

    # ---------- the cache of the sounds is let go, a recording is chosen, and the sound of a track

    random_ = random.Random(173)

    for _ in range(40):
        counted_ = bytearray(counted().data)
        allocated = random_.random() < 0.7
        struct.pack_into("<I", counted_, 0x30, len(counted_) if allocated else 0)
        pointers = [0x44] + ([0x30, len(counted_)] if allocated else [])
        counted_ += struct.pack("<3I", len(counted_) + 4, mock_address(0), mock_address(1))
        cache = bytearray(random_.randbytes(0x70))
        struct.pack_into("<I", cache, 0x6C, 0x70)
        cache += struct.pack("<3I", 0x74, mock_address(0), mock_address(1))
        t.check(
            "sound_cache_dispose",
            globals={
                "unknown_00502104": Buffer(bytes(counted_), pointers=tuple(pointers)),
                "sound_lruv_cache": Buffer(bytes(cache), pointers=(0x6C, 0x70)),
                "sound_cache_base": random_.getrandbits(32),
            },
            mocks=[
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
            ],
        )

    for _ in range(300):
        definition = bytearray(sound_data(random_))
        struct.pack_into("<h", definition, 8, random_.randrange(2))
        globals_ = world(random_, 0, sound_data(random_))
        gestalt, _permutations = recordings()
        shared = bytearray(gestalt)
        struct.pack_into("<I", shared, 0x3C, len(shared))
        shared += random_.randbytes(0x20)
        for n in range(4):
            at = 0x48 + n * 0xC
            shared[at + 4] = random_.choice((0, 1, 5, 0xFF))
            shared[at + 5] = random_.choice((0xFF, 0xFF, 0, 2, 7))
            struct.pack_into("<H", shared, at + 6, random_.randrange(0, 0x80))
            struct.pack_into("<h", shared, at + 0xA, random_.choice((0, 1, 2, 5, 9, 17)))
        globals_["global_sound_cache_file_gestalt"] = Buffer(bytes(shared), pointers=(0x24, 0x2C, 0x3C, 0x44))
        globals_["global_random_seed_reference"] = Buffer(random_.randbytes(0x10))
        chosen = random_.choice((0, 1, 4, 0x7F, 0xFFFF0003))
        mark = random_.getrandbits(32)

        def choose(harness, arguments, chosen=chosen, mark=mark):
            sound_, offset, played, last, seed, unknown, unknown2 = arguments
            record = (
                harness.normalize(sound_),
                offset & 0xFFFF,
                bytes(harness.mu.mem_read(played, 4)),
                last,
                harness.normalize(seed),
                unknown,
                unknown2,
            )
            harness.mu.mem_write(played, struct.pack("<I", mark))
            return chosen, record

        t.check(
            "sound_choose_permutation",
            Buffer(bytes(definition)),
            random_.randrange(3),
            globals=globals_,
            externals={"sound_pitch_range_choose_permutation": choose},
        )

    for _ in range(40):
        looping, playing_, indices = ring_world()
        for _case in range(8):
            made = random_.choice((NONE, random_.getrandbits(32)))
            t.check(
                "looping_sound_get_or_new_source",
                random_.choice((1, 2, 0xE0010003, 7)),
                random_.randrange(3),
                random_.choice((NONE, NONE, 0, 1)),
                random_.getrandbits(16) << 16 | random_.randrange(6),
                random_.getrandbits(32),
                random_.getrandbits(32),
                globals={"looping_sounds": looping, "sound_sources": playing_},
                externals={
                    "looping_sound_source_new": lambda harness, arguments, made=made: (
                        made,
                        (*arguments[:3], arguments[3] & 0xFFFF, *arguments[4:]),
                    )
                },
            )

    # ---------- the samples of a piece start to be read, and a byte of what a sound has more

    random_ = random.Random(174)

    for _ in range(200):
        cache = bytearray(random_.randbytes(0x70)) + bytearray(random_.randbytes(0x4C + COUNTED * 0x18))
        struct.pack_into("<i", cache, 0x34, random_.choice((0, 1, 9, 12)))
        struct.pack_into("<I", cache, 0x64, 0x70)
        struct.pack_into("<I", cache, 0x70 + 0x44, 0x70 + 0x4C)
        for index in range(COUNTED):
            struct.pack_into("<i", cache, 0x70 + 0x4C + index * 0x18 + 8, random_.randrange(0, 0x2000))
        piece = bytearray(random_.randbytes(0xC))
        struct.pack_into("<I", piece, 4, random_.getrandbits(2) << 30 | random_.choice((0, 1, 0x1FF, 0x200, 0x201, 0x12345, 0x3FFFFE00)))
        block = random_.choice((NONE, *(random_.getrandbits(16) << 16 | index for index in range(COUNTED))))
        t.check(
            "sound_cache_chunk_load",
            Buffer(bytes(piece)),
            random_.getrandbits(32),
            globals={
                "unknown_00502104": counted(),
                "sound_lruv_cache": Buffer(bytes(cache), pointers=(0x64, 0x70 + 0x44)),
                "sound_cache_base": random_.choice((0, 0x80000000, random_.getrandbits(32))),
            },
            externals={
                "lruv_block_new": lambda harness, arguments, block=block: (
                    block,
                    (harness.normalize(arguments[0]), arguments[1], arguments[2]),
                ),
                "datum_new_at_index": lambda harness, arguments: (arguments[1], (harness.normalize(arguments[0]), arguments[1])),
                "cache_file_read_async": lambda harness, arguments: (
                    0,
                    (arguments[0], arguments[1], arguments[2], arguments[3], harness.normalize(arguments[4]), *arguments[5:]),
                ),
            },
        )

    for _ in range(60):
        # what the sounds share: 4 pitch ranges, 12 recordings, and 3 of what some sounds have more
        data = bytearray(random_.randbytes(0x58))
        pointers = []

        def put(offset: int, content: bytes) -> int:
            struct.pack_into("<I", data, offset, len(data))
            pointers.append(offset)
            at = len(data)
            data.extend(content)
            return at

        ranges = bytearray(random_.randbytes(4 * 0xC))
        for n in range(4):
            struct.pack_into("<hh", ranges, n * 0xC + 8, random_.randrange(0, 6), random_.choice((0, 1, 3, 6)))
        put(0x24, bytes(ranges))
        recordings_ = bytearray(random_.randbytes(12 * 0x10))
        for n in range(12):
            recordings_[n * 0x10 + 5] = random_.randrange(4)
        put(0x2C, bytes(recordings_))
        extras = put(0x54, random_.randbytes(3 * 0x2C))
        for n in range(3):
            more = put(extras + n * 0x2C + 4, random_.randbytes(0x10))
            put(more + 4, random_.randbytes(0x100))
            entries = bytearray()
            for _entry in range(4):
                entries += struct.pack("<2i", random_.randrange(0, 0x80), random_.choice((0, 1, 5, 0x40, -1))) + random_.randbytes(8)
            put(more + 0xC, bytes(entries))
        shared = Buffer(bytes(data), pointers=tuple(pointers))
        for _case in range(12):
            sound_index = random_.randrange(TAGS)
            tags = bytearray(random_.randbytes(TAGS * 0x10))
            struct.pack_into("<I", tags, sound_index * 0x10 + 8, len(tags))
            definition = bytearray(random_.randbytes(0x10))
            struct.pack_into("<h", definition, 8, random_.randrange(2))
            struct.pack_into("<h", definition, 0xE, random_.randrange(3))
            t.check(
                "sound_unknown_00218e50",
                random_.getrandbits(16) << 16 | sound_index,
                random_.choice((0, 1, 2, 5, 6, 0xFFFF)),
                random_.randrange(3),
                random_.choice((0, 1, 4, 0x3F, 0x40, 0x7FFF, 0xFFFF)),
                globals={
                    "global_sound_cache_file_gestalt": shared,
                    "global_tag_instances": Buffer(bytes(tags) + bytes(definition), pointers=(sound_index * 0x10 + 8,)),
                },
            )

    # ---------- the bits of the recordings that were played of a track of a looping sound

    random_ = random.Random(175)

    for _ in range(300):
        definition = bytearray(sound_data(random_))
        struct.pack_into("<h", definition, 8, random_.randrange(2))
        globals_ = world(random_, 0, sound_data(random_))
        gestalt, _permutations = recordings()
        shared = bytearray(gestalt)
        struct.pack_into("<I", shared, 0x3C, len(shared))
        shared += random_.randbytes(0x20)
        for n in range(4):
            at = 0x48 + n * 0xC
            struct.pack_into("<H", shared, at + 6, random_.randrange(0, 0x80))
            struct.pack_into("<h", shared, at + 0xA, random_.choice((0, 1, 2, 5, 9, 17)))
        globals_["global_sound_cache_file_gestalt"] = Buffer(bytes(shared), pointers=(0x24, 0x2C, 0x3C, 0x44))
        looping_sound = bytearray(random_.randbytes(0xD4))
        struct.pack_into("<I", looping_sound, 0x54, random_.choice((NONE, 0, random_.getrandbits(32))))
        flags_ = bytes((random_.choice((0, 0x10, 0x20, 0x30, 0xCF, 0xFF)),)) + random_.randbytes(3)
        t.check(
            "looping_sound_track_get_played_bits",
            random_.randrange(3),
            Buffer(bytes(definition)),
            Buffer(bytes(looping_sound)),
            Buffer(flags_),
            random_.randrange(3),
            random_.choice((0, 1, 0x80)),
            globals=globals_,
        )
        t.check(
            "looping_sound_track_set_played_bits",
            random_.randrange(3),
            random_.getrandbits(32),
            Buffer(bytes(looping_sound)),
            Buffer(bytes(definition)),
            Buffer(flags_),
            random_.randrange(3),
            random_.getrandbits(8),
            globals=globals_,
        )

    # ---------- whether a sound can share a sound voice

    random_ = random.Random(176)

    VOICE = 0
    SOUND = 0x6C
    TABLE = 0x6C + 0xBC
    for _ in range(1500):
        data = bytearray(random_.randbytes(TABLE + 0x1C))
        pointers = []
        told_ = random_.choice((0, 0, 1, 1, 2))
        struct.pack_into("<I", data, TABLE + 0x18, random_.choice((mock_address(0), mock_address(0), mock_address(0), 0)))
        if told_:
            struct.pack_into("<I", data, VOICE + 0x10, TABLE)
            pointers.append(VOICE + 0x10)
        else:
            struct.pack_into("<I", data, VOICE + 0x10, 0)
        if told_ == 1:
            struct.pack_into("<I", data, SOUND + 0x14, TABLE)
            pointers.append(SOUND + 0x14)
        else:
            struct.pack_into("<I", data, SOUND + 0x14, 0)
        struct.pack_into("<I", data, SOUND + 0xC, random_.getrandbits(16) << 16 | random_.randrange(TAGS))
        data[SOUND + 0x24 : SOUND + 0x40] = reals(random_, 7)
        # the voice is as the sound, but for something now and then
        data[VOICE + 8] = data[SOUND + 0x18] & 1
        data[VOICE + 0xC : VOICE + 0x10] = data[SOUND + 0x10 : SOUND + 0x14]
        data[VOICE + 0x48 : VOICE + 0x64] = data[SOUND + 0x24 : SOUND + 0x40]
        data[VOICE + 0x64 : VOICE + 0x6A] = data[SOUND + 0x40 : SOUND + 0x46]
        change = random_.choice((None, None, None, 8, 0xC, 0x64, 0x68, "real", "real", "near"))
        if isinstance(change, int):
            data[VOICE + change] ^= 1
        elif change:
            offset = VOICE + 0x48 + random_.choice((0, 4, 8, 0x10, 0x14, 0x18))
            value = struct.unpack_from("<f", data, offset)[0]
            value += random_.choice((0.00005, -0.00005, 0.00009)) if change == "near" else random_.choice((0.0002, -0.0002, 1.0))
            struct.pack_into("<f", data, offset, value)
        buffer = Buffer(bytes(data), pointers=tuple(pointers))
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        tag_pointers = []
        for n in range(TAGS):
            struct.pack_into("<I", tags, n * 0x10 + 8, TAGS * 0x10 + n * 0x10)
            tag_pointers.append(n * 0x10 + 8)
        tags += random_.randbytes(TAGS * 0x10)
        # sound_classes_match gives 0 or 1, and nothing more
        same = random_.choice((1, 1, 1, 0))
        answer = random_.choice((1, 1, 1, 0, 0x100))
        t.check(
            "sound_voice_matches_source",
            Pointer(buffer, VOICE),
            Pointer(buffer, SOUND),
            globals={
                "global_tag_instances": Buffer(bytes(tags), pointers=tuple(tag_pointers)),
                "unknown_0055e780": bytes((random_.choice((0, 0, 0, 0, 1, 0x80)),)),
            },
            extra=[buffer],
            externals={"sound_classes_match": lambda harness, arguments, same=same: (same, tuple(value & 0xFFFF for value in arguments))},
            mocks=[Mock(lambda call, answer=answer: answer, stack_arguments=2)],
        )

    # ---------- the pitch range of a sound for a pitch

    random_ = random.Random(177)

    for _ in range(150):
        # what the sounds share: 6 parameters of pitch ranges and 8 pitch ranges
        data = bytearray(random_.randbytes(0x48))
        parameters = bytearray()
        for _n in range(6):
            low = random_.choice((-1200, -600, 0, 100))
            high = low + random_.choice((0, 100, 600, 1200))
            parameters += struct.pack("<5h", random_.randint(-50, 50), low - 50, high + 50, low, high)
        struct.pack_into("<I", data, 0x1C, len(data))
        data += parameters
        struct.pack_into("<I", data, 0x24, len(data))
        for _n in range(8):
            pitch_range = bytearray(random_.randbytes(0xC))
            struct.pack_into("<h", pitch_range, 2, random_.randrange(6))
            struct.pack_into("<h", pitch_range, 0xA, random_.choice((0, 1, 1, 4, -1)))
            data += pitch_range
        shared = Buffer(bytes(data), pointers=(0x1C, 0x24))
        for _case in range(10):
            definition = bytearray(random_.randbytes(0x10))
            struct.pack_into("<h", definition, 8, random_.randrange(4))
            definition[0xA] = random_.choice((0, 1, 2, 4, 4, 0xFF))
            t.check(
                "sound_choose_pitch_range",
                random_.choice((0xFFFF, 0, 1, 2, 3, 4, 7)),
                Buffer(bytes(definition)),
                as_bits(random_.choice((0.0, -1200.0, 100.0, 650.0, 2000.0, -3000.0, float("nan"), random_.uniform(-1500, 1500)))),
                globals={"global_sound_cache_file_gestalt": shared},
            )

    # ---------- a recording of a pitch range, at random

    random_ = random.Random(178)

    for _ in range(600):
        gestalt, _permutations = recordings()
        shared = bytearray(gestalt)
        count = random_.choice((1, 1, 2, 3, 5, 6))
        for n in range(4):
            struct.pack_into("<hh", shared, 0x48 + n * 0xC + 8, random_.randrange(0, 6), count)
        for n in range(12):
            struct.pack_into("<H", shared, 0x48 + 4 * 0xC + n * 0x10 + 2, random_.choice((0, 0, 0x4000, 0x8000, 0xC000, 0xF000)))
        definition = bytearray(random_.randbytes(0x10))
        struct.pack_into("<h", definition, 8, random_.randrange(2))
        t.check(
            "sound_pitch_range_choose_permutation",
            Buffer(bytes(definition)),
            random_.randrange(3),
            Buffer(struct.pack("<I", random_.choice((0, (1 << count) - 1, random_.getrandbits(count), random_.getrandbits(count))))),
            random_.choice((NONE, 0xFFFF, 0, count - 1, random_.randrange(count))),
            Buffer(struct.pack("<I", random_.getrandbits(32))),
            random_.choice((0, Buffer(random_.randbytes(4)))),
            random_.choice((0, 0, 1)),
            globals={"global_sound_cache_file_gestalt": Buffer(bytes(shared), pointers=(0x24, 0x2C, 0x44))},
        )

    # ---------- the listener that hears a sound, and something that comes up from silence

    random_ = random.Random(179)

    for _ in range(600):
        said = bytearray(random_.randbytes(0x48))
        struct.pack_into("<H", said, 0, random_.choice((0, 0, 0x100, 0x800, 0x900, 0xF6FF, random_.getrandbits(16))))
        said[3] = random_.choice((1, 1, 1, 0x11, 0xF1, 0, 2))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        for index in range(4):
            state[LISTENERS + index * LISTENER_SIZE + 6] = random_.choice((0, 1, 1, 0x80))
        players = bytearray(random_.randbytes(0xC)) + struct.pack("<4i", *(random_.choice((-1, -1, 0, 7)) for _ in range(4)))
        squares = [random_.choice((0.0, 1.0, 4.0, 4.0, 25.0, 100.0, 1e6)) for _ in range(4)]
        refused = random_.choice((0, 0, 0, 1))

        def square(harness, arguments, squares=squares):
            return real_bits(squares[arguments[1] & 3]), (harness.normalize(arguments[0]), arguments[1])

        def refuse_(harness, arguments, refused=refused):
            return refused, (arguments[0], harness.normalize(arguments[1]), arguments[2])

        t.check(
            "sound_source_get_listener",
            Buffer(bytes(said)),
            as_bits(random_.choice((1.0, 2.0, 5.0, 10.0, 0.5, 3.0))),
            random_.choice((0, Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4)))),
            globals={"game_sound_globals": Buffer(bytes(state)), "players_globals": Buffer(bytes(players))},
            externals={"sound_source_unknown_00127e20": square, "sound_source_unknown_00221da0": refuse_},
        )

    for kind in (0, 1, 2, 0xFFFF):
        for _ in range(200):
            t.check(
                "sound_unknown_0012a6d0",
                kind,
                as_bits(random_.choice((0.0, 0.5, 1.0, -0.5, 2.0, random_.uniform(-2, 2), random_.uniform(0, 1)))),
                as_bits(random_.choice((1.0, -1.0, 0.5, 2.0, -0.3, 0.0, random_.uniform(0.1, 3)))),
            )

    # ---------- which channel a sound takes, and which sound goes before

    random_ = random.Random(180)

    def class_world() -> dict:
        """The tags, with a sound at each but the last, which is the tag of
        the classes of sounds, and the globals tag that tells of it."""
        classes_index = TAGS - 1
        data = bytearray(random_.randbytes(TAGS * 0x10))
        pointers = []

        def add(part: bytes) -> int:
            offset = len(data)
            data.extend(part)
            return offset

        def point(offset: int, to: int) -> None:
            struct.pack_into("<I", data, offset, to)
            pointers.append(offset)

        definitions = bytearray(random_.randbytes(TAGS * 0x10))
        for n in range(TAGS):
            definitions[n * 0x10 + 2] = random_.randrange(CLASSES)
        definitions_at = add(bytes(definitions))
        for n in range(TAGS - 1):
            point(n * 0x10 + 8, definitions_at + n * 0x10)
        table = bytearray(random_.randbytes(CLASSES * CLASS_SIZE))
        for n in range(CLASSES):
            struct.pack_into("<i", table, n * CLASS_SIZE + 4, random_.choice((0, 10, 100, 1000)))
            struct.pack_into("<h", table, n * CLASS_SIZE + 0xC, random_.choice((0, 1, 1, 2, -1)))
        classes = add(struct.pack("<2I", CLASSES, 0))
        point(classes_index * 0x10 + 8, classes)
        point(classes + 4, add(bytes(table)))
        reference = add(struct.pack("<2I", random_.getrandbits(32), random_.getrandbits(16) << 16 | classes_index))
        game_globals = add(random_.randbytes(0xC0) + struct.pack("<2I", 1, 0))
        point(game_globals + 0xC4, reference)
        buffer = Buffer(bytes(data), pointers=tuple(pointers))
        return {"global_tag_instances": buffer, "global_game_globals": Pointer(buffer, game_globals)}

    def classed_sources(now: int) -> tuple:
        buffer, indices = playing()
        data = bytearray(buffer.data)
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            struct.pack_into("<I", data, at + 0xC, random_.getrandbits(16) << 16 | random_.choice((3, 3, 4, random_.randrange(TAGS - 1))))
            struct.pack_into("<i", data, at + 0x8C, now - random_.choice((0, 5, 50, 500, 5000, -20)))
            data[at + 0xA0] = random_.choice((0, 1, 1, 2, 0xFF))
            data[at + 0xA1] = random_.randrange(4)
        return Buffer(bytes(data), pointers=buffer.pointers), indices

    for _ in range(150):
        now = random_.choice((10000, 0x40000000))
        playing_, indices = classed_sources(now)
        channels = bytearray(random_.randbytes(CHANNELS * 0x24))
        for n in range(CHANNELS):
            struct.pack_into("<I", channels, n * 0x24, random_.choice(indices))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        struct.pack_into("<i", state, 0x84, now)
        count = random_.choice((0, 1, 3, CHANNELS, 0xFFFF))
        order = struct.pack(f"<{CHANNELS}h", *random_.sample(range(CHANNELS), CHANNELS))
        globals_ = class_world()
        globals_.update(
            {"sound_sources": playing_, "sound_channels": Buffer(bytes(channels)), "game_sound_globals": Buffer(bytes(state))}
        )
        if count == 0xFFFF:
            # a count under 0 is of no channel
            order = random_.randbytes(4)
        t.check("sound_source_choose_channel", random_.choice(indices), count, Buffer(order), globals=globals_)

    for _ in range(600):
        playing_, indices = classed_sources(1000)
        measure = random_.choice((0.0, 1.0, 4.0, 25.0, 100.0))

        def far(harness, arguments, measure=measure):
            return real_bits(measure), (harness.normalize(arguments[0]), arguments[1])

        globals_ = class_world()
        globals_["sound_sources"] = playing_
        t.check(
            "sound_source_goes_before",
            random_.choice(indices),
            random_.choice(indices),
            as_bits(random_.choice((0.0, 1.0, 4.0, 25.0, 50.0))),
            globals=globals_,
            externals={"sound_source_unknown_00127e20": far},
        )

    # ---------- a channel for a sound that is to play

    random_ = random.Random(181)

    for _ in range(400):
        playing_, indices = classed_sources(1000)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        tag_pointers = []
        for n in range(TAGS):
            struct.pack_into("<I", tags, n * 0x10 + 8, TAGS * 0x10 + n * 0x10)
            tag_pointers.append(n * 0x10 + 8)
        definitions = bytearray(random_.randbytes(TAGS * 0x10))
        for n in range(TAGS):
            definitions[n * 0x10 + 4] = random_.choice((0, 1, 1, 2))
        channels = bytearray(random_.randbytes(CHANNELS * 0x24))
        full = random_.random() < 0.6
        for n in range(CHANNELS):
            struct.pack_into("<I", channels, n * 0x24, random_.choice(indices) if full or random_.random() < 0.7 else NONE)
            channels[n * 0x24 + 8] = random_.choice((0, 1, 1, 2))
        before = random_.getrandbits(32)

        def goes_before(harness, arguments, before=before):
            other, mine, distance = arguments
            return (before >> ((other ^ mine) & 31)) & 1, (other, mine, distance)

        def measure_(harness, arguments):
            return real_bits((arguments[1] & 3) * 2.5 + 1.0), (harness.normalize(arguments[0]), arguments[1])

        t.check(
            "sound_source_find_channel",
            random_.choice(indices),
            Buffer(random_.randbytes(4)),
            globals={
                "sound_sources": playing_,
                "sound_channels": Buffer(bytes(channels)),
                "game_sound_globals": sound_state(channels=random_.choice((0, 1, 5, CHANNELS, CHANNELS, -1))),
                "global_tag_instances": Buffer(bytes(tags) + bytes(definitions), pointers=tuple(tag_pointers)),
            },
            externals={"sound_source_goes_before": goes_before, "sound_source_unknown_00127e20": measure_},
        )

    # ---------- the channels that play the sound of a sound that is to play

    random_ = random.Random(182)

    for _ in range(400):
        globals_ = class_world()
        definitions = globals_["global_tag_instances"].data[TAGS * 0x10 :]
        data = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        pointers = [0x44]
        struct.pack_into("<I", data, 0x44, 0x4C)
        tables = len(data)
        for _table in range(2):
            data += random_.randbytes(0x1C) + struct.pack("<I", random_.choice((mock_address(0), mock_address(0), 0)))
        now = 5000
        indices = []
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            struct.pack_into("<I", data, at + 0xC, random_.choice((0xE0010003, 0xE0010003, 0xE0020004)))
            struct.pack_into("<I", data, at + 0x10, random_.choice((NONE, 1, 1, 1, 2)))
            struct.pack_into("<I", data, at + 0x14, tables + random_.choice((0, 0, 0, 0x20)))
            pointers.append(at + 0x14)
            struct.pack_into("<i", data, at + 0x8C, random_.choice((now, now - 1, now - 100)))
            indices.append(random_.getrandbits(16) << 16 | index)
        channels = bytearray(random_.randbytes(CHANNELS * 0x24))
        for n in range(CHANNELS):
            struct.pack_into("<I", channels, n * 0x24, random_.choice((NONE, *indices)))
            # the type of one of the two sounds, most times
            channels[n * 0x24 + 8] = random_.choice((definitions[3 * 0x10 + 4], definitions[4 * 0x10 + 4], random_.getrandbits(8)))
        state = bytearray(sound_state(channels=random_.choice((0, 1, 5, CHANNELS, CHANNELS, -1))).data)
        struct.pack_into("<i", state, 0x84, now)
        globals_.update(
            {
                "sound_sources": Buffer(bytes(data), pointers=tuple(pointers)),
                "sound_channels": Buffer(bytes(channels)),
                "game_sound_globals": Buffer(bytes(state)),
            }
        )
        answer = random_.choice((1, 1, 0, 0x100))
        t.check(
            "sound_source_find_same",
            Buffer(random_.randbytes(0x4A)),
            random_.choice(indices),
            globals=globals_,
            mocks=[Mock(lambda call, answer=answer: answer, stack_arguments=4)],
        )

    # ---------- the channel that a sound that is to play gets

    random_ = random.Random(183)

    for _ in range(700):
        data = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        struct.pack_into("<I", data, 0x44, 0x4C)
        indices = []
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            struct.pack_into("<I", data, at + 0xC, random_.getrandbits(16) << 16 | random_.randrange(TAGS))
            struct.pack_into("<I", data, at + 0x10, random_.choice((NONE, 1, 1, 1, 2)))
            struct.pack_into("<H", data, at + 0xAC, random_.choice((0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 3)))
            indices.append(random_.getrandbits(16) << 16 | index)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        tag_pointers = []
        for n in range(TAGS):
            struct.pack_into("<I", tags, n * 0x10 + 8, TAGS * 0x10 + n * 0x10)
            tag_pointers.append(n * 0x10 + 8)
        definitions = bytearray(random_.randbytes(TAGS * 0x10))
        for n in range(TAGS):
            definitions[n * 0x10 + 4] = random_.choice((0, 1, 1, 2))
        table = bytearray(random_.randbytes(0x100 * 0x5C))
        exclusive = random_.random() < 0.5
        for n in range(0x100):
            table[n * 0x5C + 8] = random_.choice((2, 2, 0xFF, 0, 0xFD)) if exclusive else random_.choice((0, 0, 0xFD, 2))
        table_ = Buffer(bytes(table))
        channels = bytearray(random_.randbytes(CHANNELS * 0x24))
        for n in range(CHANNELS):
            struct.pack_into("<I", channels, n * 0x24, random_.choice((NONE, *indices)))
            channels[n * 0x24 + 8] = random_.choice((0, 1, 1, 2, 5))
        counts = random_.choice(((0, 1, 0, 1), (1, 1, 0, 1), (2, 3, 1, 1), (2, 2, 0, 1), (5, 2, 3, 4), (0, 0, 0, 0)))
        started = random_.choice((0, 0, 0, 1))
        found = random_.choice((0xFFFF, 0, 3, 7))
        chosen_ = random_.choice((0xFFFF, 1, 6))

        def same_(harness, arguments, counts=counts, started=started):
            out, source_index = arguments
            content = bytearray(bytes(range(0x4A)))
            struct.pack_into("<h", content, 0, counts[0])
            struct.pack_into("<h", content, 0x22, counts[1])
            content[0x24] = started
            struct.pack_into("<h", content, 0x26, counts[2])
            struct.pack_into("<h", content, 0x48, counts[3])
            harness.mu.mem_write(out, bytes(content))
            return 0, (source_index,)

        def choose_(harness, arguments, chosen_=chosen_):
            source_index, count, channel_indices = arguments
            return chosen_, (source_index, count & 0xFFFF, bytes(harness.mu.mem_read(channel_indices, 32)))

        def find_(harness, arguments, found=found):
            source_index, why = arguments
            record = source_index, bytes(harness.mu.mem_read(why, 4))
            harness.mu.mem_write(why, struct.pack("<I", 9 if found != 0xFFFF else 0))
            return found, record

        t.check(
            "sound_source_get_channel",
            random_.choice(indices),
            Buffer(random_.randbytes(4)),
            globals={
                "sound_sources": Buffer(bytes(data), pointers=(0x44,)),
                "sound_channels": Buffer(bytes(channels)),
                "game_sound_globals": sound_state(channels=random_.choice((0, 1, 5, CHANNELS, CHANNELS, -1))),
                "global_tag_instances": Buffer(bytes(tags) + bytes(definitions), pointers=tuple(tag_pointers)),
            },
            extra=[table_],
            externals={
                "sound_class_get": class_of(table_),
                "sound_source_find_same": same_,
                "sound_source_choose_channel": choose_,
                "sound_source_find_channel": find_,
                "sound_source_delete": lambda harness, arguments: (0, tuple(arguments)),
                "sound_channel_delete": lambda harness, arguments: (0, (arguments[0] & 0xFFFF,)),
            },
        )

    # ---------- whether a sound is to start, and its promotion

    random_ = random.Random(184)

    for _ in range(900):
        sound_index = random_.randrange(TAGS)
        definition = bytearray(sound_data(random_))
        definition[4] = random_.choice((0, 0, 1, 2, 2))
        definition[5] = random_.choice((1, 1, 1, 0, 2))
        struct.pack_into("<h", definition, 6, random_.randrange(0, PLAYBACKS))
        definition[0xB] = random_.randrange(4)
        globals_ = world(random_, sound_index, bytes(definition))
        # what the sounds share: how they are played, and four scales
        shared = bytearray(random_.randbytes(0x10))
        struct.pack_into("<I", shared, 4, len(shared))
        played = bytearray()
        for _n in range(PLAYBACKS):
            playback = bytearray(reals(random_, PLAYBACK_SIZE // 4))
            struct.pack_into("<f", playback, 8, random_.choice((0.0, 0.0, 0.0, 0.5, 1.0, 2.0)))
            struct.pack_into("<f", playback, 0x10, random_.choice((0.0, -6.0, 3.0, -70.0)))
            played += playback
        shared += played
        struct.pack_into("<I", shared, 0xC, len(shared))
        for _n in range(4):
            shared += reals(random_, 3) + struct.pack("<2f", random_.choice((0.0, 0.0, 0.3, 1.0)), random_.choice((0.0, 0.0, 0.6, 1.0)))
        globals_["global_sound_cache_file_gestalt"] = Buffer(bytes(shared), pointers=(4, 0xC))
        globals_["game_sound_globals"] = sound_state(*states[random_.randrange(len(states))])
        globals_["global_random_seed_reference"] = Buffer(random_.randbytes(0x10))
        globals_["unknown_0047f0e4"] = struct.pack("<i", random_.choice((0, 1)))
        request = bytearray(random_.randbytes(0xC + 0x48))
        struct.pack_into("<H", request, 0xC, random_.choice((0, 0, 0x100, 0x800, random_.getrandbits(16))))
        struct.pack_into("<2f", request, 0x10, random_.choice((0.0, 0.5, 1.0)), random_.choice((0.0, -3.0, -64.0)))
        heard = random_.choice((NONE, 0, 0, 1, 3))
        kind_ok = random_.choice((1, 1, 1, 0))
        gain = random_.choice((0.0, 0.0005, 0.25, 1.0, 1.0))
        loud = random_.choice((0.0, -6.0, -30.0, -64.0, -80.0))

        def listener_(harness, arguments, heard=heard):
            said_, maximum, distance = arguments
            record = harness.normalize(said_), maximum, bytes(harness.mu.mem_read(distance, 4))
            harness.mu.mem_write(distance, struct.pack("<f", 7.5))
            return heard, record

        t.check(
            "sound_start_test",
            Buffer(bytes(request)),
            random_.getrandbits(16) << 16 | sound_index,
            Buffer(random_.randbytes(4)),
            random_.choice((0, Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4)))),
            globals=globals_,
            externals={
                "sound_definition_unknown_00126bd0": lambda harness, arguments, kind_ok=kind_ok: (kind_ok, tuple(arguments)),
                "sound_get_maximum_distance": lambda harness, arguments: (
                    real_bits(40.0),
                    (harness.normalize(arguments[0]), arguments[1]),
                ),
                "sound_source_get_listener": listener_,
                "sound_source_distance_gain": lambda harness, arguments, gain=gain: (
                    real_bits(gain),
                    (arguments[0], harness.normalize(arguments[1]), arguments[2]),
                ),
                "sound_definition_decibels": lambda harness, arguments, loud=loud: (
                    real_bits(loud),
                    (harness.normalize(arguments[0]), arguments[1], arguments[2]),
                ),
            },
        )

    for _ in range(500):
        sound_index = random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        struct.pack_into("<I", tags, sound_index * 0x10 + 8, len(tags))
        definition = bytearray(random_.randbytes(0x10))
        definition[0xC] = random_.choice((0xFF, 0, 1, 2, 3, 0, 1, 2, 3))
        now = random_.choice((0, 1000, 100000))
        # what the sounds share: 4 promotions, with 3 rules and 3 times each
        data = bytearray(random_.randbytes(0x50))
        pointers = [0x4C]
        promotions = len(data)
        struct.pack_into("<I", data, 0x4C, promotions)
        data += bytes(4 * 0x1C)
        for n in range(4):
            at = promotions + n * 0x1C
            count = random_.choice((0, 1, 3, 3))
            rules = len(data)
            for _rule in range(3):
                data += struct.pack(
                    "<hhfii",
                    random_.randrange(4),
                    random_.choice((0, 1, 1, 4)),
                    random_.choice((0.0, 0.5, 2.0)),
                    random_.choice((0, 100, 1000)),
                    random_.choice((0, 50, 500)),
                )
            timers = len(data)
            data += struct.pack("<3i", *(random_.choice((0, 10, 400, 5000)) for _ in range(3)))
            struct.pack_into(
                "<iIiIiii",
                data,
                at,
                count,
                rules,
                random_.choice((0, 3, 3)),
                timers,
                random_.choice((-1, 0, 1, 2)),
                now - random_.choice((0, 5, 300, 100000)),
                now + random_.choice((-500, 0, 1, 500)),
            )
            pointers += [at + 4, at + 0xC]
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        struct.pack_into("<i", state, 0x84, now)
        t.check(
            "sound_promotion_update",
            random_.getrandbits(16) << 16 | sound_index,
            Buffer(random_.randbytes(4)),
            globals={
                "global_tag_instances": Buffer(bytes(tags) + bytes(definition), pointers=(sound_index * 0x10 + 8,)),
                "global_sound_cache_file_gestalt": Buffer(bytes(data), pointers=tuple(pointers)),
                "game_sound_globals": Buffer(bytes(state)),
            },
        )

    # ---------- the sound that plays is made

    random_ = random.Random(185)

    for _ in range(900):
        sound_index = random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        struct.pack_into("<I", tags, sound_index * 0x10 + 8, len(tags))
        definition = bytearray(random_.randbytes(0x10))
        definition[0] = random_.choice((0, 8, 0xF7, 0xFF))
        definition[0xB] = random_.randrange(4)
        shared = bytearray(random_.randbytes(0x10))
        struct.pack_into("<I", shared, 0xC, len(shared))
        for _n in range(4):
            shared += random_.randbytes(8) + struct.pack("<2h", random_.randint(-600, 0), random_.randint(0, 600)) + random_.randbytes(8)
        buffer, indices = playing()
        request = bytearray(random_.randbytes(0xA0))
        struct.pack_into("<I", request, 0, random_.getrandbits(11) | random_.getrandbits(21) << 11)
        struct.pack_into("<H", request, 0xC, random_.choice((0, 0, 0x100, 0x800, random_.getrandbits(16))))
        struct.pack_into("<2f", request, 0x10, random_.choice((0.0, 0.5, 1.0)), random_.choice((0.0, -3.0)))
        struct.pack_into("<H", request, 0x8C, random_.choice((0, 0x10, 0x2F, 0x30, 0x31, 0xFFFF)))
        struct.pack_into("<f", request, 0x94, random_.choice((0.0, -6.0, -80.0)))
        table = bytearray(random_.randbytes(0x100 * 0x5C))
        table_ = Buffer(bytes(table))
        controller_ = Buffer(random_.randbytes(0x1C))
        loud = random_.choice((0.0, 0.0, -6.0, -60.0, -64.0))
        random_gain = random_.choice((0.0, 0.0, -1.5, -5.0))
        made = random_.choice((NONE, *indices, *indices))
        far_ = random_.choice((0.0, 4.0, 400.0, 1e6, 1e12))
        acquired = random_.choice((NONE, 0x30002, 5))
        chosen_range = random_.choice((0, 1, 3, 0xFFFF))
        chosen_recording = random_.choice((0, 2, 0xFFFF))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        struct.pack_into("<i", state, 0x84, random_.choice((0, 5000, 0x7FFFFF00)))

        def pointer_record(*pointers: int):
            def handler(harness, arguments, value=0):
                return value, tuple(harness.normalize(a) if n in pointers else a for n, a in enumerate(arguments))

            return handler

        def returning(value: int, *pointers: int, mask=None):
            def handler(harness, arguments):
                record = tuple(harness.normalize(a) if n in pointers else a for n, a in enumerate(arguments))
                if mask:
                    record = tuple(a & mask.get(n, 0xFFFFFFFF) for n, a in enumerate(record))
                return value, record

            return handler

        t.check(
            "sound_source_create",
            random_.getrandbits(16) << 16 | sound_index,
            random_.choice((0, 1, 2, 3)),
            Buffer(bytes(request)),
            random_.choice((NONE, NONE, 0, 1, 2)),
            globals={
                "global_tag_instances": Buffer(bytes(tags) + bytes(definition), pointers=(sound_index * 0x10 + 8,)),
                "global_sound_cache_file_gestalt": Buffer(bytes(shared), pointers=(0xC,)),
                "sound_sources": Buffer(buffer.data, pointers=buffer.pointers),
                "game_sound_globals": Buffer(bytes(state)),
                "global_random_seed_reference": Buffer(random_.randbytes(0x10)),
                "unknown_00547f14": struct.pack("<f", random_.choice((0.0, 1.0, 3.0, 20.0))),
            },
            extra=[table_, controller_],
            externals={
                "sound_definition_random_gain": returning(real_bits(random_gain), 0),
                "sound_definition_decibels": returning(real_bits(loud), 0),
                "sound_request_unknown_002226b0": returning(0, 0, mask={1: 0xFFFF}),
                "datum_new": returning(made, 0),
                "sound_source_unknown_00127e20": returning(real_bits(far_), 0),
                "sound_definition_random_pitch": returning(real_bits(random_.choice((0.0, 100.0, -50.0))), 0, 1),
                "sound_class_get": class_of(table_),
                "sound_playback_controller_acquire": returning(acquired),
                "sound_playback_controller_get": returning(controller_.address),
                "sound_playback_controller_add_count": returning(0, 0, mask={1: 0xFF}),
                "sound_definition_promotion_rule": returning(chosen_range, 1),
                "sound_choose_pitch_range": returning(chosen_range, 1, mask={0: 0xFFFF}),
                "sound_choose_permutation": returning(chosen_recording, 0, mask={1: 0xFFFF}),
                "sound_source_set_sound": returning(0, mask={2: 0xFF, 3: 0xFF, 4: 0xFFFF}),
                "sound_source_request_data": returning(0, 0),
            },
        )

    # ---------- two things more for what the game gives to start a sound

    random_ = random.Random(186)

    NAMES = (0x1300012D, 0x14000135, 0x1600013C, 0x1600013D, 0x1600013E, 0x1600013F)
    for name in (*NAMES, 0, NONE, 0x1300012C, 0x1300012E, 0x16000140, 0x1600013B, 0x12D, 0x7FFFFFFF, 0x80000000):
        t.check("sound_unknown_001914f0", name)

    for _ in range(500):
        tag_index = random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        struct.pack_into("<I", tags, tag_index * 0x10 + 8, len(tags))
        tag = bytearray(random_.randbytes(0x1C)) + random_.randbytes(8) + struct.pack(
            "<I", random_.choice((*NAMES, NAMES[0], NAMES[0], 0, 0x1300012C))
        )
        struct.pack_into("<iI", tag, 0x14, random_.choice((1, 1, 1, 0, -1)), len(tags) + 0x1C)
        tags_ = Buffer(bytes(tags) + bytes(tag), pointers=(tag_index * 0x10 + 8, len(tags) + 0x18))

        def collection_(count: int) -> Buffer:
            data = bytearray(random_.randbytes(0x34)) + random_.randbytes(4)
            data += struct.pack("<I", random_.choice((NONE, random_.getrandbits(16) << 16 | tag_index, random_.getrandbits(16) << 16 | tag_index)))
            struct.pack_into("<iI", data, 0x2C, count, 0x34)
            return Buffer(bytes(data), pointers=(0x30,))

        first = random_.choice((None, collection_(random_.choice((0, 1, 2, -1)))))
        second = random_.choice((None, collection_(random_.choice((0, 1, 1)))))
        request = bytearray(random_.randbytes(0xA0))
        struct.pack_into("<I", request, 0, random_.getrandbits(32))
        other = bytearray(random_.randbytes(0x10))
        struct.pack_into("<h", other, 6, random_.choice((-1, 0, 5, 0x1234)))
        t.check(
            "sound_request_unknown_002226b0",
            Buffer(bytes(request)),
            random_.getrandbits(32),
            globals={"global_tag_instances": tags_, "unknown_00510c90": Buffer(bytes(other))},
            extra=[buffer for buffer in (first, second) if buffer],
            externals={
                "unknown_0018d090": lambda harness, arguments, first=first: (first.address if first else 0, tuple(arguments)),
                "unknown_0018d170": lambda harness, arguments, second=second: (second.address if second else 0, tuple(arguments)),
            },
        )

    # ---------- the time of the sound goes on

    random_ = random.Random(187)

    for _ in range(900):
        before = random_.choice((1000, 0, 0xFFFFFF00, 0x7FFFFFF0))
        now = (before + random_.choice((0, 1, 16, 33, 34, 500, 1000, 5000, -5))) & 0xFFFFFFFF
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        struct.pack_into("<Ii", state, 0x80, before, random_.choice((0, 100, 5000, 0x7FFFFF00)))
        struct.pack_into("<f", state, 0x1E0, random_.uniform(0, 1))
        struct.pack_into("<f", state, 0x1E4, random_.choice((0.0, 1.0, 0.5, 0.25, 0.999, 0.001, -0.5, 1.5)))
        struct.pack_into("<i", state, 0x1E8, random_.choice((-1, -1, 0, 10, 400, 100000)))
        struct.pack_into(
            "<3f",
            state,
            0x1EC,
            random_.choice((0.0, 0.25, 1.0)),
            random_.choice((0.0, 0.0005, 0.5, 2.0)),
            random_.choice((0.0, 0.0005, 0.5, 2.0)),
        )
        struct.pack_into("<f", state, 0x1FC, random_.uniform(0, 100))
        struct.pack_into("<f", state, 0x204, random_.uniform(0, 100))
        game = bytearray(random_.randbytes(0x1200))
        struct.pack_into("<i", game, 8, random_.choice((1, 1, 1, 0, 2, 3)))
        game[0x1120] = random_.choice((1, 1, 1, 0, 0x80))
        game[0x1121] = random_.choice((0, 1, 1, 0x80))
        pending = random_.choice((0, 0, 0, 1))
        t.check(
            "sound_update_time",
            globals={
                "game_sound_globals": Buffer(bytes(state)),
                "main_game_globals": random_.choice((Buffer(bytes(game)), Buffer(bytes(game)), Buffer(bytes(game)), 0)),
            },
            externals={
                "GetTickCount": lambda harness, arguments, now=now: now,
                "main_action_pending": lambda harness, arguments, pending=pending: pending,
            },
        )

    # ---------- the sound of each channel, each time

    random_ = random.Random(188)

    for _ in range(500):
        now = random_.choice((1000, 100000))
        data = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        pointers = [0x44]
        struct.pack_into("<I", data, 0x44, 0x4C)
        table = len(data)
        data += random_.randbytes(8) + struct.pack("<I", random_.choice((mock_address(0), mock_address(0), 0))) + random_.randbytes(0x14)
        indices = []
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            data[at + 2] = random_.choice((0, 0, 1, 2, 4, 5))
            data[at + 4] = random_.choice((0, 0x80, 0x80, 0xA0, 0x20, 0xFF, 0x7F))
            if random_.random() < 0.7:
                struct.pack_into("<I", data, at + 0x14, table)
                pointers.append(at + 0x14)
            else:
                struct.pack_into("<I", data, at + 0x14, 0)
            struct.pack_into("<f", data, at + 0x20, random_.choice((0.0, -6.0, -40.0, -64.0)))
            first, second = random_.choice(((now - 300, now), (now, now - 300), (now + 300, now), (now, now + 300), (5, 5)))
            struct.pack_into("<2i", data, at + 0xB4, first, second)
            indices.append(random_.getrandbits(16) << 16 | index)
        channels = bytearray(random_.randbytes(CHANNELS * 0x24))
        for n in range(CHANNELS):
            struct.pack_into("<I", channels, n * 0x24, random_.choice((NONE, *indices, *indices)))
            channels[n * 0x24 + 9] = random_.choice((0, 0, 0, 1))
            struct.pack_into("<h", channels, n * 0x24 + 0xC, random_.choice((-1, 0, 3, 7)))
            struct.pack_into("<f", channels, n * 0x24 + 0x10, random_.uniform(0, 5))
            struct.pack_into("<I", channels, n * 0x24 + 0x1C, random_.choice((0, 1, 2)))
        state = bytearray(sound_state(channels=random_.choice((0, 1, 5, CHANNELS, CHANNELS, -1))).data)
        struct.pack_into("<i", state, 0x84, now)
        struct.pack_into("<f", state, 0x1E4, random_.choice((1.0, 0.5, 0.0)))
        fades = [random_.choice((0.0, -3.0, -30.0, -64.0)) for _ in range(PLAYING)]

        def fade_(harness, arguments, fades=fades):
            return real_bits(fades[arguments[0] & 0xFFFF]), tuple(arguments)

        def heard_(harness, arguments):
            channel_index, decibels, parameters = arguments
            record = channel_index & 0xFFFF, bytes(harness.mu.mem_read(decibels, 4))
            harness.mu.mem_write(decibels, struct.pack("<f", -1.5 * (channel_index & 7)))
            harness.mu.mem_write(parameters, bytes((channel_index * 7 + n) & 0xFF for n in range(0x38)))
            return 0, record

        def given_(harness, arguments):
            channel_index, decibels, parameters = arguments
            return 0, (channel_index & 0xFFFF, decibels, bytes(harness.mu.mem_read(parameters, 0x38)))

        t.check(
            "sound_channels_update_sources",
            globals={
                "game_sound_globals": Buffer(bytes(state)),
                "sound_channels": Buffer(bytes(channels)),
                "sound_sources": Buffer(bytes(data), pointers=tuple(pointers)),
            },
            externals={
                "sound_gain_to_decibels": lambda harness, arguments: (
                    real_bits(-6.0 if arguments[0] == real_bits(0.5) else (-64.0 if arguments[0] == 0 else 0.0)),
                    tuple(arguments),
                ),
                "sound_console_get_state": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "sound_source_fade_decibels": fade_,
                "sound_source_delete": lambda harness, arguments: (0, tuple(arguments)),
                "sound_channel_compute_parameters": heard_,
                "looping_sound_channel_update_voice": given_,
                "sound_channel_update_voice": given_,
            },
            mocks=[Mock(lambda call: 0, stack_arguments=6)],
        )

    # ---------- a channel gets a voice of the console's sound

    random_ = random.Random(189)

    for _ in range(700):
        buffer, indices = playing()
        data = bytearray(buffer.data)
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            data[at + 6] = random_.choice((0, 0, 2, 2, 0xFD, 0xFF))
            data[at + 0xA2] = random_.choice((0xFF, 0xFF, 0, 3, 0x7F))
        channels = bytearray(random_.randbytes(CHANNELS * 0x24))
        for n in range(CHANNELS):
            struct.pack_into("<I", channels, n * 0x24, random_.choice(indices))
            channels[n * 0x24 + 8] = random_.randrange(3)
            struct.pack_into("<hh", channels, n * 0x24 + 0xC, random_.choice((-1, -1, -1, 4)), random_.choice((-1, -1, 6)))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        for kind in range(3):
            first = random_.choice((0, 5, 30, 40))
            struct.pack_into("<i", state, 0x40 + kind * 4, first)
            struct.pack_into("<i", state, 0x4C + kind * 4, first + random_.choice((-1, 0, 3, 20)))
        state[0:0x10] = random_.choice((bytes(0x10), random_.randbytes(0x10), bytes((0xFF,)) * 0x10))
        state[0x60:0x70] = random_.choice((bytes(0x10), random_.randbytes(0x10), bytes((0xFF,)) * 0x10))
        voices_ = bytearray(random_.randbytes(0x4C + VOICES * 0x6C))
        struct.pack_into("<I", voices_, 0x44, 0x4C)
        for index in range(VOICES):
            struct.pack_into("<h", voices_, 0x4C + index * 0x6C + 2, random_.choice((-1, 0, 5, 31)))
        busy = random_.choice((0, 0, 1, 0xFE, 0xFF))
        given = random_.choice((NONE, *(random_.getrandbits(16) << 16 | index for index in range(VOICES))))

        def stream_(harness, arguments, busy=busy):
            stream, status = arguments
            harness.mu.mem_write(status, struct.pack("<I", busy))
            return 0, (stream,)

        t.check(
            "sound_channel_get_voice",
            random_.randrange(CHANNELS),
            globals={
                "sound_sources": Buffer(bytes(data), pointers=buffer.pointers),
                "sound_channels": Buffer(bytes(channels)),
                "game_sound_globals": Buffer(bytes(state)),
                "sound_voices": Buffer(bytes(voices_), pointers=(0x44,)),
                "unknown_0051ebe4": Buffer(random_.randbytes(0x2AC0)),
            },
            externals={
                "unknown_0037b75b": stream_,
                "sound_voice_get_for_source": lambda harness, arguments, given=given: (given, (harness.normalize(arguments[0]),)),
                "sound_source_delete": lambda harness, arguments: (0, tuple(arguments)),
                "sound_channel_delete": lambda harness, arguments: (0, (arguments[0] & 0xFFFF,)),
            },
        )

    # ---------- the sound voice of a sound that plays

    random_ = random.Random(190)

    for _ in range(600):
        in_use = some_in_use(VOICES)
        voices_ = data_array(random_, VOICES, 0x6C, in_use)
        playing_ = bytearray(random_.randbytes(DATUM))
        sound_index = random_.randrange(TAGS)
        struct.pack_into("<I", playing_, 0xC, random_.getrandbits(16) << 16 | sound_index)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        struct.pack_into("<I", tags, sound_index * 0x10 + 8, len(tags))
        shared_ = random_.getrandbits(VOICES) if random_.random() < 0.5 else 0
        slot = random_.choice((0xFFFF, 0, 5, 63))
        made = random_.choice((NONE, *(random_.getrandbits(16) << 16 | index for index in range(VOICES))))

        def matches_(harness, arguments, shared_=shared_):
            voice, source_ = arguments
            index = calls_so_far(harness, "sound_voice_matches_source")
            return (shared_ >> index) & 1, (harness.normalize(voice), harness.normalize(source_))

        t.check(
            "sound_voice_get_for_source",
            Buffer(bytes(playing_)),
            globals={
                "sound_voices": voices_,
                "global_tag_instances": Buffer(bytes(tags) + random_.randbytes(0x10), pointers=(sound_index * 0x10 + 8,)),
            },
            externals={
                "sound_voice_matches_source": matches_,
                "sound_slot_new": lambda harness, arguments, slot=slot: slot,
                "datum_new": lambda harness, arguments, made=made: (made, (harness.normalize(arguments[0]),)),
                "sound_slot_delete": lambda harness, arguments: (0, (arguments[0] & 0xFFFF,)),
            },
        )

    # ---------- the sound of a channel and how it is heard where it is

    random_ = random.Random(191)

    for _ in range(900):
        buffer, indices = playing()
        data = bytearray(buffer.data)
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            data[at + 6] = random_.choice((2, 2, 2, 0, 0xFD))
            struct.pack_into("<I", data, at + 0xC, random_.getrandbits(16) << 16 | random_.randrange(TAGS))
            struct.pack_into("<H", data, at + 0x18, random_.choice((0x1000, 0x1000, 0, 0xEFFF, 0xFFFF)))
            data[at + 0x1B] = random_.choice((0, 0, 1, 1, 2, 2, 3, 0xF, 0x21, 0x10, 0x18))
            data[at + 0x1C : at + 0x20] = reals(random_, 1)
            data[at + 0x24 : at + 0x30] = reals(random_, 3)
            struct.pack_into("<h", data, at + 0x46, random_.choice((0, 1, 1)))
            data[at + 0xA1] = random_.randrange(4)
        channels = bytearray(random_.randbytes(CHANNELS * 0x24))
        for n in range(CHANNELS):
            struct.pack_into("<I", channels, n * 0x24, random_.choice(indices))
            struct.pack_into("<I", channels, n * 0x24 + 4, random_.choice((NONE, NONE, 0x10002, 5)))
            struct.pack_into("<h", channels, n * 0x24 + 0xE, random_.choice((-1, 0, 3, 9)))
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        tag_pointers = []
        for n in range(TAGS):
            struct.pack_into("<I", tags, n * 0x10 + 8, TAGS * 0x10 + n * 0x10)
            tag_pointers.append(n * 0x10 + 8)
        table = bytearray(random_.randbytes(0x100 * 0x5C))
        for n in range(0x100):
            struct.pack_into("<f", table, n * 0x5C + 0x10, random_.choice((0.0, -3.0, 2.0)))
            struct.pack_into(
                "<3f", table, n * 0x5C + 0x50, random_.choice((0.0, 0.5, 1.0, 2.0)), random_.uniform(0, 4), random_.uniform(0, 4)
            )
        table_ = Buffer(bytes(table))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        struct.pack_into("<f", state, 0x1E0, random_.choice((0.0, 1.0 / 30.0, 0.5)))
        for index in range(4):
            state[LISTENERS + index * LISTENER_SIZE + 7] = random_.choice((0, 1, 0x80))
        taken = random_.choice((1, 1, 0))
        seen = random_.choice((0, 1))

        def given__(harness, arguments, taken=taken, seen=seen):
            voice_index, said_, first_ = arguments
            record = voice_index, harness.normalize(said_), bytes(harness.mu.mem_read(first_, 1))
            harness.mu.mem_write(first_, bytes((seen,)))
            return taken, record

        def where_(harness, arguments):
            said_, listener_index, position = arguments
            harness.mu.mem_write(position, struct.pack("<3f", 1.5, -2.5, 4.0))
            # the game's function leaves in eax the place that it was given there, and the game goes on to use it
            return position, (harness.normalize(said_), listener_index & 0xFF)

        def seen_from(harness, arguments):
            matrix, point, result = arguments
            record = harness.normalize(matrix), bytes(harness.mu.mem_read(point, 12)), harness.normalize(point) == harness.normalize(result)
            harness.mu.mem_write(result, struct.pack("<3f", 0.25, 8.0, -1.0))
            return result, record

        def slot_(harness, arguments):
            parameters, slot = arguments
            return 0, (bytes(harness.mu.mem_read(parameters, 0x24)), slot & 0xFFFF)

        t.check(
            "sound_channel_update_slot",
            random_.randrange(CHANNELS),
            random_.choice((0, Buffer(reals(random_, 3)), Buffer(struct.pack("<3f", random_.uniform(-1, 2), random_.uniform(-1, 2), random_.uniform(-1, 2))))),
            globals={
                "sound_sources": Buffer(bytes(data), pointers=buffer.pointers),
                "sound_channels": Buffer(bytes(channels)),
                "game_sound_globals": Buffer(bytes(state)),
                "global_tag_instances": Buffer(bytes(tags) + random_.randbytes(TAGS * 0x10), pointers=tuple(tag_pointers)),
                "global_structure_bsp_index": struct.pack("<h", 1),
                "unknown_0051ebe4": Buffer(random_.randbytes(0x2AC0)),
            },
            extra=[table_],
            externals={
                "sound_class_get": class_of(table_),
                "sound_voice_unknown_0021dff0": given__,
                "sound_source_get_position": where_,
                "matrix4x3_inverse_transform_point": seen_from,
                "sound_gain_to_decibels": lambda harness, arguments: (real_bits(-2.25), tuple(arguments)),
                "sound_console_slot_set_parameters": slot_,
            },
        )

    # ---------- the sounds of which it is to be worked out how they are heard

    random_ = random.Random(192)

    for _ in range(600):
        buffer, indices = playing()
        data = bytearray(buffer.data)
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            data[at + 4] = random_.choice((0, 0, 0, 4, 0xFB, 0xFF))
            struct.pack_into("<I", data, at + 0xC, random_.getrandbits(16) << 16 | random_.randrange(TAGS))
            struct.pack_into("<H", data, at + 0x18, random_.choice((0, 0, 0x1000, 0x1000, 0x80, 1, 0x1081, 0xEF7E)))
            data[at + 0x1B] = random_.choice((0, 0, 1, 1, 1, 2, 0x11, 0x10, 0xF))
            struct.pack_into("<h", data, at + 0x46, random_.choice((1, 1, 0)))
            data[at + 0xA1] = random_.randrange(4)
        channels = bytearray(random_.randbytes(CHANNELS * 0x24))
        for n in range(CHANNELS):
            struct.pack_into("<I", channels, n * 0x24, random_.choice((NONE, *indices, *indices)))
            channels[n * 0x24 + 9] = random_.choice((0, 0, 0, 1))
            struct.pack_into("<hh", channels, n * 0x24 + 0xC, random_.choice((-1, 0, 3, 127)), random_.choice((-1, 0, 0, 5, 63)))
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        tag_pointers = []
        for n in range(TAGS):
            struct.pack_into("<I", tags, n * 0x10 + 8, TAGS * 0x10 + n * 0x10)
            tag_pointers.append(n * 0x10 + 8)
        table_ = Buffer(random_.randbytes(0x100 * 0x5C))
        listeners_ = bytearray(random_.randbytes(4 + 72 * 0x48))
        struct.pack_into("<i", listeners_, 0, random_.choice((0, 0, 0, 5, 70, 71, 72)))
        voiced = random_.getrandbits(CHANNELS) | random_.getrandbits(CHANNELS)

        t.check(
            "sound_channels_build_queries",
            Buffer(random_.randbytes(0x100)),
            Buffer(random_.randbytes(0x200)),
            Buffer(bytes(listeners_)),
            globals={
                "sound_sources": Buffer(bytes(data), pointers=buffer.pointers),
                "sound_channels": Buffer(bytes(channels)),
                "game_sound_globals": sound_state(channels=random_.choice((0, 1, 5, CHANNELS, CHANNELS, -1))),
                "global_tag_instances": Buffer(bytes(tags) + random_.randbytes(TAGS * 0x10), pointers=tuple(tag_pointers)),
                "global_structure_bsp_index": struct.pack("<h", 1),
                "unknown_00468788": Buffer(random_.randbytes(12)),
            },
            extra=[table_],
            externals={
                "sound_class_get": class_of(table_),
                "sound_channel_get_voice": lambda harness, arguments, voiced=voiced: (
                    (voiced >> (arguments[0] & 7)) & 1,
                    (arguments[0] & 0xFFFF,),
                ),
            },
        )

    # ---------- the Doppler effect of a sound for a listener

    random_ = random.Random(193)

    def some_speed() -> float:
        return random_.choice((0.0, 0.0, 1.0, random_.uniform(-5, 5), random_.uniform(-150, 150), random_.uniform(-600, 600)))

    for n in range(900):
        state = bytearray(random_.randbytes(LISTENERS + 4 * LISTENER_SIZE))
        for index in range(4):
            listener = LISTENERS + index * LISTENER_SIZE
            struct.pack_into("<f", state, listener + 8, random_.choice((1.0, 1.0, 0.5, 2.0, 0.0, random_.uniform(0, 3))))
            if random_.random() < 0.5:
                # turned and no more
                forward = [random_.gauss(0, 1) for _ in range(3)]
                length = sum(x * x for x in forward) ** 0.5 or 1.0
                forward = [x / length for x in forward]
                left = [forward[1], -forward[0], 0.0]
                up = [
                    forward[1] * left[2] - forward[2] * left[1],
                    forward[2] * left[0] - forward[0] * left[2],
                    forward[0] * left[1] - forward[1] * left[0],
                ]
                struct.pack_into("<9f", state, listener + 0xC, *forward, *left, *up)
            else:
                state[listener + 0xC : listener + 0x30] = reals(random_, 9)
            state[listener + 0x30 : listener + 0x3C] = reals(random_, 3)
            struct.pack_into("<3f", state, listener + 0x3C, some_speed(), some_speed(), some_speed())
        listener_index = random_.randrange(4)
        said = bytearray(random_.randbytes(0x1C))
        said[0:0xC] = reals(random_, 3)
        struct.pack_into("<3f", said, 0x10, some_speed(), some_speed(), some_speed())
        at = LISTENERS + listener_index * LISTENER_SIZE + 0x30
        kind = n % 6
        if kind == 0:
            # where the listener is
            said[0:0xC] = state[at : at + 0xC]
        elif kind == 1:
            # next to it
            x, y, z = struct.unpack_from("<3f", state, at)
            struct.pack_into(
                "<3f",
                said,
                0,
                x + random_.choice((0.0, 0.00005, 0.0001, 0.00011, 0.001)),
                y,
                z + random_.choice((0.0, 0.00005, -0.0001)),
            )
        elif kind == 2:
            # nothing moves
            said[0x10:0x1C] = bytes(12)
            state[at + 0xC : at + 0x18] = bytes(12)
        t.check(
            "sound_listener_doppler_pitch",
            listener_index,
            Buffer(bytes(said)),
            globals={"game_sound_globals": Buffer(bytes(state))},
        )

    # ---------- how much of a sound is heard from where its listener is

    random_ = random.Random(194)

    def some_way() -> bytes:
        kind = random_.randrange(8)
        if kind == 0:
            return bytes(12)
        if kind == 1:
            # too near for the cone to count, or just not
            if random_.random() < 0.5:
                # the square of its length is the real before the least
                return struct.pack("<3I", 0, 0x3A831247, 0x364BA58F)
            return struct.pack("<3f", random_.choice((0.0009, 0.001, 0.0011, -0.001)), 0.0, random_.choice((0.0, 0.0001)))
        if kind == 2:
            return struct.pack("<3f", *(random_.choice((0.0, 1.0, -1.0)) for _ in range(3)))
        return struct.pack("<3f", *(random_.uniform(-20, 20) for _ in range(3)))

    for n in range(1200):
        sound_index = random_.randrange(TAGS)
        flags = random_.getrandbits(16)
        if n % 3 == 0:
            flags &= ~0x30
        if n % 4 == 0:
            # the distances and the cone, of the sound itself
            flags |= 0x7C
        said = bytearray(source(random_, flags, random_.choice((None, None, 0x2000, 0x4000, 0x8000))).data)
        struct.pack_into("<2f", said, 0x34, random_.choice((0.0, 0.5, 1.0, 3.0)), random_.choice((0.0, 5.0, 30.0, 100.0)))
        struct.pack_into("<f", said, 0x40, random_.choice((0.0, -3.0, -6.0, -30.0, -64.0, -100.0, 2.0)))
        sound_flags = random_.getrandbits(16)
        if n % 2 == 0:
            sound_flags &= ~0x1000
        t.check(
            "sound_source_spatial_gain",
            Buffer(some_way()),
            Buffer(bytes(said)),
            random_.getrandbits(16) << 16 | sound_index,
            Buffer(some_way()),
            globals=world(random_, sound_index, sound_data(random_, sound_flags)),
        )

    # ---------- a slot of the console's sound takes where its sound is

    random_ = random.Random(195)
    SLOTS = 0x1A18
    SLOT_SIZE = 0x3C

    def near(value: float, least: float) -> float:
        return value + random_.choice((0.0, 0.0, least * 0.5, -least * 0.5, least, -least, least * 2, random_.uniform(-3, 3)))

    for n in range(1500):
        console = bytearray(random_.randbytes(0x2AC0))
        console[0] = random_.choice((0, 1, 1, 1, 0x80))
        slot_index = random_.randrange(32)
        slot = SLOTS + slot_index * SLOT_SIZE
        console[slot + 4] = random_.choice((0, 1, 2, 2, 3, 3, 0xFE, 0xFF, random_.getrandbits(8)))
        taken = bytearray(random_.randbytes(0x24))
        taken[0] = random_.choice((0, 1, 2, 0xFF, 0xFE))
        taken[4:0x10] = reals(random_, 3)
        struct.pack_into("<2f", taken, 0x10, random_.uniform(-2, 2), random_.uniform(-2, 2))
        struct.pack_into("<2f", taken, 0x1C, random_.choice((0.0, -1.0, 0.25, 0.25, 5.0)), random_.choice((0.0, -1.0, 0.25, 0.25, 5.0)))
        x, y, z, unknown10, unknown14 = struct.unpack_from("<5f", taken, 4)
        if n % 4:
            struct.pack_into("<3f", console, slot + 8, near(x, 0.05), near(y, 0.05), near(z, 0.05))
        if n % 3:
            struct.pack_into("<2f", console, slot + 0x2C, near(unknown10, 0.001), near(unknown14, 0.001))
        if n % 5 == 0:
            # nothing new
            struct.pack_into("<3f", console, slot + 8, x, y, z)
            struct.pack_into("<2f", console, slot + 0x2C, unknown10, unknown14)
        t.check(
            "sound_console_slot_set_parameters",
            Buffer(bytes(taken)),
            slot_index,
            globals={"unknown_0051ebe4": Buffer(bytes(console))},
            externals={
                "unknown_0037c668": lambda harness, arguments: (0, tuple(arguments)),
                "sound_console_slot_update_room": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- the console is told how the sound of a slot is heard in a room

    random_ = random.Random(196)

    def some_gain() -> float:
        return random_.choice(
            (0.0, 1.0, 1.0, 0.5, 0.001, 0.0006, 0.00063, 0.00064, 2.0, -1.0, 1e-30, 1e30, float("inf"), random_.uniform(0, 1), random_.uniform(0, 1))
        )

    def room_(harness, arguments):
        stream, room, apply = arguments
        return 0, (stream, bytes(harness.mu.mem_read(room, 0x24)), apply)

    def filter_(harness, arguments):
        stream, filter__ = arguments
        return 0, (stream, bytes(harness.mu.mem_read(filter__, 0x18)))

    for n in range(1500):
        console = bytearray(random_.randbytes(0x2AC8))
        struct.pack_into("<3f", console, 0x2ABC, some_gain(), some_gain(), some_gain())
        slot_index = random_.randrange(32)
        slot = SLOTS + slot_index * SLOT_SIZE
        struct.pack_into("<2f", console, slot + 0x2C, 1.0 - some_gain(), 1.0 - some_gain())
        if n % 7 == 0:
            struct.pack_into("<2f", console, slot + 0x2C, random_.choice((1.0, 0.0, 2.0)), random_.choice((1.0, 0.0, 2.0)))
        t.check(
            "sound_console_slot_update_room",
            slot_index,
            as_bits(random_.choice((0.0, -64.0, -32.0, -70.0, 3.0, -0.01, -63.99, random_.uniform(-64, 0), float("nan")))),
            globals={
                "unknown_0051ebe4": Buffer(bytes(console)),
                "unknown_00470084": struct.pack("<i", random_.choice((8000, 8000, 0, 29, 30, 100, 1000, 7999, 8001, 24000, -5))),
                "unknown_00470088": struct.pack("<f", random_.choice((0.0, 0.0, 3.0, 22.5, 30.0, -6.0, random_.uniform(0, 25)))),
            },
            externals={"unknown_0037c6e5": room_, "unknown_0037b68b": filter_},
        )

    # ---------- the sounds whose time has come get a channel

    random_ = random.Random(197)

    for n in range(700):
        count = random_.choice((PLAYING, PLAYING, PLAYING - 3, 1, 0))
        now = random_.choice((0, 1000, 0x7FFFFF00, -5))
        data = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        bits = len(data)
        data += struct.pack("<I", random_.getrandbits(32) | random_.getrandbits(32))
        table = len(data)
        data += random_.randbytes(4) + struct.pack("<I", mock_address(0) if random_.random() < 0.85 else 0)
        pointers = [0x44, 0x48]
        struct.pack_into("<I", data, 0x24, DATUM)
        struct.pack_into("<i", data, 0x38, count)
        struct.pack_into("<2I", data, 0x44, 0x4C, bits)
        indices = []
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            identifier_ = random_.choice((0, 1, 0x7FFF, 0x8001, 0xFFFF, random_.getrandbits(16)))
            struct.pack_into("<H", data, at, identifier_)
            data[at + 4] = random_.choice((0, 0, 0, 0x10, 0x20, 0x30, 0xCF, 0xFF, random_.getrandbits(8)))
            struct.pack_into("<I", data, at + 0xC, random_.getrandbits(16) << 16 | random_.randrange(TAGS))
            if random_.random() < 0.75:
                struct.pack_into("<I", data, at + 0x14, table)
                pointers.append(at + 0x14)
            else:
                struct.pack_into("<I", data, at + 0x14, 0)
            # whether its samples can be asked for: for the stand-in
            data[at + 0x30] = random_.choice((1, 1, 0))
            struct.pack_into("<i", data, at + 0x8C, now + random_.choice((0, 0, -1, -1000, 1, 50)))
            data[at + 0x9C] = random_.choice((0, 1, 2, 0xFF))
            struct.pack_into("<H", data, at + 0xAC, random_.choice((0xFFFF, 0xFFFF, 0, 3, 0x8000)))
            indices.append(identifier_ << 16 | index)
        channels = bytearray(random_.randbytes(CHANNELS * 0x24))
        for channel_index in range(CHANNELS):
            struct.pack_into("<I", channels, channel_index * 0x24, random_.choice((NONE, NONE, *indices)))
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        tag_pointers = []
        definitions = bytearray(random_.randbytes(TAGS * 0x10))
        for tag in range(TAGS):
            struct.pack_into("<I", tags, tag * 0x10 + 8, TAGS * 0x10 + tag * 0x10)
            tag_pointers.append(tag * 0x10 + 8)
            struct.pack_into("<h", definitions, tag * 0x10 + 8, random_.randrange(0, 6))
        table__ = bytearray(random_.randbytes(0x100 * 0x5C))
        for class_index in range(0x100):
            struct.pack_into("<h", table__, class_index * 0x5C + 0xE, random_.choice((0, 0, 0, 1, -1, 0x100)))
        table_ = Buffer(bytes(table__))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        struct.pack_into("<i", state, 0x84, now)
        gestalt_ = bytearray(random_.randbytes(0x50 + 0x10 * 0xC))
        struct.pack_into("<I", gestalt_, 0x24, 0x50)
        given_channels = [random_.choice((-1, -1, *range(CHANNELS), *range(CHANNELS))) for _ in range(PLAYING)]
        set_aside = [random_.choice((NONE, NONE, 0xFFFF, 0, 2)) for _ in range(PLAYING)]

        def asked(harness, arguments):
            return harness.mu.mem_read(arguments[0] + 0x30, 1)[0], (harness.normalize(arguments[0]),)

        def channel_(harness, arguments, given_channels=given_channels):
            source_index, reason = arguments
            harness.mu.mem_write(reason, struct.pack("<I", (source_index & 0xFFFF) + 20))
            return given_channels[source_index & 0xFFFF] & 0xFFFFFFFF, (source_index,)

        def aside(harness, arguments, set_aside=set_aside):
            calls = calls_so_far(harness, "sound_pitch_range_get_unknown5")
            return set_aside[calls % PLAYING], (harness.normalize(arguments[0]), harness.normalize(arguments[1]))

        t.check(
            "sound_sources_get_channels",
            globals={
                "sound_sources": Buffer(bytes(data), pointers=tuple(pointers)),
                "sound_channels": Buffer(bytes(channels)),
                "game_sound_globals": Buffer(bytes(state)),
                "global_tag_instances": Buffer(bytes(tags) + bytes(definitions), pointers=tuple(tag_pointers)),
                "global_sound_cache_file_gestalt": Buffer(bytes(gestalt_), pointers=(0x24,)),
            },
            extra=[table_],
            externals={
                "sound_source_request_data": asked,
                "sound_source_get_channel": channel_,
                "sound_source_delete": lambda harness, arguments: (0, tuple(arguments)),
                "sound_channel_delete": lambda harness, arguments: (0, (arguments[0] & 0xFFFF,)),
                "sound_class_get": class_of(table_),
                "sound_pitch_range_get_unknown5": aside,
                "sound_pitch_range_set_unknown5": lambda harness, arguments: (
                    0,
                    (harness.normalize(arguments[0]), harness.normalize(arguments[1]), arguments[2] & 0xFFFF),
                ),
            },
            mocks=[Mock(lambda call: 0, stack_arguments=4, record=lambda call: tuple(call.stack[:3]))],
        )

    # ---------- each frame, of each sound that plays: whether it goes on, and who hears it

    random_ = random.Random(198)

    for n in range(700):
        count = random_.choice((PLAYING, PLAYING, PLAYING - 3, 1, 0))
        times = (1000, 2000)
        data = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        bits = len(data)
        data += struct.pack("<I", random_.getrandbits(32) | random_.getrandbits(32))
        struct.pack_into("<I", data, 0x24, DATUM)
        struct.pack_into("<i", data, 0x38, count)
        struct.pack_into("<2I", data, 0x44, 0x4C, bits)
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            struct.pack_into("<H", data, at, random_.choice((0, 1, 0x7FFF, 0x8001, 0xFFFF, random_.getrandbits(16))))
            struct.pack_into(
                "<H", data, at + 4, random_.choice((0, 0, 4, 4, 0x240, 0x240, 0x244, 0x200, 0x40, 0xFDBF, 0xFFFF, random_.getrandbits(16)))
            )
            struct.pack_into("<I", data, at + 0xC, random_.getrandbits(16) << 16 | random_.randrange(TAGS))
            struct.pack_into("<H", data, at + 0x18, random_.choice((0, 0, 8, 8, 0xFFF7, random_.getrandbits(16))))
            struct.pack_into("<f", data, at + 0x50, random_.choice((0.0, 5.0, 40.0)))
            struct.pack_into("<i", data, at + 0x8C, random_.choice(times))
            data[at + 0xA0] = random_.choice((0xFF, 0, 0, 1, 2))
            struct.pack_into("<h", data, at + 0xAC, random_.choice((-1, -1, *range(CHANNELS))))
        # one before the first: the game looks at the channel of a sound with none
        channels = bytearray(random_.randbytes((CHANNELS + 1) * 0x24))
        for channel_index in range(CHANNELS + 1):
            channels[channel_index * 0x24 + 9] = random_.choice((0, 0, 0, 0, 1))
            struct.pack_into("<h", channels, channel_index * 0x24 + 0xC, random_.randrange(32))
        console = bytearray(random_.randbytes(32 * 0x34 + 0x10))
        for voice in range(32):
            console[voice * 0x34 + 0xD] = random_.choice((0, 0, 8, 0xF8, 1, 7, 0xFF))
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        tag_pointers = []
        definitions = bytearray(random_.randbytes(TAGS * 0x10))
        for tag in range(TAGS):
            struct.pack_into("<I", tags, tag * 0x10 + 8, TAGS * 0x10 + tag * 0x10)
            tag_pointers.append(tag * 0x10 + 8)
            struct.pack_into("<h", definitions, tag * 0x10 + 6, random_.randrange(0, PLAYBACKS))
            definitions[tag * 0x10 + 0xC] = random_.choice((0xFF, 0xFF, 0, 1, 2, 3))
        shared = bytearray(random_.randbytes(0x50))
        struct.pack_into("<I", shared, 4, len(shared))
        shared += reals(random_, PLAYBACKS * PLAYBACK_SIZE // 4)
        struct.pack_into("<I", shared, 0x4C, len(shared))
        for _promotion in range(4):
            shared += random_.randbytes(0x10) + struct.pack(
                "<3i", random_.choice((-1, 0, 1, 2, 3)), random_.choice(times), random_.choice((0, 1000, 1001, 2000, 2001, 5000))
            )
        table_ = Buffer(random_.randbytes(0x100 * 0x5C))
        goes_on = [random_.choice((1, 1, 1, 0)) for _ in range(PLAYING)]
        heard_by = [random_.choice((NONE, NONE, 0, 1, 3)) for _ in range(PLAYING)]

        def goes_on_(harness, arguments, goes_on=goes_on):
            return goes_on[arguments[0] & 0xFFFF], tuple(arguments)

        def heard_by_(harness, arguments, heard_by=heard_by):
            calls = calls_so_far(harness, "sound_source_get_listener")
            return heard_by[calls % PLAYING], (harness.normalize(arguments[0]), arguments[1], arguments[2])

        buffer = Buffer(bytes(channels))
        t.check(
            "sound_sources_update",
            globals={
                "sound_sources": Buffer(bytes(data), pointers=(0x44, 0x48)),
                "sound_channels": Pointer(buffer, 0x24),
                "unknown_0051ebe4": Buffer(bytes(console)),
                "global_tag_instances": Buffer(bytes(tags) + bytes(definitions), pointers=tuple(tag_pointers)),
                "global_sound_cache_file_gestalt": Buffer(bytes(shared), pointers=(4, 0x4C)),
            },
            extra=[table_, buffer],
            externals={
                "sound_source_unknown_00127fc0": goes_on_,
                "sound_source_get_listener": heard_by_,
                "sound_source_fade_decibels": lambda harness, arguments: (fade_bits(arguments[0]), tuple(arguments)),
                "sound_sources_cross_fade": lambda harness, arguments: (
                    0,
                    (arguments[0], arguments[1], arguments[2] & 0xFFFF, arguments[3]),
                ),
                "sound_source_delete": lambda harness, arguments: (0, tuple(arguments)),
                "sound_class_get": class_of(table_),
            },
        )

    # ---------- four things of a sound from the tags, for a voice of the console's sound

    random_ = random.Random(199)

    for _ in range(900):
        tag_index = random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        struct.pack_into("<I", tags, tag_index * 0x10 + 8, len(tags))
        tag = bytearray(random_.randbytes(0x14))
        tag += struct.pack("<I", random_.choice((*NAMES, 0, 0x1300012C))) + random_.randbytes(0xC)
        struct.pack_into("<iI", tag, 0xC, random_.choice((1, 1, 1, 2, 0, -1)), len(tags) + 0x14)
        tags_ = Buffer(bytes(tags) + bytes(tag), pointers=(tag_index * 0x10 + 8, len(tags) + 0x10))

        data = bytearray(random_.randbytes(0x7C))
        pointers = [0x30, 0x58, 0x68]
        struct.pack_into("<iI", data, 0x2C, random_.choice((1, 1, 1, 2, 0, -1, -0x80000000)), 0x34)
        if random_.random() < 0.1:
            # no elements where they should be
            struct.pack_into("<I", data, 0x30, 0)
            pointers.remove(0x30)
        struct.pack_into(
            "<I", data, 0x38, random_.choice((NONE, random_.getrandbits(16) << 16 | tag_index, random_.getrandbits(16) << 16 | tag_index))
        )
        struct.pack_into("<iI", data, 0x54, random_.choice((1, 1, 0, -1, 3)), 0x5C)
        struct.pack_into("<iI", data, 0x64, random_.choice((1, 1, 0, -1, 3)), 0x6C)
        first = random_.choice((None, Buffer(bytes(data), pointers=tuple(pointers)), Buffer(bytes(data), pointers=tuple(pointers))))
        t.check(
            "sound_unknown_00222930",
            random_.getrandbits(32),
            random_.choice((NONE, 0, random_.getrandbits(32))),
            Buffer(random_.randbytes(0x10)),
            globals={"global_tag_instances": tags_},
            extra=[first] if first else [],
            externals={
                "unknown_0018d090": lambda harness, arguments, first=first: (first.address if first else 0, tuple(arguments)),
            },
        )

    # ---------- a list for the console's sound, from the elements of a block of a tag

    random_ = random.Random(200)

    for _ in range(600):
        count = random_.choice((0, 1, 2, 3, 4, 4, -1))
        block = bytearray(struct.pack("<iI", count, 8)) + random_.randbytes(4 * 0x18)
        for element in range(4):
            struct.pack_into("<I", block, 8 + element * 0x18, random_.choice((0x11000138, 0x11000138, 0x11000139, 0, 0x1300012D)))
        overrides = bytearray(random_.randbytes(0x14 + 4 * 0x1C))
        struct.pack_into("<I", overrides, 4, 0x14)
        struct.pack_into("<I", overrides, 0x10, random_.choice((0, 0xF, 5, 0xFFFFFFF0, random_.getrandbits(32))))
        taken = [random_.choice((0, 0, 1, 3, 4, 5, 0x10, 0x25, 0x30)) for _ in range(16)]

        def entry_(harness, arguments, taken=taken):
            entry, element, override, unknown, number = arguments
            calls = calls_so_far(harness, "sound_effect_entry_build")
            harness.mu.mem_write(entry, random.Random(calls).randbytes(0xA) + struct.pack("<H", taken[calls % 16]))
            return 0, (harness.normalize(entry), harness.normalize(element), harness.normalize(override), unknown, number)

        t.check(
            "sound_unknown_002225a0",
            Buffer(bytes(overrides), pointers=(4,)),
            Buffer(bytes(block), pointers=(4,)),
            random_.getrandbits(32),
            Buffer(random_.randbytes(0x500)),
            Buffer(random_.randbytes(4)),
            externals={"sound_effect_entry_build": entry_},
        )

    # ---------- what a voice of the console's sound takes of a sound from the tags

    random_ = random.Random(201)

    for _ in range(1200):
        tag_index = random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        struct.pack_into("<I", tags, tag_index * 0x10 + 8, len(tags))
        tag = bytearray(random_.randbytes(0x40))
        struct.pack_into("<I", tag, 0x18, len(tags) + 0x20)
        tags_ = Buffer(bytes(tags) + bytes(tag), pointers=(tag_index * 0x10 + 8, len(tags) + 0x18))

        def collection__() -> tuple:
            """What one of the two functions gives, and whether the list can be made from it."""
            data = bytearray(random_.randbytes(0x34 + 0x28))
            for offset in (0x14, 0x1C, 0x24):
                struct.pack_into(
                    "<iI", data, offset, random_.choice((1, 1, 2, 0, -1, -0x80000000)), random_.choice((0, random_.getrandbits(32) | 1))
                )
            count = random_.choice((1, 1, 1, 2, 0, -1))
            has_elements = random_.random() < 0.9
            named = random_.random() < 0.85
            struct.pack_into("<iI", data, 0x2C, count, 0x34 if has_elements else 0)
            struct.pack_into("<I", data, 0x38, random_.getrandbits(16) << 16 | tag_index if named else NONE)
            struct.pack_into("<i", data, 0x54, random_.choice((1, 1, 1, 3, 0, -1)))
            return Buffer(bytes(data), pointers=(0x30,) if has_elements else ()), count > 0 and has_elements and named

        first, first_usable = random_.choice((None, collection__(), collection__(), collection__()))  or (None, False)
        second, _second_usable = random_.choice((None, collection__(), collection__())) or (None, False)
        if random_.random() < 0.1 and first:
            second = first
        if not first_usable and second is not first:
            # the game makes the list from the first: with nothing to make it from, the second has none either
            if second:
                second = Buffer(second.data[:0x38] + struct.pack("<I", NONE) + second.data[0x3C:], pointers=second.pointers)

        def list_(harness, arguments):
            overrides, block, unknown2, list__, size = arguments
            harness.mu.mem_write(size, struct.pack("<I", 0x123))
            return 0, (overrides, harness.normalize(block), unknown2, harness.normalize(list__), harness.normalize(size))

        t.check(
            "sound_unknown_00222770",
            random_.getrandbits(32),
            random_.choice((NONE, 0, random_.getrandbits(32))),
            random_.getrandbits(32),
            Buffer(random_.randbytes(0x420)),
            globals={"global_tag_instances": tags_},
            extra=[buffer for buffer in (first, second) if buffer and (buffer is first or second is not first)],
            externals={
                "unknown_0018d090": lambda harness, arguments, first=first: (first.address if first else 0, tuple(arguments)),
                "unknown_0018d170": lambda harness, arguments, second=second: (second.address if second else 0, tuple(arguments)),
                "sound_unknown_002225a0": list_,
            },
        )

    # ---------- what a voice of the console's sound takes of the sound of a channel

    random_ = random.Random(202)

    for _ in range(900):
        sound_index = random_.randrange(TAGS)
        classes_index = random_.choice([index for index in range(TAGS) if index != sound_index])
        data = bytearray(random_.randbytes(TAGS * 0x10))
        pointers = []

        def add(part: bytes) -> int:
            offset = len(data)
            data.extend(part)
            return offset

        def point(offset: int, to: int) -> None:
            struct.pack_into("<I", data, offset, to)
            pointers.append(offset)

        definition = bytearray(random_.randbytes(0x20))
        definition[2] = random_.randrange(CLASSES)
        point(sound_index * 0x10 + 8, add(bytes(definition)))
        classes = add(struct.pack("<2I", CLASSES, 0))
        point(classes_index * 0x10 + 8, classes)
        table = bytearray(random_.randbytes(CLASSES * CLASS_SIZE))
        for class_index in range(CLASSES):
            table[class_index * CLASS_SIZE + 0x4C] = random_.choice((0, 1, 1, 2, 0xFF))
            struct.pack_into("<H", table, class_index * CLASS_SIZE + 0xA, random_.choice((0, 2, 8, 0x200, 0x20A, 0xFDF5, random_.getrandbits(16))))
        point(classes + 4, add(bytes(table)))
        reference = add(struct.pack("<2I", random_.getrandbits(32), random_.getrandbits(16) << 16 | classes_index))
        game_globals = add(random_.randbytes(0xC0) + struct.pack("<2I", random_.choice((1, 1, 3)), 0))
        point(game_globals + 0xC4, reference)
        buffer = Buffer(bytes(data), pointers=tuple(pointers))

        time = random_.choice((0, 100000, 0x7FFF0000))
        playing_ = bytearray(random_.randbytes(DATUM))
        playing_[6] = random_.choice((0, 8, 0xF7, 0xFF, random_.getrandbits(8)))
        struct.pack_into("<I", playing_, 0xC, random_.getrandbits(16) << 16 | sound_index)
        playing_[0x19] = random_.choice((0, 1, 1, 0xFE, 0xFF))
        struct.pack_into("<f", playing_, 0x1C, random_.choice((0.0, 1.0, 0.5, random_.uniform(0, 2))))
        struct.pack_into("<i", playing_, 0x8C, time)
        playing_[0xA4] = random_.choice((0xFF, 0xFF, 0, 3, 0x80))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        struct.pack_into("<i", state, 0x84, time + random_.choice((0, 999, 1000, 1001, 5500, 60000, -1, -999, -1000, -2500)))
        heard = bytearray(random_.randbytes(0x38))
        heard[0] = random_.choice((0, 0x10, 0x10, 0xEF, 0xFF))
        struct.pack_into("<f", heard, 0x18, random_.choice((0.0, 0.25, 1.0, 0.75, 2.0, random_.uniform(0, 1))))

        def effects__(harness, arguments):
            index, unknown, played, result = arguments
            harness.mu.mem_write(result, bytes(range(0x30, 0x40)))
            return 0, (index, unknown, bytes(harness.mu.mem_read(played, 0x1C)), harness.normalize(result))

        def environment__(harness, arguments):
            index, unknown, result = arguments
            harness.mu.mem_write(result, bytes(range(0x50, 0x60)))
            return 0, (index, unknown, harness.normalize(result))

        t.check(
            "sound_channel_build_voice_parameters",
            random_.getrandbits(16) << 16 | random_.randrange(CHANNELS),
            Buffer(bytes(heard)),
            Buffer(random_.randbytes(0x478)),
            Buffer(bytes(playing_)),
            globals={
                "global_tag_instances": buffer,
                "global_game_globals": Pointer(buffer, game_globals),
                "sound_channels": Buffer(random_.randbytes(CHANNELS * 0x24)),
                "game_sound_globals": Buffer(bytes(state)),
            },
            externals={"sound_unknown_00222770": effects__, "sound_unknown_00222930": environment__},
        )

    # ---------- what is done each time with the sound of a channel that has a voice

    random_ = random.Random(203)

    for _ in range(900):
        shared = bytearray(random_.randbytes(0x50))
        pointers = []

        def block(offset: int, part: bytes) -> int:
            at = len(shared)
            struct.pack_into("<I", shared, offset, at)
            pointers.append(offset)
            shared.extend(part)
            return at

        scales = bytearray(random_.randbytes(4 * 0x14))
        for scale in range(4):
            struct.pack_into("<2h", scales, scale * 0x14 + 8, random_.randint(-1200, 1200), random_.randint(-1200, 1200))
        block(0xC, bytes(scales))
        natural = bytearray(random_.randbytes(4 * 0xA))
        for index in range(4):
            struct.pack_into("<h", natural, index * 0xA, random_.choice((0, 0, 100, -300)))
        block(0x1C, bytes(natural))
        ranges = bytearray(random_.randbytes(8 * 0xC))
        for index in range(8):
            struct.pack_into("<h", ranges, index * 0xC + 2, random_.randrange(4))
            struct.pack_into("<h", ranges, index * 0xC + 8, random_.randrange(6))
        block(0x24, bytes(ranges))
        recordings_ = bytearray(random_.randbytes(12 * 0x10))
        for index in range(12):
            recordings_[index * 0x10 + 4] = random_.choice((0, 0, 3, 0xFA, 0x80, 0x7F))
            struct.pack_into("<h", recordings_, index * 0x10 + 0xE, random_.choice((1, 2, 3, 3, 0)))
        permutations = block(0x2C, bytes(recordings_))
        channels = len(shared)
        shared += random_.randbytes(CHANNELS * 0x24)

        data = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        struct.pack_into("<I", data, 0x24, DATUM)
        struct.pack_into("<I", data, 0x44, 0x4C)
        indices = []
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            identifier_ = random_.getrandbits(16)
            struct.pack_into("<H", data, at, identifier_)
            data[at + 3] = random_.choice((0xFF, 0, 5))
            struct.pack_into("<H", data, at + 4, random_.choice((0, 0, 0x200, 0x200, 0x208, 0x8, 0x240, 0x40, 0xFDB7, random_.getrandbits(16))))
            data[at + 6] = random_.choice((0, 2, 2, 0xFD, 0xFF))
            struct.pack_into("<I", data, at + 0xC, random_.getrandbits(16) << 16 | random_.randrange(TAGS))
            struct.pack_into("<2f", data, at + 0x1C, random_.choice((0.0, 1.0, 0.5, 0.3)), random_.choice((0.0, -3.0, -70.0, 2.0)))
            data[at + 0x24 : at + 0x30] = reals(random_, 3)
            struct.pack_into("<2f", data, at + 0x90, random_.choice((0.0, 100.0, -250.5)), random_.choice((0.0, -6.0, 1.5)))
            data[at + 0x9C] = random_.choice((0, 1, 2))
            data[at + 0x9D] = random_.choice((0, 1, 3, 5))
            struct.pack_into("<h", data, at + 0x9E, random_.choice((0, 0, 1, 2, 3, -2, 0x7FFF)))
            data[at + 0xA1] = random_.randrange(4)
            struct.pack_into("<I", data, at + 0xA8, random_.choice((NONE, NONE, random_.getrandbits(16) << 16 | random_.randrange(CONTROLLERS))))
            indices.append(identifier_ << 16 | index)
        for channel_index in range(CHANNELS):
            at = channels + channel_index * 0x24
            struct.pack_into("<I", shared, at, random_.choice(indices))
            struct.pack_into("<h", shared, at + 0xC, random_.randrange(32))
            for field in (0x1C, 0x20):
                if random_.random() < 0.6:
                    struct.pack_into("<I", shared, at + field, permutations + random_.randrange(12) * 0x10)
                    pointers.append(at + field)
                else:
                    struct.pack_into("<I", shared, at + field, 0)
        buffer = Buffer(bytes(shared), pointers=tuple(pointers))

        tags = bytearray(random_.randbytes(TAGS * 0x10))
        tag_pointers = []
        definitions = bytearray(random_.randbytes(TAGS * 0x10))
        for tag in range(TAGS):
            struct.pack_into("<I", tags, tag * 0x10 + 8, TAGS * 0x10 + tag * 0x10)
            tag_pointers.append(tag * 0x10 + 8)
            struct.pack_into("<h", definitions, tag * 0x10 + 8, random_.randrange(0, 6))
            definitions[tag * 0x10 + 0xB] = random_.randrange(4)
        controllers = bytearray(random_.randbytes(0x4C + CONTROLLERS * 0x1C))
        struct.pack_into("<I", controllers, 0x44, 0x4C)
        for index in range(CONTROLLERS):
            at = 0x4C + index * 0x1C
            controllers[at + 3] = random_.choice((0, 1, 5, 0x80, 0x81, 0xFF))
            struct.pack_into("<I", controllers, at + 4, random_.choice((NONE, 0, random_.getrandbits(32))))
            controllers[at + 0xC : at + 0x1C] = reals(random_, 4)
        table__ = bytearray(random_.randbytes(0x100 * 0x5C))
        for class_index in range(0x100):
            struct.pack_into("<f", table__, class_index * 0x5C + 0x48, random_.choice((0.0, 1.0, 0.5)))
        table_ = Buffer(bytes(table__))
        heard = bytearray(random_.randbytes(0x38))
        struct.pack_into("<f", heard, 0x14, random_.choice((0.0, 1.0, 0.75)))
        loud = random_.choice((0.0, -6.0, -30.0, -63.5, -64.0, -80.0, 4.0, 0.5))
        asked_ = [random_.choice((1, 1, 0)) for _ in range(4)]

        def filled(harness, arguments):
            index, parameters, result, source_ = arguments
            before = bytes(harness.mu.mem_read(result, 0x478))
            harness.mu.mem_write(result, random.Random(index & 7).randbytes(0x478))
            return 0, (index & 0xFFFF, harness.normalize(parameters), before, harness.normalize(source_))

        def started(harness, arguments):
            parameters, effects, voice_index = arguments
            return 0, (bytes(harness.mu.mem_read(parameters, 0x478)), effects - parameters, voice_index)

        def goes_on__(harness, arguments):
            parameters, voice_index, effects, unknown = arguments
            return 0, (bytes(harness.mu.mem_read(parameters, 0x478)), voice_index, effects - parameters, unknown)

        t.check(
            "sound_channel_update_voice",
            random_.getrandbits(16) << 16 | random_.randrange(CHANNELS),
            as_bits(random_.choice((0.0, -3.0, -10.5, 2.0))),
            Buffer(bytes(heard)),
            globals={
                "sound_sources": Buffer(bytes(data), pointers=(0x44,)),
                "sound_channels": Pointer(buffer, channels),
                "global_sound_cache_file_gestalt": buffer,
                "global_tag_instances": Buffer(bytes(tags) + bytes(definitions), pointers=tuple(tag_pointers)),
                "unknown_0051ebd8": Buffer(bytes(controllers), pointers=(0x44,)),
                "unknown_0051ebe4": Buffer(random_.randbytes(32 * 0x34 + 0x10)),
            },
            extra=[table_],
            externals={
                "sound_definition_decibels": lambda harness, arguments, loud=loud: (
                    real_bits(loud),
                    (harness.normalize(arguments[0]), arguments[1], arguments[2]),
                ),
                "sound_channel_build_voice_parameters": filled,
                "sound_class_get": class_of(table_),
                "sound_listener_doppler_pitch": lambda harness, arguments: (
                    real_bits((arguments[0] & 3) * 17.5 - 20.0),
                    (arguments[0], harness.normalize(arguments[1])),
                ),
                "sound_console_voice_start": started,
                "sound_console_stream_set_function": lambda harness, arguments: (0, (bytes(harness.mu.mem_read(arguments[0], 0x10)),)),
                "sound_channel_queue_chunk": lambda harness, arguments: (
                    0,
                    (arguments[0] & 0xFFFF, harness.normalize(arguments[1]), arguments[2] & 0xFFFF),
                ),
                "sound_console_voice_set_parameters": goes_on__,
                "sound_source_request_data": lambda harness, arguments, asked_=asked_: (
                    asked_[calls_so_far(harness, "sound_source_request_data") % 4],
                    (harness.normalize(arguments[0]),),
                ),
                "sound_source_set_sound": lambda harness, arguments: (
                    0,
                    (arguments[0], arguments[1], arguments[2] & 0xFF, arguments[3] & 0xFF, arguments[4] & 0xFFFF),
                ),
            },
        )

    # ---------- how the sound of a channel is heard, for its voice of the console's sound

    random_ = random.Random(205)

    for n in range(1500):
        time = random_.choice((5000, 100000, 0x7FFF0000, 700))
        data = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        struct.pack_into("<I", data, 0x24, DATUM)
        struct.pack_into("<I", data, 0x44, 0x4C)
        table = len(data)
        data += random_.randbytes(0xC) + struct.pack("<I", mock_address(0) if random_.random() < 0.85 else 0)
        pointers = [0x44]
        indices = []
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            identifier_ = random_.getrandbits(16)
            struct.pack_into("<H", data, at, identifier_)
            data[at + 4] = random_.choice((0, 0, 0, 0x20, 0xDF, random_.getrandbits(8)))
            data[at + 6] = random_.choice((0, 2, 2, 0xFD, 0xFF))
            struct.pack_into("<I", data, at + 0xC, random_.getrandbits(16) << 16 | random_.randrange(TAGS))
            if random_.random() < 0.6:
                struct.pack_into("<I", data, at + 0x14, table)
                pointers.append(at + 0x14)
            else:
                struct.pack_into("<I", data, at + 0x14, 0)
            struct.pack_into("<H", data, at + 0x18, random_.choice((0, 0, 0x800, 0x100, 0x900, 0xF7FF, random_.getrandbits(16))))
            data[at + 0x1B] = random_.choice((0, 0, 1, 1, 1, 2, 2, 0x11, 0x12, 0x10, 3, 0xF, 0x9))
            data[at + 0x24 : at + 0x30] = reals(random_, 3)
            struct.pack_into("<I", data, at + 0x30, random_.choice((0, 0xFFFFFFFF, random_.getrandbits(32), random_.getrandbits(32))))
            data[at + 0x34 : at + 0x40] = reals(random_, 3)
            data[at + 0xA1] = random_.randrange(4)
            indices.append(identifier_ << 16 | index)
        channels = bytearray(random_.randbytes(CHANNELS * 0x24))
        for channel_index in range(CHANNELS):
            at = channel_index * 0x24
            struct.pack_into("<I", channels, at, random_.choice(indices))
            channels[at + 0xA] = random_.choice((0, 1, 2, 3, 0xFF))
            channels[at + 0xB] = random_.choice((0, 0, 1, 1, 2, 2, 3))
            struct.pack_into(
                "<i", channels, at + 0x14, random_.choice((0, 0, time, time - 1, time - 400, time - 1000, time - 1001, time - 5000, time + 300, -5, 1, -time - 500))
            )
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        struct.pack_into("<i", state, 0x84, time)
        for index in range(4):
            listener = LISTENERS + index * LISTENER_SIZE
            struct.pack_into("<f", state, listener + 8, random_.choice((1.0, 1.0, 0.5, 2.0)))
            state[listener + 0xC : listener + 0x3C] = reals(random_, 12)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        tag_pointers = []
        for tag in range(TAGS):
            struct.pack_into("<I", tags, tag * 0x10 + 8, TAGS * 0x10 + tag * 0x10)
            tag_pointers.append(tag * 0x10 + 8)
        table__ = bytearray(random_.randbytes(0x100 * 0x5C))
        for class_index in range(0x100):
            struct.pack_into("<f", table__, class_index * 0x5C + 0x14, random_.choice((0.0, -3.0, 2.5)))
        table_ = Buffer(bytes(table__))
        has_cone = random_.choice((1, 1, 0))
        heard_gain = random_.choice((0.0, 0.5, 1.0, 0.25))
        changes = random_.choice((1, 1, 0))

        def place_(harness, arguments):
            said_, listener_index, position = arguments
            harness.mu.mem_write(position, struct.pack("<3f", 1.5 + listener_index, -2.0, 0.25))
            return position, (harness.normalize(said_), listener_index)

        def spatial(harness, arguments, heard_gain=heard_gain):
            offset, said_, sound_index, forward = arguments
            record = bytes(harness.mu.mem_read(offset, 12)), harness.normalize(said_), sound_index, bytes(harness.mu.mem_read(forward, 12))
            return real_bits(heard_gain), record

        def fade__(harness, arguments):
            kind, milliseconds, duration = arguments
            value = bits_real(milliseconds) / 1000.0
            return real_bits(-64.0 * (1.0 - value) if bits_real(duration) > 0 else -64.0 * value), (kind & 0xFFFF, milliseconds, duration)

        def changed_(call, changes=changes):
            if changes:
                t.harness.mu.mem_write(call.stack[3] + 4, bytes(range(0x60, 0x70)))
            return changes

        t.check(
            "sound_channel_compute_parameters",
            random_.getrandbits(16) << 16 | random_.randrange(CHANNELS),
            Buffer(struct.pack("<f", random_.choice((0.0, -3.0, -10.5)))),
            Buffer(random_.randbytes(0x38)),
            globals={
                "sound_sources": Buffer(bytes(data), pointers=tuple(pointers)),
                "sound_channels": Buffer(bytes(channels)),
                "game_sound_globals": Buffer(bytes(state)),
                "global_tag_instances": Buffer(bytes(tags) + random_.randbytes(TAGS * 0x10), pointers=tuple(tag_pointers)),
                "unknown_00468788": Buffer(reals(random_, 3)),
            },
            extra=[table_],
            externals={
                "sound_source_get_position": place_,
                "sound_unknown_00125060": lambda harness, arguments, has_cone=has_cone: (
                    has_cone,
                    (harness.normalize(arguments[0]), arguments[1]),
                ),
                "sound_source_spatial_gain": spatial,
                "sound_gain_to_decibels": lambda harness, arguments: (
                    real_bits(-6.0 if arguments[0] == real_bits(0.5) else (-64.0 if arguments[0] == 0 else (-12.0 if arguments[0] == real_bits(0.25) else 0.0))),
                    tuple(arguments),
                ),
                "sound_unknown_0012a6d0": fade__,
                "sound_decibels_to_gain": lambda harness, arguments: (
                    real_bits(10.0 ** (max(bits_real(arguments[0]), -64.0) / 20.0)),
                    tuple(arguments),
                ),
                "sound_class_get": class_of(table_),
            },
            mocks=[
                Mock(
                    changed_,
                    stack_arguments=4,
                    record=lambda call: (call.stack[0], call.stack[1], bytes(t.harness.mu.mem_read(call.stack[3], 0x38))),
                )
            ],
        )

    # ---------- how loud a class of sounds is

    random_ = random.Random(206)

    def some_seconds() -> float:
        return random_.choice((0.0, 0.0, 0.0005, 0.001, 0.25, 0.5, 1.0, 1.0, 2.0, 3.5, random_.uniform(0, 4)))

    for _ in range(2500):
        gains_ = bytearray(reals(random_, 8 * 4))
        for class_index in range(8):
            gains_[class_index * 0x10 + 0xC] = random_.choice((0, 0, 0, 1, 2, 0xFE, 0xFF))
        table__ = bytearray(random_.randbytes(0x100 * 0x5C))
        for class_index in range(0x100):
            for state_index in range(3):
                struct.pack_into(
                    "<4f",
                    table__,
                    class_index * 0x5C + 0x28 + state_index * 0x10,
                    random_.choice((0.0, -6.0, -20.0, 3.0)),
                    some_seconds(),
                    some_seconds(),
                    some_seconds(),
                )
        table_ = Buffer(bytes(table__))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        current = random_.choice((NONE, 0, 1, 2))
        struct.pack_into("<I", state, 0x1F8, current)
        struct.pack_into("<I", state, 0x200, random_.choice((NONE, 0, 1, 2, current, current)))
        struct.pack_into("<f", state, 0x1FC, some_seconds())
        struct.pack_into("<f", state, 0x204, some_seconds())
        t.check(
            "sound_class_get_decibels",
            random_.getrandbits(16) << 16 | random_.randrange(8),
            globals={"sound_class_gains": Buffer(bytes(gains_)), "game_sound_globals": Buffer(bytes(state))},
            extra=[table_],
            externals={"sound_class_get": class_of(table_)},
        )

    # ---------- the two states that the globals of the sound keep at 0x1A8

    random_ = random.Random(207)

    def some_state(index: int) -> bytes:
        return struct.pack(
            "<2fI4f",
            random_.choice((0.0, 0.0005, 0.001, 0.5, 1.0, 2.0, -1.0)),
            random_.choice((0.0, 0.0, 0.0005, 0.001, 0.0011, 0.5, 1.0, 1.0, 0.99, 1.5)),
            index,
            random_.uniform(-4, 4),
            random_.uniform(-4, 4),
            random_.uniform(-2, 2),
            random_.uniform(-2, 2),
        )

    for _ in range(2500):
        things = (NONE, NONE, 0x10001, 0x20002, 0x30003, 0)
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        state[0x1A8:0x1E0] = some_state(random_.choice(things)) + some_state(random_.choice(things))
        struct.pack_into("<f", state, 0x1E0, random_.choice((0.0, 1.0 / 30.0, 1.0 / 30.0, 0.5, 2.0, 0.0001)))
        wanted = bytearray(some_state(random_.choice(things)) + some_state(random_.choice(things)))
        if random_.random() < 0.2:
            # both of the same thing
            wanted[0x1C + 8 : 0x1C + 0xC] = wanted[8:0xC]
        t.check("sound_unknown_states_set", Buffer(bytes(wanted)), globals={"game_sound_globals": Buffer(bytes(state))})

    # ---------- the console's sound takes another handler of the sound of the places of the map

    random_ = random.Random(208)

    def handler_mocks() -> list:
        mocks = []
        for _ in range(2):
            mocks += [
                Mock(lambda call: 0, stack_arguments=3, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=0, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=2, registers=("ecx",)),
            ]
        return mocks

    for _ in range(200):
        console = bytearray(random_.randbytes(0x2AC0))
        old = len(console)
        # the handler that it has, with its functions after it
        console += struct.pack("<4I", old + 4, mock_address(3), mock_address(4), mock_address(5))
        pointers = [old]
        if random_.random() < 0.7:
            struct.pack_into("<I", console, 0x2AB4, old)
            pointers.append(0x2AB4)
        else:
            struct.pack_into("<I", console, 0x2AB4, 0)
        t.check(
            "sound_console_set_environment_handler",
            Buffer(struct.pack("<4I", 4, mock_address(0), mock_address(1), mock_address(2)) + random_.randbytes(8), pointers=(0,)),
            globals={"unknown_0051ebe4": Buffer(bytes(console), pointers=tuple(pointers)), "unknown_00470060": GlobalRange(0x24)},
            mocks=handler_mocks(),
        )

    # ---------- a voice of the console's sound is stopped

    random_ = random.Random(209)

    for _ in range(300):
        console = bytearray(console_voices(8))
        pointers = []
        stream = len(console)
        console += struct.pack("<I", stream + 4) + random_.randbytes(0xC) + struct.pack("<I", mock_address(0))
        console += random_.randbytes(8) + struct.pack("<I", mock_address(1))
        pointers.append(stream)
        other = len(console)
        console += struct.pack("<I", other + 4) + random_.randbytes(0x1C) + struct.pack("<I", mock_address(2))
        pointers.append(other)
        for voice in range(8):
            at = 0xC + voice * 0x34
            console[at + 1] = random_.getrandbits(8)
            console[at + 3] = random_.getrandbits(8)
            struct.pack_into("<I", console, at + 0x24, stream)
            struct.pack_into("<I", console, at + 0x30, other)
            pointers += [at + 0x24, at + 0x30]
        console_ = Buffer(bytes(console), pointers=tuple(pointers))
        voice_index = random_.randrange(8)

        def asked__(call, console_=console_, voice_index=voice_index):
            # the flags of the voice while it is stopped
            return call.stack[0], t.harness.mu.mem_read(console_.address + 0xC + voice_index * 0x34 + 3, 1)[0]

        t.check(
            "sound_console_voice_stop",
            voice_index,
            random_.choice((0, 0, 1, 500, 0x7FFFFFFF, NONE, 0x80000000)),
            globals={"unknown_0051ebe4": console_},
            externals={
                "unknown_002ae8e0": console_call(0),
                "unknown_002ae6f0": console_call(0),
                "unknown_0037b822": console_call(0),
                "unknown_002ae7a0": console_call(0),
            },
            mocks=[
                Mock(lambda call: 0, stack_arguments=2, record=asked__),
                Mock(lambda call: 0, stack_arguments=1),
                Mock(lambda call: 0, stack_arguments=0, registers=("ecx",)),
            ],
        )

    # ---------- the classes of sounds whose name has some text are to be as loud as some decibels

    random_ = random.Random(210)

    texts = ("weapon", "projectile", "", "unit_dialog", "music", "x", "ambient_", "test", "multilingual_test", "e", "zzz", "_", "ui", "cinematic", "dialog", "weapon_fire!", "huge_ass", "ass")
    # each name of a class, whole: a name that is not as the game has it is found out
    texts += ("projectile_impact", "projectile_detonation", "projectile_flyby", "weapon_fire", "weapon_ready", "weapon_reload")
    texts += ("weapon_empty", "weapon_charge", "weapon_overheat", "weapon_idle", "weapon_melee", "weapon_animation")
    texts += ("object_impacts", "particle_impacts", "unit_footsteps", "unit_animation", "vehicle_collision", "vehicle_engine")
    texts += ("vehicle_animation", "device_door", "device_machinery", "device_stationary", "ambient_nature", "ambient_machinery")
    texts += ("object_looping", "cinematic_music", "cortana_mission", "cortana_cinematic", "mission_dialog", "cinematic_dialog")
    texts += ("scripted_cinematic_foley", "game_event")
    for text in texts:
        for _ in range(12):
            t.check(
                "sound_classes_set_decibels",
                Buffer(text.encode() + b"\0" + random_.randbytes(3)),
                as_bits(random_.choice((0.0, -0.0, -6.0, -64.0, -64.5, -100.0, 3.0, 0.001, float("nan"), random_.uniform(-64, 0)))),
                as_bits(random_.choice((0.0, 0.0, -0.0, 1.5, 0.001, -1.0, float("nan"), 30.0))),
                globals={"sound_class_gains": Buffer(reals(random_, 54 * 4))},
            )

    # ---------- how the samples of a sound of a kind are, for the console

    random_ = random.Random(211)

    for kind in (0, 1, 2, 3, NONE, 0x102, 0x80000000, 5) * 4:
        t.check(
            "sound_console_get_format",
            kind,
            Buffer(random_.randbytes(0x14)),
            Buffer(random_.randbytes(4), places=(0,)),
            globals={"unknown_0047f0d0": GlobalRange(0x20), "unknown_0047f0f0": GlobalRange(0x20)},
        )

    # ---------- what the console calls for the stream of a voice

    random_ = random.Random(212)

    for _ in range(500):
        count = random_.choice((8, 8, 4, 1, 0, -1))
        console = bytearray(console_voices(count))
        other = len(console)
        console += struct.pack("<I", other + 4) + random_.randbytes(0x18) + struct.pack("<I", mock_address(0)) + random_.randbytes(4)
        pointers = [other]
        for voice in range(8):
            at = 0xC + voice * 0x34
            console[at + 3] = random_.choice((0, 0, 4, 8, 0xC, 0xF3, 0xFF, random_.getrandbits(8)))
            struct.pack_into("<I", console, at + 0x30, other)
            pointers.append(at + 0x30)
        t.check(
            "sound_console_voice_stream_callback",
            random_.choice((0, 1, 3, 7, 7, 8, NONE, 0x80000000, random_.randrange(8), random_.randrange(8))),
            random_.choice((0, 1, random_.getrandbits(32), random_.getrandbits(32) | 1)),
            random_.choice((0, 0, 1, random_.getrandbits(32))),
            globals={"unknown_0051ebe4": Buffer(bytes(console), pointers=tuple(pointers))},
            externals={"unknown_002ae820": console_call(0)},
            mocks=[Mock(lambda call: 0, stack_arguments=3, registers=("ecx",))],
        )

    # ---------- a voice of the console's sound is given its stream

    random_ = random.Random(213)

    def made_(harness, arguments):
        voice, thing, description, kind = arguments
        flags, packets, format_, callback, context, unknown14 = struct.unpack("<6I", harness.mu.mem_read(description, 0x18))
        record = (
            harness.normalize(voice),
            harness.normalize(thing),
            flags,
            packets,
            # 0x12 bytes of the format are set for the samples that are not compressed
            bytes(harness.mu.mem_read(format_, 0x14 if kind != 2 else 0x12)),
            harness.normalize(callback),
            context,
            unknown14,
            kind,
        )
        return 1, record

    for kind in (0, 1, 2, 3, NONE, 0x102) * 8:
        t.check(
            "sound_console_voice_create_stream",
            kind,
            random_.randrange(8),
            globals={
                "unknown_0051ebe4": Buffer(bytes(console_voices(8))),
                "unknown_0047f0d0": GlobalRange(0x20),
                "unknown_0047f0f0": GlobalRange(0x20),
            },
            externals={"unknown_002ae660": made_},
        )

    # ---------- how loud a pitch range is played at a pitch that is out of its own

    random_ = random.Random(214)

    for _ in range(1500):
        shared = bytearray(random_.randbytes(0x50))
        struct.pack_into("<I", shared, 0x1C, len(shared))
        bounds = sorted(random_.randint(-1200, 1200) for _ in range(4))
        if random_.random() < 0.2:
            random_.shuffle(bounds)
        low_start, low, high, high_end = bounds
        # nothing between where it comes up from and where it is all there
        if random_.random() < 0.2:
            low_start = low
        if random_.random() < 0.2:
            high_end = high
        for index in range(4):
            shared += struct.pack("<5h", random_.randint(-100, 100), low_start, high_end, low, high) if index == 2 else random_.randbytes(0xA)
        pitch_range = bytearray(random_.randbytes(0xC))
        struct.pack_into("<h", pitch_range, 2, 2)
        pitch = random_.choice(
            (
                float(low),
                float(high),
                float(low_start),
                float(high_end),
                low - 0.00005,
                low - 0.0002,
                high + 0.00005,
                high + 0.0002,
                (low_start + low) / 2.0,
                (high + high_end) / 2.0,
                random_.uniform(-1500, 1500),
                random_.uniform(-1500, 1500),
            )
        )
        t.check(
            "sound_pitch_range_fade_decibels",
            Buffer(bytes((random_.getrandbits(8), random_.choice((0x20, 0x20, 0x20, 0xFF, 0, 0xDF))))),
            Buffer(bytes(pitch_range)),
            as_bits(pitch),
            globals={"global_sound_cache_file_gestalt": Buffer(bytes(shared), pointers=(0x1C,))},
        )

    # ---------- all the voices of the console's sound are stopped

    random_ = random.Random(215)

    for _ in range(200):
        console = bytearray(console_voices(random_.choice((8, 8, 3, 1, 0, -1))))
        pointers = []
        stream = len(console)
        console += struct.pack("<I", stream + 4) + random_.randbytes(0xC) + struct.pack("<I", mock_address(0))
        console += random_.randbytes(8) + struct.pack("<I", mock_address(1))
        pointers.append(stream)
        other = len(console)
        console += struct.pack("<I", other + 4) + random_.randbytes(0x1C) + struct.pack("<I", mock_address(2))
        pointers.append(other)
        for voice in range(8):
            at = 0xC + voice * 0x34
            console[at + 1] = random_.getrandbits(8)
            console[at + 2] = random_.choice((0, 7, 0x38, 0xC0, 0xFF, random_.getrandbits(8)))
            console[at + 3] = random_.getrandbits(8)
            struct.pack_into("<I", console, at + 0x24, stream)
            struct.pack_into("<I", console, at + 0x30, other)
            pointers += [at + 0x24, at + 0x30]
        states = [random_.choice((0, 0, 1, 1, 2, 9)) for _ in range(16)]

        def halted(harness, arguments, states=states):
            # what the voice does after: it can play again
            calls = calls_so_far(harness, "unknown_002ae8e0")
            flags = harness.mu.mem_read(arguments[0] + 1, 4)
            harness.mu.mem_write(arguments[0] + 1, bytes((flags[0] | states[calls % 16],)))
            return 0, (harness.normalize(arguments[0]), bytes(flags))

        t.check(
            "sound_console_voices_stop",
            globals={"unknown_0051ebe4": Buffer(bytes(console), pointers=tuple(pointers))},
            externals={"unknown_002ae8e0": halted},
            mocks=[
                Mock(lambda call: 0, stack_arguments=2, record=lambda call: (call.stack[0],)),
                Mock(lambda call: 0, stack_arguments=1),
                Mock(lambda call: 0, stack_arguments=0, registers=("ecx",)),
            ],
        )

    # ---------- the console's sound is let go

    random_ = random.Random(216)

    for _ in range(200):
        console = bytearray(console_voices(random_.choice((8, 8, 3, 1, 0, -1))))
        struct.pack_into("<h", console, 6, random_.choice((0, 1, 5, 16, -1)))
        pointers = []
        stream = len(console)
        console += struct.pack("<2I", stream + 4, random_.getrandbits(32)) + struct.pack("<I", mock_address(0)) + random_.randbytes(8)
        pointers.append(stream)
        for voice in range(8):
            at = 0xC + voice * 0x34
            if random_.random() < 0.7:
                struct.pack_into("<I", console, at + 0x24, stream)
                pointers.append(at + 0x24)
            else:
                struct.pack_into("<I", console, at + 0x24, 0)
        for slot in range(16):
            at = 0x1A18 + slot * 0x3C
            struct.pack_into("<2I", console, at + 0x34, random_.choice((0, random_.getrandbits(32) | 1)), random_.choice((0, random_.getrandbits(32) | 1)))
        struct.pack_into("<I", console, 0x2AB0, random_.choice((0, random_.getrandbits(32) | 1, random_.getrandbits(32) | 1)))
        t.check(
            "sound_console_dispose",
            globals={
                "unknown_0051ebe4": Buffer(bytes(console), pointers=tuple(pointers)),
                "xmem_static_buffer": struct.pack("<I", random_.getrandbits(32)),
                "unknown_0047f0d0": GlobalRange(0x20),
            },
            externals={
                "sound_console_voices_stop": lambda harness, arguments: 0,
                "unknown_00379f45": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_00379f2a": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_002ae170": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
            },
            mocks=[Mock(lambda call: 0, stack_arguments=1)],
        )

    # ---------- the list for what unknown_0018d090 gives for no sound, and a real as 24 bits

    random_ = random.Random(217)

    for _ in range(600):
        tag_index = random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        struct.pack_into("<I", tags, tag_index * 0x10 + 8, len(tags))
        tag = bytearray(random_.randbytes(0x40))
        struct.pack_into("<I", tag, 0x18, len(tags) + 0x20)
        tags_ = Buffer(bytes(tags) + bytes(tag), pointers=(tag_index * 0x10 + 8, len(tags) + 0x18))
        data = bytearray(random_.randbytes(0x34 + 0x28))
        has_elements = random_.random() < 0.9
        struct.pack_into("<iI", data, 0x2C, random_.choice((1, 1, 1, 2, 0, -1, -0x80000000)), 0x34 if has_elements else 0)
        struct.pack_into("<I", data, 0x38, random_.choice((NONE, random_.getrandbits(16) << 16 | tag_index, random_.getrandbits(16) << 16 | tag_index)))
        struct.pack_into("<i", data, 0x54, random_.choice((1, 1, 1, 3, 0, -1)))
        first = random_.choice((None, Buffer(bytes(data), pointers=(0x30,) if has_elements else ()), Buffer(bytes(data), pointers=(0x30,) if has_elements else ())))

        def list__(harness, arguments):
            overrides, block, unknown2, list___, size = arguments
            harness.mu.mem_write(size, struct.pack("<I", 0x77))
            return 0, (overrides, harness.normalize(block), unknown2, harness.normalize(list___), harness.normalize(size))

        t.check(
            "sound_unknown_002228d0",
            random_.choice((NONE, 0, random_.getrandbits(32))),
            Buffer(random_.randbytes(0x404)),
            random_.getrandbits(32),
            globals={"global_tag_instances": tags_},
            extra=[first] if first else [],
            externals={
                "unknown_0018d090": lambda harness, arguments, first=first: (first.address if first else 0, tuple(arguments)),
                "sound_unknown_002225a0": list__,
            },
        )

    values = [0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 1.5, -1.5, 100.0, -100.0, 1e-9, -1e-9, 0.99999994, -0.99999994, float("nan"), float("inf"), float("-inf")]
    values += [random_.uniform(-1.2, 1.2) for _ in range(400)]
    for value in values:
        t.check("sound_real_to_fixed24", as_bits(value))

    # ---------- what the game gives to start a sound becomes that of a sound of a sound effect

    random_ = random.Random(218)

    # the table of who is told: the same functions, in the same places, as the game's
    harness = t.harness
    tables = []
    for original in (True, False):
        names = {0: None}
        for n, name in enumerate(harness.externals):
            names[harness.symbols[name].address if original else harness_external_address(n)] = name
        for name, symbol in harness.symbols.items():
            if symbol.source:
                names[symbol.address if original else harness.functions[name]] = name
        address = harness.globals["sound_effect_callbacks"] if original else harness.variables["sound_effect_callbacks"]
        tables.append([names.get(value, hex(value)) for value in struct.unpack("<8I", harness.mu.mem_read(address, 0x20))])
    if not t.only or "sound_effect_prepare_request" in t.only:
        t.cases["sound_effect_prepare_request"] = t.cases.get("sound_effect_prepare_request", 0) + 1
        if tables[0] != tables[1]:
            t._fail("sound_effect_prepare_request", f"sound_effect_callbacks: the game has {tables[0]}, source/ has {tables[1]}")

    for _ in range(600):
        effects = bytearray(random_.randbytes(0x4C + EFFECTS * 0x14))
        struct.pack_into("<I", effects, 0x44, 0x4C)
        request = bytearray(random_.randbytes(0xA0))
        struct.pack_into("<I", request, 0, random_.choice((0, 0x10, 0x20, 0x30, 0xFFFFFFCF, random_.getrandbits(32))))
        got = random_.choice((NONE, NONE, 0x10003, 5))
        t.check(
            "sound_effect_prepare_request",
            random_.getrandbits(16) << 16 | random_.randrange(EFFECTS),
            Buffer(bytes(request), places=(0x58, 0x88)),
            globals={"sound_effects": Buffer(bytes(effects), pointers=(0x44,)), "sound_effect_callbacks": GlobalRange(0x20)},
            externals={
                "sound_playback_controller_acquire": lambda harness, arguments, got=got: (got, tuple(arguments)),
                "sound_playback_controller_new": lambda harness, arguments, got=got: (got, tuple(arguments)),
            },
        )

    # ---------- what a sound effect is told of its sounds

    random_ = random.Random(219)

    def told_bytes(table: bool, functions: tuple) -> Buffer:
        """The bytes for who is told of a sound of a sound effect, with who was to be told before after them."""
        data = bytearray(random_.randbytes(0x30))
        struct.pack_into("<I", data, 0x28, random_.getrandbits(16) << 16 | random_.randrange(EFFECTS))
        struct.pack_into("<I", data, 0x2C, 0x30 if table else 0)
        data += struct.pack("<8I", *(mock_address(n) if function else 0 for n, function in enumerate(functions)))
        return Buffer(bytes(data), pointers=(0x2C,) if table else ())

    for _ in range(300):
        effects = bytearray(random_.randbytes(0x4C + EFFECTS * 0x14))
        struct.pack_into("<I", effects, 0x44, 0x4C)
        t.check(
            "sound_effect_source_unknown0",
            random_.getrandbits(32),
            random_.getrandbits(32),
            told_bytes(False, (False,) * 8),
            Buffer(random_.randbytes(0x48)),
            globals={"sound_effects": Buffer(bytes(effects), pointers=(0x44,))},
            externals={"sound_effect_get_source": lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1])))},
        )
    for _ in range(200):
        table = random_.random() < 0.8
        functions = tuple(random_.random() < 0.8 for _ in range(8))
        for name, arguments in (
            ("sound_effect_source_got_channel", lambda told_: (random_.getrandbits(32), random_.getrandbits(32), random_.getrandbits(32), told_)),
            (
                "sound_effect_source_playing",
                lambda told_: (random_.getrandbits(32), told_, random_.getrandbits(32), random_.getrandbits(32), random_.getrandbits(32), as_bits(1.5)),
            ),
            ("sound_effect_source_heard", lambda told_: (random_.getrandbits(32), random_.getrandbits(32), told_, Buffer(random_.randbytes(0x38)))),
        ):
            random_before = random.Random(random_.getrandbits(32))
            answer = random_before.choice((0, 1, 1))
            mocks = [
                Mock(lambda call: 0, stack_arguments=4),
                Mock(lambda call: 0, stack_arguments=4),
                Mock(lambda call: 0, stack_arguments=6),
                Mock(lambda call, answer=answer: answer, stack_arguments=4),
            ]
            t.check(name, *arguments(told_bytes(table, functions)), mocks=mocks)
    for _ in range(200):
        buffer, indices = playing()
        t.check(
            "sound_effect_source_stopped",
            random_.getrandbits(32),
            random_.choice(indices),
            globals={"sound_sources": buffer},
            externals={"sound_effect_source_deleted": lambda harness, arguments: (0, tuple(arguments))},
        )

    # ---------- the sound playback controllers are set up

    random_ = random.Random(220)

    for give in (True, True, True, False):
        for allocated in (0, 0x1290, 0x1291, 0x1292, 0x1293, 0x345):
            memory = Buffer(random_.randbytes(0x4C)) if give else None
            state = bytearray(random_.randbytes(GLOBALS_SIZE))
            state[0x7F] = random_.choice((0, 1, 2, 0xFF, 0xFE))
            when, some, ticks = random_.getrandbits(31), random_.getrandbits(15), random_.getrandbits(32)
            t.check(
                "sound_playback_controllers_initialize",
                globals={
                    "shell_memory_globals": shell_memory(allocated),
                    "game_sound_globals": Buffer(bytes(state)),
                    "unknown_0051ebd8": random_.getrandbits(32),
                    "unknown_0051ebdc": random_.getrandbits(32),
                },
                extra=[memory] if memory else [],
                memory={STATE: Memory(random_.randbytes(STATE_SIZE))},
                externals={
                    "shell_memory_data_new": new_array(memory),
                    "data_delete_all": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                    "time": lambda harness, arguments, when=when: (when, tuple(arguments)),
                    "rand": lambda harness, arguments, some=some: some,
                    "GetTickCount": lambda harness, arguments, ticks=ticks: ticks,
                },
            )

    # ---------- a piece of the samples of a sound is asked for

    random_ = random.Random(221)
    PIECES = 12

    for _ in range(900):
        elements_ = bytearray(random_.randbytes(0x4C + PIECES * 0x10))
        struct.pack_into("<I", elements_, 0x44, 0x4C)
        for piece in range(PIECES):
            elements_[0x4C + piece * 0x10 + 2] = random_.choice((0, 0, 1, 1, 0x80))
            elements_[0x4C + piece * 0x10 + 3] = random_.choice((0, 0, 1, 5))
            elements_[0x4C + piece * 0x10 + 4] = random_.choice((0, 1, 0xFF, 7))
        elements = Buffer(bytes(elements_), pointers=(0x44,))
        cache = bytearray(random_.randbytes(0x70))
        struct.pack_into("<I", cache, 0x64, len(cache))
        cache += random_.randbytes(0x4C)
        struct.pack_into("<I", cache, 0x70 + 0x44, len(cache))
        cache += random_.randbytes(PIECES * 0x18)
        piece = random_.randrange(PIECES)
        index = random_.choice((NONE, NONE, random_.getrandbits(16) << 16 | piece, random_.getrandbits(16) << 16 | piece))
        loaded = random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(PIECES), random_.getrandbits(16) << 16 | random_.randrange(PIECES)))
        turns = random_.choice((1, 2, 5))

        def load_(harness, arguments, loaded=loaded):
            chunk, sound_index = arguments
            harness.mu.mem_write(chunk + 8, struct.pack("<I", loaded))
            return 0, (harness.normalize(chunk), sound_index)

        def wait_(harness, arguments, elements=elements, turns=turns):
            if calls_so_far(harness, "SwitchToThread") + 1 >= turns:
                # the other thread is done with all of them
                for piece_ in range(PIECES):
                    harness.mu.mem_write(elements.address + 0x4C + piece_ * 0x10 + 2, b"\x01")
            return 0

        t.check(
            "sound_cache_chunk_request",
            random_.choice((NONE, random_.getrandbits(32), random_.getrandbits(32))),
            Buffer(random_.randbytes(8) + struct.pack("<I", index)),
            random_.choice((0, 1, 2, 3, 4, 5, 6, 7, 0xFFFFFFF8, random_.getrandbits(32))),
            globals={
                "unknown_00502104": elements,
                "sound_lruv_cache": Buffer(bytes(cache), pointers=(0x64, 0x70 + 0x44)),
            },
            externals={"sound_cache_chunk_load": load_, "SwitchToThread": wait_},
        )

    # ---------- the cache of the sound is made, and what it asks of its pieces

    random_ = random.Random(222)

    for give_array in (True, True, False):
        for case in range(8):
            blocks_memory = Buffer(random_.randbytes(0x208C))
            cache_memory = Buffer(random_.randbytes(0x30FC), places=(0x20, 0x24, 0x28))
            allocator = Buffer(struct.pack("<3I", 4, mock_address(0), mock_address(1)), pointers=(0,))

            def allocate_(call, give_array=give_array, memory={0x208C: blocks_memory, 0x30FC: cache_memory}):
                if call.stack[0] != 0x30FC and not give_array:
                    return 0
                return memory[call.stack[0]].address

            stage = random_.randrange(5)
            lows = [random_.getrandbits(26) & ~0xFFF for _ in range(5)]
            highs = [random_.getrandbits(26) & ~0xFFF for _ in range(5)]
            lows[stage] = random_.choice((0x1000000, 0x1000000, 0x61000, 0))
            highs[stage] = lows[stage] + random_.choice((0x300000, 0x300000, 0x2FF000, 0x800000, 0x301000, 0))
            t.check(
                "sound_cache_initialize",
                globals={
                    "heap_allocation": allocator,
                    "lruv_blocks_allocation": random_.choice((0, 0x60F00100)),
                    "physical_memory_globals": struct.pack("<3I5I5I", stage, 0x61000, 0x3145000, *lows, *highs),
                    "unknown_00502104": random_.getrandbits(32),
                    "sound_lruv_cache": random_.getrandbits(32),
                    "sound_cache_base": struct.pack("<I", random_.getrandbits(32)),
                },
                mocks=[
                    Mock(allocate_, stack_arguments=1, registers=("ecx",)),
                    Mock(lambda call: 0, stack_arguments=1, registers=("ecx",)),
                ],
                extra=[blocks_memory, cache_memory],
                externals={"XPhysicalProtect": lambda harness, arguments: (0, tuple(arguments))},
            )

    for _ in range(400):
        elements_ = bytearray(random_.randbytes(0x4C + PIECES * 0x10))
        struct.pack_into("<I", elements_, 0x44, 0x4C)
        pointers = [0x44]
        chunks_ = len(elements_)
        elements_ += random_.randbytes(PIECES * 0xC)
        for piece in range(PIECES):
            at = 0x4C + piece * 0x10
            elements_[at + 2] = random_.choice((0, 1, 1, 0x80))
            elements_[at + 4] = random_.choice((0, 0, 0, 1, 0xFF))
            elements_[at + 5] = random_.choice((0, 0, 0, 1, 0xFF))
            struct.pack_into("<I", elements_, at + 0xC, chunks_ + random_.randrange(PIECES) * 0xC)
            pointers.append(at + 0xC)
        index = random_.getrandbits(16) << 16 | random_.randrange(PIECES)
        t.check("sound_cache_block_locked", index, globals={"unknown_00502104": Buffer(bytes(elements_), pointers=tuple(pointers))})
        t.check(
            "sound_cache_block_deleted",
            index,
            globals={"unknown_00502104": Buffer(bytes(elements_), pointers=tuple(pointers))},
            externals={"datum_delete": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1]))},
        )

    # ---------- the cache of the sound is closed

    random_ = random.Random(223)

    for _ in range(300):
        count = random_.choice((PIECES, PIECES, PIECES - 4, 1, 0))
        elements_ = bytearray(random_.randbytes(0x4C + PIECES * 0x10))
        elements_[0x29] = random_.choice((1, 1, 0xFF))
        struct.pack_into("<I", elements_, 0x24, 0x10)
        struct.pack_into("<i", elements_, 0x38, count)
        struct.pack_into("<I", elements_, 0x44, 0x4C)
        pointers = [0x44, 0x48]
        struct.pack_into("<I", elements_, 0x48, len(elements_))
        elements_ += struct.pack("<I", random_.getrandbits(32) | random_.getrandbits(32))
        chunks_ = len(elements_)
        elements_ += random_.randbytes(PIECES * 0xC)
        for piece in range(PIECES):
            at = 0x4C + piece * 0x10
            elements_[at + 2] = random_.choice((0, 1, 1, 1, 0x80))
            elements_[at + 4] = random_.choice((0, 0, 0, 0, 1, 0xFF))
            elements_[at + 5] = random_.choice((0, 0, 0, 0, 1))
            struct.pack_into("<I", elements_, at + 0xC, chunks_ + piece * 0xC)
            pointers.append(at + 0xC)
            struct.pack_into("<I", elements_, chunks_ + piece * 0xC + 8, random_.choice((NONE, random_.getrandbits(32), random_.getrandbits(32))))
        elements = Buffer(bytes(elements_), pointers=tuple(pointers))
        tasks = bytearray(random_.randbytes(0x25A0))
        struct.pack_into("<I", tasks, 0x2598, 0)
        turns = random_.choice((1, 2, 3, 1000))
        times = [1000]
        for _time in range(40):
            times.append(times[-1] + random_.choice((0, 10, 500, 2000, 4999, 5000)))

        def waited_(harness, arguments, elements=elements, turns=turns):
            if calls_so_far(harness, "WaitForSingleObject") + 1 >= turns:
                # the thread of the tasks is done with all of them, and nothing holds them
                for piece_ in range(PIECES):
                    harness.mu.mem_write(elements.address + 0x4C + piece_ * 0x10 + 2, b"\x01")
                    harness.mu.mem_write(elements.address + 0x4C + piece_ * 0x10 + 4, b"\x00\x00")
            return 0, tuple(arguments)

        t.check(
            "sound_cache_close",
            globals={
                "unknown_00502104": elements,
                "sound_lruv_cache": random_.getrandbits(32),
                "async_globals": bytes(tasks),
            },
            externals={
                "GetTickCount": clock(tuple(times)),
                "WaitForSingleObject": waited_,
                "ReleaseMutex": lambda harness, arguments: (0, tuple(arguments)),
                "lruv_block_delete": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- the sounds of a ring start to fade out, in some seconds

    random_ = random.Random(224)

    for _ in range(150):
        looping, playing_, indices = ring_world()
        sound_index = random_.randrange(TAGS)
        data = bytearray(playing_.data)
        for n in range(PLAYING):
            at = 0x4C + n * DATUM
            data[at + 4] = random_.choice((0, 0x80, 0x7F, 0xFF))
            struct.pack_into("<I", data, at + 0xC, random_.getrandbits(16) << 16 | sound_index)
        globals_ = world(random_, sound_index, sound_data(random_))
        for first in (NONE, *random_.sample(indices, 4)):
            t.check(
                "sound_source_ring_fade_out_in_seconds",
                as_bits(random_.choice((0.0, 0.5, 1.0, 2.5, 0.0005, 0.0015, -1.0, 30.0))),
                first,
                globals={**globals_, "sound_sources": Buffer(bytes(data), pointers=playing_.pointers)},
                externals={"sound_source_fade_decibels": lambda harness, arguments: (fade_bits(arguments[0]), tuple(arguments))},
            )

    # ---------- a place at random for a sound of a looping sound that plays now and then

    random_ = random.Random(225)

    for _ in range(1500):
        detail = bytearray(random_.randbytes(0x34))
        for offset in (0x1C, 0x24):
            low = random_.choice((0.0, -3.14159, -1.0, 0.5, random_.uniform(-3.2, 3.2)))
            struct.pack_into("<2f", detail, offset, low, random_.choice((low, low + 1.0, 3.14159, random_.uniform(-3.2, 3.2))))
        low = random_.choice((0.0, 0.0, 1.0, 5.0, random_.uniform(0, 10)))
        struct.pack_into("<2f", detail, 0x2C, low, random_.choice((low, low, low + 2.0, 0.0, random_.uniform(0, 10))))
        t.check(
            "looping_sound_detail_random_position",
            Buffer(bytes(detail)),
            Buffer(random_.randbytes(12)),
            globals={"global_random_seed_reference": Buffer(struct.pack("<4I", *(random_.getrandbits(32) for _ in range(4))))},
        )

    # ---------- the sounds of a looping sound that play now and then

    random_ = random.Random(227)

    harness = t.harness
    tables = []
    for original in (True, False):
        names = {0: None}
        for n, name in enumerate(harness.externals):
            names[harness.symbols[name].address if original else harness_external_address(n)] = name
        for name, symbol in harness.symbols.items():
            if symbol.source:
                names[symbol.address if original else harness.functions[name]] = name
        address = harness.globals["looping_sound_callbacks"] if original else harness.variables["looping_sound_callbacks"]
        tables.append([names.get(value, hex(value)) for value in struct.unpack("<8I", harness.mu.mem_read(address, 0x20))])
    if not t.only or "looping_sound_detail_request" in t.only:
        t.cases["looping_sound_detail_request"] = t.cases.get("looping_sound_detail_request", 0) + 1
        if tables[0] != tables[1]:
            t._fail("looping_sound_detail_request", f"looping_sound_callbacks: the game has {tables[0]}, source/ has {tables[1]}")

    for _ in range(400):
        first = [random_.uniform(-5, 5) for _ in range(3)]
        second = [value + random_.choice((0.0, 0.0, 0.00005, -0.00005, 0.0001, 0.0002, -0.0002, 1.0)) for value in first]
        t.check(
            "looping_sound_source_matches",
            Buffer(struct.pack("<3f", *first) + random_.randbytes(4)),
            Buffer(struct.pack("<3f", *second) + random_.randbytes(4)),
        )

    def looping_world() -> tuple:
        """The looping sounds: 6 of them, some with no place, with their indices."""
        data = bytearray(random_.randbytes(0x4C + 6 * 0xD4))
        struct.pack_into("<I", data, 0x24, 0xD4)
        struct.pack_into("<i", data, 0x38, random_.choice((6, 6, 6, 3)))
        struct.pack_into("<I", data, 0x44, 0x4C)
        indices = []
        for n in range(6):
            at = 0x4C + n * 0xD4
            identifier_ = random_.choice((0, 1, 0x7FFF, 0x8001, 0xFFFF, random_.getrandbits(16)))
            struct.pack_into("<H", data, at, identifier_)
            data[at + 0xF] = random_.choice((0, 0x10, 1, 1, 0x11, 2, 0xF1))
            data[at + 0x18 : at + 0x24] = reals(random_, 3)
            indices.append(identifier_ << 16 | n)
        return Buffer(bytes(data), pointers=(0x44,)), indices

    for _ in range(900):
        looping, indices = looping_world()
        index = random_.choice((NONE, *indices, *indices, indices[0] ^ 0x10000, 6, 0x10007))
        said = bytearray(random_.randbytes(0x48))
        said[3] = random_.choice((0, 1, 1, 0x11, 2, 0xF1, 0xF0))
        said[0xC:0x18] = reals(random_, 3)
        t.check(
            "looping_sound_source_unknown0",
            index,
            random_.getrandbits(32),
            Buffer(reals(random_, 3)),
            Buffer(bytes(said)),
            globals={"looping_sounds": looping},
        )

    for _ in range(900):
        looping, indices = looping_world()
        sound_index = random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        struct.pack_into("<I", tags, sound_index * 0x10 + 8, len(tags))
        definition = bytearray(random_.randbytes(0x10))
        definition[4] = random_.choice((0, 0, 1, 0xFF))
        detail = bytearray(random_.randbytes(0x34))
        struct.pack_into("<I", detail, 8, random_.getrandbits(16) << 16 | sound_index)
        detail[0x1C:0x2C] = struct.pack("<4f", -1.0, 2.0, -0.5, 0.5)
        struct.pack_into("<2f", detail, 0x2C, random_.choice((0.0, 1.0, 3.0)), random_.choice((0.0, 4.0, 8.0)))
        players = bytearray(random_.randbytes(0xC))
        struct.pack_into("<h", players, 8, random_.choice((0, 1, 2, 2, 4, 4, -1)))
        local = [NONE if random_.random() < 0.4 else random_.getrandbits(32) for _ in range(4)]
        players += struct.pack("<4I", *local)
        first_player = next((n for n in range(4) if local[n] != NONE), NONE)
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        for n in range(-1, 4):
            at = LISTENERS + n * LISTENER_SIZE + 0x30
            state[at : at + 12] = reals(random_, 3)

        def located(harness, arguments):
            location, point = arguments
            harness.mu.mem_write(location, bytes(range(0x70, 0x78)))
            return 0, (harness.normalize(location), bytes(harness.mu.mem_read(point, 12)))

        def told__(harness, arguments):
            data_, index_, offset, said_ = arguments
            # the sound is not compared: the function does not use it, and the compiler of source/ does not pass it
            return 1, (data_, bytes(harness.mu.mem_read(offset, 12)), harness.normalize(said_))

        t.check(
            "looping_sound_detail_request",
            Buffer(bytes(detail)),
            random_.choice(indices),
            Buffer(random_.randbytes(0xA0), places=(0x58,)),
            globals={
                "looping_sounds": looping,
                "global_tag_instances": Buffer(bytes(tags) + bytes(definition), pointers=(sound_index * 0x10 + 8,)),
                "players_globals": Buffer(bytes(players)),
                "game_sound_globals": Buffer(bytes(state)),
                "global_random_seed_reference": Buffer(struct.pack("<4I", *(random_.getrandbits(32) for _ in range(4)))),
                "looping_sound_callbacks": GlobalRange(0x20),
            },
            externals={
                "unknown_0014dde0": lambda harness, arguments, first_player=first_player: first_player,
                "scenario_location_from_point": located,
                "looping_sound_source_unknown0": told__,
            },
        )

    # ---------- a sound starts to play, with a sound effect if it has one

    random_ = random.Random(228)

    for _ in range(900):
        effects = bytearray(random_.randbytes(0x4C + EFFECTS * 0x14))
        struct.pack_into("<I", effects, 0x44, 0x4C)
        request = bytearray(random_.randbytes(0xA0))
        struct.pack_into("<I", request, 0, random_.choice((0, 0x100, 0x100, 0xFFFFFEFF, random_.getrandbits(32))))
        definition = Buffer(random_.randbytes(8) + struct.pack("<I", random_.choice((0, 1, 1, random_.getrandbits(32)))) + random_.randbytes(4))
        has_definition = random_.choice((1, 1, 1, 0))
        collection = random_.choice((None, Buffer(random_.randbytes(8) + bytes((random_.choice((0, 1, 1, 0xFE)),)) + random_.randbytes(7))))
        made_index = random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(EFFECTS), random_.getrandbits(16) << 16 | random_.randrange(EFFECTS)))
        starts = random_.choice((1, 1, 1, 0))
        promoted = random_.choice((0, 0, 0, 1))
        created = random_.choice((NONE, random_.getrandbits(32), random_.getrandbits(32)))
        states = ((1, 1, 1), (1, 1, 1), (1, 1, 1), (0, 1, 1), (1, 0, 1), (1, 1, 0))

        def has_(harness, arguments, definition=definition, has_definition=has_definition):
            tag_index, unknown, where = arguments
            if has_definition or random.Random(tag_index).random() < 0.3:
                harness.mu.mem_write(where, struct.pack("<I", definition.address))
            return has_definition, (tag_index, unknown)

        def test_(harness, arguments, starts=starts):
            harness.mu.mem_write(arguments[2], struct.pack("<I", 2))
            return starts, (harness.normalize(arguments[0]), arguments[1], arguments[3])

        def promotion_(harness, arguments, promoted=promoted):
            harness.mu.mem_write(arguments[1], struct.pack("<I", 5))
            return promoted, (arguments[0],)

        t.check(
            "sound_effect_source_new",
            Buffer(bytes(request)),
            random_.getrandbits(32),
            globals={
                "sound_effects": Buffer(bytes(effects), pointers=(0x44,)),
                "game_sound_globals": sound_state(*states[random_.randrange(len(states))]),
            },
            extra=[definition] + ([collection] if collection else []),
            externals={
                "sound_effect_definition_get": has_,
                "datum_new": lambda harness, arguments, made_index=made_index: (made_index, (harness.normalize(arguments[0]),)),
                "unknown_0018d090": lambda harness, arguments, collection=collection: (
                    collection.address if collection else 0,
                    tuple(arguments),
                ),
                "sound_effect_prepare_request": lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1]))),
                "sound_start_test": test_,
                "sound_promotion_update": promotion_,
                "sound_source_create": lambda harness, arguments, created=created: (
                    created,
                    (arguments[0], arguments[1], harness.normalize(arguments[2]), arguments[3]),
                ),
                "sound_effect_start_sounds": lambda harness, arguments: (0, tuple(arguments)),
                "sound_effect_delete": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- a slot of the console's sound is given what plays its sound

    random_ = random.Random(229)

    for _ in range(200):
        console = Buffer(random_.randbytes(0x2AC0))
        answer = random_.choice((0, 0, 1, 0x80004005, 0x8007000E, 0x7FFFFFFF))
        stream = random_.getrandbits(32)

        def makes(harness, arguments, answer=answer, stream=stream):
            sound_, description, where, unknown = arguments
            record = sound_, bytes(harness.mu.mem_read(description, 0x18)), harness.normalize(where), unknown
            if not answer & 0x80000000:
                harness.mu.mem_write(where, struct.pack("<I", stream))
            return answer, record

        def heard_in(harness, arguments):
            count, pairs = struct.unpack("<2I", harness.mu.mem_read(arguments[1], 8))
            return 0, (arguments[0], count, bytes(harness.mu.mem_read(pairs, 0x28)))

        t.check(
            "sound_console_slot_create_stream",
            random_.randrange(16),
            globals={"unknown_0051ebe4": console},
            externals={
                "unknown_0037d4be": makes,
                "unknown_0037c5e4": heard_in,
                "unknown_0037c620": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037c644": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037c6c1": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037c69d": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037c600": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- where the sound of a stream is to be heard, and what has changed

    random_ = random.Random(230)

    def all_again(harness, arguments):
        count, pairs = struct.unpack("<2I", harness.mu.mem_read(arguments[1], 8))
        return 0, (arguments[0], count, harness.normalize(pairs), harness.normalize(arguments[1]))

    def loudness_(harness, arguments):
        count, pairs = struct.unpack("<2I", harness.mu.mem_read(arguments[1], 8))
        return 0, (arguments[0], count, bytes(harness.mu.mem_read(pairs, count * 8)))

    for n in range(1500):
        count = random_.choice((0, 1, 2, 3, 5, 8))
        if n % 5:
            places = random_.sample(range(11), count)
        else:
            # a place can be twice
            places = [random_.randrange(8) for _ in range(count)]
        pairs = [(place, random_.choice((0, -600, -6400, -1234))) for place in places]
        new = bytearray(random_.randbytes(0x48))
        for index, (place, volume) in enumerate(pairs):
            struct.pack_into("<2i", new, index * 8, place, volume)
        struct.pack_into("<i", new, 0x40, count)
        if n % 7 == 0:
            # a place twice, the second time where its own number is: with the same loudness both times
            volume = random_.choice((0, -600))
            pairs = [(1, volume), (1, volume)] + pairs[:3]
            count = len(pairs)
            for index, (place, volume_) in enumerate(pairs):
                struct.pack_into("<2i", new, index * 8, place, volume_)
            struct.pack_into("<i", new, 0x40, count)
        before = list(pairs)
        kind = random_.randrange(6)
        if n % 7 == 0:
            kind = 0
            before[0] = (1, before[0][1] - random_.choice((0, 100)))
            before[1] = (1, before[1][1] - random_.choice((0, 100, 100)))
        if kind == 1 and before:
            index = random_.randrange(len(before))
            before[index] = (before[index][0], before[index][1] - 100)
        elif kind == 2:
            before = [(place, random_.choice((volume, volume, -700))) for place, volume in before]
        elif kind == 3 and before:
            index = random_.randrange(len(before))
            before[index] = (before[index][0] ^ 1, before[index][1])
        old_count = len(before) if kind != 4 else len(before) + random_.choice((1, -1))
        old = struct.pack("<i", old_count) + b"".join(struct.pack("<2i", place, volume) for place, volume in before) + random_.randbytes(16)
        for name in ("sound_console_stream_set_mixbins", "sound_console_slot_stream_set_mixbins"):
            t.check(
                name,
                0 if kind == 5 else Buffer(old),
                Buffer(bytes(new)),
                random_.getrandbits(32),
                externals={
                    "unknown_0037c70a": all_again,
                    "unknown_0037c5e4": all_again,
                    "unknown_0037b81d": loudness_,
                    "unknown_0037b6c3": loudness_,
                },
            )

    # ---------- the console makes something that plays silence over and over

    random_ = random.Random(231)

    for allocated in (0, 0x1290, 0x1291, 0x1292, 0x1293, 0x345) * 4:
        made_buffer = random_.getrandbits(32)

        def makes_(harness, arguments, made_buffer=made_buffer):
            description, where = arguments
            size, flags, bytes_, format_, mixbins, unknown14 = struct.unpack("<6I", harness.mu.mem_read(description, 0x18))
            count, pairs = struct.unpack("<2I", harness.mu.mem_read(mixbins, 8))
            record = (
                size,
                flags,
                bytes_,
                bytes(harness.mu.mem_read(format_, 0x12)),
                count,
                bytes(harness.mu.mem_read(pairs, 8)),
                unknown14,
                bytes(harness.mu.mem_read(where, 4)),
            )
            harness.mu.mem_write(where, struct.pack("<I", made_buffer))
            return 0, record

        def samples_(harness, arguments):
            buffer_, samples, size = arguments
            return 0, (buffer_, samples, size, bytes(harness.mu.mem_read(samples, size)))

        t.check(
            "sound_console_create_silence",
            Buffer(random_.randbytes(4)),
            globals={"shell_memory_globals": shell_memory(allocated)},
            memory={STATE: Memory(random_.randbytes(STATE_SIZE))},
            externals={
                "unknown_0037d7de": makes_,
                "unknown_0037cc4a": samples_,
                "unknown_0037b6a7": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037b66f": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037b6df": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- what the console's sound does each frame

    random_ = random.Random(232)

    for _ in range(300):
        console = bytearray(console_voices(random_.choice((8, 8, 3, 1, 0, -1))))
        pointers = []
        stream = len(console)
        console += struct.pack("<I", stream + 4) + random_.randbytes(0xC) + struct.pack("<I", mock_address(0))
        console += random_.randbytes(8) + struct.pack("<I", mock_address(1))
        pointers.append(stream)
        other = len(console)
        console += struct.pack("<I", other + 4) + random_.randbytes(0x1C) + struct.pack("<I", mock_address(2))
        pointers.append(other)
        for voice in range(8):
            at = 0xC + voice * 0x34
            console[at + 1] = random_.choice((1, 1, 9, 0xF9, 0, 2, 3, 0xFA, random_.getrandbits(8)))
            console[at + 3] = random_.getrandbits(8)
            struct.pack_into("<I", console, at + 0x24, stream)
            struct.pack_into("<I", console, at + 0x30, other)
            pointers += [at + 0x24, at + 0x30]
        number = random_.getrandbits(32)
        struct.pack_into("<I", console, 0x2AD4, random_.choice((number, number, number ^ 1)))
        doing = [random_.choice((0x10000, 0x10000, 0, 0xFFFEFFFF, 0xFFFFFFFF)) for _ in range(8)]

        def does(call, doing=doing):
            # by how many times the stream was asked in this call
            asked_ = sum(1 for made_call in t.harness._calls if made_call[0] == 0)
            t.harness.mu.mem_write(call.stack[1], struct.pack("<I", doing[asked_ % 8]))
            return 0

        t.check(
            "sound_console_update",
            globals={
                "unknown_0051ebe4": Buffer(bytes(console), pointers=tuple(pointers)),
                "unknown_00386ad8": struct.pack("<I", number),
            },
            externals={
                "unknown_001915f0": lambda harness, arguments: 0,
                "unknown_0037b844": lambda harness, arguments: 0,
                "unknown_002ae8e0": console_call(0),
            },
            mocks=[
                Mock(does, stack_arguments=2, record=lambda call: (call.stack[0],)),
                Mock(lambda call: 0, stack_arguments=1),
                Mock(lambda call: 0, stack_arguments=0, registers=("ecx",)),
            ],
        )

    # ---------- what unknown_00192b90 says of a direction and one of five things

    random_ = random.Random(233)

    for _ in range(600):
        answer = random_.choice((0.0, 0.5, 1.0, 1.5, -0.25, -0.0, 0.999, float("nan"), random_.uniform(-1, 2)))
        t.check(
            "sound_unknown_00222250",
            Buffer(reals(random_, 3)),
            random_.choice((0, 1, 2, 3, 4, 4, 5, NONE, 0x80000000, 0x7FFFFFFF)),
            externals={
                "unknown_00192b90": lambda harness, arguments, answer=answer: (
                    real_bits(answer),
                    (arguments[0], bytes(harness.mu.mem_read(arguments[1], 12)), arguments[2]),
                ),
            },
        )

    # ---------- the console is given a list of what its effects are to have

    random_ = random.Random(234)

    for _ in range(700):
        known = bytearray(random_.randbytes(0x30))
        effects_ = len(known)
        struct.pack_into("<I", known, 0, effects_)
        pointers = [0]
        known += random_.randbytes(32 * 0x20)
        states = len(known)
        known += random_.randbytes(32 * 0x40)
        for effect in range(32):
            struct.pack_into("<2I", known, effects_ + effect * 0x20 + 0x10, states + effect * 0x40, random_.choice((0x10, 0x10, 4, 0, 1)))
            pointers.append(effects_ + effect * 0x20 + 0x10)
        entries = bytearray(struct.pack("<I", random_.getrandbits(32)))
        for _entry in range(random_.choice((0, 1, 2, 4, 6))):
            mask = random_.choice((0, 1, 2, 0x10, 0x8001, 0x80000000, 0x105, random_.getrandbits(32), 1 << random_.randrange(32)))
            flags = random_.choice((0, 0, 0, 1, 2, 3, 0xFC))
            bytes_ = random_.choice((0, 1, 4, 5, 8, 0x10, 0x23))
            offset = random_.choice((0, 0, 4, 8, 0x30, 0x3C))
            data_ = bytearray(random_.randbytes(bytes_))
            first_effect = next((bit for bit in range(32) if mask >> bit & 1), 0)
            if random_.random() < 0.4 and offset + bytes_ <= 0x40:
                # what the game knows that the first of its effects has there
                at = states + first_effect * 0x40 + offset
                data_ = bytearray(known[at : at + bytes_])
                if random_.random() < 0.3 and bytes_:
                    data_[-1] ^= 1
            entries += struct.pack("<IB3xHH", mask, flags, offset, bytes_) + data_ + random_.randbytes(-bytes_ % 4)
        size = random_.choice((len(entries), len(entries), len(entries), 0, 4, len(entries) + 3))
        t.check(
            "sound_console_apply_effect_list",
            size,
            random_.choice((0, Buffer(bytes(entries) + random_.randbytes(0x40)), Buffer(bytes(entries) + random_.randbytes(0x40)), Buffer(bytes(entries) + random_.randbytes(0x40)))),
            globals={
                "unknown_00510c90": Buffer(bytes(known), pointers=tuple(pointers)),
                "unknown_0051ebe4": Buffer(random_.randbytes(0x2AC0)),
            },
            externals={
                "unknown_0037ba6f": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037b60d": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- an effect of the console is given how a place sounds

    random_ = random.Random(235)

    def some_loudness() -> float:
        return random_.choice((0.0, -64.0, -32.0, -6.0, -70.0, 5.0, 10.0, 15.0, 20.0, 25.0, -0.01, random_.uniform(-64, 20)))

    for _ in range(900):
        place = bytearray(reals(random_, 14))
        for offset in (8, 0xC, 0x1C, 0x24):
            struct.pack_into("<f", place, offset, some_loudness())
        struct.pack_into("<2f", place, 0x2C, random_.choice((0.0, 1.0, 0.5, 0.333)), random_.choice((0.0, 1.0, 0.75, 0.123)))
        t.check(
            "sound_console_set_reverb",
            Buffer(bytes(place)),
            random_.getrandbits(32),
            globals={"unknown_00468834": struct.pack("<f", random_.choice((1.0, 1.0, 0.5, 0.0, 2.0)))},
            externals={
                "unknown_0037ba6f": lambda harness, arguments: (
                    0,
                    (arguments[0], bytes(harness.mu.mem_read(arguments[1], 0x34)), arguments[2]),
                ),
            },
        )

    # ---------- a looping sound is made

    random_ = random.Random(236)

    for _ in range(700):
        looping, indices = looping_world()
        definition_index = random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        struct.pack_into("<I", tags, definition_index * 0x10 + 8, len(tags))
        definition = bytearray(random_.randbytes(0x30))
        definition[0] = random_.choice((0, 0x10, 0x20, 0x30, 0xCF, 0xFF))
        struct.pack_into("<i", definition, 0x1C, random_.choice((0, 1, 2, 3, 3)))
        details_count = random_.choice((0, 1, 3, 6, 12))
        struct.pack_into("<iI", definition, 0x24, details_count, len(tags) + 0x30)
        details = bytearray(random_.randbytes(12 * 0x34))
        for detail in range(12):
            low = random_.choice((0.0, 1.0, 2.5, 30.0))
            struct.pack_into("<2f", details, detail * 0x34 + 0xC, low, low + random_.choice((0.0, 1.0, 10.0)))
            details[detail * 0x34 + 0x18] = random_.choice((0, 0, 4, 0xFB, 0xFF))
        controllers = bytearray(random_.randbytes(0x4C + CONTROLLERS * 0x1C))
        struct.pack_into("<I", controllers, 0x44, 0x4C)
        for controller in range(CONTROLLERS):
            struct.pack_into("<I", controllers, 0x4C + controller * 0x1C + 4, random_.choice((NONE, NONE, 0, random_.getrandbits(32))))
        apart = bytearray(random_.randbytes(0x1C))
        struct.pack_into("<I", apart, 4, random_.choice((NONE, NONE, 5)))
        state = bytearray(sound_state(*random_.choice(((1, 1, 1), (1, 1, 1), (1, 1, 1), (0, 1, 1), (1, 0, 1), (1, 1, 0)))).data)
        struct.pack_into("<i", state, 0x84, random_.choice((0, 1000, 123456, 0x7F000000)))
        made_index = random_.choice((NONE, *indices, *indices))
        given = random_.choice((NONE, NONE, random_.getrandbits(16) << 16 | random_.randrange(CONTROLLERS)))
        t.check(
            "looping_sound_new",
            random_.getrandbits(16) << 16 | definition_index,
            random_.getrandbits(32),
            random_.getrandbits(32),
            random_.getrandbits(32),
            globals={
                "looping_sounds": looping,
                "game_sound_globals": Buffer(bytes(state)),
                "global_tag_instances": Buffer(bytes(tags) + bytes(definition) + bytes(details), pointers=(definition_index * 0x10 + 8, len(tags) + 0x28)),
                "unknown_0051ebd8": Buffer(bytes(controllers), pointers=(0x44,)),
                "unknown_0051ebdc": Buffer(bytes(apart)),
                "global_random_seed_reference": Buffer(struct.pack("<4I", *(random_.getrandbits(32) for _ in range(4)))),
            },
            externals={
                "datum_new": lambda harness, arguments, made_index=made_index: (made_index, (harness.normalize(arguments[0]),)),
                "sound_playback_controller_new": lambda harness, arguments, given=given: (given, tuple(arguments)),
            },
        )

    # ---------- the console's sound takes its gains

    random_ = random.Random(237)

    def a_gain() -> float:
        return random_.choice((0.0, -0.0, 1.0, 0.5, 0.25, 0.001, 0.0006, 0.00063, 0.00064, 2.0, -1.0, 1e-30, float("nan"), random_.uniform(0, 1), random_.uniform(0, 1)))

    for _ in range(900):
        console = bytearray(random_.randbytes(0x2AD8))
        struct.pack_into("<h", console, 6, random_.choice((0, 1, 5, 16, 32, -1)))
        t.check(
            "sound_console_set_gains",
            Buffer(struct.pack("<4f", a_gain(), a_gain(), a_gain(), a_gain()) + random_.randbytes(8)),
            globals={
                "unknown_0051ebe4": Buffer(bytes(console)),
                "unknown_00470060": random_.randbytes(0x24),
                "unknown_00470084": random_.randbytes(4),
                "unknown_00470088": random_.randbytes(4),
                "unknown_0047005c": bytes((random_.choice((0, 1, 0xFF)),)),
            },
        )

    # ---------- the console's sound has one slot more

    random_ = random.Random(238)

    for _ in range(600):
        console = bytearray(random_.randbytes(0x2AC0))
        struct.pack_into("<h", console, 6, random_.choice((0, 0, 5, 20, 0x3F, 0x40, 0x41, 0x7FFF, -1)))
        answers_ = [random_.choice((0, 0, 0, 1, 0x80004005)), random_.choice((0, 0, 0, 1, 0x8007000E))]
        streams = [random_.getrandbits(32), random_.getrandbits(32)]

        def makes__(harness, arguments, answers_=answers_, streams=streams):
            sound_, description, where, unknown = arguments
            n = calls_so_far(harness, "unknown_0037d4be")
            record = sound_, bytes(harness.mu.mem_read(description, 0x18)), harness.normalize(where), unknown
            if not answers_[n % 2] & 0x80000000:
                harness.mu.mem_write(where, struct.pack("<I", streams[n % 2]))
            return answers_[n % 2], record

        def heard_in_(harness, arguments):
            count, pairs = struct.unpack("<2I", harness.mu.mem_read(arguments[1], 8))
            return 0, (arguments[0], count, bytes(harness.mu.mem_read(pairs, count * 8)))

        t.check(
            "sound_console_slot_new",
            random_.choice((0, 1, 2, 3, 3, 1, 0xFFFFFFFC, 0xFFFFFFFD, random_.getrandbits(32))),
            random_.getrandbits(32),
            globals={"unknown_0051ebe4": Buffer(bytes(console))},
            externals={
                "unknown_0037d4be": makes__,
                "unknown_0037c5e4": heard_in_,
                "unknown_0037b66f": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037b6df": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037c620": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037c644": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- the sound of a track of a looping sound goes on to the next state

    random_ = random.Random(239)

    for _ in range(900):
        buffer, indices = playing()
        data = bytearray(buffer.data)
        for index in range(PLAYING):
            at = 0x4C + index * DATUM
            data[at + 4] = random_.choice((0, 8, 0xF7, 0xFF))
            data[at + 0x60] = random_.randrange(3)
            struct.pack_into("<f", data, at + 0x90, random_.choice((0.0, 100.0, -300.0)))
            data[at + 0x9C] = random_.choice((0, 1, 2))
            struct.pack_into("<h", data, at + 0xAC, random_.choice((-1, 0, 3, 5)))
        definition_index = random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        tag_pointers = []
        definitions = bytearray(random_.randbytes(TAGS * 0x10))
        for tag in range(TAGS):
            struct.pack_into("<I", tags, tag * 0x10 + 8, TAGS * 0x10 + tag * 0x10)
            tag_pointers.append(tag * 0x10 + 8)
            struct.pack_into("<h", definitions, tag * 0x10 + 8, random_.randrange(0, 6))
        looping_definition = TAGS * 0x20
        struct.pack_into("<I", tags, definition_index * 0x10 + 8, looping_definition)
        definition = bytearray(random_.randbytes(0x28))
        definition[0] = random_.choice((0, 0, 0x10, 0x20, 0x30, 0xCF, 0xFF))
        struct.pack_into("<I", definition, 0x20, looping_definition + 0x28)
        tracks = bytearray(random_.randbytes(3 * 0x58))
        for track in range(3):
            for offset in (0x14, 0x1C, 0x24, 0x40, 0x2C, 0x48, 0x34):
                sound_index = random_.choice([tag for tag in range(TAGS) if tag != definition_index])
                struct.pack_into("<I", tracks, track * 0x58 + offset + 4, random_.getrandbits(16) << 16 | sound_index)
        tags_ = Buffer(bytes(tags) + bytes(definitions) + bytes(definition) + bytes(tracks), pointers=tuple(tag_pointers) + (looping_definition + 0x20,))
        looping_ = bytearray(random_.randbytes(0xD4))
        struct.pack_into("<I", looping_, 4, random_.getrandbits(16) << 16 | definition_index)
        struct.pack_into("<I", looping_, 0x54, random_.choice((NONE, NONE, 0x10002, 5)))
        state_ = bytearray(random_.randbytes(0x10))
        state_[0xD] = random_.randrange(7)
        shared = bytearray(random_.randbytes(0x50 + 0x10 * 0xC))
        struct.pack_into("<I", shared, 0x24, 0x50)
        channels = bytearray(random_.randbytes(CHANNELS * 0x24))
        for channel_index in range(CHANNELS):
            struct.pack_into("<I", channels, channel_index * 0x24, random_.choice((NONE, *indices, *indices)))
        source_index = random_.choice(indices)
        for channel_index in random_.sample(range(CHANNELS), 2):
            struct.pack_into("<I", channels, channel_index * 0x24, source_index)
        like = bytearray(random_.randbytes(0x4A))
        struct.pack_into("<h", like, 0, random_.choice((0, 1, 3, -1)))
        struct.pack_into("<h", like, 0x22, random_.choice((0, 1, 2, 4)))
        struct.pack_into("<h", like, 0x26, random_.choice((0, 1, 2, -1)))
        struct.pack_into("<h", like, 0x48, random_.choice((0, 1, 2, 4)))
        range_ = random_.choice((0, 1, 2))
        bits_ = random_.getrandbits(32)
        # as the function gives it: a number of 8 bits with its sign
        last_ = random_.choice((NONE, NONE, 0, 2, 5))
        chosen_ = random_.choice((0, 1, 5, 0x105))
        channel_ = random_.choice((NONE, NONE, *range(CHANNELS)))

        def choose_(harness, arguments, chosen_=chosen_):
            sound_, range__, played_, last__, seed_, all_, unknown_ = arguments
            record = harness.normalize(sound_), range__ & 0xFFFF, bytes(harness.mu.mem_read(played_, 4)), last__, seed_, all_, unknown_ & 0xFF
            harness.mu.mem_write(played_, struct.pack("<I", 0xA5A50000 | chosen_))
            return chosen_, record

        def same_(harness, arguments, like=like):
            harness.mu.mem_write(arguments[0], bytes(like))
            return 0, (arguments[1],)

        t.check(
            "looping_sound_source_next_state",
            Buffer(bytes(looping_)),
            Buffer(bytes(state_)),
            source_index,
            Buffer(random_.randbytes(4)),
            globals={
                "sound_sources": Buffer(bytes(data), pointers=buffer.pointers),
                "sound_channels": Buffer(bytes(channels)),
                "global_tag_instances": tags_,
                "global_sound_cache_file_gestalt": Buffer(bytes(shared), pointers=(0x24,)),
            },
            externals={
                "sound_choose_pitch_range": lambda harness, arguments, range_=range_: (
                    range_,
                    (arguments[0] & 0xFFFF, harness.normalize(arguments[1]), arguments[2]),
                ),
                "sound_pitch_range_get_played_bits": lambda harness, arguments, bits_=bits_: (
                    bits_,
                    (harness.normalize(arguments[0]), harness.normalize(arguments[1])),
                ),
                "sound_pitch_range_get_unknown4": lambda harness, arguments, last_=last_: (
                    last_,
                    (harness.normalize(arguments[0]), harness.normalize(arguments[1])),
                ),
                "sound_pitch_range_choose_permutation": choose_,
                "sound_pitch_range_set_played_bits": lambda harness, arguments: (
                    0,
                    (harness.normalize(arguments[0]), harness.normalize(arguments[1]), arguments[2]),
                ),
                "sound_pitch_range_set_unknown4": lambda harness, arguments: (
                    0,
                    (harness.normalize(arguments[0]), harness.normalize(arguments[1]), arguments[2] & 0xFF),
                ),
                "sound_source_release_chunk": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "sound_source_find_same": same_,
                "sound_source_choose_channel": lambda harness, arguments, channel_=channel_: (
                    channel_,
                    (arguments[0], arguments[1] & 0xFFFF, bytes(harness.mu.mem_read(arguments[2], 4))),
                ),
                "sound_source_delete": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- the console's sound is told of who hears

    random_ = random.Random(240)
    SILENCE = struct.pack("<2I", 0, 0x8000) + struct.pack("<12f", -100, -100, 0, 1, 1, -100, 0, -100, 0, 1, 1, 5000) + bytes(16)

    # the place with no sound of its own: as the game has it
    harness = t.harness
    if not t.only or "sound_console_set_listener" in t.only:
        both = [
            bytes(harness.mu.mem_read(harness.globals["sound_environment_default"], 0x48)),
            bytes(harness.mu.mem_read(harness.variables["sound_environment_default"], 0x48)),
        ]
        t.cases["sound_console_set_listener"] = t.cases.get("sound_console_set_listener", 0) + 1
        if both[0] != both[1] or both[0] != SILENCE:
            t._fail("sound_console_set_listener", "sound_environment_default is not the game's")

    for n in range(900):
        heard = bytearray(random_.randbytes(0x38))
        heard[0:0x24] = reals(random_, 9)
        count = random_.choice((0, 1, 2, 2, 3, -1))
        places = len(heard)
        struct.pack_into("<iI", heard, 0x30, count, places)
        pointers = [0x34]
        heard += random_.randbytes(3 * 0x18)
        for place in range(3):
            at = len(heard)
            heard += random_.choice((SILENCE, random_.randbytes(0x48), random_.randbytes(0x48)))
            struct.pack_into("<I", heard, places + place * 0x18, at)
            pointers.append(places + place * 0x18)
            struct.pack_into("<f", heard, places + place * 0x18 + 0x14, random_.choice((0.0, 1.0, 0.5, 0.00005, 0.0002)))
        console = bytearray(random_.randbytes(0x2AC0))
        console[0] = random_.choice((1, 1, 1, 0, 0x80))
        for index in range(6):
            value = struct.unpack_from("<f", heard, 0xC + index * 4)[0]
            if n % 3:
                value += random_.choice((0.0, 0.0, 0.0, 0.02, -0.02, 0.05, 0.06))
            else:
                value = random_.uniform(-1, 1)
            struct.pack_into("<f", console, 0x2924 + index * 4, value)
        for place in range(2):
            source_ = struct.unpack_from("<I", heard, places + place * 0x18)[0]
            kind = random_.randrange(4)
            if kind == 0:
                console[0x2950 + place * 0x48 : 0x2950 + (place + 1) * 0x48] = heard[source_ : source_ + 0x48]
            elif kind == 1:
                console[0x2950 + place * 0x48 : 0x2950 + (place + 1) * 0x48] = SILENCE
            if random_.random() < 0.5:
                console[0x29E8 + place * 0x10 : 0x29E8 + (place + 1) * 0x10] = heard[places + place * 0x18 + 4 : places + place * 0x18 + 0x14]
                if random_.random() < 0.4:
                    # all but the last of the four numbers
                    console[0x29E8 + place * 0x10 + 0xF] ^= 1
            amount = struct.unpack_from("<f", heard, places + place * 0x18 + 0x14)[0]
            struct.pack_into("<f", console, 0x29E0 + place * 4, random_.choice((amount, amount, 0.0, amount + 0.00005, amount + 0.0002, 0.7)))
        t.check(
            "sound_console_set_listener",
            Buffer(bytes(heard), pointers=tuple(pointers)),
            globals={"unknown_0051ebe4": Buffer(bytes(console))},
            externals={
                "unknown_0037d598": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037d54e": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- what a listener hears of each cluster, and its queries

    random_ = random.Random(241)

    for n in range(700):
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        listener_index = random_.randrange(4)
        clusters = random_.choice((0, 1, 3, 4, 5, 7, 8, 9, 31, 32, 33, 64, 100, 511, 512))
        own = random_.randrange(clusters) if clusters and n % 9 else random_.choice((-1, -1, 0))
        struct.pack_into("<h", state, LISTENERS + listener_index * LISTENER_SIZE + 4, own)
        openings = random_.choice((0, 1, 2, 5, 33, 100, 128))
        said = bytes(random_.choice((0xFF, 0, 1, 2, 3, 0x7F, 0x80, 0xFE)) for _ in range(openings))
        table = bytearray(random_.randbytes(0x34))
        struct.pack_into("<i", table, 0, openings)
        struct.pack_into("<I", table, 0x30, 0x22C + 0x34)
        table += said + bytes(4)
        structure = bytearray(random_.randbytes(0x22C))
        struct.pack_into("<i", structure, 0x9C, clusters)
        struct.pack_into("<i", structure, 0x224, random_.choice((1, 1, 1, 1, 3, 0, -1)))
        struct.pack_into("<I", structure, 0x228, 0x22C)
        seed = random_.getrandbits(32)
        dense = random_.random() < 0.5

        def mine_(harness, arguments, seed=seed):
            bits = random.Random(seed * 3 + (arguments[2] & 0xFFFF)).randbytes(16)
            harness.mu.mem_write(arguments[1], bits)
            return 0, (harness.normalize(arguments[0]), arguments[2])

        def sides_(harness, arguments, seed=seed, own=own, dense=dense, clusters=clusters):
            source_ = random.Random(seed * 5 + arguments[2])
            rows = []
            for row in range(2):
                bits = bytearray(source_.randbytes(0x40))
                if not dense:
                    other = bytes(source_.randbytes(0x40))
                    bits = bytearray(a & b for a, b in zip(bits, other))
                rows.append(bits)
            kind = source_.randrange(4)
            if 0 <= own < 512 and kind < 3:
                for row in range(2):
                    rows[row][own >> 3] &= ~(1 << (own & 7)) & 0xFF
                if kind < 2:
                    rows[kind][own >> 3] |= 1 << (own & 7)
            harness.mu.mem_write(arguments[0], bytes(rows[0]))
            harness.mu.mem_write(arguments[3], bytes(rows[1]))
            return 0, (harness.normalize(arguments[1]), arguments[2])

        def amount_(harness, arguments, seed=seed):
            value = random.Random(seed * 7 + arguments[0]).choice((0.0, 1.0, 0.25, 0.5, 0.75, 0.1, -1.0, 2.0))
            return real_bits(value), (arguments[0],)

        def query_(harness, arguments):
            return 0, (
                harness.normalize(arguments[0]),
                bytes(harness.mu.mem_read(arguments[1], 0x800)),
                harness.normalize(arguments[2]),
            )

        count = random_.choice((0, 1, 2, 5, 72))
        queries = bytearray(struct.pack("<i", count) + random_.randbytes(72 * 0x48))
        for index in range(72):
            struct.pack_into("<i", queries, 4 + index * 0x48, random_.choice((listener_index, listener_index, 0, 1, 2, 3, -1)))
        t.check(
            "sound_listener_compute_queries",
            listener_index,
            Buffer(bytes(queries)),
            globals={
                "game_sound_globals": Buffer(bytes(state)),
                "global_structure_bsp": Buffer(bytes(structure) + bytes(table), pointers=(0x228, 0x22C + 0x30)),
            },
            externals={
                "unknown_00249d10": mine_,
                "unknown_00249c90": sides_,
                "unknown_0018cac0": amount_,
                "unknown_00020670": lambda harness, arguments: (0, (arguments[1], arguments[2])),
                "sound_query_compute": query_,
            },
        )

    # ---------- two reals for a sound in another cluster, from the nearest portal

    random_ = random.Random(242)

    for n in range(600):
        portals = 8
        heard = reals(random_, 16)
        structure = bytearray(random_.randbytes(0x64) + random_.randbytes(portals * 0x24))
        struct.pack_into("<I", structure, 0x60, 0x64)
        portal = random_.choice((NONE, NONE, 0, 1, 5, 7))
        away = random_.choice((0.0, 1.0, 0.5, 3.0, 100.0, 1e-7, 0.3333, 1e30, -0.5, random_.uniform(0, 50), float("inf")))
        if n % 4 == 0:
            # where rounding each step to 32 bits gives another real
            away = bits_real(random_.choice((0x3DD7A8ED, 0x3F8988BC, 0x4114B99A, 0x41423126, 0x41DB84FE, 0x421BB043)))
        cluster = random_.randrange(16)
        listener_cluster = random_.choice((0, 1, 7, NONE))

        def nearest(harness, arguments, portal=portal):
            harness.mu.mem_write(arguments[2], struct.pack("<f", 12.5))
            return portal, (arguments[0], harness.normalize(arguments[1]))

        def far(harness, arguments, away=away):
            return real_bits(away), tuple(harness.normalize(argument) for argument in arguments)

        t.check(
            "sound_query_portal_results",
            listener_cluster,
            Buffer(reals(random_, 3)),
            Buffer(random_.randbytes(8)),
            Buffer(heard),
            Buffer(reals(random_, 3)),
            cluster,
            globals={"global_structure_bsp": Buffer(bytes(structure), pointers=(0x60,))},
            externals={"unknown_0018cfd0": nearest, "unknown_0014b240": far},
        )

    # ---------- what is worked out of a query

    random_ = random.Random(243)

    for n in range(1200):
        clusters = 6
        leaf = random_.getrandbits(32)
        listener = bytearray(random_.randbytes(0x48))
        listener[8:0x48] = reals(random_, 16)
        struct.pack_into("<Ih", listener, 0, leaf, random_.choice((0, 1, 2, 3, 4, 5, 2, -1)))
        query = bytearray(random_.randbytes(0x48))
        query[4:0x3C] = reals(random_, 14)
        struct.pack_into(
            "<iI4B",
            query,
            0x3C,
            random_.choice((0, 1, 2, 3, 4, 5, 2, -1)),
            leaf if n % 11 == 0 else leaf ^ 1,
            random_.choice((0, 0, 1)),
            random_.choice((0, 1)),
            random_.choice((0, 1)),
            random_.choice((0, 0, 1)),
        )
        structure = bytearray(random_.randbytes(0xA4))
        struct.pack_into("<I", structure, 0xA0, 0xA4)
        for cluster in range(clusters):
            entry = bytearray(random_.randbytes(0xB0))
            struct.pack_into("<h", entry, 0x7A, random_.choice((1, 2)))
            structure += entry
        answers = [
            (random_.choice((1, 1, 1, 0, 0x100)), random_.choice((0.0, 0.25, 0.5, 0.75, 1.0, 0.1, -0.25, 0.3, random_.random())))
            for _ in range(2)
        ]
        if n % 7 == 0:
            answers[1] = (1, 1.0 - answers[0][1])

        def line(harness, arguments, answers=answers):
            hit, fraction = answers[calls_so_far(harness, "unknown_001de630") % 2]
            harness.mu.mem_write(arguments[0], struct.pack("<f", fraction))
            return hit, (
                arguments[1],
                arguments[2],
                arguments[3],
                arguments[4],
                arguments[5],
                bytes(harness.mu.mem_read(arguments[6], 12)),
                bytes(harness.mu.mem_read(arguments[7], 12)),
            )

        def around_(harness, arguments, value=random_.choice((0.0, 1.0, 5.0, 100.0, -1000.0, 0.0005, random_.uniform(0, 20)))):
            return real_bits(value), (arguments[0], arguments[1], harness.normalize(arguments[2]))

        def portal_(harness, arguments, pair=reals(random_, 2)):
            harness.mu.mem_write(arguments[2], pair)
            return 0, (
                arguments[0],
                bytes(harness.mu.mem_read(arguments[1], 12)),
                bytes(harness.mu.mem_read(arguments[3], 64)),
                bytes(harness.mu.mem_read(arguments[4], 12)),
                arguments[5],
            )

        t.check(
            "sound_query_compute",
            Buffer(bytes(query)),
            Buffer(reals(random_, 16)),
            Buffer(bytes(listener)),
            globals={
                "global_structure_bsp": Buffer(bytes(structure), pointers=(0xA0,)),
                "global_collision_bsp": random_.getrandbits(32),
                "global_structure_bsp_index": struct.pack("<h", random_.choice((0, 1, 5, -1))),
                "unknown_004ed280": random_.getrandbits(30),
            },
            externals={
                "unknown_001de630": line,
                "unknown_00249d60": around_,
                "sound_query_portal_results": portal_,
            },
        )

    # ---------- an entry of the list for the console's sound, from an element of a tag

    random_ = random.Random(244)

    for n in range(1200):
        count = random_.choice((0, 1, 2, 3, 5, 8, 33))
        data = bytearray()
        element = bytearray(random_.randbytes(0x18))
        struct.pack_into("<I", element, 0, random_.choice((0x0D00012E, 0x1300012D, 0x11000138, 0x17000137, random_.getrandbits(32))))
        element[8] = random_.getrandbits(8)
        struct.pack_into("<i", element, 0x10, count)
        parameters = bytearray()
        for index in range(count):
            low, high = sorted((random_.uniform(-50, 50), random_.uniform(-50, 50)))
            if random_.random() < 0.1:
                low, high = high, low
            parameters += struct.pack(
                "<i3f", random_.choice((0, 0, 1, 1, 1, 2, -1)), random_.choice((random_.uniform(-60, 60), 0.5, 2.5, -1.5)), low, high
            )
        # the override: 0x1C bytes, and after it its functions, its reals and its bytes
        override = bytearray(random_.randbytes(0x1C))
        functions = 6
        pointers = []
        struct.pack_into("<I", override, 4, 0x1C)
        pointers.append(4)
        table = bytearray()
        datas = bytearray()
        datas_at = 0x1C + functions * 0x10
        for index in range(functions):
            entry_ = bytearray(0x10)
            struct.pack_into(
                "<2hiIf",
                entry_,
                0,
                random_.choice((0, 1, 1, 2, 3, 3, 5)),
                random_.choice((0, 1, 1, 2, 3, 3, 5)),
                random_.choice((1, 1, 1, 20, 0, -1)),
                datas_at + len(datas),
                random_.choice((1.0, 2.0, 0.5, 3.0, 10.0)),
            )
            pointers.append(0x1C + index * 0x10 + 8)
            table += entry_
            piece = bytearray(random_.randbytes(0xC))
            piece[1] = random_.choice((0, 1, 1, 0x0E, 0x0F, 0x10, 0x80, 0x11))
            piece[4:0xC] = struct.pack("<2f", random_.uniform(-80, 80), random_.uniform(-80, 80))
            datas += piece
        override += table + datas
        struct.pack_into("<I", override, 0xC, len(override))
        pointers.append(0xC)
        override += reals(random_, 8)
        struct.pack_into("<I", override, 0x14, len(override))
        pointers.append(0x14)
        override += bytes(
            random_.choice((random_.randrange(functions) * 2 + 1, random_.randrange(8) * 2)) for _ in range(count + 1)
        )
        struct.pack_into("<I", override, 0x18, random_.choice((0, 0xFFFFFFFF, random_.getrandbits(32), random_.getrandbits(32))))
        inputs = bytearray(struct.pack("<8f", *(random_.choice((0.0, 0.25, 0.5, 1.0, 2.0, -1.0, 7.5, random_.uniform(-3, 3))) for _ in range(8))))
        number = random_.choice((NONE, NONE, 0, 1, 2, 3))

        def which_(harness, arguments):
            return (arguments[0] * 7 + arguments[1]) & 0xFF, (arguments[0], arguments[1])

        def function_(harness, arguments, seed=random_.getrandbits(32)):
            value = random.Random(seed + calls_so_far(harness, "unknown_0013b390")).choice(
                (0.0, 0.25, 0.5, 1.0, 1.5, -0.5, 0.75, 0.1, float("nan"))
            )
            return real_bits(value), (bytes(harness.mu.mem_read(arguments[0], 8))[:4], arguments[1], arguments[2])

        def directional_(harness, arguments, seed=random_.getrandbits(32)):
            value = random.Random(seed + arguments[1]).choice((0.0, 0.5, 1.0, 0.3))
            return real_bits(value), (bytes(harness.mu.mem_read(arguments[0], 12)), arguments[1])

        with_override = random_.random() < 0.8
        t.check(
            "sound_effect_entry_build",
            Buffer(random_.randbytes(0x10 + (count + 1) * 4 + 8)),
            Buffer(bytes(element[:0x14]) + struct.pack("<I", 0x18) + bytes(parameters) + bytes(4), pointers=(0x14,)),
            Buffer(bytes(override), pointers=tuple(pointers)) if with_override else 0,
            Buffer(bytes(inputs)),
            number,
            externals={
                "unknown_00191660": which_,
                "unknown_0013b390": function_,
                "sound_unknown_00222250": directional_,
            },
        )

    # ---------- a stream of the console's sound is given the shape of a function

    random_ = random.Random(245)

    for n in range(160):
        console = bytearray(random_.randbytes(0x2AC0))
        stream_index = random_.choice((0, 0, 1, 1, 1, 2, -1, 0x100))
        last = random_.getrandbits(32)
        if 0 <= stream_index < 2:
            struct.pack_into("<I", console, 0x2A98 + stream_index * 4, last)
        environment = bytearray(random_.randbytes(0x10))
        struct.pack_into(
            "<2hIII",
            environment,
            0,
            stream_index,
            random_.choice((0, 1, 5, 31, 31, 0, 32, -1, 0x100)),
            random_.getrandbits(32),
            0x10,
            last if n % 4 == 0 else last ^ 0x10,
        )
        data = bytearray(random_.randbytes(0xC))
        data[1] = random_.choice((0, 1, 0x0F, 0x10, 0x80, 0x40, 0x20))
        data[4:0xC] = struct.pack("<2f", *random_.choice(((0.0, 1.0), (0.25, 0.75), (1.0, 0.0), (-1.0, 2.0), (0.0, 1.004))))
        environment += struct.pack("<iI", random_.choice((1, 1, 1, 30, 0, -1)), 0x18) + data
        size = random_.choice((1000, 1000, 10, 0, 1, 257))

        def lock_(harness, arguments, size=size):
            harness.mu.mem_write(arguments[3], struct.pack("<I", STATE))
            harness.mu.mem_write(arguments[4], struct.pack("<I", size))
            return 0, (arguments[0], arguments[1], arguments[2], arguments[5], arguments[6], arguments[7])

        def function_(harness, arguments, seed=random_.getrandbits(32), kind=random_.randrange(3)):
            source_ = random.Random(seed + arguments[1])
            if kind == 0:
                value = bits_real(arguments[1]) * 1.0
            elif kind == 1:
                value = source_.choice((0.0, 1.0, 0.5, 0.25, 1.5, -0.5, 0.002, 0.998, 0.5019608, 0.4980392, 3.0, -3.0))
            else:
                value = source_.uniform(-0.2, 1.2)
            return real_bits(value), (bytes(harness.mu.mem_read(arguments[0], 4)), arguments[1], arguments[2])

        t.check(
            "sound_console_stream_set_function",
            Buffer(bytes(environment), pointers=(8, 0x14)),
            globals={"unknown_0051ebe4": Buffer(bytes(console))},
            memory={STATE: Memory(random_.randbytes(1024))},
            externals={
                "unknown_0037b73f": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037b7b3": lock_,
                "unknown_0013b390": function_,
                "unknown_0037b797": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037b66f": lambda harness, arguments: (0, tuple(arguments)),
            },
        )

    # ---------- what is left of a look at a looping sound's tag

    random_ = random.Random(246)

    for n in range(500):
        tag_index = random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        struct.pack_into("<I", tags, tag_index * 0x10 + 8, len(tags))
        tag = bytearray(random_.randbytes(0x2C))
        tracks = random_.choice((0, 1, 1, 2, 3, -1))
        details = random_.choice((0, 1, 2, 4, -1))
        struct.pack_into("<iIiI", tag, 0x1C, tracks, len(tags) + 0x2C, details, len(tags) + 0x2C + 3 * 0x58)
        for index in range(3):
            track = bytearray(random_.randbytes(0x58))
            struct.pack_into("<I", track, 0x20, random_.choice((NONE, NONE, random_.getrandbits(32))))
            tag += track
        for index in range(4):
            detail = bytearray(random_.randbytes(0x34))
            struct.pack_into("<I", detail, 8, random_.choice((NONE, NONE, random_.getrandbits(32))))
            tag += detail
        said = bytearray(random_.randbytes(0x44))
        said[3] = random_.choice((1, 1, 1, 0x11, 0xF1, 0, 2, 0x10))
        nearer = random_.choice((0.0, 0.0, -0.0, 1.0, 5.0, float("nan")))
        t.check(
            "looping_sound_check_distances",
            tag_index | (random_.getrandbits(16) << 16),
            Buffer(bytes(said)),
            globals={
                "global_tag_instances": Buffer(bytes(tags) + bytes(tag), pointers=(tag_index * 0x10 + 8, len(tags) + 0x20, len(tags) + 0x28)),
                "unknown_004e636d": bytes([random_.choice((1, 1, 1, 0, 0x80))]),
            },
            externals={
                "sound_get_minimum_distance": lambda harness, arguments, nearer=nearer: (
                    real_bits(nearer),
                    (bytes(harness.mu.mem_read(arguments[0], 0x44)), arguments[1]),
                ),
                "sound_get_maximum_distance": lambda harness, arguments: (
                    real_bits(9.0),
                    (bytes(harness.mu.mem_read(arguments[0], 0x44)), arguments[1]),
                ),
            },
        )

    # ---------- the first thing a sound that a sound effect started by itself is told

    random_ = random.Random(247)

    harness = t.harness
    tables = []
    for original in (True, False):
        names = {0: None}
        for name, symbol in harness.symbols.items():
            if symbol.source:
                names[symbol.address if original else harness.functions[name]] = name
        address = harness.globals["sound_effect_sound_callbacks"] if original else harness.variables["sound_effect_sound_callbacks"]
        tables.append([names.get(value, hex(value)) for value in struct.unpack("<8I", harness.mu.mem_read(address, 0x20))])
    if not t.only or "sound_effect_sound_unknown0" in t.only:
        t.cases["sound_effect_sound_unknown0"] = t.cases.get("sound_effect_sound_unknown0", 0) + 1
        if tables[0] != tables[1] or tables[0][0] != "sound_effect_sound_unknown0":
            t._fail("sound_effect_sound_unknown0", f"sound_effect_sound_callbacks: the game has {tables[0]}, source/ has {tables[1]}")

    for n in range(600):
        effects = bytearray(random_.randbytes(0x4C + EFFECTS * 0x14))
        first_unallocated = random_.choice((EFFECTS, EFFECTS, 8, 1, 0))
        struct.pack_into("<i", effects, 0x24, 0x14)
        struct.pack_into("<i", effects, 0x38, first_unallocated)
        struct.pack_into("<I", effects, 0x44, 0x4C)
        index = random_.randrange(EFFECTS)
        salt = random_.choice((1, 2, 0x7FFF, 0x8000, 0xFFFF, 0, 0))
        struct.pack_into("<H", effects, 0x4C + index * 0x14, salt)
        effects[0x4C + index * 0x14 + 4] = random_.choice((0, 0, 0x10, 0x10, 1, 0x11, 0xEE, 0xFE, random_.getrandbits(8)))
        told = bytearray(random_.randbytes(0x30))
        kind = random_.randrange(8)
        effect_index = (salt << 16 | index) if kind else NONE
        if kind == 1:
            effect_index ^= 0x10000
        struct.pack_into("<I", told, 0x28, effect_index)
        t.check(
            "sound_effect_sound_unknown0",
            random_.getrandbits(32),
            random_.getrandbits(32),
            Buffer(bytes(told)),
            Buffer(random_.randbytes(0x48)),
            globals={"sound_effects": Buffer(bytes(effects), pointers=(0x44,))},
            externals={"sound_effect_get_source": lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1])))},
        )

    # ---------- the sounds of a sound effect are started

    random_ = random.Random(248)

    harness = t.harness
    if not t.only or "sound_effect_start_sounds" in t.only:
        t.cases["sound_effect_start_sounds"] = t.cases.get("sound_effect_start_sounds", 0) + 1
        for name, size, expected in (
            ("sound_effect_looping_sound_states", 12, (0, 1, 2)),
            ("sound_effect_looping_sound_unknown_states", 16, (0, 1, 2, 2)),
        ):
            both = [bytes(harness.mu.mem_read(place[name], size)) for place in (harness.globals, harness.variables)]
            if both[0] != both[1] or both[0] != struct.pack(f"<{size // 4}i", *expected):
                t._fail("sound_effect_start_sounds", f"{name} is not the game's")

    for n in range(900):
        effects = bytearray(random_.randbytes(0x4C + EFFECTS * 0x14))
        struct.pack_into("<I", effects, 0x44, 0x4C)
        index = random_.randrange(EFFECTS)
        at = 0x4C + index * 0x14
        effect_index = random_.getrandbits(16) << 16 | index
        kind = random_.choice((0, 1, 2, 2))
        effects[at + 3] = kind
        effects[at + 4] = random_.choice((0, 0x12, 0x12, 0x10, 2, 0xFF, random_.getrandbits(8)))
        struct.pack_into("<I", effects, at + 8, random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(CONTROLLERS))))
        things = 4
        struct.pack_into("<I", effects, at + 0xC, random_.getrandbits(16) << 16 | random_.randrange(things))
        # the definition, after the effects: 0x10 bytes, and its sounds
        sounds = random_.choice((0, 1, 2, 3, 6))
        definition_at = len(effects)
        struct.pack_into("<I", effects, at + 0x10, definition_at)
        pointers = [0x44, at + 0x10, definition_at + 0xC]
        effects += random_.randbytes(8) + struct.pack("<iI", sounds, definition_at + 0x10)
        for number in range(6):
            effects += struct.pack(
                "<IIfI",
                random_.choice((0x6C736E64, 0x6C736E64, 0x736E6421, 0x736E6421, 0x736E6422, 0)),
                random_.choice((NONE, random_.getrandbits(16) << 16 | random_.randrange(TAGS), random_.randrange(TAGS), random_.randrange(TAGS))),
                random_.uniform(-6, 6),
                random_.getrandbits(32) if number % 2 else random_.choice((0, 1, 2, 3, 8, 0x10, 0x20, 0x30, 0x38)),
            )
        controllers = bytearray(random_.randbytes(0x4C + CONTROLLERS * 0x1C))
        struct.pack_into("<I", controllers, 0x44, 0x4C)
        unknown = bytearray(random_.randbytes(0x4C + things * 0x18))
        struct.pack_into("<I", unknown, 0x44, 0x4C)
        for thing in range(things):
            unknown[0x4C + thing * 0x18 + 2] = random_.randrange(4)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        tag_pointers = []
        for tag in range(TAGS):
            struct.pack_into("<I", tags, tag * 0x10 + 8, TAGS * 0x10 + (tag % 8) * 4)
            tag_pointers.append(tag * 0x10 + 8)
        tags += b"".join(struct.pack("<I", random_.choice((0, 8, 0x40, 0x48, random_.getrandbits(32)))) for _ in range(8))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        state[0x78:0x7B] = bytes(random_.choice((1, 1, 1, 1, 1, 1, 0, 0x80)) for _ in range(3))
        phase = random_.choice((0, 0, 1, 2, 2))
        said = random_.randbytes(0x42)

        def said_(harness, arguments, said=said):
            harness.mu.mem_write(arguments[1] + 2, said)
            return 0, (arguments[0],)

        def request_(harness, address):
            flags = struct.unpack("<I", harness.mu.mem_read(address, 4))[0]
            parts = [flags, bytes(harness.mu.mem_read(address + 4, 4)), bytes(harness.mu.mem_read(address + 0xC, 0x44))]
            if flags & 0x20:
                parts += [
                    bytes(harness.mu.mem_read(address + 0x54, 4)),
                    harness.normalize(struct.unpack("<I", harness.mu.mem_read(address + 0x58, 4))[0]),
                    bytes(harness.mu.mem_read(address + 0x5C, 0x32)),
                ]
            parts.append(bytes(harness.mu.mem_read(address + 0x94, 4)))
            return tuple(parts)

        def test_(harness, arguments, answer=random_.choice((1, 1, 1, 0)), listener=random_.getrandbits(32)):
            harness.mu.mem_write(arguments[2], struct.pack("<I", listener))
            return answer, (request_(harness, arguments[0]), arguments[1], arguments[3])

        def promotion_(harness, arguments, answer=random_.choice((0, 0, 0, 1, 0x100)), rule=random_.getrandbits(32)):
            harness.mu.mem_write(arguments[1], struct.pack("<I", rule))
            return answer, (arguments[0],)

        def create_(harness, arguments):
            return 0, (arguments[0], arguments[1], request_(harness, arguments[2]), arguments[3])

        def looping_(harness, arguments):
            return 0, (*arguments[:4], bytes(harness.mu.mem_read(arguments[4], 0x44)), *arguments[5:])

        t.check(
            "sound_effect_start_sounds",
            effect_index,
            phase,
            globals={
                "sound_effects": Buffer(bytes(effects), pointers=tuple(pointers)),
                "unknown_0051ebd8": Buffer(bytes(controllers), pointers=(0x44,)),
                "unknown_0051ebdc": Buffer(random_.randbytes(0x1C)),
                "unknown_004ed28c": Buffer(bytes(unknown), pointers=(0x44,)),
                "global_tag_instances": Buffer(bytes(tags), pointers=tuple(tag_pointers)),
                "game_sound_globals": Buffer(bytes(state)),
                "sound_effect_sound_callbacks": GlobalRange(0x20),
            },
            externals={
                "sound_effect_get_source": said_,
                "sound_start_test": test_,
                "sound_promotion_update": promotion_,
                "sound_source_create": create_,
                "unknown_0021a250": looping_,
            },
        )

    # ---------- a looping sound is told that a sound of one of its tracks has a channel

    random_ = random.Random(249)

    for n in range(700):
        looping, indices = looping_world()
        sources = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        struct.pack_into("<I", sources, 0x44, 0x4C)
        number = random_.randrange(PLAYING)
        at = 0x4C + number * DATUM
        sources[at + 4] = random_.choice((0, 0x80, 0x80, 0x7F, 0xFF))
        fade = random_.randrange(-5, 1000)
        struct.pack_into("<2i", sources, at + 0xB4, fade, random_.choice((fade, fade - 1, fade + 1, -1, 5000)))
        told = bytearray(random_.randbytes(0x30))
        told[4] = random_.randrange(3)
        t.check(
            "looping_sound_track_source_got_channel",
            random_.choice((NONE, *indices, *indices, indices[0] ^ 0x10000, 6, 0x10007)),
            random_.getrandbits(32),
            random_.getrandbits(16) << 16 | number,
            Buffer(bytes(told)),
            globals={"looping_sounds": looping, "sound_sources": Buffer(bytes(sources), pointers=(0x44,))},
        )

    # ---------- a looping sound is asked how a sound of one of its tracks is heard

    random_ = random.Random(250)

    for n in range(700):
        looping, indices = looping_world()
        data = bytearray(looping.data)
        for number in range(6):
            at = 0x4C + number * 0xD4
            tag_ = random_.getrandbits(32)
            struct.pack_into("<2I", data, at + 4, tag_, random_.choice((tag_ | 0x6000, tag_ | 0x6000, tag_, tag_ | 0x4000, tag_ | 0x2000, NONE)))
            data[at + 0x10 : at + 0x18] = reals(random_, 2)
            data[at + 0xC4 : at + 0xD4] = reals(random_, 4)
        sound_index = random_.getrandbits(16) << 16 | random_.randrange(TAGS)
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        struct.pack_into("<I", tags, (sound_index & 0xFFFF) * 0x10 + 8, len(tags))

        def decibels_(harness, arguments):
            return real_bits(bits_real(arguments[0]) * 0.5 - 3.0), (arguments[0],)

        def as_sound(harness, arguments):
            return real_bits(bits_real(arguments[1]) + bits_real(arguments[2]) * 0.25), (
                harness.normalize(arguments[0]),
                arguments[1],
                arguments[2],
            )

        t.check(
            "looping_sound_track_source_heard",
            random_.choice((NONE, *indices, *indices, *indices, indices[0] ^ 0x10000, 6, 0x10007)),
            sound_index,
            Buffer(random_.randbytes(0x30)),
            Buffer(random_.randbytes(0x40)),
            globals={
                "looping_sounds": Buffer(bytes(data), pointers=looping.pointers),
                "global_tag_instances": Buffer(bytes(tags) + random_.randbytes(0x20), pointers=((sound_index & 0xFFFF) * 0x10 + 8,)),
            },
            externals={"sound_gain_to_decibels": decibels_, "sound_definition_decibels": as_sound},
        )

    # ---------- a looping sound is told that a sound of one of its tracks is let go

    random_ = random.Random(251)

    for n in range(900):
        looping, indices = looping_world()
        data = bytearray(looping.data)
        sources = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        struct.pack_into("<I", sources, 0x44, 0x4C)
        salts = [random_.getrandbits(16) << 16 | number for number in range(PLAYING)]
        number = random_.randrange(PLAYING)
        at = 0x4C + number * DATUM
        own = salts[number]
        others = random_.sample([salt for salt in salts if salt != own], 2)
        kind = random_.randrange(6)
        if kind == 0:
            following, before = own, own
        elif kind == 5:
            # the same place in the array, of another sound
            following, before = own ^ 0x10000, others[0]
        elif kind == 1:
            following, before = others[0], others[0]
        elif kind == 2:
            following, before = own, others[1]
        else:
            following, before = others
        struct.pack_into("<2I", sources, at + 0x6C, following, before)
        track = random_.randrange(3)
        sources[at + 0x60] = track
        sources[at + 0x62] = random_.choice((0xFF, 0xFF, 0xFF, 0, 0x7F))
        controller = bytearray(random_.randbytes(0x1C))
        struct.pack_into("<I", controller, 4, random_.choice((NONE, random_.getrandbits(32), random_.getrandbits(32))))
        controller[3] = random_.choice((0, 1, 0x80, 0x81, 0x7F, 0xFF, 5))
        struct.pack_into("<I", sources, at + 0x5C, random_.choice((NONE, 0x30002, 0x30002, random_.getrandbits(32))))
        for index in range(6):
            first = 0x4C + index * 0xD4 + 0x88 + track * 0x14
            struct.pack_into("<I", data, first, random_.choice((own, own, own ^ 0x10000, others[0], NONE)))

        def controller_(harness, arguments, there=random_.random() < 0.85):
            return (STATE if there else 0), (arguments[0],)

        t.check(
            "looping_sound_track_source_deleted",
            random_.choice((NONE, *indices, *indices, *indices, indices[0] ^ 0x10000, 6, 0x10007)),
            own,
            random_.getrandbits(32),
            globals={
                "looping_sounds": Buffer(bytes(data), pointers=looping.pointers),
                "sound_sources": Buffer(bytes(sources), pointers=(0x44,)),
            },
            memory={STATE: Memory(bytes(controller))},
            externals={"sound_playback_controller_try_and_get": controller_},
        )

    # ---------- the sound that plays of a track of a looping sound is made

    random_ = random.Random(252)

    harness = t.harness
    tables = []
    for original in (True, False):
        names = {0: None}
        for number, name in enumerate(harness.externals):
            names[harness.symbols[name].address if original else harness_external_address(number)] = name
        for name, symbol in harness.symbols.items():
            if symbol.source:
                names[symbol.address if original else harness.functions[name]] = name
        address = harness.globals["looping_sound_track_callbacks"] if original else harness.variables["looping_sound_track_callbacks"]
        tables.append([names.get(value, hex(value)) for value in struct.unpack("<8I", harness.mu.mem_read(address, 0x20))])
    if not t.only or "looping_sound_source_new" in t.only:
        t.cases["looping_sound_source_new"] = t.cases.get("looping_sound_source_new", 0) + 1
        expected = [
            "looping_sound_get_source",
            "looping_sound_track_source_got_channel",
            None,
            "looping_sound_track_source_heard",
            "looping_sound_track_source_deleted",
            None,
            "unknown_000a5750",
            None,
        ]
        if tables[0] != tables[1] or tables[0] != expected:
            t._fail("looping_sound_source_new", f"looping_sound_track_callbacks: the game has {tables[0]}, source/ has {tables[1]}")

    for n in range(900):
        looping, indices = looping_world()
        data = bytearray(looping.data)
        looping_index = random_.choice(indices)
        at = 0x4C + (looping_index & 0xFFFF) * 0xD4
        definition_index = random_.randrange(TAGS)
        sound_index = random_.choice([number for number in range(TAGS) if number != definition_index])
        struct.pack_into("<I", data, at + 4, random_.getrandbits(16) << 16 | definition_index)
        for number in range(3):
            struct.pack_into("<f", data, at + 0x88 + number * 0x14 + 0x10, random_.choice((0.0, 1.0, 0.5, 0.25)))
        # the tags, and after them the looping sound's, the sound's, and what the sounds share
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        pointers = []
        definition_at = len(tags)
        struct.pack_into("<I", tags, definition_index * 0x10 + 8, definition_at)
        pointers.append(definition_index * 0x10 + 8)
        definition = bytearray(random_.randbytes(0x24))
        definition[0] = random_.choice((0, 0x20, 0x20, 0x10, 0x30, 0xFF, 0xDF))
        struct.pack_into("<I", definition, 0x20, definition_at + 0x24)
        pointers.append(definition_at + 0x20)
        tags += definition + random_.randbytes(3 * 0x58)
        sound_at = len(tags)
        struct.pack_into("<I", tags, sound_index * 0x10 + 8, sound_at)
        pointers.append(sound_index * 0x10 + 8)
        sound_ = bytearray(random_.randbytes(0x10))
        sound_[1] = random_.choice((0, 0x20, 0x20, 0xDF, 0xFF))
        struct.pack_into("<h", sound_, 6, random_.randrange(3))
        sound_[0xB] = random_.randrange(3)
        tags += sound_
        gestalt = len(tags)
        tags += random_.randbytes(0x20)
        struct.pack_into("<I", tags, gestalt + 4, gestalt + 0x20)
        struct.pack_into("<I", tags, gestalt + 0xC, gestalt + 0x20 + 3 * 0x38)
        pointers += [gestalt + 4, gestalt + 0xC]
        for number in range(3):
            playback = bytearray(random_.randbytes(0x38))
            struct.pack_into("<2h", playback, 0x18, random_.choice((0, -1200, 1200, 100, -32768, 32767)), random_.randrange(-1200, 1200))
            tags += playback
        for number in range(3):
            scale = bytearray(random_.randbytes(0x14))
            struct.pack_into("<2h", scale, 8, random_.choice((0, -1200, -100, 50)), random_.choice((0, 1200, 100, 50)))
            tags += scale
        sources = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        struct.pack_into("<I", sources, 0x44, 0x4C)
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        made_index = random_.choice((NONE, *(random_.getrandbits(16) << 16 | number for number in range(PLAYING))))
        listener = random_.choice((NONE, 0, 1, 2, 3, 0x100, 1))
        can = random_.choice((1, 1, 1, 1, 0))
        chosen = random_.choice((0, 1, 2, 0x10001, 0xFFFF0002))
        recording = random_.choice((0, 1, 5, 0x100, 0x7F))
        new_played = random_.getrandbits(32)

        def normal(harness, value):
            return harness.normalize(value)

        def permutation_(harness, arguments, recording=recording, new_played=new_played):
            before = struct.unpack("<I", harness.mu.mem_read(arguments[2], 4))[0]
            harness.mu.mem_write(arguments[2], struct.pack("<I", new_played))
            seed_ = struct.unpack("<I", harness.mu.mem_read(arguments[4], 4))[0]
            return recording, (normal(harness, arguments[0]), arguments[1] & 0xFFFF, before, arguments[3], seed_, arguments[5], arguments[6] & 0xFF)

        t.check(
            "looping_sound_source_new",
            looping_index,
            random_.getrandbits(16) << 16 | sound_index,
            random_.choice((NONE, NONE, 0, 1, 2)),
            random_.getrandbits(16) << 16 | random_.randrange(3),
            random_.choice((0, 1, 2, 2, 0x10002, 0x102)),
            Buffer(struct.pack("<I", random_.getrandbits(32))),
            globals={
                "looping_sounds": Buffer(bytes(data), pointers=looping.pointers),
                "global_tag_instances": (tags_ := Buffer(bytes(tags), pointers=tuple(pointers))),
                "global_sound_cache_file_gestalt": Pointer(tags_, gestalt),
                "sound_sources": Buffer(bytes(sources), pointers=(0x44,), places=tuple(0x4C + number * DATUM + 0x14 for number in range(PLAYING))),
                "game_sound_globals": Buffer(bytes(state)),
                "looping_sound_track_callbacks": GlobalRange(0x20),
            },
            memory={STATE: Memory(random_.randbytes(0x1C))},
            externals={
                "sound_definition_unknown_00126bd0": lambda harness, arguments, can=can: (can, tuple(arguments)),
                "sound_get_maximum_distance": lambda harness, arguments: (real_bits(12.5), (normal(harness, arguments[0]), arguments[1])),
                "sound_source_get_listener": lambda harness, arguments, listener=listener: (
                    listener,
                    (normal(harness, arguments[0]), arguments[1], arguments[2]),
                ),
                "datum_new": lambda harness, arguments, made_index=made_index: (made_index, (normal(harness, arguments[0]),)),
                "sound_playback_controller_get": lambda harness, arguments: (STATE, tuple(arguments)),
                "sound_playback_controller_add_count": lambda harness, arguments: (0, (arguments[0], arguments[1] & 0xFF)),
                "sound_choose_pitch_range": lambda harness, arguments, chosen=chosen: (
                    chosen,
                    (arguments[0] & 0xFFFF, normal(harness, arguments[1]), arguments[2]),
                ),
                "looping_sound_track_get_played_bits": lambda harness, arguments: (
                    0x1234ABCD,
                    (arguments[0], normal(harness, arguments[1]), normal(harness, arguments[2]), normal(harness, arguments[3]), arguments[4] & 0xFFFF, arguments[5] & 0xFF),
                ),
                "sound_unknown_0021cc30": lambda harness, arguments: (
                    0x77,
                    (normal(harness, arguments[0]), normal(harness, arguments[1]), arguments[2]),
                ),
                "sound_pitch_range_choose_permutation": permutation_,
                "looping_sound_track_set_played_bits": lambda harness, arguments: (
                    0,
                    (arguments[0], arguments[1], normal(harness, arguments[2]), normal(harness, arguments[3]), normal(harness, arguments[4]), arguments[5] & 0xFFFF, arguments[6] & 0xFF),
                ),
                "sound_source_set_sound": lambda harness, arguments: (
                    0,
                    (arguments[0], arguments[1], arguments[2] & 0xFF, arguments[3] & 0xFF, arguments[4] & 0xFFFF),
                ),
                "sound_source_request_data": lambda harness, arguments: (1, (normal(harness, arguments[0]),)),
                "looping_sound_add_source": lambda harness, arguments: (0, (arguments[0], arguments[1] & 0xFFFF, arguments[2])),
            },
        )

    # ---------- the console's sound tells the console what has changed

    random_ = random.Random(253)

    harness = t.harness
    if not t.only or "sound_console_commit" in t.only:
        t.cases["sound_console_commit"] = t.cases.get("sound_console_commit", 0) + 1
        both = [bytes(harness.mu.mem_read(place_["sound_console_place_effects"], 8)) for place_ in (harness.globals, harness.variables)]
        if both[0] != both[1] or both[0] != struct.pack("<2i", 9, 8):
            t._fail("sound_console_commit", "sound_console_place_effects is not the game's")

    PI = 3.14159265
    for n in range(1500):
        console = bytearray(random_.randbytes(0x2AC0))
        struct.pack_into("<h", console, 4, random_.choice((0, 1, 2, 5, 5, -1)))
        for place_ in range(2):
            console[0x2A08 + place_] = random_.choice((0, 0, 1, 0x80))
            console[0x2A0A + place_] = random_.choice((0, 1, 1, 1, 0x80))
            kind = random_.randrange(6)
            if kind == 0:
                start, end = sorted((random_.uniform(-7, 7), random_.uniform(-7, 7)))
            elif kind == 1:
                start = random_.choice((-PI / 4, PI / 4, 3 * PI / 4, -3 * PI / 4, 0.0, PI, -PI, 2 * PI, -2 * PI))
                end = start + random_.choice((0.0, PI / 2, PI, 2 * PI, 0.1))
            elif kind == 2:
                end, start = sorted((random_.uniform(-7, 7), random_.uniform(-7, 7)))
            else:
                start = random_.uniform(-PI, PI)
                end = start + random_.uniform(0, PI)
            struct.pack_into(
                "<4f",
                console,
                0x29E8 + place_ * 0x10,
                start,
                end,
                random_.choice((0.0, 1.0, 0.5, 0.25, 2.0, -1.0)),
                random_.choice((0.0, 1.0, 0.5, 0.1, -0.5, 3.0)),
            )
            struct.pack_into("<f", console, 0x29E0 + place_ * 4, random_.choice((0.0, 1.0, 1.0, 0.5, 0.7, 0.01, 2.0, -1.0, 1e-9)))
        old = len(console)
        console += struct.pack("<4I", old + 4, mock_address(0), mock_address(1), mock_address(2))
        struct.pack_into("<I", console, 0x2AB4, old)
        t.check(
            "sound_console_commit",
            globals={
                "unknown_0051ebe4": Buffer(bytes(console), pointers=(old, 0x2AB4)),
                "unknown_0047005c": bytes([random_.choice((0, 1, 1, 0x80))]),
                "unknown_00470060": GlobalRange(0x24),
            },
            mocks=[
                Mock(lambda call: 0, stack_arguments=3, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=0, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=2, registers=("ecx",)),
            ],
            externals={
                "sound_console_set_reverb": lambda harness, arguments: (0, (harness.normalize(arguments[0]), arguments[1])),
                "unknown_0037b66f": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_0037d141": lambda harness, arguments: (0, tuple(arguments)),
                "unknown_002ae820": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "unknown_00191550": lambda harness, arguments: (0, ()),
            },
        )

    # ---------- the stream of a voice of the console's sound takes the effects of its sound

    random_ = random.Random(254)

    for n in range(1200):
        console = bytearray(random_.randbytes(0x2AC0))
        voice_index = random_.randrange(8)
        at = 0xC + voice_index * 0x34
        fraction = random_.choice((0.0, 0.5, 1.0, 0.25, random_.random()))
        console[at + 1] = random_.choice((0, 0, 0xC7, 0x08, 0x20, 0x38, random_.getrandbits(8)))
        struct.pack_into(
            "<f", console, at + 4, fraction + random_.choice((0.0, 0.0, 0.00005, -0.00005, 0.0002, -0.0002, 0.3))
        )
        effects = bytearray(random_.randbytes(0x420))
        struct.pack_into("<If", effects, 0, random_.getrandbits(32), fraction)
        pointers = []
        for number, size in enumerate((0x48, 0x30, 0x40)):
            where = 0x10 + number * 4
            if random_.random() < 0.75:
                struct.pack_into("<I", effects, where, len(effects))
                pointers.append(where)
                thing = bytearray(random_.randbytes(size))
                struct.pack_into("<2i", thing, 0, random_.choice((0, 1, 2, 2, 3, -1)), random_.choice((0, 1, 5, 7, 8, -1, 100)))
                effects += thing
            else:
                struct.pack_into("<I", effects, where, 0)
        answers = [
            random_.choice((0.0, 1.0, 0.5, 2.0, 10.0, 43.69, 100.0, -1.0, -10.0, 1e10, -1e10, 0.0234, 7.99, 8.01, -8.01, random_.uniform(-12, 12)))
            for _ in range(12)
        ]

        def number_(harness, arguments, answers=answers):
            answer = answers[calls_so_far(harness, "sound_unknown_00219880") % 12]
            seed_ = struct.unpack("<I", harness.mu.mem_read(arguments[1], 4))[0]
            harness.mu.mem_write(arguments[1], struct.pack("<I", (seed_ * 5 + 1) & 0xFFFFFFFF))
            return real_bits(answer), (harness.normalize(arguments[0]), seed_, arguments[2])

        def told_(harness, arguments):
            return 0, (arguments[0], bytes(harness.mu.mem_read(arguments[1], 24)))

        effects_ = Buffer(bytes(effects), pointers=tuple(pointers))
        t.check(
            "sound_console_voice_apply_effects",
            voice_index,
            effects_ if n % 23 else 0,
            random_.choice((0, 0, 1, 1, 1, 0x100, 0x80)),
            globals={"unknown_0051ebe4": Buffer(bytes(console))},
            externals={
                "sound_unknown_00219880": number_,
                "sound_frequency_to_filter_coefficient": lambda harness, arguments: ((arguments[0] >> 3) ^ 0x1111, tuple(arguments)),
                "sound_unknown_0021e2d0": lambda harness, arguments: ((arguments[0] >> 5) ^ 0x2222, tuple(arguments)),
                "sound_unknown_0021e330": lambda harness, arguments: ((arguments[0] >> 7) ^ 0x3333, tuple(arguments)),
                "unknown_0037b813": told_,
                "unknown_0037b809": told_,
                "sound_console_apply_effect_list": lambda harness, arguments: (0, (arguments[0], harness.normalize(arguments[1]))),
            },
        )

    # ---------- a voice of the console's sound takes how its sound is to be heard

    random_ = random.Random(255)

    def mixbins_(harness, address) -> tuple:
        """What some outputs say: how many they are, and each."""
        count_ = struct.unpack("<i", harness.mu.mem_read(address + 0x40, 4))[0]
        return count_, bytes(harness.mu.mem_read(address, 8 * max(0, min(count_, 8))))

    def places_(harness, address) -> tuple:
        flags_ = struct.unpack("<I", harness.mu.mem_read(address, 4))[0]
        parts = [flags_]
        if flags_ & 1:
            parts.append(bytes(harness.mu.mem_read(address + 4, 16)))
        if flags_ & 2:
            parts.append(bytes(harness.mu.mem_read(address + 0x14, 4)))
        if flags_ & 0x20:
            parts.append(bytes(harness.mu.mem_read(address + 4, 8)))
        return tuple(parts)

    for n in range(1500):
        console = bytearray(random_.randbytes(0x2AC0))
        console[0] = random_.choice((1, 1, 1, 1, 0, 0x80))
        slot_count = random_.choice((0, 0, 3, 3, 64))
        struct.pack_into("<h", console, 8, slot_count)
        voice_index = random_.randrange(8)
        at = 0xC + voice_index * 0x34
        parameters = bytearray(random_.randbytes(0x48))
        struct.pack_into(
            "<if",
            parameters,
            4,
            random_.choice((0, 1, 2, 3, -1)),
            random_.choice((0.0, 1200.0, -1200.0, 100.0, 5000.0, -9000.0, random_.uniform(-2400, 2400))),
        )
        struct.pack_into("<f", parameters, 0xC, random_.choice((0.0, -6.0, -64.0, -80.0, 3.0, -20.05, random_.uniform(-64, 0))))
        struct.pack_into("<4f", parameters, 0x14, *(random_.choice((0.0, 0.0, 0.05, 0.2, 0.1, -0.1, 0.12, 1.0, -0.5, random_.uniform(-1, 1))) for _ in range(4)))
        struct.pack_into("<h", parameters, 0x24, random_.choice((-1, -1, 0, 1, 2, 3, 5, -2)))
        struct.pack_into("<f", parameters, 0x28, random_.choice((0.0, -6.0, -64.0, random_.uniform(-64, 0))))
        struct.pack_into("<I", parameters, 0x30, random_.choice((0, 1, 0x20, 0x21, 2, 0x5A, 0x7F, random_.getrandbits(7), random_.getrandbits(32))))
        struct.pack_into("<4f", parameters, 0x38, *(random_.choice((0.0, -6.0, -64.0, -100.0, 5.0, random_.uniform(-64, 0))) for _ in range(4)))
        kind = random_.randrange(3)
        console[at + 1] = random_.choice((0, 0, 0x08, 0x08, 0x10, 0x38, random_.getrandbits(8)))
        console[at + 3] = random_.choice((1, 1, 1, 0x21, 0x21, 0, 0x20, 0xFF, 0xDF))

        def near(offset: int, value: float) -> None:
            struct.pack_into("<f", console, at + offset, value + (random_.choice((0.0, 0.05, -0.05, 0.2, -0.2, 3.0, 0.12, -0.12)) if kind else 0.0))

        near(0xC, max(-64.0, struct.unpack_from("<f", parameters, 0xC)[0]))
        near(0x10, struct.unpack_from("<f", parameters, 0x18)[0])
        near(0x18, struct.unpack_from("<f", parameters, 0x14)[0])
        near(0x14, struct.unpack_from("<f", parameters, 0x28)[0])
        struct.pack_into("<i", console, at + 8, random_.choice((0, -4597, -501, -2396, 8191, -32767, random_.randrange(-9000, 9000))))
        if random_.random() < 0.3:
            base = (-4597, -501, -2396, -2396)[min(3, struct.unpack_from("<i", parameters, 4)[0] & 3)]
            wanted = int(max(-32767.0, min(8191.0, struct.unpack_from("<f", parameters, 8)[0] * 3.4133334 + base)))
            struct.pack_into("<i", console, at + 8, wanted + random_.choice((0, 1, -1, 2, -2)))
        old = len(console)
        console += struct.pack("<8I", old + 4, *(mock_address(number) for number in range(7)))
        struct.pack_into("<I", console, 0x2AB4, old)
        harness = t.harness

        def filled(call, harness=harness):
            # the handler fills one more output
            address = call.stack[3]
            count_ = struct.unpack("<i", harness.mu.mem_read(address + 0x40, 4))[0]
            if 0 <= count_ < 8:
                harness.mu.mem_write(address + count_ * 8, struct.pack("<2i", call.mock, -100 * count_))
                harness.mu.mem_write(address + 0x40, struct.pack("<i", count_ + 1))
            return 0

        def add_(harness, arguments):
            address = arguments[0]
            before = mixbins_(harness, address)
            if 0 <= before[0] < 8:
                harness.mu.mem_write(address + before[0] * 8, struct.pack("<2I", arguments[1], arguments[2]))
                harness.mu.mem_write(address + 0x40, struct.pack("<i", before[0] + 1))
            return 0, (before, arguments[1], arguments[2])

        def given(harness, arguments, seed=random_.getrandbits(32)):
            harness.mu.mem_write(arguments[1], random.Random(seed + arguments[0]).randbytes(0x5C))
            return 0, (arguments[0],)

        def set_(harness, arguments):
            return 0, (bytes(harness.mu.mem_read(arguments[0], 0x5C)), mixbins_(harness, arguments[1]), arguments[2])

        def fraction_(harness, arguments):
            value = bits_real(arguments[2])
            return real_bits(max(0.0, min(1.0, (value + 64.0) / 64.0))), (arguments[0], arguments[1], arguments[2], arguments[3] & 0xFF)

        def status_(harness, arguments, status=random_.choice((0, 1, 1, 2, 3))):
            harness.mu.mem_write(arguments[1], struct.pack("<I", status))
            return 0, (arguments[0],)

        def outputs_(harness, arguments):
            count_ = struct.unpack("<i", harness.mu.mem_read(arguments[1], 4))[0]
            pairs = struct.unpack("<I", harness.mu.mem_read(arguments[1] + 4, 4))[0]
            return 0, (arguments[0], count_, bytes(harness.mu.mem_read(pairs, 8 * max(0, min(count_, 8)))))

        def all_(harness, arguments):
            return 0, tuple(arguments)

        with_places = lambda call, harness=harness: (call.mock, places_(harness, call.stack[0]), call.stack[1], call.stack[2], mixbins_(harness, call.stack[3]))
        t.check(
            "sound_console_voice_set_parameters",
            Buffer(bytes(parameters)),
            voice_index,
            random_.getrandbits(32),
            random_.choice((0, 0, 0, 1, 0x100, 0x80)),
            globals={"unknown_0051ebe4": Buffer(bytes(console), pointers=(old, 0x2AB4))},
            mocks=[
                Mock(lambda call: 0, stack_arguments=3, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=0, registers=("ecx",)),
                Mock(lambda call: 0, stack_arguments=2, registers=("ecx",)),
                Mock(filled, stack_arguments=4, registers=("ecx",), record=with_places),
                Mock(
                    filled,
                    stack_arguments=4,
                    registers=("ecx",),
                    record=lambda call, harness=harness: (call.mock, call.stack[0], call.stack[1], call.stack[2], mixbins_(harness, call.stack[3])),
                ),
                Mock(
                    filled,
                    stack_arguments=4,
                    registers=("ecx",),
                    record=lambda call, harness=harness: (
                        call.mock,
                        bytes(harness.mu.mem_read(call.stack[0], 0x48)),
                        call.stack[1],
                        call.stack[2],
                        mixbins_(harness, call.stack[3]),
                    ),
                ),
                Mock(filled, stack_arguments=4, registers=("ecx",), record=with_places),
            ],
            externals={
                "unknown_0037b83f": given,
                "unknown_0037b7e3": given,
                "sound_mixbins_add": add_,
                "sound_console_stream_set_mixbins": set_,
                "sound_console_slot_stream_set_mixbins": set_,
                "sound_interpolation_fraction": fraction_,
                "unknown_0037b7ff": all_,
                "unknown_0037b66f": all_,
                "unknown_0037b75b": status_,
                "unknown_0037b6df": all_,
                "unknown_0037b71b": all_,
                "unknown_0037b804": all_,
                "unknown_0037c70a": outputs_,
                "sound_console_voice_apply_effects": all_,
            },
        )

    # ---------- what is done each time with the sound of a track that has a voice

    random_ = random.Random(256)

    for n in range(2000):
        channel_index = random_.randrange(CHANNELS)
        channels = bytearray(random_.randbytes(CHANNELS * 0x24))
        number = random_.randrange(PLAYING)
        own = random_.getrandbits(16) << 16 | number
        struct.pack_into("<I", channels, channel_index * 0x24, own)
        for index in range(CHANNELS):
            struct.pack_into("<h", channels, index * 0x24 + 0xC, random_.randrange(8))
        sources = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        struct.pack_into("<I", sources, 0x44, 0x4C)
        at = 0x4C + number * DATUM
        looping, indices = looping_world()
        data = bytearray(looping.data)
        looping_number = random_.randrange(6)
        where = 0x4C + looping_number * 0xD4
        definition_index = random_.randrange(TAGS)
        sound_index = random_.choice([index for index in range(TAGS) if index != definition_index])
        track = random_.randrange(3)
        struct.pack_into("<I", data, where + 4, random_.getrandbits(16) << 16 | definition_index)
        struct.pack_into("<I", data, where + 0x54, random_.choice((NONE, NONE, 0x50000 | random_.randrange(CONTROLLERS), 0x50000 | random_.randrange(CONTROLLERS))))
        struct.pack_into("<I", data, where + 0x88 + track * 0x14, random_.choice((own, own, own, own ^ 0x10000, NONE)))
        struct.pack_into("<f", data, where + 0x88 + track * 0x14 + 0x10, random_.choice((0.0, 1.0, 0.5)))
        sources[at + 2] = random_.choice((1, 1, 2, 2, 3, 3, 4, 0))
        struct.pack_into("<H", sources, at + 4, random_.choice((0x200, 0x200, 0x200, 0x208, 0x208, 0, 0x248, 0x8, 0xFDF7, random_.getrandbits(16))))
        sources[at + 6] = random_.choice((0, 2, 2, 0xFD, 0xFF))
        struct.pack_into("<I", sources, at + 0xC, random_.getrandbits(16) << 16 | sound_index)
        struct.pack_into("<I", sources, at + 0x10, random_.getrandbits(16) << 16 | looping_number)
        struct.pack_into("<2f", sources, at + 0x1C, random_.choice((0.0, 1.0, 0.5)), random_.uniform(-20, 0))
        sources[at + 0x24 : at + 0x3C] = reals(random_, 6)
        sources[at + 0x60] = track
        sources[at + 0x61] = random_.choice((0, 0, 0, 1, 0xFE))
        struct.pack_into("<2f", sources, at + 0x90, random_.uniform(-600, 600), random_.uniform(-12, 3))
        sources[at + 0x9C] = random_.randrange(2)
        sources[at + 0x9D] = random_.randrange(3)
        struct.pack_into("<h", sources, at + 0x9E, random_.randrange(3))
        sources[at + 0xA1] = random_.randrange(4)
        struct.pack_into("<h", sources, at + 0xAC, random_.choice((channel_index, channel_index, random_.randrange(CHANNELS))))
        # the tags, the sound, the looping sound's tag and what the sounds share
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        pointers = [sound_index * 0x10 + 8, definition_index * 0x10 + 8]
        struct.pack_into("<I", tags, sound_index * 0x10 + 8, len(tags))
        sound_ = bytearray(random_.randbytes(0x10))
        struct.pack_into("<h", sound_, 8, random_.randrange(2))
        sound_[0xB] = random_.randrange(3)
        tags += sound_
        definition_at = len(tags)
        struct.pack_into("<I", tags, definition_index * 0x10 + 8, definition_at)
        definition = bytearray(random_.randbytes(0x24))
        definition[0] = random_.choice((0, 0x20, 0x20, 0x10, 0x30, 2, 0x22, 0x32, 0xFD, random_.getrandbits(8)))
        struct.pack_into("<I", definition, 0x20, definition_at + 0x24)
        pointers.append(definition_at + 0x20)
        tags += definition
        for index in range(3):
            piece = bytearray(random_.randbytes(0x58))
            struct.pack_into("<h", piece, 0x3C, random_.choice((0, 0, 1, 5, -1)))
            tags += piece
        gestalt = len(tags)
        tags += random_.randbytes(0x48)
        blocks = {}
        for offset, count, size in ((0xC, 3, 0x14), (0x1C, 4, 10), (0x24, 4, 0xC), (0x2C, 8, 0x10), (0x44, 8, 0xC)):
            struct.pack_into("<I", tags, gestalt + offset, len(tags))
            pointers.append(gestalt + offset)
            blocks[offset] = len(tags)
            tags += random_.randbytes(count * size)
        for index in range(3):
            struct.pack_into("<2h", tags, blocks[0xC] + index * 0x14 + 8, random_.choice((0, -1200, -100)), random_.choice((0, 1200, 100)))
        for index in range(4):
            struct.pack_into("<h", tags, blocks[0x1C] + index * 10, random_.choice((0, 100, -300, 1200)))
            struct.pack_into("<h", tags, blocks[0x24] + index * 0xC + 2, random_.randrange(4))
            struct.pack_into("<h", tags, blocks[0x24] + index * 0xC + 8, random_.randrange(3))
        for index in range(8):
            struct.pack_into("<2h", tags, blocks[0x2C] + index * 0x10 + 0xC, random_.randrange(4), random_.choice((1, 2, 3, 3)))
            struct.pack_into("<I", tags, blocks[0x44] + index * 0xC + 4, random_.getrandbits(31) | random_.choice((0, 0x80000000)))
        controllers = bytearray(random_.randbytes(0x4C + CONTROLLERS * 0x1C))
        struct.pack_into("<I", controllers, 0x44, 0x4C)
        for index in range(CONTROLLERS):
            struct.pack_into("<I", controllers, 0x4C + index * 0x1C + 4, random_.choice((NONE, random_.getrandbits(32), random_.getrandbits(32))))
            controllers[0x4C + index * 0x1C + 0xC : 0x4C + index * 0x1C + 0x1C] = reals(random_, 4)
        lone = bytearray(random_.randbytes(0x1C))
        struct.pack_into("<I", lone, 4, random_.choice((NONE, random_.getrandbits(32))))
        console = bytearray(random_.randbytes(0x2AC0))
        for index in range(8):
            console[0xC + index * 0x34 + 1] = random_.choice((0, 1, 2, 3, 3, 3, 0xFB, random_.getrandbits(8)))
        class_ = bytearray(random_.randbytes(0x60))
        struct.pack_into("<f", class_, 0x48, random_.choice((0.0, 1.0, 0.5)))
        parameters = bytearray(random_.randbytes(0x40))
        struct.pack_into("<f", parameters, 0x14, random_.choice((0.0, 1.0, 0.25)))
        go_on = [random_.choice((0, 0, 1, 1)) for _ in range(4)]
        harness = t.harness

        def normal(harness, value):
            return harness.normalize(value)

        def asked(harness, address):
            """What is asked of a voice: its start as it is, and the rest in short."""
            whole = bytes(harness.mu.mem_read(address, 0x478))
            return whole[:0x48].hex(), zlib.crc32(whole[0x48:])

        def filled_(harness, arguments):
            # what is asked of the voice so far, and some more of it
            before = asked(harness, arguments[2])
            harness.mu.mem_write(arguments[2] + 0x10, bytes(range(0x10, 0x20)))
            return 0, (arguments[0] & 0xFFFF, normal(harness, arguments[1]), before, normal(harness, arguments[3]))

        def state_(harness, arguments, go_on=go_on):
            return go_on[calls_so_far(harness, "looping_sound_unknown_0021a050") % 4], (normal(harness, arguments[0]), normal(harness, arguments[1]))

        def chosen_(harness, arguments, recording=random_.choice((0, 1, 2, NONE, 0xFFFF, 0x10001)), new_played=random_.getrandbits(32)):
            before = struct.unpack("<I", harness.mu.mem_read(arguments[2], 4))[0]
            harness.mu.mem_write(arguments[2], struct.pack("<I", new_played))
            return recording, (normal(harness, arguments[0]), arguments[1] & 0xFFFF, before, arguments[3] & 0xFFFF, normal(harness, arguments[4]), arguments[5], arguments[6] & 0xFF)

        def voice_(harness, arguments):
            return 0, (asked(harness, arguments[0]), arguments[1], arguments[2] - arguments[0], arguments[3] & 0xFF)

        t.check(
            "looping_sound_channel_update_voice",
            channel_index,
            as_bits(random_.uniform(-10, 0)),
            Buffer(bytes(parameters)),
            globals={
                "sound_channels": Buffer(bytes(channels)),
                "sound_sources": Buffer(bytes(sources), pointers=(0x44,)),
                "looping_sounds": Buffer(bytes(data), pointers=looping.pointers),
                "global_tag_instances": (tags_ := Buffer(bytes(tags), pointers=tuple(pointers))),
                "global_sound_cache_file_gestalt": Pointer(tags_, gestalt),
                "unknown_0051ebd8": Buffer(bytes(controllers), pointers=(0x44,)),
                "unknown_0051ebdc": Buffer(bytes(lone)),
                "unknown_0051ebe4": Buffer(bytes(console)),
                "global_random_seed_reference": Buffer(random_.randbytes(16)),
            },
            memory={STATE: Memory(bytes(class_))},
            externals={
                "sound_definition_decibels": lambda harness, arguments: (
                    real_bits(bits_real(arguments[1]) * 0.5 - bits_real(arguments[2])),
                    (normal(harness, arguments[0]), arguments[1], arguments[2]),
                ),
                "sound_class_get": lambda harness, arguments: (STATE, (arguments[0] & 0xFFFF,)),
                "sound_listener_doppler_pitch": lambda harness, arguments: (real_bits(37.5), (arguments[0], normal(harness, arguments[1]))),
                "sound_channel_build_voice_parameters": filled_,
                "sound_console_voice_start": lambda harness, arguments: (
                    0,
                    (asked(harness, arguments[0]), arguments[1] - arguments[0], arguments[2] & 0xFFFF),
                ),
                "sound_console_stream_set_function": lambda harness, arguments: (0, (bytes(harness.mu.mem_read(arguments[0], 0x10)),)),
                "sound_channel_queue_chunk": lambda harness, arguments: (
                    0,
                    (arguments[0] & 0xFFFF, normal(harness, arguments[1]), arguments[2] & 0xFFFF),
                ),
                "sound_channel_set_voice_flag3": lambda harness, arguments: (0, (arguments[0] & 0xFFFF,)),
                "looping_sound_unknown_0021a050": state_,
                "looping_sound_unknown_0021a080": lambda harness, arguments, answer=random_.choice((1, 1, 0)): (
                    answer,
                    (normal(harness, arguments[0]),),
                ),
                "looping_sound_source_next_state": lambda harness, arguments, answer=random_.choice((0, 0, 0, 1)): (
                    answer,
                    (normal(harness, arguments[0]), normal(harness, arguments[1]), arguments[2], normal(harness, arguments[3])),
                ),
                "looping_sound_track_get_played_bits": lambda harness, arguments: (
                    0x1234ABCD,
                    (arguments[0], normal(harness, arguments[1]), normal(harness, arguments[2]), normal(harness, arguments[3]), arguments[4] & 0xFFFF, arguments[5] & 0xFF),
                ),
                "sound_pitch_range_choose_permutation": chosen_,
                "looping_sound_track_set_played_bits": lambda harness, arguments: (
                    0,
                    (arguments[0], arguments[1], normal(harness, arguments[2]), normal(harness, arguments[3]), normal(harness, arguments[4]), arguments[5] & 0xFFFF, arguments[6] & 0xFF),
                ),
                "sound_source_set_sound": lambda harness, arguments: (
                    0,
                    (arguments[0], arguments[1], arguments[2] & 0xFF, arguments[3] & 0xFF, arguments[4] & 0xFFFF),
                ),
                "sound_source_request_data": lambda harness, arguments, answer=random_.choice((1, 1, 1, 0)): (
                    answer,
                    (normal(harness, arguments[0]),),
                ),
                "sound_permutation_next_chunk": lambda harness, arguments, answer=random_.choice((NONE, 0xFFFF, 1, 0x1FFFF)): (
                    answer,
                    (normal(harness, arguments[0]), arguments[1] & 0xFFFF),
                ),
                "sound_console_voice_set_parameters": voice_,
            },
        )

    # ---------- what is done each time with each looping sound

    random_ = random.Random(257)

    for n in range(1200):
        LOOPING = 4
        data = bytearray(random_.randbytes(0x50 + LOOPING * 0xD4))
        struct.pack_into("<I", data, 0x24, 0xD4)
        struct.pack_into("<i", data, 0x38, random_.choice((LOOPING, LOOPING, 2)))
        struct.pack_into("<2I", data, 0x44, 0x50, 0x4C)
        struct.pack_into("<I", data, 0x4C, random_.choice((0xF, 0xF, 0x5, 0xA, 0x1, 0, random_.getrandbits(4))))
        sources = bytearray(random_.randbytes(0x4C + PLAYING * DATUM))
        struct.pack_into("<I", sources, 0x44, 0x4C)
        salts = [random_.getrandbits(16) << 16 | number for number in range(PLAYING)]
        tags = bytearray(random_.randbytes(TAGS * 0x10))
        pointers = []
        # two sounds, two tags of looping sounds, and what the sounds share
        sound_indices = random_.sample(range(TAGS), 4)
        sounds_at = []
        for number in range(2):
            struct.pack_into("<I", tags, sound_indices[number] * 0x10 + 8, len(tags))
            pointers.append(sound_indices[number] * 0x10 + 8)
            sound_ = bytearray(random_.randbytes(0x10))
            sound_[1] = random_.choice((0, 0x20, 0x20, 0xDF, 0xFF))
            struct.pack_into("<2h", sound_, 6, random_.randrange(3), random_.randrange(2))
            sound_[0xA] = random_.choice((1, 2, 3, 0))
            sound_[0xB] = random_.randrange(3)
            tags += sound_
        definitions_at = []
        for number in range(2):
            at = len(tags)
            definitions_at.append(at)
            struct.pack_into("<I", tags, sound_indices[2 + number] * 0x10 + 8, at)
            pointers.append(sound_indices[2 + number] * 0x10 + 8)
            definition = bytearray(random_.randbytes(0x2C))
            struct.pack_into("<iIiI", definition, 0x1C, random_.choice((0, 1, 2, 3)), at + 0x2C, random_.choice((0, 1, 2, 4)), at + 0x2C + 3 * 0x58)
            pointers += [at + 0x20, at + 0x28]
            tags += definition + random_.randbytes(3 * 0x58)
            for index in range(4):
                detail = bytearray(random_.randbytes(0x34))
                struct.pack_into("<I2f", detail, 8, random_.choice((NONE, 0x30000 | sound_indices[0], 0x30000 | sound_indices[1])), random_.uniform(0, 2), random_.uniform(2, 5))
                struct.pack_into("<I", detail, 0x18, random_.choice((0, 0, 1, 2, 3, random_.getrandbits(32))))
                tags += detail
        gestalt = len(tags)
        tags += random_.randbytes(0x48)
        blocks = {}
        for offset, count, size in ((4, 3, 0x38), (0xC, 3, 0x14), (0x1C, 4, 10), (0x24, 6, 0xC)):
            struct.pack_into("<I", tags, gestalt + offset, len(tags))
            pointers.append(gestalt + offset)
            blocks[offset] = len(tags)
            tags += random_.randbytes(count * size)
        for index in range(3):
            struct.pack_into("<f", tags, blocks[4] + index * 0x38 + 0xC, random_.choice((0.0, 0.00005, 0.0005, -0.0005, 1.0, 0.5, -2.0, 30.0)))
            struct.pack_into("<2h", tags, blocks[0xC] + index * 0x14 + 8, random_.choice((0, -1200, -100)), random_.choice((0, 1200, 100)))
        for index in range(4):
            low = random_.choice((-1200, -100, 0, 100))
            struct.pack_into("<2h", tags, blocks[0x1C] + index * 10 + 2, low, low + random_.choice((0, 100, 1200)))
        for index in range(6):
            struct.pack_into("<h", tags, blocks[0x24] + index * 0xC + 2, random_.randrange(4))
        for number in range(LOOPING):
            at = 0x50 + number * 0xD4
            struct.pack_into("<I", data, at + 4, 0x70000 | sound_indices[2 + random_.randrange(2)])
            struct.pack_into("<f", data, at + 0x10, random_.choice((0.0, 1.0, 0.5, 0.25)))
            data[at + 0x50] = random_.choice((0, 0, 1, 2, 3, 4, 6, random_.getrandbits(8)))
            data[at + 0x51] = random_.choice((0, 0, 1, 3))
            data[at + 0x52] = random_.choice((0, 0, 0, 1, 2, 3))
            data[at + 0x53] = random_.choice((0, 0, 1))
            struct.pack_into("<I", data, at + 0x54, random_.choice((NONE, 0x50000 | random_.randrange(CONTROLLERS), 0x50000 | random_.randrange(CONTROLLERS))))
            for index in range(4):
                struct.pack_into("<i", data, at + 0x58 + index * 4, random_.choice((0, 100, 1000, 50, 5000, 100000)))
            for track in range(3):
                first = random_.choice((NONE, *salts[:6]))
                struct.pack_into("<I", data, at + 0x88 + track * 0x14, first)
                struct.pack_into("<f", data, at + 0x88 + track * 0x14 + 0x10, random_.choice((0.0, 1.0, 0.5, 0.25, 0.26)))
        for number in range(PLAYING):
            at = 0x4C + number * DATUM
            sources[at + 2] = random_.choice((2, 2, 2, 1, 3))
            struct.pack_into("<I", sources, at + 0xC, 0x30000 | sound_indices[random_.randrange(2)])
            struct.pack_into("<I", sources, at + 0x10, 0x90000 | random_.randrange(LOOPING))
            struct.pack_into("<I", sources, at + 0x6C, salts[(number + 1) % 6] if number < 6 else salts[number])
            struct.pack_into("<2f", sources, at + 0x90, random_.choice((0.0, 100.0, -100.0, 600.0, -1300.0, random_.uniform(-1300, 1300))), random_.uniform(-12, 3))
            sources[at + 0x9C] = random_.randrange(3)
        controllers = bytearray(random_.randbytes(0x4C + CONTROLLERS * 0x1C))
        struct.pack_into("<I", controllers, 0x44, 0x4C)
        for index in range(CONTROLLERS):
            struct.pack_into("<I", controllers, 0x4C + index * 0x1C + 4, random_.choice((NONE, random_.getrandbits(32))))
        lone = bytearray(random_.randbytes(0x1C))
        struct.pack_into("<I", lone, 4, random_.choice((NONE, random_.getrandbits(32))))
        state = bytearray(random_.randbytes(GLOBALS_SIZE))
        state[0x7F] = random_.choice((0, 0, 1))
        struct.pack_into("<i", state, 0x84, random_.choice((1000, 1000, 50, 200000)))
        struct.pack_into("<f", state, 0x1E0, random_.choice((1.0 / 30.0, 0.5, 0.0, 1.0)))
        harness = t.harness

        def normal(harness, value):
            return harness.normalize(value)

        def made_(harness, arguments, salts=salts, seed=random_.getrandbits(32)):
            call_ = calls_so_far(harness, "looping_sound_source_new")
            seed_ = struct.unpack("<I", harness.mu.mem_read(arguments[5], 4))[0]
            harness.mu.mem_write(arguments[5], struct.pack("<I", (seed_ * 3 + 7) & 0xFFFF_FFFF))
            answer = random.Random(seed + call_).choice((NONE, *salts[6:]))
            return answer, (*arguments[:5], seed_)

        def request_(harness, arguments, answer=random_.choice((1, 1, 1, 0))):
            flags_ = bytes(harness.mu.mem_read(arguments[2] + 0xC, 2))
            harness.mu.mem_write(arguments[2], bytes(range(0xA0)))
            return answer, (normal(harness, arguments[0]), arguments[1], flags_)

        def new_(harness, arguments, answer=random_.choice((NONE, NONE, 0x10005)), failure=random_.choice((0, 1, 1, 2))):
            harness.mu.mem_write(arguments[1], struct.pack("<I", failure))
            return answer, (arguments[0], bytes(harness.mu.mem_read(arguments[2], 0xA0)))

        t.check(
            "looping_sounds_update",
            random_.choice((1, 1, 1, 0, 0x100)),
            globals={
                "looping_sounds": Buffer(bytes(data), pointers=(0x44, 0x48)),
                "sound_sources": Buffer(bytes(sources), pointers=(0x44,)),
                "global_tag_instances": (tags_ := Buffer(bytes(tags), pointers=tuple(pointers))),
                "global_sound_cache_file_gestalt": Pointer(tags_, gestalt),
                "unknown_0051ebd8": Buffer(bytes(controllers), pointers=(0x44,)),
                "unknown_0051ebdc": Buffer(bytes(lone)),
                "game_sound_globals": Buffer(bytes(state)),
                "global_random_seed_reference": Buffer(random_.randbytes(16)),
            },
            externals={
                "looping_sound_delete": lambda harness, arguments: (0, tuple(arguments)),
                "sound_choose_pitch_range": lambda harness, arguments, answers=tuple(random_.choice((0, 1, 2, 0x10001, 0xFFFF)) for _ in range(4)): (
                    answers[calls_so_far(harness, "sound_choose_pitch_range") % 4],
                    (arguments[0] & 0xFFFF, normal(harness, arguments[1]), arguments[2]),
                ),
                "sound_pitch_range_fade_decibels": lambda harness, arguments: (
                    real_bits(-3.0 - (arguments[1] % 7)),
                    (normal(harness, arguments[0]), normal(harness, arguments[1]), arguments[2]),
                ),
                "sound_source_delete": lambda harness, arguments: (0, tuple(arguments)),
                "looping_sound_source_new": made_,
                "looping_sound_find_source": lambda harness, arguments, answers=(NONE, NONE, salts[7], salts[8]): (
                    answers[calls_so_far(harness, "looping_sound_find_source") % 4],
                    (arguments[0], arguments[1] & 0xFFFF, arguments[2], arguments[3]),
                ),
                "sound_sources_cross_fade": lambda harness, arguments: (
                    0,
                    (arguments[0], arguments[1], arguments[2] & 0xFFFF, arguments[3]),
                ),
                "looping_sound_detail_request": request_,
                "sound_source_new": new_,
            },
        )
