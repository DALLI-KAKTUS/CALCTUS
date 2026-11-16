#include "apps/statistics/statistics.h"

#include <cstdio>
#include <cstring>

#include "kernel/system.h"

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

void StatisticsApp::soft_keys(SoftKeys& keys) const {
  keys.label[0] = "Ekle"; keys.label[1] = "Geri";
  keys.label[2] = "Ort."; keys.label[3] = "Sapma";
  keys.label[4] = "Aralık"; keys.label[5] = "Temiz";
}

}  // namespace calctus::apps
