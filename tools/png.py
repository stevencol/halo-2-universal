# writes PNG images, and reads back the ones it writes, with no library

import struct
import zlib
from pathlib import Path

SIGNATURE = b"\x89PNG\r\n\x1a\n"
# PNG color types, by the bytes of a pixel
COLOR_TYPES = {3: 2, 4: 6}


def write_png(path: Path, width: int, height: int, pixels: bytes, channels: int = 3) -> None:
    """Writes 8-bit RGB (3 channels) or RGBA (4 channels) pixels, the top row first."""

    def chunk(kind: bytes, data: bytes) -> bytes:
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))

    row_size = width * channels
    if len(pixels) != row_size * height:
        raise ValueError("the pixels do not match the size of the image")
    # each row starts with its filter: 0, none
    rows = b"".join(b"\0" + bytes(pixels[y * row_size : (y + 1) * row_size]) for y in range(height))
    Path(path).write_bytes(
        SIGNATURE
        + chunk(b"IHDR", struct.pack(">2I5B", width, height, 8, COLOR_TYPES[channels], 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(rows, 3))
        + chunk(b"IEND", b"")
    )


def read_png(path: Path) -> tuple[int, int, int, bytes]:
    """Reads an image that write_png wrote: (width, height, channels, pixels)."""
    data = Path(path).read_bytes()
    if data[:8] != SIGNATURE:
        raise ValueError(f"'{path}' is not a PNG image")
    at = 8
    compressed = b""
    width = height = channels = 0
    while at < len(data):
        (size,) = struct.unpack_from(">I", data, at)
        kind = data[at + 4 : at + 8]
        body = data[at + 8 : at + 8 + size]
        if kind == b"IHDR":
            width, height, bits, color_type, _, _, interlace = struct.unpack(">2I5B", body)
            channels = {value: key for key, value in COLOR_TYPES.items()}.get(color_type, 0)
            if bits != 8 or not channels or interlace:
                raise ValueError(f"'{path}' is not an image of the kind write_png writes")
        elif kind == b"IDAT":
            compressed += body
        at += 12 + size

    rows = zlib.decompress(compressed)
    row_size = width * channels + 1
    if any(rows[y * row_size] for y in range(height)):
        raise ValueError(f"'{path}' has filtered rows, which are not read")
    return width, height, channels, b"".join(rows[y * row_size + 1 : (y + 1) * row_size] for y in range(height))
