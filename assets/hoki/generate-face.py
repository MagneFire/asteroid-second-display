#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Darrel Griët <dgriet@gmail.com>
# SPDX-License-Identifier: GPL-3.0-only
"""Renders the PNG assets of the hoki timepiece face.

The BG co-processor draws the time itself from a digit strip font, so the
face is a black background, a strip of the glyphs 0-9 and a colon.
The BG has a 16 colour palette with 1-bit alpha, so the glyphs are drawn
without antialiasing. Run it once with Pillow installed and commit the
output next to it.
"""

import argparse
import pathlib

from PIL import Image, ImageDraw, ImageFont

SIZE = 412
GLYPH_WIDTH = 84
GLYPH_HEIGHT = 120
COLON_WIDTH = 16
FONT_CANDIDATES = [
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
    "/usr/share/fonts/TTF/DejaVuSansMono-Bold.ttf",
    "/System/Library/Fonts/Menlo.ttc",
]


def load_font(path, size):
    for candidate in [path] if path else FONT_CANDIDATES:
        try:
            return ImageFont.truetype(candidate, size)
        except OSError:
            continue
    raise SystemExit("no usable font found, pass --font")


def draw_centered(draw, box, text, font):
    left, top, right, bottom = draw.textbbox((0, 0), text, font=font)
    x = box[0] + (box[2] - box[0] - (right - left)) / 2 - left
    y = box[1] + (box[3] - box[1] - (bottom - top)) / 2 - top
    draw.text((x, y), text, font=font, fill=(255, 255, 255, 255))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--font", help="TrueType font to render the digits with")
    parser.add_argument("--out", default=pathlib.Path(__file__).parent, type=pathlib.Path)
    args = parser.parse_args()
    font = load_font(args.font, 132)

    Image.new("L", (SIZE, SIZE), 0).save(args.out / "background.png", optimize=True)

    digits = Image.new("RGBA", (GLYPH_WIDTH, GLYPH_HEIGHT * 10), (0, 0, 0, 0))
    draw = ImageDraw.Draw(digits)
    draw.fontmode = "1"
    for digit in range(10):
        top = digit * GLYPH_HEIGHT
        draw_centered(draw, (0, top, GLYPH_WIDTH, top + GLYPH_HEIGHT), str(digit), font)
    digits.save(args.out / "digits.png", optimize=True)

    colon = Image.new("RGBA", (COLON_WIDTH, GLYPH_HEIGHT), (0, 0, 0, 0))
    draw = ImageDraw.Draw(colon)
    draw.fontmode = "1"
    radius = COLON_WIDTH // 2 - 1
    for cy in (GLYPH_HEIGHT * 0.35, GLYPH_HEIGHT * 0.68):
        draw.ellipse((1, cy - radius, COLON_WIDTH - 1, cy + radius), fill=(255, 255, 255, 255))
    colon.save(args.out / "colon.png", optimize=True)


if __name__ == "__main__":
    main()
