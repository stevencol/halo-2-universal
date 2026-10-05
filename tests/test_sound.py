# source/sound/sound_manager.c and source/sound/sound_classes.c
#
# The tags of a map that the functions read are the test's: a sound, the
# playback parameters of the sounds, and the classes of sounds that the
# globals tag tells of

import random
import struct

from difftest import Buffer, Memory, Mock, Pointer, mock_address
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
        names = ("unknown_00219b40", "looping_sounds_initialize", "sound_effects_initialize", "sound_voices_initialize", "unknown_0021e4b0")

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
                externals={**answers(*values), "unknown_002185d0": console_sound},
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
            externals={"unknown_002186b0": lambda harness, arguments: 0, "unknown_0021eae0": lambda harness, arguments: 0},
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
                "unknown_0021dde0": lambda harness, arguments: 0,
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

    idle_names = ("unknown_0021ec00", "unknown_001269f0", "sound_channels_update", "unknown_001293f0", "unknown_0021ee80", "sound_cache_idle")

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
                "unknown_00218850": request(n % 4 | random_.choice((0, 0, 0xFFFFFFFC, 4))),
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
            externals={"unknown_00218850": request(random_.getrandbits(32))},
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
            "unknown_0021f5d0": restart,
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

        return {"sound_unknown_00125ef0": release, "unknown_00218850": request}

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
            "unknown_00127010": class_decibels,
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
        "unknown_0021ec00",
        "sound_playback_controllers_unknown_00219d90",
        "sound_console_streams_resume",
        "unknown_001269f0",
        "unknown_00127400",
        "sound_playback_controllers_update",
        "unknown_0021b0e0",
        "sound_channels_update",
        "unknown_00128020",
        "unknown_001282d0",
        "unknown_001293f0",
        "unknown_0021ee80",
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
                "unknown_0021f360": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "sound_classes_update": lambda harness, arguments: (0, (arguments[0] & 0xFFFFFFFF,)),
                "unknown_0021ec00": first_update,
                "unknown_00129100": tables,
                "unknown_0014dde0": lambda harness, arguments: first_player,
                "unknown_00221a70": lambda harness, arguments: (0, (arguments[0], stack_bytes(harness, arguments[1], 4))),
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
                "unknown_0021f290": lambda harness, arguments: 0,
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

    for _ in range(20):
        t.check("unknown_00218420_set_defaults", Buffer(random_.randbytes(0x1C)))

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
            externals={"unknown_0014dde0": first_player, "unknown_0014de10": next_player, "unknown_0012a9d0": listener_real},
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
                externals={"unknown_0021d630": effect_update},
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
                externals={"unknown_0021d630": effect_update},
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
            unknown2, unknown, first, failure = arguments
            harness.mu.mem_write(first, struct.pack("<I", 0xA1B2C3D4))
            return looked, (unknown2, harness.normalize(unknown), harness.normalize(failure))

        def refuse(harness, arguments, refused=refused):
            unknown, second = arguments
            record = harness.normalize(unknown), bytes(harness.mu.mem_read(second, 4))
            harness.mu.mem_write(second, struct.pack("<I", 0x0F1E2D3C))
            return refused, record

        said = Buffer(random_.randbytes(0x90))
        t.check(
            "sound_source_new",
            said,
            random_.choice((0, Buffer(random_.randbytes(4)), Buffer(random_.randbytes(4)))),
            random_.getrandbits(32),
            globals={"game_sound_globals": sound_state(*states[n % len(states)])},
            externals={
                "unknown_00126c30": look,
                "unknown_00126ec0": refuse,
                "unknown_00126000": lambda harness, arguments, made=made: (
                    made,
                    (harness.normalize(arguments[0]), *arguments[1:]),
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
                externals={"unknown_00218850": request},
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
            externals={"unknown_00129790": lambda harness, arguments: (0, tuple(arguments))},
        )

    for _ in range(40):
        t.check(
            "sound_console_set_environment",
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(8)),
            Buffer(random_.randbytes(0x30)),
            globals={"unknown_0051ebe4": Buffer(random_.randbytes(0x2AC0))},
            externals={"unknown_0021e3b0": lambda harness, arguments: (0, tuple(arguments))},
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
                    "unknown_00218850": piece_request,
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
                externals={"unknown_0021f430": lambda harness, arguments: (0, tuple(arguments))},
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
