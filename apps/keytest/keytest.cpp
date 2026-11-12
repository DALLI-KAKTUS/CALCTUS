#include "apps/keytest/keytest.h"

#include <cstdio>

#include "calctus/hal.h"

namespace calctus::apps {

namespace {

struct Theme {
  Color bg, panel, text, muted, line, accent, on_accent, seen;
};

constexpr Theme kLight = {
    rgb(244, 241, 234), rgb(255, 255, 255), rgb(30, 36, 32),    rgb(112, 120, 114),
    rgb(201, 196, 184), rgb(47, 143, 91),   rgb(255, 255, 255), rgb(170, 208, 186),
};

constexpr Theme kDark = {
    rgb(18, 22, 20), rgb(28, 34, 31),  rgb(230, 237, 232), rgb(140, 150, 144),
    rgb(52, 62, 56), rgb(76, 195, 138), rgb(10, 20, 14),   rgb(40, 84, 62),
};

constexpr int kTopBarH = 20;
constexpr int kSoftBarY = 220;

// Mini tuş haritası: tuş takımının 0,44 ölçekli kopyası.
constexpr int kMapX = 172;
constexpr int kMapY = 28;
constexpr int map_scale(int v) { return v * 11 / 25; }

}  // namespace

void KeyTestApp::on_open() {
  start_ms_ = hal::millis();
  now_ms_ = start_ms_;
}

void KeyTestApp::on_key(const KeyEvent& ev) {
  const int i = key_index(ev.key);
  if (i < 0 || i >= kKeyCount) return;

  if (ev.down) {
    down_[i] = true;
    if (!seen_[i]) {
      seen_[i] = true;
      ++seen_count_;
    }
    ++presses_;
    last_ = ev.key;
    last_down_ = true;
    if (ev.key == Key::Mod) dark_ = !dark_;
  } else {
    down_[i] = false;
    if (ev.key == last_) last_down_ = false;
  }
  request_redraw();
}

void KeyTestApp::on_tick(uint32_t now_ms) {
  now_ms_ = now_ms;
  request_redraw();
}

void KeyTestApp::draw(ui::Canvas& c) {
  using ui::Align;
  using ui::font_sans12;
  using ui::font_sans24;
  const Theme& t = dark_ ? kDark : kLight;
  char buf[64];

  c.fill(t.bg);

  // Üst çubuk
  c.fill_rect(make_rect(0, 0, hal::kScreenWidth, kTopBarH), t.accent);
  c.text_in(make_rect(6, 0, 150, kTopBarH), "CALCTUS", font_sans12, t.on_accent, Align::Left);
  c.text_in(make_rect(160, 0, 154, kTopBarH), "tuş testi · S0", font_sans12, t.on_accent,
            Align::Right);

  // Son basılan tuş
  c.text(8, 26, "Son tuş", font_sans12, t.muted);
  const Rect last_box = make_rect(8, 44, 152, 62);
  c.box(last_box, 6, last_down_ ? t.accent : t.line, t.panel);
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
  c.text(8, 184, buf, font_sans12, t.text);

  if (seen_count_ == kKeyCount) {
    c.text(8, 202, "✓ Tüm tuşlar çalışıyor", font_sans12, t.accent);
  } else {
    c.text(8, 202, "mod: tema değiştir", font_sans12, t.muted);
  }

  // Mini tuş haritası
  for (int i = 0; i < kKeyCount; ++i) {
    const KeyInfo& k = key_info(static_cast<Key>(i));
    const Rect r = make_rect(kMapX + map_scale(k.x), kMapY + map_scale(k.y), map_scale(k.w),
                             map_scale(k.h));
    if (down_[i]) {
      c.fill_round_rect(r, 2, t.accent);
    } else if (seen_[i]) {
      c.fill_round_rect(r, 2, t.seen);
    } else {
      c.box(r, 2, t.line, t.bg);
    }
  }

  // Alt menü çubuğu: F1..F6 basılıyken hücresi yanar.
  c.fill_rect(make_rect(0, kSoftBarY, hal::kScreenWidth, 1), t.line);
  for (int i = 0; i < 6; ++i) {
    const int x0 = i * hal::kScreenWidth / 6;
    const int x1 = (i + 1) * hal::kScreenWidth / 6;
    const Rect cell = make_rect(x0, kSoftBarY + 1, x1 - x0, hal::kScreenHeight - kSoftBarY - 1);
    const Key fkey = static_cast<Key>(key_index(Key::F1) + i);
    const bool pressed = down_[key_index(fkey)];
    if (pressed) c.fill_rect(cell, t.accent);
    if (i > 0) c.fill_rect(make_rect(x0, kSoftBarY + 4, 1, 12), t.line);
    c.text_in(cell, key_info(fkey).label, font_sans12, pressed ? t.on_accent : t.muted);
  }
}

}  // namespace calctus::apps
