#include "apps/graph/graph.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "kernel/system.h"
#include "math/eval.h"

namespace calctus::apps {
namespace {

constexpr Rect kPlot{8, 37, 304, 135};
const char* const kPresets[] = {"x^2", "sin(30*x)", "1/x"};

const char* key_text(Key key) {
  switch (key) {
    case Key::Xnt: return "x";
    case Key::N0: return "0"; case Key::N1: return "1";
    case Key::N2: return "2"; case Key::N3: return "3";
    case Key::N4: return "4"; case Key::N5: return "5";
    case Key::N6: return "6"; case Key::N7: return "7";
    case Key::N8: return "8"; case Key::N9: return "9";
    case Key::Plus: return "+"; case Key::Minus: return "-";
    case Key::Mul: return "*"; case Key::Div: return "/";
    case Key::Dot: return "."; case Key::LParen: return "(";
    case Key::RParen: return ")"; case Key::Pow: return "^";
    case Key::Square: return "^2"; case Key::Sqrt: return "sqrt(";
    case Key::Sin: return "sin("; case Key::Cos: return "cos(";
    case Key::Tan: return "tan("; case Key::Ln: return "ln(";
    case Key::Log: return "log("; case Key::Exp: return "exp(";
    case Key::Pi: return "pi";
    default: return nullptr;
  }
}

}  // namespace

void GraphApp::on_open() { dirty_ = true; }

void GraphApp::changed() {
  dirty_ = true;
  System::get().invalidate_content();
}

void GraphApp::append(const char* text) {
  const int count = static_cast<int>(std::strlen(text));
  if (length_ + count >= static_cast<int>(sizeof expression_)) {
    System::get().toast("İfade çok uzun");
    return;
  }
  std::memcpy(expression_ + length_, text, count + 1);
  length_ += count;
  changed();
}

bool GraphApp::on_key(const KeyEvent& ev) {
  if (!ev.down) return false;
  switch (ev.key) {
    case Key::F1:
      if (!ev.repeat) { expression_[0] = '\0'; length_ = 0; changed(); }
      return true;
    case Key::F2: case Key::Exe: case Key::Ok:
      if (!ev.repeat) changed();
      return true;
    case Key::F3:
      if (!ev.repeat && pixels_per_unit_ < 160) { pixels_per_unit_ *= 2; changed(); }
      return true;
    case Key::F4:
      if (!ev.repeat && pixels_per_unit_ > 5) { pixels_per_unit_ /= 2; changed(); }
      return true;
    case Key::F5:
      if (!ev.repeat) { cx_ = cy_ = 0; pixels_per_unit_ = 20; changed(); }
      return true;
    case Key::F6:
      if (!ev.repeat) {
        preset_ = (preset_ + 1) % 3;
        std::strcpy(expression_, kPresets[preset_]);
        length_ = static_cast<int>(std::strlen(expression_));
        changed();
      }
      return true;
    case Key::Backspace:
      if (length_) { expression_[--length_] = '\0'; changed(); }
      return true;
    case Key::Left: cx_ -= 40 / pixels_per_unit_; changed(); return true;
    case Key::Right: cx_ += 40 / pixels_per_unit_; changed(); return true;
    case Key::Up: cy_ += 30 / pixels_per_unit_; changed(); return true;
    case Key::Down: cy_ -= 30 / pixels_per_unit_; changed(); return true;
    default: break;
  }
  if (!ev.repeat) {
    const char* text = key_text(ev.key);
    if (text) { append(text); return true; }
  }
  return false;
}

void GraphApp::sample() {
  dirty_ = false;
  valid_ = length_ > 0;
  const Settings& settings = System::get().settings();
  for (int i = 0; i < kSamples; ++i) {
    points_[i] = -1;
    if (!valid_) continue;
    const double x = cx_ + (i * 4 - kPlot.w / 2.0) / pixels_per_unit_;
    char x_text[32], result[128];
    std::snprintf(x_text, sizeof x_text, "%.12g", x);
    const math::Error error = math::evaluate(expression_, "0", NumberFormat::Auto,
                                               result, sizeof result, settings.angle, x_text);
    if (error != math::Error::None) {
      // Bir noktadaki tanımsızlık (1/x gibi) eğriyi keser; sözdizimi tüm grafiği keser.
      if (error == math::Error::Syntax || error == math::Error::Memory) valid_ = false;
      continue;
    }
    const double y = std::strtod(result, nullptr);
    const double py = kPlot.y + kPlot.h / 2.0 - (y - cy_) * pixels_per_unit_;
    if (std::isfinite(py) && py >= kPlot.y && py < kPlot.bottom())
      points_[i] = static_cast<int16_t>(py);
  }
}

void GraphApp::soft_keys(SoftKeys& keys) const {
  keys.label[0] = "Sil";
  keys.label[1] = "Çiz";
  keys.label[2] = "+";
  keys.label[3] = "-";
  keys.label[4] = "Sıfırla";
  keys.label[5] = "Örnek";
}

}  // namespace calctus::apps
