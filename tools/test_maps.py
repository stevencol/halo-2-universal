#!/usr/bin/env python3

# runs the tools on every map of the game and checks that each opens, that
# every tag's data is where the loader expects it, and that the geometry of
# its structure bsps is whole: the collision geometry and the geometry that
# the game draws, with the textures of its shaders. Reads every section of
# every render model too
#
# test_maps.py <maps folder>
#
# the OBJ files are left in build/obj

import contextlib
import io
import re
import struct
import subprocess
import sys
from pathlib import Path

from bsp2obj import Totals, export_map
from cache_file import CacheFile, CacheFileError, group_tag
from render_model import RenderModel

BUILD = Path(__file__).resolve().parent.parent / "build"
MAPINFO = BUILD / "mapinfo.exe"


def field(output: str, name: str) -> str:
    match = re.search(rf"^\s*{re.escape(name)}\s+(.+)$", output, re.MULTILINE)
    return match.group(1).strip() if match else "?"


def check_models(path: Path, seen: set[str], sums: dict[str, int]) -> tuple[int, list[str]]:
    """Reads every section of the render models of a map that no map before it had.

    Returns the number of models read and the errors.
    """
    cache = CacheFile(path)
    errors: list[str] = []
    count = 0
    for tag_index, instance in enumerate(cache.tag_instances):
        name = cache.tag_name(tag_index)
        if instance[0] != group_tag("mode") or instance[3] == 0 or name in seen:
            continue
        seen.add(name)
        count += 1
        try:
            model = RenderModel(cache, tag_index)
            for section_index in range(model.section_count):
                section = model.section(section_index)
                sums["model sections"] += 1
                sums["model triangles"] += section.triangle_count
        except (CacheFileError, struct.error) as error:
            errors.append(f"{name}: {error}")
    cache.close()
    sums["models"] += count
    return count, errors


def main() -> None:
    maps = sorted(Path(sys.argv[1]).glob("*.map"))
    obj_folder = BUILD / "obj"
    obj_folder.mkdir(parents=True, exist_ok=True)
    failures = 0
    sums = {"bsps": 0, "surfaces": 0, "clusters": 0, "instances": 0, "triangles": 0}
    sums |= {"models": 0, "model sections": 0, "model triangles": 0, "objects": 0, "objects without model": 0}
    # the textures go to one folder, and each is decoded once
    written_textures: dict[str, str | None] = {}
    # the render models read already, by name: the maps repeat them
    models_seen: set[str] = set()

    print(
        f"{'map':<22} {'type':<21} {'tags':>5} {'bad':>4}   {'bsps':>4} "
        f"{'surfaces':>8} {'broken':>6} {'reversed':>8}   {'clusters':>8} {'instances':>9} {'triangles':>9}   "
        f"{'no map':>6} {'unread':>6}   {'models':>6} {'bad':>4}   {'objs':>5}"
    )
    for path in maps:
        try:
            info = subprocess.run([MAPINFO, path], capture_output=True, text=True)
        except OSError as error:
            # Windows can refuse to start a program that is not signed
            sys.exit(f"cannot run {MAPINFO.name}: {error}")
        if info.returncode != 0:
            print(f"{path.stem:<22} FAILED: {info.stderr.strip()}")
            failures += 1
            continue

        try:
            with contextlib.redirect_stdout(io.StringIO()):
                collision: Totals = export_map(path, obj_folder / (path.stem + "_collision.obj"), collision=True)
                render: Totals = export_map(
                    path,
                    obj_folder / (path.stem + ".obj"),
                    texture_folder=obj_folder / "textures",
                    written_textures=written_textures,
                )
            model_count, model_errors = check_models(path, models_seen, sums)
        except (CacheFileError, struct.error) as error:
            print(f"{path.stem:<22} FAILED: {error}")
            failures += 1
            continue

        bad = field(info.stdout, "outside (bad)")
        ok = (
            bad == "0"
            and field(info.stdout, "name") == path.stem
            and collision.broken == 0
            and collision.reversed == 0
            and collision.surfaces > 0
            and render.surfaces > 0
            and render.texture_failures == 0
            and not model_errors
        )
        failures += not ok
        sums["bsps"] += collision.bsps
        sums["surfaces"] += collision.surfaces
        sums["clusters"] += render.clusters
        sums["instances"] += render.instances
        sums["triangles"] += render.surfaces
        sums["objects"] += render.objects
        sums["objects without model"] += render.objects_without_model
        print(
            f"{path.stem:<22} {field(info.stdout, 'type'):<21} {field(info.stdout, 'tags'):>5} {bad:>4}   "
            f"{collision.bsps:>4} {collision.surfaces:>8} {collision.broken:>6} {collision.reversed:>8}   "
            f"{render.clusters:>8} {render.instances:>9} {render.surfaces:>9}   "
            f"{render.untextured:>6} {render.texture_failures:>6}   "
            f"{model_count:>6} {len(model_errors):>4}   {render.objects:>5}{'' if ok else '  <-- FAILED'}"
        )
        for error in model_errors[:5]:
            print(f"    {error}")

    print(
        f"\ntotal: {sums['bsps']} structure bsps, {sums['surfaces']} collision surfaces, "
        f"{sums['clusters']} clusters, {sums['instances']} instances, {sums['triangles']} triangles, "
        f"{sum(1 for texture in written_textures.values() if texture)} textures"
    )
    print(
        f"       {sums['models']} render models, {sums['model sections']} sections, "
        f"{sums['model triangles']} triangles"
    )
    print(
        f"       {sums['objects']} objects placed in the levels, "
        f"{sums['objects without model']} more with no model"
    )
    print(f"{len(maps) - failures} of {len(maps)} maps passed")
    sys.exit(1 if failures else 0)


if __name__ == "__main__":
    main()
