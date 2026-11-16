#!/usr/bin/env python3
"""Sabit periyodik tabloyu bilimsel olgulardan C++ kaynağına dönüştürür.

Kaynak JSON örneği (derleme sırasında indirilmez):
https://raw.githubusercontent.com/Bowserinator/Periodic-Table-JSON/
a78c7a48fe7235cebd6153161a9e7fa70f0cfb30/PeriodicTableJSON.json

Yalnız sembol, ad, sayı, yaklaşık kütle, periyot, grup, faz ve blok alanları
çıkarılır. Kaynaktaki doğrulanmamış 119. element dahil edilmez.
"""

import json
import pathlib
import sys


def generate(input_path: pathlib.Path, output_path: pathlib.Path):
    data = json.loads(input_path.read_text(encoding="utf-8"))["elements"]
    rows = []
    for index, item in enumerate(data[:118], 1):
        if item["number"] != index or not item["symbol"].isascii():
            raise ValueError(f"bozuk atom numarası / sembol: {index}")
        phase = {"Gas": "G", "Liquid": "L", "Solid": "S"}[item["phase"]]
        block = item["block"]
        group = 0 if block == "f" else item["group"]
        name = json.dumps(item["name"], ensure_ascii=True)
        mass = format(float(item["atomic_mass"]), ".10g")
        if "e" not in mass and "." not in mass:
            mass += ".0"
        rows.append(
            f'    {{"{item["symbol"]}", {name}, {mass}f, '
            f'{item["period"]}, {group}, \'{phase}\', \'{block}\'}},'
        )
    if len(rows) != 118:
        raise ValueError("118 element gerekli")
    source = '''// tools/gen_elements.py tarafından üretildi; el ile değiştirmeyin.
// Bilgiler: Bowserinator/Periodic-Table-JSON, a78c7a48 (yalnız olgusal alanlar).
// Veri lisansı: CC BY-SA 3.0 (LICENSES/CC-BY-SA-3.0.txt). Bkz. THIRD_PARTY.md.
#include "apps/elements/data.h"

#include <cctype>

namespace calctus::apps {
namespace {

constexpr Element kElements[kElementCount] = {
''' + "\n".join(rows) + '''
};

}  // namespace

const Element* element_by_number(int number) {
  return number >= 1 && number <= kElementCount ? &kElements[number - 1] : nullptr;
}

int element_number_for_symbol(const char* symbol) {
  if (!symbol || !*symbol) return 0;
  for (int i = 0; i < kElementCount; ++i) {
    const char* name = kElements[i].symbol;
    const char* cursor = symbol;
    while (*name && *cursor &&
           std::tolower(static_cast<unsigned char>(*name)) ==
               std::tolower(static_cast<unsigned char>(*cursor))) {
      ++name;
      ++cursor;
    }
    if (!*name && !*cursor) return i + 1;
  }
  return 0;
}

}  // namespace calctus::apps
'''
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(source, encoding="utf-8")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit("kullanım: gen_elements.py INPUT.json OUTPUT.cpp")
    generate(pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2]))
