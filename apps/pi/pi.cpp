#include "apps/pi/pi.h"

#include <cstdio>

#include "kernel/system.h"
#include "services/pi_link.h"
#include "ui/theme.h"

namespace calctus::apps {
void PiApp::on_open() {
  connected_ = pi::ping(); // No Pi: return immediately after bounded timeout.
  calculated_ = false;
  info_[0] = '\0';
  System::get().invalidate_all();
}

bool PiApp::on_key(const KeyEvent& event) {
  if (!event.down || event.repeat) return false;
  switch (event.key) {
    case Key::F1: connected_ = pi::ping(); calculated_ = false; break;
    case Key::F2:
      info_[0] = '\0';
      if (connected_ && !pi::info(info_, sizeof info_)) connected_ = false;
      break;
    case Key::F3:
      calculated_ = connected_ && pi::square(input_, result_);
      if (connected_ && !calculated_) connected_ = false;
      break;
    case Key::Up: if (input_ < 46340) ++input_; calculated_ = false; break;
    case Key::Down: if (input_ > -46340) --input_; calculated_ = false; break;
    case Key::F6: System::get().go_home(); return true;
    default: return false;
  }
  System::get().invalidate_content();
  return true;
}

void PiApp::draw(ui::Canvas& c, Rect area) {
  const ui::Theme& t = ui::theme();
  c.fill_rect(area, t.bg);
  c.text(9, 25, "Opsiyonel Pi Zero 2 W / UART", ui::font_sans12, t.muted);
  c.text(9, 58, connected_ ? "Pi bagli" : "Pi yok: ESP bagimsiz calisir",
         ui::font_sans16, connected_ ? t.accent : t.text);
  if (info_[0]) c.text(9, 92, info_, ui::font_sans12, t.text);
  char line[56];
  std::snprintf(line, sizeof line, "Uzak hesap: %ld x %ld", static_cast<long>(input_),
                static_cast<long>(input_));
  c.text(9, 125, line, ui::font_sans12, t.text);
  if (calculated_) {
    std::snprintf(line, sizeof line, "Pi yaniti: %ld", static_cast<long>(result_));
    c.text(9, 151, line, ui::font_sans16, t.accent);
  }
  c.text(9, 194, "Terminal/ekran aktarimi: sonraki asama", ui::font_sans12, t.muted);
}

void PiApp::soft_keys(SoftKeys& keys) const {
  keys.label[0] = "Baglan";
  keys.label[1] = "Bilgi";
  keys.label[2] = "Hesap";
  keys.label[5] = "Ana";
}
}  // namespace calctus::apps
