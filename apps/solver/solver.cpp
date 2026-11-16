#include "apps/solver/solver.h"

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
    case Key::Pi: return "pi"; default: return nullptr;
  }
}

void result_row(ui::Canvas& c, int y, const char* label, const char* text,
                const ui::Theme& t) {
  c.text(8, y, label, ui::font_sans12, t.muted);
  char compact[128];
  if (ui::Canvas::text_width(text, ui::font_sans16) > 245 &&
      math::evaluate("Ans", text, NumberFormat::Scientific, compact, sizeof compact) ==
          math::Error::None) text = compact;
  const int width = ui::Canvas::text_width(text, ui::font_sans16);
  c.set_clip(make_rect(58, y, 254, 20));
  c.text(307 - width, y, text, ui::font_sans16, t.text);
  c.set_clip(System::kContent);
}

}  // namespace

void SolverApp::select(int index) {
  selected_ = index;
  editing_ = false;
  System::get().invalidate_content();
}

void SolverApp::append(const char* text) {
  char* field = active();
  size_t length = std::strlen(field);
  if (!editing_) { length = 0; field[0] = '\0'; editing_ = true; }
  const size_t count = std::strlen(text);
  if (length + count >= sizeof coefficient_[0]) {
    System::get().toast("İfade çok uzun");
    return;
  }
  std::memcpy(field + length, text, count + 1);
  has_result_ = false;
  System::get().invalidate_content();
}

void SolverApp::calculate() {
  char values[3][128];
  const Settings& settings = System::get().settings();
  for (int i = 0; i < 3; ++i) {
    math::Error error = math::evaluate(coefficient_[i], "0", NumberFormat::Auto,
                                         values[i], sizeof values[i], settings.angle);
    if (error != math::Error::None) {
      System::get().toast(math::error_text(error));
      has_result_ = false;
      return;
    }
  }
  const math::Error error = math::solve_quadratic(values[0], values[1], values[2], result_);
  if (error != math::Error::None) System::get().toast(math::error_text(error));
  has_result_ = error == math::Error::None;
  editing_ = false;
  System::get().invalidate_content();
}

bool SolverApp::on_key(const KeyEvent& ev) {
  if (!ev.down) return false;
  switch (ev.key) {
    case Key::F1: if (!ev.repeat) select(0); return true;
    case Key::F2: if (!ev.repeat) select(1); return true;
    case Key::F3: if (!ev.repeat) select(2); return true;
    case Key::Up: if (!ev.repeat) select(selected_ == 0 ? 2 : selected_ - 1); return true;
    case Key::Down: if (!ev.repeat) select((selected_ + 1) % 3); return true;
    case Key::F4: case Key::Exe: case Key::Ok:
      if (!ev.repeat) calculate();
      return true;
    case Key::F5:
      if (!ev.repeat) { std::strcpy(active(), "0"); editing_ = false;
                        has_result_ = false; System::get().invalidate_content(); }
      return true;
    case Key::F6:
      if (!ev.repeat) {
        std::strcpy(coefficient_[0], "1"); std::strcpy(coefficient_[1], "0");
        std::strcpy(coefficient_[2], "-1");
        selected_ = 0; editing_ = has_result_ = false;
        System::get().invalidate_content();
      }
      return true;
    case Key::Backspace:
      if (size_t n = std::strlen(active())) {
        active()[n - 1] = '\0'; editing_ = true; has_result_ = false;
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

void SolverApp::draw(ui::Canvas& c, Rect area) {
  const ui::Theme& t = ui::theme();
  c.fill_rect(area, t.bg);
  c.text(8, 24, "ax² + bx + c = 0", ui::font_sans16, t.text);
  const char* labels[] = {"a", "b", "c"};
  for (int i = 0; i < 3; ++i) {
    const int y = 49 + i * 29;
    c.text(10, y + 4, labels[i], ui::font_sans16, t.muted);
    c.box(make_rect(38, y, 273, 26), 4, selected_ == i ? t.accent : t.line,
          selected_ == i ? t.select : t.surface);
    c.set_clip(make_rect(44, y + 1, 261, 24));
    const char* text = *coefficient_[i] ? coefficient_[i] : "_";
    const int width = ui::Canvas::text_width(text, ui::font_sans16);
    c.text(303 - width, y + 5, text, ui::font_sans16, t.text);
    c.set_clip(area);
  }
  c.fill_rect(make_rect(0, 140, 320, 1), t.line);
  if (!has_result_) {
    c.text(8, 151, "EXE ile çöz", ui::font_sans12, t.muted);
    return;
  }
  switch (result_.kind) {
    case math::Roots::None:
      c.text(8, 153, "Çözüm yok", ui::font_sans16, t.text); break;
    case math::Roots::Any:
      c.text(8, 153, "Her x çözüm", ui::font_sans16, t.text); break;
    case math::Roots::Linear: case math::Roots::Double:
      result_row(c, 154, "x =", result_.first, t);
      if (result_.kind == math::Roots::Double)
        c.text(8, 184, "Çift katlı kök", ui::font_sans12, t.muted);
      break;
    case math::Roots::Two:
      result_row(c, 151, "x1 =", result_.first, t);
      result_row(c, 183, "x2 =", result_.second, t);
      break;
    case math::Roots::Complex:
      result_row(c, 151, "Re =", result_.first, t);
      result_row(c, 183, "±Im =", result_.second, t);
      break;
  }
}

void SolverApp::soft_keys(SoftKeys& keys) const {
  keys.label[0] = "a"; keys.label[1] = "b"; keys.label[2] = "c";
  keys.label[3] = "Çöz"; keys.label[4] = "0 yap"; keys.label[5] = "Sıfırla";
}

}  // namespace calctus::apps
