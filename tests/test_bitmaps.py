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
