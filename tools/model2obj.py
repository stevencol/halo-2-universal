#!/usr/bin/env python3

# writes a render model (mode) of a map as a Wavefront OBJ file, with its
# textures: the first permutation of each region, at its highest level of
# detail, in the pose the model is stored in
#
# model2obj.py <map> <tag name> <output.obj> [--no-textures]
#
# the tag name can be the end of the name; mapinfo --tags lists the names

import sys
from pathlib import Path

from cache_file import NONE, CacheFile, CacheFileError
from obj_writer import ObjWriter, TextureTotals, export_textures
from render_model import RenderModel


def export_model(
    cache: CacheFile,
    tag_index: int,
    obj_path: Path,
    textures: bool = True,
    texture_folder: Path | None = None,
    written_textures: dict[str, str | None] | None = None,
) -> tuple[ObjWriter, TextureTotals]:
    model = RenderModel(cache, tag_index)
    totals = TextureTotals()
    mtl_path = obj_path.with_suffix(".mtl")

    with obj_path.open("w", encoding="utf-8", newline="\n") as obj:
        obj.write(f"# {model.name}: written by model2obj.py\n")
        obj.write(f"mtllib {mtl_path.name}\n")
        writer = ObjWriter(obj)
        for region in model.regions:
            if not region.permutations:
                continue
            permutation = region.permutations[0]
            section_index = model.most_detailed_section(permutation)
            if section_index < 0:
                continue
            section = model.section(section_index)
            writer.section(f"{region.name}/{permutation.name}", section, section.positions, model.materials)

    material_textures: dict[str, str] = {}
    if textures:
        material_textures = export_textures(
            cache,
            writer.materials,
            texture_folder or obj_path.with_name(obj_path.stem + "_textures"),
            obj_path.parent,
            {} if written_textures is None else written_textures,
            totals,
        )
    writer.write_materials(mtl_path, material_textures)
    return writer, totals


def main() -> None:
    arguments = [argument for argument in sys.argv[1:] if not argument.startswith("--")]
    if len(arguments) < 3:
        sys.exit("usage: model2obj.py <map> <tag name> <output.obj> [--no-textures]")
    try:
        cache = CacheFile(Path(arguments[0]))
        tag_index = cache.find_tag("mode", arguments[1])
        if tag_index == NONE:
            sys.exit(f"model2obj: no render model has a name that ends with '{arguments[1]}'")
        writer, totals = export_model(cache, tag_index, Path(arguments[2]), "--no-textures" not in sys.argv)
    except CacheFileError as error:
        sys.exit(f"model2obj: {error}")

    print(f"{cache.tag_name(tag_index)}")
    print(f"vertices           {writer.vertex_count}")
    print(f"triangles          {writer.triangle_count}")
    print(f"textures written   {totals.textures}")
    print(f"no diffuse map     {totals.untextured}")
    print(f"textures not read  {totals.texture_failures}")


if __name__ == "__main__":
    main()
