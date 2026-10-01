#!/usr/bin/env python3

# opens a Halo 2 cache file as the game does and lists what it holds: the
# header, the tag groups and the tags
#
# mapinfo.py <map> [--tags]

import sys
from dataclasses import dataclass, field
from pathlib import Path

from cache_file import NONE, CacheFile, CacheFileError, group_tag_to_string


@dataclass
class MapInfo:
    # the tags by where their data is: in the tag data of the file; nowhere
    # (a structure bsp or its lightmap, loaded apart, or a tag of a shared
    # map); and outside the tag data, which is an error
    local: int = 0
    no_data: int = 0
    empty: int = 0
    outside: int = 0
    # per group tag: the tags, those with no data, and the bytes
    groups: dict[int, list[int]] = field(default_factory=dict)


def map_info(cache: CacheFile, list_tags: bool = False) -> MapInfo:
    info = MapInfo()
    for tag_index, (group, index, address, size) in enumerate(cache.tag_instances):
        if group == NONE:
            info.empty += 1
            continue
        no_data = False
        if cache.tag_data_address <= address and address + size <= cache.tag_data_address + cache.tag_data_size:
            info.local += 1
        elif address == 0 or size == 0:
            no_data = True
            info.no_data += 1
        else:
            info.outside += 1

        totals = info.groups.setdefault(group, [0, 0, 0])
        totals[0] += 1
        totals[1] += no_data
        totals[2] += size
        if list_tags:
            print(f"{group_tag_to_string(group):>4}  {index:08X}  {address:08X}  {size:8}  {cache.tag_name(tag_index)}")
    return info


def main() -> None:
    if len(sys.argv) < 2:
        sys.exit("usage: mapinfo.py <map> [--tags]")
    try:
        cache = CacheFile(Path(sys.argv[1]))
    except CacheFileError as error:
        sys.exit(f"mapinfo: {error}")

    print(f"name            {cache.name}")
    print(f"scenario        {cache.scenario_path}")
    print(f"type            {cache.scenario_type}")
    print(f"build           {cache.build}")
    print(f"version         {cache.version}")
    print(f"file length     {cache.file_length}")
    print(
        f"tag data        offset 0x{cache.tag_data_offset:X}, index 0x{cache.tag_index_size:X} + "
        f"data 0x{cache.tag_data_size:X}, memory 0x{cache.tag_memory_size:X}"
    )
    print(f"base address    0x{cache.base_address:08X}")
    print(f"tag groups      {cache.tag_group_count}")
    print(f"tags            {cache.tag_count}")
    print(f"string ids      {cache.string_id_count}")
    print(f"scenario tag    {cache.tag_name(cache.scenario_index)}")
    print(f"globals tag     {cache.tag_name(cache.globals_index)}")

    info = map_info(cache, "--tags" in sys.argv)
    print("\ntags by where their data is")
    print(f"  in this file    {info.local}")
    print(f"  no data here    {info.no_data}")
    print(f"  empty slots     {info.empty}")
    print(f"  outside (bad)   {info.outside}")

    print("\ngroup   tags  no data       bytes")
    for group, (count, no_data, size) in sorted(info.groups.items(), key=lambda item: -item[1][2]):
        print(f"{group_tag_to_string(group):>4}  {count:6}  {no_data:7}  {size:10}")

    print("\nstring ids (the first and the last)")
    for index in [*range(min(6, cache.string_id_count)), *range(max(cache.string_id_count - 4, 6), cache.string_id_count)]:
        print(f"  {index:5}  {cache.string(index)}")


if __name__ == "__main__":
    main()
