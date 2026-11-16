#!/usr/bin/env python3
"""TTF yazı tipinden CALCTUS bit eşlem yazı tipi (C++ kaynağı) üretir.

Cihazda yazı tipi dosyası ya da çizici yoktur; glifler burada, derleme
zamanında piksel başına 4 bit saydamlıkla çizilip kaynağa gömülür.

Kullanım:
  fontgen.py --ttf DejaVuSans.ttf --size 12 --name sans12 \
             --charset assets/fonts/charset.txt --out ui/fonts/font_sans12.cpp
"""

import argparse
import sys

from fontTools.ttLib import TTFont
from PIL import Image, ImageDraw, ImageFont


def read_charset(path):
    chars = set(chr(c) for c in range(0x20, 0x7F))
    with open(path, encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            chars.update(ch for ch in line if not ch.isspace())
    return sorted(chars)


def render(font, ch):
    """(genişlik, yükseklik, xoff, yoff, ilerleme, 4 bit pikseller) döndürür."""
    advance = int(round(font.getlength(ch)))
    x0, y0, x1, y1 = font.getbbox(ch, anchor="ls")
    w, h = x1 - x0, y1 - y0
    if w <= 0 or h <= 0:
        return 0, 0, 0, 0, advance, []
    img = Image.new("L", (w, h), 0)
    ImageDraw.Draw(img).text((-x0, -y0), ch, font=font, fill=255, anchor="ls")
    box = img.getbbox()
    if box is None:
        return 0, 0, 0, 0, advance, []
    img = img.crop(box)
    x0 += box[0]
    y0 += box[1]
    w, h = img.size
    pixels = [(v + 8) // 17 for v in img.tobytes()]
    return w, h, x0, y0, advance, pixels


def pack4(pixels):
    out = bytearray()
    for i in range(0, len(pixels), 2):
        hi = pixels[i]
        lo = pixels[i + 1] if i + 1 < len(pixels) else 0
        out.append((hi << 4) | lo)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ttf", required=True)
    ap.add_argument("--size", type=int, required=True)
    ap.add_argument("--name", required=True)
    ap.add_argument("--charset", required=True)
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    cmap = TTFont(args.ttf, lazy=True).getBestCmap() or {}
    font = ImageFont.truetype(args.ttf, args.size)
    ascent, descent = font.getmetrics()

    glyphs = []
    bitmap = bytearray()
    missing = []
    for ch in read_charset(args.charset):
        if ord(ch) not in cmap:
            missing.append(ch)
            continue
        w, h, xoff, yoff, adv, pixels = render(font, ch)
        if not (-128 <= xoff <= 127 and -128 <= yoff <= 127 and w <= 255 and h <= 255 and adv <= 255):
            sys.exit(f"glif sınır dışı: U+{ord(ch):04X}")
        glyphs.append((ord(ch), len(bitmap), w, h, xoff, yoff, adv))
        bitmap += pack4(pixels)

    if missing:
        listed = " ".join(f"{c}(U+{ord(c):04X})" for c in missing)
        print(f"uyarı: {args.name}: yazı tipinde olmayan {len(missing)} karakter atlandı: {listed}",
              file=sys.stderr)

    lines = [
        "// Otomatik üretildi (tools/fontgen.py). Elle düzenlemeyin.",
        f"// Kaynak: {args.ttf.rsplit('/', 1)[-1]}, {args.size} px, {len(glyphs)} glif.",
        '#include "ui/font.h"',
        "",
        "namespace calctus::ui {",
        "",
        "namespace {",
        "",
        "const uint8_t kBitmap[] = {",
    ]
    for i in range(0, len(bitmap), 20):
        lines.append("    " + ",".join(f"0x{b:02X}" for b in bitmap[i:i + 20]) + ",")
    if not bitmap:
        lines.append("    0,")
    lines += ["};", "", "const Glyph kGlyphs[] = {"]
    for cp, off, w, h, xoff, yoff, adv in glyphs:
        lines.append(f"    {{0x{cp:04X}, {off}, {w}, {h}, {xoff}, {yoff}, {adv}}},")
    lines += [
        "};",
        "",
        "}  // namespace",
        "",
        f"const Font font_{args.name} = {{kGlyphs, {len(glyphs)}, kBitmap, {ascent + descent}, {ascent}}};",
        "",
        "}  // namespace calctus::ui",
        "",
    ]
    with open(args.out, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))
    print(f"{args.out}: {len(glyphs)} glif, {len(bitmap)} bayt")


if __name__ == "__main__":
    main()
