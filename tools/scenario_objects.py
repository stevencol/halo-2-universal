# reads the objects that a scenario places in its level; the layouts are those
# of source/scenario/scenario_definitions.h and
# source/objects/object_definitions.h

import math
import struct
from collections.abc import Iterator
from dataclasses import dataclass

from cache_file import NONE, CacheFile, group_tag

# the kinds of object: the offsets in the scenario of the placements and of
# the palette, and the size of a placement
OBJECT_KINDS = {
    "scenery": (0x50, 0x58, 0x5C),
    "biped": (0x60, 0x68, 0x54),
    "vehicle": (0x70, 0x78, 0x54),
    "equipment": (0x80, 0x88, 0x38),
    "weapon": (0x90, 0x98, 0x54),
    "machine": (0xA8, 0xB0, 0x48),
    "control": (0xB8, 0xC0, 0x44),
    "crate": (0x328, 0x330, 0x4C),
}
PALETTE_ENTRY_SIZE = 0x28

OBJECT_MODEL_OFFSET = 0x34
MODEL_RENDER_MODEL_OFFSET = 0x00


@dataclass
class Placement:
    kind: str
    # the tag of the object, or NONE
    tag_index: int
    position: tuple[float, float, float]
    # yaw, pitch and roll, in radians
    rotation: tuple[float, float, float]
    scale: float

    def transform(self, point: tuple[float, float, float]) -> tuple[float, float, float]:
        """A point of the object's model, in the level."""
        forward, left, up = rotation_vectors(*self.rotation)
        x, y, z = (coordinate * self.scale for coordinate in point)
        return (
            self.position[0] + x * forward[0] + y * left[0] + z * up[0],
            self.position[1] + x * forward[1] + y * left[1] + z * up[1],
            self.position[2] + x * forward[2] + y * left[2] + z * up[2],
        )


def rotation_vectors(yaw: float, pitch: float, roll: float) -> tuple[tuple, tuple, tuple]:
    """The forward, left and up vectors of a rotation.

    As matrix4x3_rotation_from_angles of Halo 1 (source/math/matrix_math.c in
    halo-ce-universal), which the objects of that game are placed with.
    """
    cosine_yaw, sine_yaw = math.cos(yaw), math.sin(yaw)
    cosine_pitch, sine_pitch = math.cos(pitch), math.sin(pitch)
    cosine_roll, sine_roll = math.cos(roll), math.sin(roll)
    forward = (
        cosine_yaw * cosine_pitch,
        sine_yaw * cosine_roll - cosine_yaw * sine_pitch * sine_roll,
        cosine_yaw * sine_pitch * cosine_roll + sine_yaw * sine_roll,
    )
    left = (
        -sine_yaw * cosine_pitch,
        sine_yaw * sine_pitch * sine_roll + cosine_yaw * cosine_roll,
        cosine_yaw * sine_roll - sine_yaw * sine_pitch * cosine_roll,
    )
    up = (-sine_pitch, -cosine_pitch * sine_roll, cosine_pitch * cosine_roll)
    return forward, left, up


def scenario_objects(cache: CacheFile) -> Iterator[Placement]:
    scenario = cache.scenario()
    for kind, (placements_offset, palette_offset, placement_size) in OBJECT_KINDS.items():
        count, address = struct.unpack_from("<iI", scenario, placements_offset)
        palette_count, palette_address = struct.unpack_from("<iI", scenario, palette_offset)
        if count <= 0:
            continue
        # the palette: the tag reference of each object the level uses
        palette = (
            [
                struct.unpack_from("<2I", cache.tag_data(palette_address + n * PALETTE_ENTRY_SIZE, 8))[1]
                for n in range(palette_count)
            ]
            if palette_count > 0
            else []
        )
        placements = cache.tag_data(address, count * placement_size)
        for n in range(count):
            # the index in the palette, the index of the name, flags, the
            # position, the rotation and the scale
            palette_index, _, _, x, y, z, yaw, pitch, roll, scale = struct.unpack_from(
                "<2hI7f", placements, n * placement_size
            )
            yield Placement(
                kind,
                palette[palette_index] if 0 <= palette_index < len(palette) else NONE,
                (x, y, z),
                (yaw, pitch, roll),
                # 0 is no scale
                scale if scale > 0 else 1.0,
            )


def object_render_model(cache: CacheFile, object_index: int) -> int:
    """The render model of an object's tag, through its model (hlmt); NONE if it has none."""
    if object_index == NONE:
        return NONE
    _, _, address, size = cache.tag_instances[object_index & 0xFFFF]
    if size < OBJECT_MODEL_OFFSET + 8:
        return NONE
    group, model_index = struct.unpack_from("<2I", cache.tag_data(address, size), OBJECT_MODEL_OFFSET)
    if group != group_tag("hlmt") or model_index == NONE:
        return NONE

    _, _, address, size = cache.tag_instances[model_index & 0xFFFF]
    if size < MODEL_RENDER_MODEL_OFFSET + 8:
        return NONE
    group, render_model_index = struct.unpack_from("<2I", cache.tag_data(address, size), MODEL_RENDER_MODEL_OFFSET)
    return render_model_index if group == group_tag("mode") else NONE
