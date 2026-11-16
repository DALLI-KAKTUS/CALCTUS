// Canvas: RGB565 piksel belleğine çizim. Kırpma yapar ve değişen bölgeyi izler.
#pragma once

#include <cstdint>
#include <string_view>

#include "calctus/types.h"
#include "ui/font.h"

namespace calctus::ui {

enum class Align : uint8_t { Left, Center, Right };

class Canvas {
 public:
  Canvas(Color* pixels, int width, int height);

  int width() const { return w_; }
  int height() const { return h_; }
  const Color* pixels() const { return px_; }

  // Kırpma bölgesi; çizimler bunun dışına taşmaz.
  void set_clip(Rect clip);
  void reset_clip();

  void fill(Color c);
  void fill_rect(Rect r, Color c);
  void frame_rect(Rect r, Color c);
  void fill_round_rect(Rect r, int radius, Color c);
  // Kenarlıklı yuvarlak köşeli kutu.
  void box(Rect r, int radius, Color border, Color fill);

  // (x, y) satırın sol üst köşesidir. İlerleme miktarını (piksel) döndürür.
  int text(int x, int y, std::string_view s, const Font& font, Color c);
  // Metni kutunun içinde yatayda hizalar, dikeyde ortalar.
  void text_in(Rect box, std::string_view s, const Font& font, Color c,
               Align align = Align::Center);
  // Metni kutunun genişliğine göre satırlara böler ('\n' satır atlatır).
  // Kullanılan yüksekliği döndürür.
  int text_wrapped(Rect box, std::string_view s, const Font& font, Color c);
  static int text_width(std::string_view s, const Font& font);

  // Son temizlemeden beri çizilen bölge.
  Rect dirty() const { return dirty_; }
  void clear_dirty() { dirty_ = Rect{}; }

 private:
  void glyph(int left, int top, const Glyph& g, const Font& font, Color c);

  Color* px_;
  int w_, h_;
  Rect clip_;
  Rect dirty_{};
};

}  // namespace calctus::ui
