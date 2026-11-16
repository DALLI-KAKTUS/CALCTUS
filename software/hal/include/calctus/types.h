// Ortak temel tipler. Hem simülatörde hem ESP32'de aynı.
#pragma once

#include <cstdint>

namespace calctus {

// Ekran rengi: RGB565 (panelin kendi biçimi).
using Color = uint16_t;

constexpr Color rgb(uint8_t r, uint8_t g, uint8_t b) {
  return static_cast<Color>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

struct Rect {
  int16_t x = 0, y = 0, w = 0, h = 0;

  constexpr bool empty() const { return w <= 0 || h <= 0; }
  constexpr int right() const { return x + w; }
  constexpr int bottom() const { return y + h; }
  constexpr bool contains(int px, int py) const {
    return px >= x && py >= y && px < right() && py < bottom();
  }
};

constexpr Rect make_rect(int x, int y, int w, int h) {
  return Rect{static_cast<int16_t>(x), static_cast<int16_t>(y), static_cast<int16_t>(w),
              static_cast<int16_t>(h)};
}

constexpr Rect intersect(Rect a, Rect b) {
  const int x0 = a.x > b.x ? a.x : b.x;
  const int y0 = a.y > b.y ? a.y : b.y;
  const int x1 = a.right() < b.right() ? a.right() : b.right();
  const int y1 = a.bottom() < b.bottom() ? a.bottom() : b.bottom();
  if (x1 <= x0 || y1 <= y0) return Rect{};
  return make_rect(x0, y0, x1 - x0, y1 - y0);
}

// İki dikdörtgeni kapsayan en küçük dikdörtgen; boş olan yok sayılır.
constexpr Rect unite(Rect a, Rect b) {
  if (a.empty()) return b;
  if (b.empty()) return a;
  const int x0 = a.x < b.x ? a.x : b.x;
  const int y0 = a.y < b.y ? a.y : b.y;
  const int x1 = a.right() > b.right() ? a.right() : b.right();
  const int y1 = a.bottom() > b.bottom() ? a.bottom() : b.bottom();
  return make_rect(x0, y0, x1 - x0, y1 - y0);
}

}  // namespace calctus
