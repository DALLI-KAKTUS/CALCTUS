#include "apps/sequences/sequences.h"

#include <cstdio>
#include <cstdlib>
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
    case Key::Xnt: return "x"; case Key::Ans: return "Ans";
    case Key::Plus: return "+"; case Key::Minus: return "-";
    case Key::Mul: return "*"; case Key::Div: return "/";
    case Key::Dot: return "."; case Key::EE: return "e";
    case Key::LParen: return "("; case Key::RParen: return ")";
    case Key::Pow: return "^"; case Key::Square: return "^2";
    case Key::Sqrt: return "sqrt("; case Key::Pi: return "pi";
    case Key::Sin: return "sin("; case Key::Cos: return "cos(";
    case Key::Tan: return "tan("; case Key::Ln: return "ln(";
    case Key::Log: return "log("; case Key::Exp: return "exp(";
    default: return nullptr;
  }
}

}  // namespace

char* SequencesApp::active() {
  return selected_ == 0 ? formula_ : selected_ == 1 ? initial_ : first_;
}

void SequencesApp::select(int field) {
  selected_ = field;
  editing_ = false;
  System::get().invalidate_content();
}

void SequencesApp::append(const char* text) {
  if (selected_ == 2) {
    for (const char* p = text; *p; ++p)
      if (*p < '0' || *p > '9') return;
  }
  char* value = active();
  const size_t capacity = selected_ == 2 ? sizeof first_ : sizeof formula_;
  size_t length = std::strlen(value);
  const size_t size = std::strlen(text);
  if (!editing_) length = 0;
  if (length + size >= capacity) { System::get().toast("İfade çok uzun"); return; }
  if (!editing_) { value[0] = '\0'; editing_ = true; }
  std::memcpy(value + length, text, size + 1);
  has_rows_ = false;
  System::get().invalidate_content();
}

void SequencesApp::calculate() {
  if (!*first_) { System::get().toast("İlk n gerekli"); return; }
  const unsigned first = static_cast<unsigned>(std::strtoul(first_, nullptr, 10));
  char fresh[5][128];
  const math::Error error = math::generate_sequence(mode_, formula_, initial_,
                         first, 5, fresh, System::get().settings().angle);
  if (error != math::Error::None) {
    System::get().toast(math::error_text(error));
    has_rows_ = false;
  } else {
    std::memcpy(rows_, fresh, sizeof rows_);
    has_rows_ = true;
  }
  editing_ = false;
  System::get().invalidate_content();
}

void SequencesApp::on_open() { calculate(); }

bool SequencesApp::on_key(const KeyEvent& ev) {
  if (!ev.down) return false;
  switch (ev.key) {
    case Key::F1: case Key::F2:
      if (!ev.repeat) {
        mode_ = ev.key == Key::F1 ? math::SequenceMode::Explicit :
                                    math::SequenceMode::Recurrence;
        calculate();
      }
      return true;
    case Key::F3: if (!ev.repeat) select(0); return true;
    case Key::F4: if (!ev.repeat) select(1); return true;
    case Key::F5: if (!ev.repeat) select(2); return true;
    case Key::Up: if (!ev.repeat) select(selected_ == 0 ? 2 : selected_ - 1); return true;
    case Key::Down: if (!ev.repeat) select((selected_ + 1) % 3); return true;
    case Key::F6: case Key::Exe: case Key::Ok:
      if (!ev.repeat) calculate();
      return true;
    case Key::Backspace:
      if (size_t n = std::strlen(active())) {
        active()[n - 1] = '\0'; editing_ = true;
        has_rows_ = false;
        System::get().invalidate_content();
      }
      return true;
    default: break;
  }
  if (!ev.repeat) {
    const char* text = key_text(ev.key);
    if (text) { append(text); return true; }
  }
  return false;
}

void SequencesApp::soft_keys(SoftKeys& keys) const {
  keys.label[0] = "Açık"; keys.label[1] = "Yinele";
  keys.label[2] = "f(n)"; keys.label[3] = "u(0)";
  keys.label[4] = "İlk n"; keys.label[5] = "Hesap";
}

}  // namespace calctus::apps
