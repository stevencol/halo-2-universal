#!/usr/bin/env python3

# writes a bitmap tag of a map as a PNG image
#
# bitmap2png.py <map> <tag name> <output.png>
#
# the tag name can be the end of the name; mapinfo --tags lists the names

import sys
from pathlib import Path

from bitmap import read_bitmap
from cache_file import CacheFile, CacheFileError, group_tag
from png import write_png


def main() -> None:
    if len(sys.argv) < 4:
        sys.exit("usage: bitmap2png.py <map> <tag name> <output.png>")
    try:
        cache = CacheFile(Path(sys.argv[1]))
        wanted = sys.argv[2].replace("/", "\\")
        matches = [
            index
            for index, instance in enumerate(cache.tag_instances)
            if instance[0] == group_tag("bitm") and cache.tag_name(index).endswith(wanted)
        ]
        if not matches:
            sys.exit(f"bitmap2png: no bitmap tag has a name that ends with '{wanted}'")
        image = read_bitmap(cache, matches[0])
    except CacheFileError as error:
        sys.exit(f"bitmap2png: {error}")

    write_png(Path(sys.argv[3]), image.width, image.height, image.pixels, 4)
    print(f"{cache.tag_name(matches[0])}: {image.width} x {image.height} -> {sys.argv[3]}")


if __name__ == "__main__":
    main()
