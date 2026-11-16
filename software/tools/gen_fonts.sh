#!/bin/sh
# ui/fonts/ altındaki yazı tipi kaynaklarını yeniden üretir.
# Geliştirme kabuğunda çalıştırılır (CALCTUS_FONT_DIR oradan gelir).
# Üretilen dosyalar depoya eklenir: cihaz derlemesi Python ya da TTF istemez.
set -eu
cd "$(dirname "$0")/.."
: "${CALCTUS_FONT_DIR:?geliştirme kabuğunda çalıştırın (nix develop)}"

gen() {
  python3 tools/fontgen.py --ttf "$CALCTUS_FONT_DIR/$1" --size "$2" --name "$3" \
    --charset assets/fonts/charset.txt --out "ui/fonts/font_$3.cpp"
}

mkdir -p ui/fonts
gen DejaVuSans.ttf 12 sans12
gen DejaVuSans.ttf 16 sans16
gen DejaVuSans.ttf 24 sans24
