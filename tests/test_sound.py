# source/sound/sound_manager.c and source/sound/sound_classes.c
#
# The tags of a map that the functions read are the test's: a sound, the
# playback parameters of the sounds, and the classes of sounds that the
# globals tag tells of

import random
import struct

from difftest import Buffer, Pointer

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
