#include "apps/keytest/keytest.h"

#include <cstdio>

#include "kernel/system.h"
#include "ui/theme.h"

namespace calctus::apps {

namespace {

// Mini tuş haritası: tuş takımının 0,44 ölçekli kopyası.
constexpr int kMapX = 172;
constexpr int kMapY = 28;
constexpr int map_scale(int v) { return v * 11 / 25; }

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

void KeyTestApp::draw(ui::Canvas& c, Rect area) {
  using ui::font_sans12;
  using ui::font_sans24;
  const ui::Theme& t = ui::theme();
  char buf[64];

  c.fill_rect(area, t.bg);

  // Son basılan tuş
  c.text(8, 26, "Son tuş", font_sans12, t.muted);
  const Rect last_box = make_rect(8, 44, 152, 62);
  c.box(last_box, 6, last_down_ ? t.accent : t.line, t.surface);
  if (last_ == Key::None) {
    c.text_in(last_box, "bir tuşa bas", font_sans12, t.muted);
    c.text(8, 112, "Fare ya da klavye", font_sans12, t.muted);
  } else {
    const KeyInfo& info = key_info(last_);
    c.text_in(last_box, info.label, font_sans24, t.text);
    std::snprintf(buf, sizeof buf, "%s · %s", info.name, last_down_ ? "basılı" : "bırakıldı");
    c.text(8, 112, buf, font_sans12, t.text);
    std::snprintf(buf, sizeof buf, "Matris: satır %d, sütun %d", key_row(last_), key_col(last_));
    c.text(8, 130, buf, font_sans12, t.text);
  }

  std::snprintf(buf, sizeof buf, "Görülen: %d / %d", seen_count_, kKeyCount);
  c.text(8, 148, buf, font_sans12, t.text);
  std::snprintf(buf, sizeof buf, "Basış: %u", static_cast<unsigned>(presses_));
  c.text(8, 166, buf, font_sans12, t.text);
  const unsigned secs = static_cast<unsigned>((now_ms_ - start_ms_) / 1000);
  std::snprintf(buf, sizeof buf, "Süre: %u:%02u", secs / 60, secs % 60);
  c.text(kTimeRect.x, kTimeRect.y, buf, font_sans12, t.text);

  if (seen_count_ == kKeyCount) {
    c.text(8, 202, "✓ Tüm tuşlar çalışıyor", font_sans12, t.accent);
  } else {
    c.text(8, 202, "Çıkış: ⌂ tuşuna iki kez", font_sans12, t.muted);
  }

  // Mini tuş haritası
  for (int i = 0; i < kKeyCount; ++i) {
    const KeyInfo& k = key_info(static_cast<Key>(i));
    const Rect r = make_rect(kMapX + map_scale(k.x), kMapY + map_scale(k.y), map_scale(k.w),
                             map_scale(k.h));
    if (down_[i]) {
      c.fill_round_rect(r, 2, t.accent);
    } else if (seen_[i]) {
      c.fill_round_rect(r, 2, t.select);
    } else {
      c.box(r, 2, t.line, t.bg);
    }
  }
}

}  // namespace calctus::apps
