#include "kernel/system.h"

#include <cstdio>
#include <cstring>

#include "ui/theme.h"

namespace calctus {

namespace {

constexpr uint32_t kRepeatDelayMs = 400;
constexpr uint32_t kRepeatRateMs = 70;
constexpr uint32_t kToastMs = 1600;

constexpr Rect kScreen = {0, 0, hal::kScreenWidth, hal::kScreenHeight};
constexpr Rect kStatus = {0, 0, hal::kScreenWidth, System::kStatusH};
constexpr Rect kSoft = {0, hal::kScreenHeight - System::kSoftH, hal::kScreenWidth, System::kSoftH};

// Sayaç taşmasına dayanıklı: deadline'a kalan süre, geçtiyse 0.
uint32_t until(uint32_t deadline, uint32_t now) {
  const auto d = static_cast<int32_t>(deadline - now);
  return d <= 0 ? 0 : static_cast<uint32_t>(d);
}

bool repeatable(Key key) {
  return key == Key::Up || key == Key::Down || key == Key::Left || key == Key::Right ||
         key == Key::Backspace;
}

const char* angle_label(AngleUnit unit) {
  switch (unit) {
    case AngleUnit::Degree:
      return "DER";
    case AngleUnit::Radian:
      return "RAD";
    case AngleUnit::Gradian:
      return "GRA";
  }
  return "";
}

}  // namespace

System& System::get() {
  static System instance;
  return instance;
}

void System::set_apps(const AppEntry* entries, int count, App& home) {
  apps_ = entries;
  app_count_ = count;
  home_ = &home;
}

int System::run() {
  hal::init();
  ui::Canvas canvas(hal::framebuffer(), hal::kScreenWidth, hal::kScreenHeight);
  canvas_ = &canvas;
  apply_settings();
  open(*home_);

  while (!hal::should_exit()) {
    redraw();
    KeyEvent ev;
    if (hal::keys_wait(ev, next_timeout(hal::millis()))) dispatch(ev);
    service_timers(hal::millis());
  }
  canvas_ = nullptr;
  return 0;
}

void System::open(App& app) {
  if (app_ == &app) return;
  if (app_ != nullptr) app_->on_close();
  app_ = &app;
  last_tick_ = hal::millis();
  app.on_open();
  invalidate_all();
}

void System::go_home() { open(*home_); }

void System::invalidate(Rect area) { invalid_ = unite(invalid_, intersect(area, kScreen)); }

void System::invalidate_all() { invalid_ = kScreen; }

void System::show(ui::Modal& modal) {
  modal_ = &modal;
  invalidate(modal.bounds());
}

void System::close_modal() {
  if (modal_ == nullptr) return;
  invalidate(modal_->bounds());
  modal_ = nullptr;
}

void System::toast(const char* text) {
  if (toast_active_) invalidate(toast_rect_);
  std::snprintf(toast_, sizeof toast_, "%s", text);
  const int w = ui::Canvas::text_width(toast_, ui::font_sans12) + 28;
  toast_rect_ = make_rect((hal::kScreenWidth - w) / 2, kSoft.y - 32, w, 24);
  toast_active_ = true;
  toast_until_ = hal::millis() + kToastMs;
  invalidate(toast_rect_);
}

bool System::key_down(Key key) const {
  const int i = key_index(key);
  return i >= 0 && i < kKeyCount && down_[i];
}

void System::apply_settings() {
  const bool theme_changed = ui::dark_theme() != settings_.dark;
  ui::set_dark_theme(settings_.dark);
  if (screen_on_) {
    // Yüzde 0'da bile ekran okunabilsin diye alt sınır var.
    hal::backlight(static_cast<uint8_t>(20 + settings_.brightness * 235 / 100));
  }
  // Tema her şeyi değiştirir; diğer ayarlar yalnız durum çubuğunu etkiler.
  if (theme_changed) {
    invalidate_all();
  } else {
    invalidate(kStatus);
  }
}

void System::set_screen(bool on) {
  screen_on_ = on;
  if (on) {
    apply_settings();
    invalidate_all();
  } else {
    hal::backlight(0);
  }
}

void System::dispatch(const KeyEvent& ev) {
  const int i = key_index(ev.key);
  if (i < 0 || i >= kKeyCount) return;

  if (!ev.repeat) {
    down_[i] = ev.down;
    if (ev.down && repeatable(ev.key)) {
      repeat_key_ = ev.key;
      repeat_at_ = hal::millis() + kRepeatDelayMs;
    } else if (!ev.down && ev.key == repeat_key_) {
      repeat_key_ = Key::None;
    }
    // Menü çubuğu basılı F tuşunu gösterir.
    if (ev.key >= Key::F1 && ev.key <= Key::F6) invalidate(kSoft);
  }

  const bool captured = app_->captures_system_keys() && modal_ == nullptr;

  if (!captured && ev.key == Key::Power) {
    if (ev.down && !ev.repeat) set_screen(!screen_on_);
    return;
  }
  if (!screen_on_) return;

  if (!captured && ev.key == Key::Home) {
    if (ev.down) {
      close_modal();
      go_home();
    }
    return;
  }

  if (modal_ != nullptr) {
    ui::Modal* m = modal_;
    switch (m->on_key(ev)) {
      case ui::Modal::Result::Redraw:
        invalidate(m->bounds());
        break;
      case ui::Modal::Result::Close:
        // Pencere kapanırken yenisi açılmış olabilir (geri çağrı içinden).
        invalidate(m->bounds());
        if (modal_ == m) modal_ = nullptr;
        break;
      case ui::Modal::Result::Stay:
        break;
    }
    return;
  }

  const bool handled = app_->on_key(ev);
  if (!handled && ev.down && !ev.repeat && ev.key == Key::Back && app_ != home_) go_home();
}

uint32_t System::next_timeout(uint32_t now) const {
  uint32_t timeout = hal::kWaitForever;
  auto consider = [&](uint32_t deadline) {
    const uint32_t left = until(deadline, now);
    if (left < timeout) timeout = left;
  };
  if (repeat_key_ != Key::None) consider(repeat_at_);
  if (toast_active_) consider(toast_until_);
  const uint32_t interval = app_->tick_interval_ms();
  if (interval != 0) consider(last_tick_ + interval);
  return timeout;
}

void System::service_timers(uint32_t now) {
  if (repeat_key_ != Key::None && until(repeat_at_, now) == 0) {
    repeat_at_ = now + kRepeatRateMs;
    KeyEvent ev;
    ev.key = repeat_key_;
    ev.down = true;
    ev.time_ms = now;
    ev.repeat = true;
    dispatch(ev);
  }
  if (toast_active_ && until(toast_until_, now) == 0) {
    toast_active_ = false;
    invalidate(toast_rect_);
  }
  const uint32_t interval = app_->tick_interval_ms();
  if (interval != 0 && until(last_tick_ + interval, now) == 0) {
    last_tick_ = now;
    app_->on_tick(now);
  }
}

void System::redraw() {
  if (invalid_.empty() || canvas_ == nullptr) return;
  ui::Canvas& c = *canvas_;
  const Rect area = invalid_;
  invalid_ = Rect{};

  c.set_clip(area);
  if (!intersect(area, kStatus).empty()) draw_status(c);

  const Rect content = intersect(area, kContent);
  if (!content.empty()) {
    c.set_clip(content);
    app_->draw(c, kContent);
    c.set_clip(area);
  }

  if (!intersect(area, kSoft).empty()) draw_soft(c);
  if (modal_ != nullptr) modal_->draw(c);
  if (toast_active_) draw_toast(c);

  c.reset_clip();
  c.clear_dirty();
  hal::display_push(area);
}

void System::draw_status(ui::Canvas& c) const {
  const ui::Theme& t = ui::theme();
  c.fill_rect(kStatus, t.accent);
  c.text_in(make_rect(8, 0, 180, kStatusH), app_->title(), ui::font_sans12, t.on_accent,
            ui::Align::Left);

  // Pil: gövde + uç, içi doluluk kadar.
  const int percent = hal::battery_percent();
  const Rect body = make_rect(hal::kScreenWidth - 30, 5, 20, 10);
  c.box(body, 2, t.on_accent, t.accent);
  c.fill_rect(make_rect(body.right(), body.y + 3, 2, 4), t.on_accent);
  const int fill = (body.w - 4) * percent / 100;
  if (fill > 0) c.fill_rect(make_rect(body.x + 2, body.y + 2, fill, body.h - 4), t.on_accent);

  c.text_in(make_rect(hal::kScreenWidth - 80, 0, 42, kStatusH), angle_label(settings_.angle),
            ui::font_sans12, t.on_accent, ui::Align::Right);
}

void System::draw_soft(ui::Canvas& c) const {
  const ui::Theme& t = ui::theme();
  SoftKeys keys;
  app_->soft_keys(keys);

  c.fill_rect(kSoft, t.bg);
  c.fill_rect(make_rect(0, kSoft.y, kSoft.w, 1), t.line);
  for (int i = 0; i < 6; ++i) {
    const int x0 = i * kSoft.w / 6;
    const int x1 = (i + 1) * kSoft.w / 6;
    const Rect cell = make_rect(x0, kSoft.y + 1, x1 - x0, kSoft.h - 1);
    const bool pressed = down_[key_index(Key::F1) + i];
    if (pressed) c.fill_rect(cell, t.accent);
    if (i > 0) c.fill_rect(make_rect(x0, kSoft.y + 4, 1, 12), t.line);
    if (keys.label[i] != nullptr) {
      c.text_in(cell, keys.label[i], ui::font_sans12, pressed ? t.on_accent : t.text);
    }
  }
}

void System::draw_toast(ui::Canvas& c) const {
  const ui::Theme& t = ui::theme();
  // Ters renkli: zeminden ayrışsın.
  c.fill_round_rect(toast_rect_, 12, t.text);
  c.text_in(toast_rect_, toast_, ui::font_sans12, t.bg);
}

}  // namespace calctus
