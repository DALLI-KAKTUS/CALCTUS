#include "ui/font.h"

namespace calctus::ui {

const Glyph* Font::find(uint32_t codepoint) const {
  int lo = 0, hi = static_cast<int>(count) - 1;
  while (lo <= hi) {
    const int mid = (lo + hi) / 2;
    const uint32_t cp = glyphs[mid].codepoint;
    if (cp == codepoint) return &glyphs[mid];
    if (cp < codepoint) {
      lo = mid + 1;
    } else {
      hi = mid - 1;
    }
  }
  return nullptr;
}

uint32_t utf8_next(std::string_view& s) {
  if (s.empty()) return 0;
  const auto b0 = static_cast<uint8_t>(s[0]);
  int len = 1;
  uint32_t cp = b0;
  if (b0 >= 0xF0) {
    len = 4;
    cp = b0 & 0x07;
  } else if (b0 >= 0xE0) {
    len = 3;
    cp = b0 & 0x0F;
  } else if (b0 >= 0xC0) {
    len = 2;
    cp = b0 & 0x1F;
  } else if (b0 >= 0x80) {
    s.remove_prefix(1);
    return 0xFFFD;
  }
  if (static_cast<size_t>(len) > s.size()) {
    s = {};
    return 0xFFFD;
  }
  for (int i = 1; i < len; ++i) {
    const auto b = static_cast<uint8_t>(s[static_cast<size_t>(i)]);
    if ((b & 0xC0) != 0x80) {
      s.remove_prefix(static_cast<size_t>(i));
      return 0xFFFD;
    }
    cp = (cp << 6) | (b & 0x3F);
  }
  s.remove_prefix(static_cast<size_t>(len));
  return cp;
}

}  // namespace calctus::ui
