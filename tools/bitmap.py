# reads the bitmaps (textures) of a map and decodes them to RGBA; the layouts
# are those of source/bitmaps/bitmap_definitions.h and
# source/shaders/shader_definitions.h

import struct
from dataclasses import dataclass

from cache_file import NONE, CacheFile, CacheFileError, group_tag

SHADER_RUNTIME_PROPERTIES_OFFSET = 0x0C
BITMAP_GROUP_BITMAPS_OFFSET = 0x44
BITMAP_DATA_SIZE = 0x74

BITMAP_TYPE_2D = 0
BITMAP_FLAG_SWIZZLED = 0x8

FORMAT_Y8 = 1
FORMAT_R5G6B5 = 6
FORMAT_A4R4G4B4 = 9
FORMAT_X8R8G8B8 = 10
FORMAT_A8R8G8B8 = 11
FORMAT_DXT1 = 14
FORMAT_DXT3 = 15
FORMAT_DXT5 = 16

# the bytes of a texel of the formats that are not compressed
TEXEL_SIZES = {FORMAT_Y8: 1, FORMAT_R5G6B5: 2, FORMAT_A4R4G4B4: 2, FORMAT_X8R8G8B8: 4, FORMAT_A8R8G8B8: 4}
# the bytes of a block of 4 x 4 texels of the compressed formats
BLOCK_SIZES = {FORMAT_DXT1: 8, FORMAT_DXT3: 16, FORMAT_DXT5: 16}

# the four 2-bit color indices that a byte of a DXT block holds
ROW_INDICES = [(byte & 3, (byte >> 2) & 3, (byte >> 4) & 3, byte >> 6) for byte in range(256)]


@dataclass
class Image:
    width: int
    height: int
    pixels: bytes  # RGBA, the top row first


def shader_diffuse_bitmap(cache: CacheFile, shader_index: int) -> int:
    """The index of the bitmap tag that a shader gives as its diffuse map, or NONE."""
    if shader_index == NONE:
        return NONE
    _, _, address, size = cache.tag_instances[shader_index & 0xFFFF]
    count, properties = struct.unpack_from("<iI", cache.tag_data(address, size), SHADER_RUNTIME_PROPERTIES_OFFSET)
    if count < 1:
        return NONE
    # the properties start with the tag reference of the diffuse map
    _, bitmap_index = struct.unpack("<2I", cache.tag_data(properties, 8))
    return bitmap_index


def read_bitmap(cache: CacheFile, bitmap_index: int) -> Image:
    """Decodes the largest image of the first bitmap of a bitmap tag."""
    group, _, address, size = cache.tag_instances[bitmap_index & 0xFFFF]
    if group != group_tag("bitm"):
        raise CacheFileError(f"'{cache.tag_name(bitmap_index)}' is not a bitmap")
    count, bitmaps = struct.unpack_from("<iI", cache.tag_data(address, size), BITMAP_GROUP_BITMAPS_OFFSET)
    if count < 1:
        raise CacheFileError(f"'{cache.tag_name(bitmap_index)}' has no bitmaps")

    data = cache.tag_data(bitmaps, BITMAP_DATA_SIZE)
    signature, width, height, _, type, format, flags = struct.unpack_from("<I6H", data, 0)
    (offset,) = struct.unpack_from("<I", data, 0x1C)
    (pixels_size,) = struct.unpack_from("<I", data, 0x34)
    if signature != group_tag("bitm") or type != BITMAP_TYPE_2D or offset == NONE:
        raise CacheFileError(f"'{cache.tag_name(bitmap_index)}' is not a 2D bitmap with pixels")

    if format in BLOCK_SIZES:
        needed = max(1, (width + 3) // 4) * max(1, (height + 3) // 4) * BLOCK_SIZES[format]
    elif format in TEXEL_SIZES:
        needed = width * height * TEXEL_SIZES[format]
    else:
        raise CacheFileError(f"'{cache.tag_name(bitmap_index)}' has the format {format}, which is not read")
    if needed > pixels_size:
        raise CacheFileError(f"'{cache.tag_name(bitmap_index)}' has fewer pixels than its size needs")
    # the largest image is the first of the pixels; its mipmaps follow it
    pixels = cache.read_resource(offset, needed)

    if format in BLOCK_SIZES:
        return Image(width, height, decode_dxt(pixels, width, height, format))
    return Image(width, height, decode_texels(pixels, width, height, format, bool(flags & BITMAP_FLAG_SWIZZLED)))


def color_565(color: int) -> tuple[int, int, int]:
    red, green, blue = color >> 11, (color >> 5) & 0x3F, color & 0x1F
    return (red << 3) | (red >> 2), (green << 2) | (green >> 4), (blue << 3) | (blue >> 2)


def decode_dxt(data: bytes, width: int, height: int, format: int) -> bytes:
    blocks_wide = max(1, (width + 3) // 4)
    blocks_high = max(1, (height + 3) // 4)
    block_size = BLOCK_SIZES[format]
    # decoded with whole blocks, then cut to the size
    full_width = blocks_wide * 4
    out = bytearray(full_width * blocks_high * 4 * 4)

    at = 0
    for block_y in range(blocks_high):
        for block_x in range(blocks_wide):
            color_at = at + block_size - 8
            color_0, color_1 = struct.unpack_from("<2H", data, color_at)
            r0, g0, b0 = color_565(color_0)
            r1, g1, b1 = color_565(color_1)
            if format != FORMAT_DXT1 or color_0 > color_1:
                palette = (
                    (r0, g0, b0, 255),
                    (r1, g1, b1, 255),
                    ((2 * r0 + r1) // 3, (2 * g0 + g1) // 3, (2 * b0 + b1) // 3, 255),
                    ((r0 + 2 * r1) // 3, (g0 + 2 * g1) // 3, (b0 + 2 * b1) // 3, 255),
                )
            else:
                palette = (
                    (r0, g0, b0, 255),
                    (r1, g1, b1, 255),
                    ((r0 + r1) // 2, (g0 + g1) // 2, (b0 + b1) // 2, 255),
                    (0, 0, 0, 0),
                )

            # the alpha of the 16 texels
            alphas = None
            if format == FORMAT_DXT3:
                bits = int.from_bytes(data[at : at + 8], "little")
                alphas = [((bits >> (4 * n)) & 0xF) * 17 for n in range(16)]
            elif format == FORMAT_DXT5:
                a0, a1 = data[at], data[at + 1]
                if a0 > a1:
                    table = [a0, a1] + [((7 - n) * a0 + n * a1) // 7 for n in range(1, 7)]
                else:
                    table = [a0, a1] + [((5 - n) * a0 + n * a1) // 5 for n in range(1, 5)] + [0, 255]
                bits = int.from_bytes(data[at + 2 : at + 8], "little")
                alphas = [table[(bits >> (3 * n)) & 7] for n in range(16)]

            for row in range(4):
                texels = [palette[index] for index in ROW_INDICES[data[color_at + 4 + row]]]
                if alphas:
                    texels = [texel[:3] + (alphas[row * 4 + n],) for n, texel in enumerate(texels)]
                start = ((block_y * 4 + row) * full_width + block_x * 4) * 4
                out[start : start + 16] = bytes(value for texel in texels for value in texel)
            at += block_size

    if full_width == width and blocks_high * 4 == height:
        return bytes(out)
    return b"".join(out[y * full_width * 4 :][: width * 4] for y in range(height))


def swizzle_tables(width: int, height: int) -> tuple[list[int], list[int]]:
    """The parts of a swizzled texel's index that its x and its y give.

    The bits of x and y alternate, x first, while both have bits; the bits
    that the larger one has left follow.
    """
    mask_x = mask_y = 0
    bit, position = 1, 1
    while bit < width or bit < height:
        if bit < width:
            mask_x |= position
            position <<= 1
        if bit < height:
            mask_y |= position
            position <<= 1
        bit <<= 1

    def spread(value: int, mask: int) -> int:
        result, position = 0, 1
        while mask:
            if mask & 1:
                if value & 1:
                    result |= position
                value >>= 1
            mask >>= 1
            position <<= 1
        return result

    return [spread(x, mask_x) for x in range(width)], [spread(y, mask_y) for y in range(height)]


def decode_texels(data: bytes, width: int, height: int, format: int, swizzled: bool) -> bytes:
    size = TEXEL_SIZES[format]
    if format == FORMAT_Y8:
        colors = [(value, value, value, 255) for value in data[: width * height]]
    elif format == FORMAT_R5G6B5:
        colors = [color_565(value) + (255,) for (value,) in struct.iter_unpack("<H", data[: width * height * 2])]
    elif format == FORMAT_A4R4G4B4:
        colors = [
            (((value >> 8) & 0xF) * 17, ((value >> 4) & 0xF) * 17, (value & 0xF) * 17, (value >> 12) * 17)
            for (value,) in struct.iter_unpack("<H", data[: width * height * 2])
        ]
    else:
        opaque = format == FORMAT_X8R8G8B8
        colors = [
            (red, green, blue, 255 if opaque else alpha)
            for blue, green, red, alpha in struct.iter_unpack("<4B", data[: width * height * size])
        ]

    if swizzled:
        table_x, table_y = swizzle_tables(width, height)
        colors = [colors[table_x[x] | table_y[y]] for y in range(height) for x in range(width)]
    return bytes(value for color in colors for value in color)
