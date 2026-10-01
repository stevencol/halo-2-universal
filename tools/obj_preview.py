#!/usr/bin/env python3

# draws an OBJ file written by bsp2obj.py as a PNG image, seen from above and
# to one side, to look at a level without a 3D program
#
# obj_preview.py <input.obj> <output.png> [width]

import math
import struct
import sys
import zlib
from pathlib import Path

# the view: degrees around the vertical axis, then degrees down from the horizon
TURN = 35.0
TILT = 50.0
LIGHT = (0.35, 0.85, 0.4)
BACKGROUND = (24, 26, 32)


def read_obj(path: Path) -> tuple[list[tuple], list[tuple[list[int], str]], dict[str, tuple]]:
    vertices: list[tuple] = []
    faces: list[tuple[list[int], str]] = []
    colors: dict[str, tuple] = {}
    material = "none"

    mtl = path.with_suffix(".mtl")
    if mtl.exists():
        name = None
        for line in mtl.read_text(encoding="utf-8").splitlines():
            parts = line.split()
            if parts[:1] == ["newmtl"]:
                name = parts[1]
            elif parts[:1] == ["Kd"] and name:
                colors[name] = tuple(float(value) for value in parts[1:4])

    with path.open(encoding="utf-8") as f:
        for line in f:
            if line.startswith("v "):
                vertices.append(tuple(float(value) for value in line.split()[1:4]))
            elif line.startswith("f "):
                faces.append(([int(value.split("/")[0]) - 1 for value in line.split()[1:]], material))
            elif line.startswith("usemtl "):
                material = line.split()[1]
    return vertices, faces, colors


def write_png(path: Path, width: int, height: int, pixels: bytearray) -> None:
    def chunk(kind: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))

    rows = b"".join(b"\0" + bytes(pixels[y * width * 3 : (y + 1) * width * 3]) for y in range(height))
    path.write_bytes(
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack(">2I5B", width, height, 8, 2, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(rows, 6))
        + chunk(b"IEND", b"")
    )


def main() -> None:
    if len(sys.argv) < 3:
        sys.exit("usage: obj_preview.py <input.obj> <output.png> [width]")
    vertices, faces, colors = read_obj(Path(sys.argv[1]))
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

    for ring, material in faces:
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
        red, green, blue = (int(255 * shade * value) for value in colors.get(material, (0.7, 0.7, 0.7)))

        # a fan of triangles, each filled with the nearest surface kept
        for n in range(1, len(ring) - 1):
            (ax, ay, az), (bx, by, bz), (cx, cy, cz) = screen[ring[0]], screen[ring[n]], screen[ring[n + 1]]
            area = (bx - ax) * (cy - ay) - (cx - ax) * (by - ay)
            if area == 0:
                continue
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
                    if z > depth[at]:
                        depth[at] = z
                        pixels[at * 3 : at * 3 + 3] = bytes((red, green, blue))

    write_png(Path(sys.argv[2]), width, height, pixels)
    print(f"{sys.argv[2]}: {width} x {height}, {len(faces)} surfaces")


if __name__ == "__main__":
    main()
