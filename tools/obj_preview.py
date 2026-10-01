#!/usr/bin/env python3

# draws an OBJ file written by bsp2obj.py as a PNG image, seen from above and
# to one side, to look at a level without a 3D program
#
# obj_preview.py <input.obj> <output.png> [width]

import math
import sys
from pathlib import Path

from png import read_png, write_png

# the view: degrees around the vertical axis, then degrees down from the horizon
TURN = 35.0
TILT = 50.0
LIGHT = (0.35, 0.85, 0.4)
BACKGROUND = (24, 26, 32)
DEFAULT_COLOR = (0.7, 0.7, 0.7)


class Material:
    def __init__(self) -> None:
        self.color = DEFAULT_COLOR
        # (width, height, RGB pixels)
        self.texture: tuple[int, int, bytes] | None = None


def read_materials(path: Path) -> dict[str, Material]:
    materials: dict[str, Material] = {}
    if not path.exists():
        return materials
    material = None
    for line in path.read_text(encoding="utf-8").splitlines():
        parts = line.split(maxsplit=1)
        if len(parts) < 2:
            continue
        if parts[0] == "newmtl":
            material = materials[parts[1]] = Material()
        elif parts[0] == "Kd" and material:
            material.color = tuple(float(value) for value in parts[1].split()[:3])
        elif parts[0] == "map_Kd" and material:
            image = path.parent / parts[1]
            if image.exists():
                width, height, channels, pixels = read_png(image)
                if channels == 4:
                    pixels = bytes(value for n, value in enumerate(pixels) if n % 4 != 3)
                material.texture = (width, height, pixels)
    return materials


def read_obj(path: Path) -> tuple[list[tuple], list[tuple], list[tuple[list[int], list[int], str]]]:
    """The vertices, the texture coordinates and the faces: (vertices, texture coordinates, material)."""
    vertices: list[tuple] = []
    texcoords: list[tuple] = []
    faces: list[tuple[list[int], list[int], str]] = []
    material = "none"
    with path.open(encoding="utf-8") as f:
        for line in f:
            if line.startswith("v "):
                vertices.append(tuple(float(value) for value in line.split()[1:4]))
            elif line.startswith("vt "):
                texcoords.append(tuple(float(value) for value in line.split()[1:3]))
            elif line.startswith("f "):
                corners = [corner.split("/") for corner in line.split()[1:]]
                faces.append(
                    (
                        [int(corner[0]) - 1 for corner in corners],
                        [int(corner[1]) - 1 for corner in corners] if all(len(c) > 1 and c[1] for c in corners) else [],
                        material,
                    )
                )
            elif line.startswith("usemtl "):
                material = line.split()[1]
    return vertices, texcoords, faces


def main() -> None:
    if len(sys.argv) < 3:
        sys.exit("usage: obj_preview.py <input.obj> <output.png> [width]")
    obj_path = Path(sys.argv[1])
    vertices, texcoords, faces = read_obj(obj_path)
    materials = read_materials(obj_path.with_suffix(".mtl"))
    width = int(sys.argv[3]) if len(sys.argv) > 3 else 1400
    if not vertices or not faces:
        sys.exit("the OBJ file has no geometry")

    # the file has y up: turn around y, then tilt around x; the view looks
    # along -z, so a larger z is nearer
    turn, tilt = math.radians(TURN), math.radians(TILT)
    cos_turn, sin_turn, cos_tilt, sin_tilt = math.cos(turn), math.sin(turn), math.cos(tilt), math.sin(tilt)

    def rotate(x: float, y: float, z: float) -> tuple[float, float, float]:
        x, z = x * cos_turn + z * sin_turn, -x * sin_turn + z * cos_turn
        return x, y * cos_tilt - z * sin_tilt, y * sin_tilt + z * cos_tilt

    view = [rotate(*vertex) for vertex in vertices]
    x0, x1 = min(v[0] for v in view), max(v[0] for v in view)
    y0, y1 = min(v[1] for v in view), max(v[1] for v in view)
    margin = 20
    scale = (width - 2 * margin) / max(x1 - x0, 1e-6)
    height = int((y1 - y0) * scale) + 2 * margin
    screen = [((v[0] - x0) * scale + margin, (y1 - v[1]) * scale + margin, v[2]) for v in view]

    pixels = bytearray(bytes(BACKGROUND) * (width * height))
    depth = [-math.inf] * (width * height)
    light_length = math.sqrt(sum(value * value for value in LIGHT))
    default_material = Material()

    for ring, uv_ring, material_name in faces:
        # the normal in the file's space, for the shade
        a, b, c = vertices[ring[0]], vertices[ring[1]], vertices[ring[2]]
        ux, uy, uz = b[0] - a[0], b[1] - a[1], b[2] - a[2]
        vx, vy, vz = c[0] - a[0], c[1] - a[1], c[2] - a[2]
        nx, ny, nz = uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx
        length = math.sqrt(nx * nx + ny * ny + nz * nz)
        # the surfaces that face away are not drawn: the walls and the ceiling
        # that close a level would hide it
        if length == 0 or rotate(nx, ny, nz)[2] <= 0:
            continue
        shade = 0.3 + 0.7 * abs(nx * LIGHT[0] + ny * LIGHT[1] + nz * LIGHT[2]) / (length * light_length)

        material = materials.get(material_name, default_material)
        texture = material.texture if uv_ring else None
        flat = bytes(min(int(255 * shade * value), 255) for value in material.color)
        if texture:
            texture_width, texture_height, texels = texture
            # the texels with the shade applied
            shaded = bytes(min(int(value * shade * 1.15), 255) for value in range(256))

        # a fan of triangles, each filled with the nearest surface kept
        for n in range(1, len(ring) - 1):
            (ax, ay, az), (bx, by, bz), (cx, cy, cz) = screen[ring[0]], screen[ring[n]], screen[ring[n + 1]]
            area = (bx - ax) * (cy - ay) - (cx - ax) * (by - ay)
            if area == 0:
                continue
            if texture:
                (au, av), (bu, bv), (cu, cv) = texcoords[uv_ring[0]], texcoords[uv_ring[n]], texcoords[uv_ring[n + 1]]
            for py in range(max(int(min(ay, by, cy)), 0), min(int(max(ay, by, cy)) + 1, height)):
                y = py + 0.5
                for px in range(max(int(min(ax, bx, cx)), 0), min(int(max(ax, bx, cx)) + 1, width)):
                    x = px + 0.5
                    w0 = ((bx - x) * (cy - y) - (cx - x) * (by - y)) / area
                    w1 = ((cx - x) * (ay - y) - (ax - x) * (cy - y)) / area
                    w2 = 1 - w0 - w1
                    if w0 < 0 or w1 < 0 or w2 < 0:
                        continue
                    z = w0 * az + w1 * bz + w2 * cz
                    at = py * width + px
                    if z <= depth[at]:
                        continue
                    depth[at] = z
                    if texture:
                        # the texture repeats; the file has v up and the image has its top row first
                        u = w0 * au + w1 * bu + w2 * cu
                        v = 1 - (w0 * av + w1 * bv + w2 * cv)
                        texel = (int(v * texture_height) % texture_height * texture_width + int(u * texture_width) % texture_width) * 3
                        pixels[at * 3 : at * 3 + 3] = texels[texel : texel + 3].translate(shaded)
                    else:
                        pixels[at * 3 : at * 3 + 3] = flat

    write_png(Path(sys.argv[2]), width, height, bytes(pixels))
    print(f"{sys.argv[2]}: {width} x {height}, {len(faces)} surfaces")


if __name__ == "__main__":
    main()
