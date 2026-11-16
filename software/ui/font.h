// Bit eşlem yazı tipi. Veriler tools/fontgen.py ile üretilir (ui/fonts/).
#pragma once

#include <cstdint>
#include <string_view>

namespace calctus::ui {

struct Glyph {
  uint32_t codepoint;
  uint32_t offset;  // bitmap içindeki başlangıç (bayt)
  uint8_t w, h;     // piksel
  int8_t xoff;      // kalem konumundan sola/sağa kayma
  int8_t yoff;      // taban çizgisinden üst kenara (yukarı negatif)
  uint8_t advance;  // kalemin ilerleyeceği miktar
};

struct Font {
  const Glyph* glyphs;  // kod noktasına göre sıralı
  uint16_t count;
  const uint8_t* bitmap;  // piksel başına 4 bit saydamlık, glif başına bayt hizalı
  uint8_t line_height;
  uint8_t ascent;

  const Glyph* find(uint32_t codepoint) const;
};

// UTF-8 dizgiden bir kod noktası okur ve dizgiyi ilerletir. Bozuk baytta U+FFFD.
uint32_t utf8_next(std::string_view& s);

extern const Font font_sans12;
extern const Font font_sans16;
extern const Font font_sans24;

}  // namespace calctus::ui
