#include "apps/pi/pi.h"

#include <cstdio>

#include "kernel/system.h"
#include "services/pi_link.h"

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

void PiApp::soft_keys(SoftKeys& keys) const {
  keys.label[0] = "Baglan";
  keys.label[1] = "Bilgi";
  keys.label[2] = "Hesap";
  keys.label[5] = "Ana";
}
}  // namespace calctus::apps
