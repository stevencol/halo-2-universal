# writes Wavefront OBJ files, their materials and the textures of the materials

import zlib
from dataclasses import dataclass
from pathlib import Path
from typing import TextIO

from bitmap import read_bitmap, shader_diffuse_bitmap
from cache_file import NONE, CacheFile, CacheFileError
from geometry import Section
from png import write_png


def obj_name(name: str) -> str:
    """A name with no characters that end a name in an OBJ file."""
    return name.replace("\\", "/").replace(" ", "_")


@dataclass
class TextureTotals:
    # the textures written, the materials whose shader has no diffuse map
    # and the materials whose diffuse map could not be read
    textures: int = 0
    untextured: int = 0
    texture_failures: int = 0


class ObjWriter:
    """Writes the geometry. The game's z axis points up; the file has y up, as
    OBJ readers expect: (x, y, z) is written as (x, z, -y)."""

    def __init__(self, obj: TextIO) -> None:
        self.obj = obj
        self.vertex_count = 0
        self.triangle_count = 0
        # the materials used, with the index of the shader tag of each
        self.materials: dict[str, int] = {}

    def object(self, name: str) -> None:
        self.obj.write(f"o {obj_name(name)}\n")

    def material(self, name: str, shader_index: int = NONE) -> None:
        self.materials[name] = shader_index
        self.obj.write(f"usemtl {name}\n")

    def vertices(self, positions: list[tuple], texcoords: list[tuple] | None = None) -> int:
        """Writes the vertices and returns the OBJ index of the first one."""
        first = self.vertex_count + 1
        for x, y, z in positions:
            self.obj.write(f"v {x:.6f} {z:.6f} {-y:.6f}\n")
        # the texture coordinates have v down; OBJ has it up
        for u, v in texcoords or ():
            self.obj.write(f"vt {u:.6f} {1 - v:.6f}\n")
        self.vertex_count += len(positions)
        return first

    def section(self, name: str, section: Section, positions: list[tuple], materials: list[tuple[str, int]]) -> None:
        """Writes a geometry section as an object, with the positions given for its vertices.

        `materials` is the name and the shader tag of each material of the tag.
        """
        if not section.parts:
            return
        self.object(name)
        first = self.vertices(positions, section.texcoords)

        # the triangles of each material together
        by_material: dict[int, list[int]] = {}
        for part in section.parts:
            by_material.setdefault(part.material_index, []).extend(part.triangles)
        for material_index in sorted(by_material):
            self.material(*(materials[material_index] if material_index < len(materials) else ("none", NONE)))
            indices = by_material[material_index]
            for n in range(0, len(indices), 3):
                a, b, c = (first + index for index in indices[n : n + 3])
                self.obj.write(f"f {a}/{a} {b}/{b} {c}/{c}\n")
            self.triangle_count += len(indices) // 3

    def write_materials(self, path: Path, textures: dict[str, str]) -> None:
        """Writes the materials; `textures` gives the image of each material that has one."""
        with path.open("w", encoding="utf-8", newline="\n") as mtl:
            for material in self.materials:
                if material in textures:
                    mtl.write(f"newmtl {material}\nKd 1.000 1.000 1.000\nmap_Kd {textures[material]}\n\n")
                    continue
                # a color from the name, to tell the materials apart
                color = zlib.crc32(material.encode())
                red, green, blue = (0.25 + 0.7 * ((color >> shift) & 0xFF) / 255 for shift in (0, 8, 16))
                mtl.write(f"newmtl {material}\nKd {red:.3f} {green:.3f} {blue:.3f}\n\n")


def export_textures(
    cache: CacheFile,
    materials: dict[str, int],
    folder: Path,
    obj_folder: Path,
    written: dict[str, str | None],
    totals: TextureTotals,
) -> dict[str, str]:
    """Writes the diffuse map of each material's shader as a PNG image in the folder.

    Returns the image of each material that has one, as a path from the OBJ
    file's folder. `written` keeps the bitmaps written already, by tag name,
    so that maps exported to the same folder decode each bitmap once.
    """
    folder.mkdir(parents=True, exist_ok=True)
    textures: dict[str, str] = {}
    for material, shader_index in materials.items():
        bitmap_index = shader_diffuse_bitmap(cache, shader_index)
        if bitmap_index == NONE:
            totals.untextured += 1
            continue
        name = cache.tag_name(bitmap_index)
        if name not in written:
            path = folder / (name.replace("\\", "_").replace(" ", "_") + ".png")
            try:
                image = read_bitmap(cache, bitmap_index)
            except CacheFileError as error:
                print(f"  texture not written: {error}")
                written[name] = None
            else:
                # without the alpha, which is not transparency in most diffuse maps
                rgb = bytes(value for n, value in enumerate(image.pixels) if n % 4 != 3)
                write_png(path, image.width, image.height, rgb, 3)
                written[name] = path.relative_to(obj_folder).as_posix()
                totals.textures += 1
        if written[name]:
            textures[material] = written[name]
        else:
            totals.texture_failures += 1
    return textures
