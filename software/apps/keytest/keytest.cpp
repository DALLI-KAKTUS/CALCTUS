#include "apps/keytest/keytest.h"

#include <cstdio>

#include "kernel/system.h"

namespace calctus::apps {

namespace {

constexpr uint32_t kDoublePressMs = 600;
constexpr Rect kTimeRect = {8, 184, 160, 16};

}  // namespace

void KeyTestApp::on_open() {
  for (int i = 0; i < kKeyCount; ++i) down_[i] = seen_[i] = false;
  seen_count_ = 0;
  presses_ = 0;
  last_ = Key::None;
  last_down_ = false;
  home_once_ = false;
  start_ms_ = hal::millis();
  now_ms_ = start_ms_;
}

bool KeyTestApp::on_key(const KeyEvent& ev) {
  const int i = key_index(ev.key);
  if (i < 0 || i >= kKeyCount || ev.repeat) return true;
  System& sys = System::get();

  if (ev.down) {
    // Ana ekran tuşuna art arda iki kez basılırsa testten çık.
    if (ev.key == Key::Home) {
      if (home_once_ && ev.time_ms - last_home_ms_ <= kDoublePressMs) {
        sys.go_home();
        return true;
      }
      home_once_ = true;
      last_home_ms_ = ev.time_ms;
    } else {
      home_once_ = false;
    }

    down_[i] = true;
    if (!seen_[i]) {
      seen_[i] = true;
      ++seen_count_;
    }
    ++presses_;
    last_ = ev.key;
    last_down_ = true;
  } else {
    down_[i] = false;
    if (ev.key == last_) last_down_ = false;
  }
  sys.invalidate_content();
  return true;
}

void KeyTestApp::on_tick(uint32_t now_ms) {
  now_ms_ = now_ms;
  System::get().invalidate(kTimeRect);  // yalnız süre satırı değişir
}

}  // namespace calctus::apps
