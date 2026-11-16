#pragma once

#include <cstdint>

#include "calctus/keys.h"

namespace calctus::esp32 {

// TCA8418 ROW0..7 / COL0..9 olay tablosu 1..80; PCB'de yalnız COL0..6 bağlı.
constexpr int key_index_from_tca8418(uint8_t event) {
  const int code = (event & 0x7f) - 1;
  if (code < 0 || code >= 80) return -1;
  const int row = code / 10, col = code % 10;
  const int index = row * kKeyMatrixCols + col;
  return col < kKeyMatrixCols && index < kKeyCount ? index : -1;
}

}  // namespace calctus::esp32
