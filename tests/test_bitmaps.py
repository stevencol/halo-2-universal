# source/bitmaps/bitmap_utilities.c and source/bitmaps/bitmap_color_conversion.c

import random
import struct

from difftest import Buffer, f32


def reals(*values: float) -> Buffer:
    return Buffer(struct.pack(f"<{len(values)}f", *values), format=f"<{len(values)}f")


def run(t) -> None:
    random_ = random.Random(5)

    for color in (0, 0xFFFFFFFF, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x80808080, 0x01020304):
        t.check("pixel32_to_real_rgb_color", color, reals(7, 7, 7))
    for _ in range(100):
        t.check("pixel32_to_real_rgb_color", random_.getrandbits(32), reals(7, 7, 7))

    # every value of a component, and the values halfway between two, which
    # are rounded to the even one
    components = [n / 255 for n in range(256)] + [(n + 0.5) / 255 for n in range(255)]
    for component in components:
        t.check("real_alpha_to_pixel32", f32(component))
        t.check("real_alpha_and_rgb_color_to_pixel32", f32(1 - component), reals(component, 1 - component, 0.25))
        t.check("real_rgb_color_to_pixel32", reals(component, 1 - component, component / 2))
        t.check("real_argb_color_to_pixel32", reals(component, 1 - component, component / 2, 1.0))
    # components outside 0 to 1
    for _ in range(100):
        color = [random_.uniform(-2, 3) for _ in range(4)]
        t.check("real_rgb_color_to_pixel32", reals(*color[:3]))
        t.check("real_argb_color_to_pixel32", reals(*color))

    # ---------- the bytes of the pixels of a bitmap (source/bitmaps/bitmaps.c)

    BITMAP_SIZE = 0x74
    formats = (-1, 0, 1, 2, 3, 4, 6, 8, 9, 10, 11, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23)
    sizes = (0, 1, 2, 3, 4, 5, 7, 8, 16, 17, 64, 100, 255, 256, 257, 640, 1024, 0x7FFF, -1, -5, -0x8000)

    def signed(value: int) -> int:
        """As an argument of 16 bits with its sign, which is how the game gives them."""
        return value & 0xFFFFFFFF

    def bitmap(width: int, height: int, depth: int, type_: int, format_: int, flags: int, mipmap_count: int) -> Buffer:
        data = bytearray(random_.randbytes(BITMAP_SIZE))
        struct.pack_into("<4h", data, 4, width, height, depth, type_)
        struct.pack_into("<hH", data, 0xC, format_, flags)
        struct.pack_into("<h", data, 0x14, mipmap_count)
        return Buffer(bytes(data))

    def a_bitmap() -> Buffer:
        return bitmap(
            random_.choice(sizes),
            random_.choice(sizes),
            # only the low byte of the depth is looked at
            random_.choice((0, 1, 1, 1, 2, 4, 6, 0x40, 0x7F, 0x80, 0xFF, 0x101, 0x100)) - (0x10000 if random_.random() < 0.1 else 0) & 0x7FFF,
            random_.choice((0, 0, 1, 2, 2, 3)),
            random_.choice(formats),
            random_.choice((0, 2, 2, 0xFFFD, 0xFFFF, 0x0A)),
            random_.choice((0, 0, 1, 3, 9, 12, -1, -3)),
        )

    for _ in range(500):
        t.check("bitmap_pixels_size", a_bitmap())
        t.check("bitmap_mipmap_pixel_count", a_bitmap(), random_.choice((0, 0, 1, 2, 3, 7, 8, 10, 15, 16, 20, 31, 32, 33, 0xFF)))
        t.check("bitmap_row_size", random_.choice((0, 0, 1, 2, 3, 7, 8, 10, 15, 16, 20, 31, 32, 33, 0xFF)), a_bitmap())
    # so many pixels that their count is past 32 bits, and what is left of it
    # times the bits is less than zero and no multiple of 8: the division
    # rounds to zero, which is not what a shift does
    wrapped = 0
    for depth in range(1, 128):
        for type_ in (0, 2):
            count = 0x7FFF * 0x7FFF * depth * (6 if type_ == 2 else 1) & 0xFFFFFFFF
            bits = 4 * count & 0xFFFFFFFF
            if bits & 0x80000000 and bits % 8:
                t.check("bitmap_pixels_size", bitmap(0x7FFF, 0x7FFF, depth, type_, 14, 0, 0))
                wrapped += 1
    assert wrapped > 8

    for _ in range(900):
        width, height, depth = (random_.choice(sizes) for _ in range(3))
        format_ = random_.choice(formats)
        alignment = random_.choice((0, 0, 1, 2, 4, 4, 8, 16, 64, 0x1000, -1, -4, 3))
        t.check(
            "bitmap_dimensions_mipmap_size",
            signed(width),
            random_.choice((0, 0, 1, 2, 3, 7, 10, 15, 16, 31, 32, 0xFF)),
            signed(height),
            signed(depth),
            signed(format_),
            signed(alignment),
        )
        t.check(
            "bitmap_dimensions_size",
            signed(random_.choice((0, 0, 1, 3, 9, 12, -1, -3))),
            signed(width),
            signed(height),
            signed(depth),
            signed(format_),
            signed(alignment),
        )

    # ---------- reals of 16 bits

    for value in (0, 1, 0x1F, 0x20, 0x8000, 0x8001, 0x7FFF, 0xFFFF, 0x7FE0, 0xF, 0x10, 0x801F):
        t.check("real16_to_real", value)
    for _ in range(300):
        # with more than 16 bits: the rest is not looked at
        t.check("real16_to_real", random_.getrandbits(16) | random_.choice((0, 0, 0x10000, 0xABCD0000)))

    for format_ in formats:
        t.check("bitmap_format_get_bits", signed(format_))
        t.check("bitmap_format_get_bits", signed(format_) & 0xFFFF | 0x77770000)

    # ---------- where a pixel of a cube map is

    for _ in range(700):
        data = a_bitmap()
        t.check(
            "bitmap_cube_map_get_pixel_address",
            data,
            signed(random_.choice(sizes)),
            signed(random_.choice(sizes)),
            signed(random_.choice((0, 0, 1, 2, 5, 6, -1, 0x7FFF))),
            signed(random_.choice((0, 0, 1, 2, 3, 5, 9, 12, 16, 40, -1, -3))),
        )

    mipmaps = (0, 0, 1, 2, 3, 5, 9, 12, 16, 40, -1, -3)
    for _ in range(700):
        t.check(
            "bitmap_2d_get_pixel_address",
            a_bitmap(),
            signed(random_.choice(mipmaps)),
            signed(random_.choice(sizes)),
            signed(random_.choice(sizes)),
        )
        t.check(
            "bitmap_3d_get_pixel_address",
            a_bitmap(),
            signed(random_.choice(sizes)),
            signed(random_.choice(sizes)),
            signed(random_.choice((0, 0, 1, 2, 5, 63, -1, 0x7FFF))),
            signed(random_.choice(mipmaps)),
        )

    # a pixel before the first, with 4 bits for each: the bits are less than
    # zero and no multiple of 8, and the division rounds to zero
    for x in (-1, -3, -5, -7, -9):
        for mipmap in (0, 1):
            data = bitmap(64, 64, 1, 0, 14, 0, 3)
            t.check("bitmap_2d_get_pixel_address", data, mipmap, signed(x), 0)
            t.check("bitmap_3d_get_pixel_address", data, signed(x), 0, 0, mipmap)
            t.check("bitmap_cube_map_get_pixel_address", data, signed(x), 0, 0, mipmap)

    # ---------- the end of a bitmap

    for n in range(120):
        flags = random_.choice((0x100, 0x100, 0x102, 0, 0xFEFF, 0xFFFF))
        data = bytearray(random_.randbytes(BITMAP_SIZE))
        struct.pack_into("<H", data, 0xE, flags)
        struct.pack_into("<2I", data, 0x50, random_.choice((0, 0x80123400, 0x80123400)), random_.choice((0, 0x80456000, 0x80456000)))
        failed = random_.choice((0, 0, 1, 2))

        def free(harness, arguments, failed=failed):
            address, size, type_ = arguments
            from test_async import calls_so_far

            # the bitmap is given back too: where it is is not the same in
            # the game and in source/
            first = calls_so_far(harness, "VirtualFree") == 0
            return not (failed == 2 or failed == 1 and first), (harness.normalize(address), size, type_)

        t.check(
            "bitmap_delete",
            0 if n % 10 == 9 else Buffer(bytes(data)),
            externals={
                "texture_cache_bitmap_delete": lambda harness, arguments: (0, (harness.normalize(arguments[0]),)),
                "D3DResource_Release": lambda harness, arguments: 0,
                "VirtualFree": free,
                "GetLastError": lambda harness, arguments: 5,
            },
        )

    # ---------- a new bitmap

    from test_async import calls_so_far

    for n in range(300):
        made = Buffer(random_.randbytes(BITMAP_SIZE + 8))
        pixels = Buffer(random_.randbytes(16))
        # there is no memory for the bitmap, or for its pixels
        fails = random_.choice((None, None, None, None, 0, 1))

        def allocate(harness, arguments, made=made, pixels=pixels, fails=fails):
            address, size, type_, protection = arguments
            call = calls_so_far(harness, "VirtualAlloc")
            return 0 if call == fails else (made, pixels)[min(call, 1)].address

        t.check(
            "bitmap_2d_new",
            random_.choice((1, 2, 3, 4, 16, 100, 256, 0x100, 0x4000, 0, 0xFFFF, 0x8000)) | random_.choice((0, 0x10000, 0xABCD0000)),
            random_.choice((1, 2, 5, 8, 16, 100, 256, 0x100, 0, 0xFFFE, 0x8000)) | random_.choice((0, 0x10000, 0xABCD0000)),
            random_.choice((0, 0, 1, 3, 9, 0xFFFF)),
            random_.choice((0, 1, 6, 9, 10, 11, 13, 14, 15, 16, 17, 18, 22, 0xFFFF, 0x1000E)),
            random_.choice((0, 0x810, 0x800, 0x10, 0x100, 0x7FF, 0xF7FF, 0xFFFF, 0x10000, 0x10800, 1, 2, 4, 6)),
            extra=[made, pixels],
            externals={
                "VirtualAlloc": allocate,
                "GetLastError": lambda harness, arguments: 8,
            },
        )
