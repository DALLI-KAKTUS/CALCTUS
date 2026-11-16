#include "apps/statistics/statistics.h"

#include <cstdio>
#include <cstring>

#include "kernel/system.h"
#include "ui/theme.h"

namespace calctus::apps {
namespace {

const char* key_text(Key key) {
  switch (key) {
    case Key::N0: return "0"; case Key::N1: return "1";
    case Key::N2: return "2"; case Key::N3: return "3";
    case Key::N4: return "4"; case Key::N5: return "5";
    case Key::N6: return "6"; case Key::N7: return "7";
    case Key::N8: return "8"; case Key::N9: return "9";
    case Key::Dot: return "."; case Key::EE: return "e";
    case Key::Plus: return "+"; case Key::Minus: return "-";
    case Key::Mul: return "*"; case Key::Div: return "/";
    case Key::LParen: return "("; case Key::RParen: return ")";
    case Key::Pow: return "^"; case Key::Sqrt: return "sqrt(";
    case Key::Pi: return "pi"; case Key::Ans: return "Ans";
    default: return nullptr;
  }
}

void row(ui::Canvas& c, int y, const char* label, const char* value,
         const ui::Theme& theme) {
  c.text(8, y, label, ui::font_sans12, theme.muted);
  char compact[128];
  if (ui::Canvas::text_width(value, ui::font_sans16) > 200 &&
      math::evaluate("Ans", value, NumberFormat::Scientific, compact, sizeof compact) ==
          math::Error::None) value = compact;
  const int width = ui::Canvas::text_width(value, ui::font_sans16);
  c.set_clip(make_rect(100, y, 210, 20));
  c.text(307 - width, y, value, ui::font_sans16, theme.text);
  c.set_clip(System::kContent);
}

}  // namespace

void StatisticsApp::reset_input() {
  input_[0] = '\0';
  length_ = 0;
  System::get().invalidate_content();
}

void StatisticsApp::append(const char* text) {
  const int size = static_cast<int>(std::strlen(text));
  if (length_ + size >= static_cast<int>(sizeof input_)) {
    System::get().toast("İfade çok uzun");
    return;
  }
  std::memcpy(input_ + length_, text, size + 1);
  length_ += size;
  System::get().invalidate_content();
}

math::Error StatisticsApp::update() {
  if (!count_) { summary_ = {}; return math::Error::None; }
  const char* items[32];
  for (int i = 0; i < count_; ++i) items[i] = values_[i];
  const math::Error error = math::summarize(items, count_, summary_);
  return error;
}

void StatisticsApp::add() {
  if (!length_) return;
  if (count_ == 32) { System::get().toast("En çok 32 değer"); return; }
  const Settings& settings = System::get().settings();
  const math::Error error = math::evaluate(input_, "0", NumberFormat::Auto,
                                            values_[count_], sizeof values_[0], settings.angle);
  if (error != math::Error::None) { System::get().toast(math::error_text(error)); return; }
  ++count_;
  const math::Error summary_error = update();
  if (summary_error != math::Error::None) {
    --count_;
    System::get().toast(math::error_text(summary_error));
    return;
  }
  page_ = Page::Data;
  reset_input();
}

bool StatisticsApp::on_key(const KeyEvent& ev) {
  if (!ev.down) return false;
  System& sys = System::get();
  switch (ev.key) {
    case Key::Exe: case Key::Ok: case Key::F1:
      if (!ev.repeat) add();
      return true;
    case Key::Backspace:
      if (length_) { input_[--length_] = '\0'; sys.invalidate_content(); }
      return true;
    case Key::F2:
      if (!ev.repeat && count_) { --count_; update(); page_ = Page::Data; sys.invalidate_content(); }
      return true;
    case Key::F3: page_ = Page::Mean; sys.invalidate_content(); return true;
    case Key::F4: page_ = Page::Spread; sys.invalidate_content(); return true;
    case Key::F5: page_ = Page::Range; sys.invalidate_content(); return true;
    case Key::F6:
      if (!ev.repeat) { count_ = 0; summary_ = {}; page_ = Page::Data; reset_input(); }
      return true;
    default: break;
  }
  if (!ev.repeat) {
    const char* text = key_text(ev.key);
    if (text) { append(text); return true; }
  }
  return false;
}

void StatisticsApp::draw(ui::Canvas& c, Rect area) {
  const ui::Theme& t = ui::theme();
  c.fill_rect(area, t.bg);
  char count_text[32];
  std::snprintf(count_text, sizeof count_text, "Veri sayısı: %d / 32", count_);
  c.text(8, 24, count_text, ui::font_sans12, t.muted);
  if (page_ == Page::Data) {
    const int first = count_ > 4 ? count_ - 4 : 0;
    for (int i = first; i < count_; ++i) {
      char label[16];
      std::snprintf(label, sizeof label, "x%d", i + 1);
      row(c, 49 + (i - first) * 27, label, values_[i], t);
    }
    if (!count_) c.text_in(make_rect(10, 55, 300, 60), "Sayı yaz, EXE ile ekle",
                           ui::font_sans12, t.muted);
  } else if (!count_) {
    c.text_in(make_rect(10, 55, 300, 60), "Önce veri ekle", ui::font_sans12, t.muted);
  } else if (page_ == Page::Mean) {
    row(c, 55, "Toplam", summary_.sum, t);
    row(c, 90, "Ortalama", summary_.mean, t);
  } else if (page_ == Page::Spread) {
    row(c, 55, "Varyans", summary_.variance, t);
    row(c, 90, "Std. sapma", summary_.deviation, t);
    c.text(8, 130, "Popülasyon: n böleni", ui::font_sans12, t.muted);
  } else {
    row(c, 55, "En küçük", summary_.minimum, t);
    row(c, 90, "En büyük", summary_.maximum, t);
  }
  c.fill_rect(make_rect(0, 155, 320, 1), t.line);
  c.text(8, 162, "YENİ DEĞER", ui::font_sans12, t.muted);
  c.fill_round_rect(make_rect(6, 180, 308, 34), 5, t.surface);
  const char* text = length_ ? input_ : "0";
  const int width = ui::Canvas::text_width(text, ui::font_sans16);
  c.set_clip(make_rect(10, 181, 300, 32));
  c.text(305 - width, 187, text, ui::font_sans16, length_ ? t.text : t.muted);
  c.set_clip(area);
}

void StatisticsApp::soft_keys(SoftKeys& keys) const {
  keys.label[0] = "Ekle"; keys.label[1] = "Geri";
  keys.label[2] = "Ort."; keys.label[3] = "Sapma";
  keys.label[4] = "Aralık"; keys.label[5] = "Temiz";
}

}  // namespace calctus::apps
