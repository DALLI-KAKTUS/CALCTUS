#include "apps/calc/calc.h"

#include <cstring>
#include <cstdio>

#include "kernel/system.h"
#include "math/eval.h"
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
    case Key::Plus: return "+"; case Key::Minus: return "-";
    case Key::Mul: return "*"; case Key::Div: return "/";
    case Key::Dot: return "."; case Key::LParen: return "(";
    case Key::RParen: return ")"; case Key::Pow: return "^";
    case Key::Ans: return "Ans"; case Key::Sqrt: return "sqrt(";
    case Key::Square: return "^2"; case Key::EE: return "e";
    case Key::Sin: return "sin("; case Key::Cos: return "cos(";
    case Key::Tan: return "tan("; case Key::Pi: return "pi";
    case Key::Ln: return "ln("; case Key::Log: return "log(";
    case Key::Exp: return "exp("; case Key::Xnt: return "x";
    default: return nullptr;
  }
}

void right_text(ui::Canvas& c, Rect rect, const char* text, const ui::Font& font, Color color) {
  const int width = ui::Canvas::text_width(text, font);
  c.set_clip(rect);
  c.text(rect.right() - 5 - width, rect.y + (rect.h - font.line_height) / 2, text, font, color);
  c.set_clip(System::kContent);
}

}  // namespace

void CalcApp::insert(const char* text) {
  const int size = static_cast<int>(std::strlen(text));
  if (length_ + size >= static_cast<int>(sizeof input_)) {
    System::get().toast("İfade çok uzun");
    return;
  }
  std::memcpy(input_ + length_, text, size + 1);
  length_ += size;
  System::get().invalidate_content();
}

void CalcApp::clear() {
  length_ = 0;
  input_[0] = '\0';
  System::get().invalidate_content();
}

void CalcApp::toggle_sign() {
  if (!length_) { insert("-"); return; }
  if (length_ == 1 && input_[0] == '-') { clear(); return; }
  if (length_ && input_[0] == '-' && input_[1] == '(' && input_[length_ - 1] == ')') {
    std::memmove(input_, input_ + 2, length_ - 2);
    length_ -= 3;
    input_[length_] = '\0';
  } else if (length_ + 3 < static_cast<int>(sizeof input_)) {
    std::memmove(input_ + 2, input_, length_);
    input_[0] = '-'; input_[1] = '(';
    input_[length_ + 2] = ')';
    length_ += 3;
    input_[length_] = '\0';
  }
  System::get().invalidate_content();
}

void CalcApp::execute() {
  if (!length_) return;
  char result[128];
  const Settings& settings = System::get().settings();
  const math::Error error = math::evaluate(input_, ans_, settings.format,
                                            result, sizeof result, settings.angle);
  if (error != math::Error::None) {
    System::get().toast(math::error_text(error));
    return;
  }
  for (int i = 2; i > 0; --i) history_[i] = history_[i - 1];
  std::memcpy(history_[0].expression, input_, length_ + 1);
  std::strcpy(history_[0].result, result);
  if (history_count_ < 3) ++history_count_;
  std::strcpy(ans_, result);
  clear();
}

void CalcApp::stack_push(const char* value) {
  if (stack_size_ == 4) {
    for (int i = 1; i < 4; ++i) std::strcpy(stack_[i - 1], stack_[i]);
    --stack_size_;
  }
  std::strcpy(stack_[stack_size_++], value);
  System::get().invalidate_content();
}

bool CalcApp::enter() {
  if (!length_) {
    if (stack_size_) {
      char copy[128];
      std::strcpy(copy, stack_[stack_size_ - 1]);
      stack_push(copy);
    }
    return stack_size_ > 0;
  }
  char value[128];
  const Settings& settings = System::get().settings();
  const math::Error error = math::evaluate(input_, ans_, settings.format,
                                            value, sizeof value, settings.angle);
  if (error != math::Error::None) {
    System::get().toast(math::error_text(error));
    return false;
  }
  stack_push(value);
  clear();
  return true;
}

void CalcApp::operate(char op) {
  if ((length_ && stack_size_ == 0) || (!length_ && stack_size_ < 2)) {
    System::get().toast("Yığında iki sayı gerekli");
    return;
  }
  char rhs[128];
  if (length_) {
    const Settings& s = System::get().settings();
    const math::Error e = math::evaluate(input_, ans_, s.format, rhs, sizeof rhs, s.angle);
    if (e != math::Error::None) { System::get().toast(math::error_text(e)); return; }
  } else {
    std::strcpy(rhs, stack_[stack_size_ - 1]);
  }
  const char* lhs = stack_[stack_size_ - (length_ ? 1 : 2)];
  char expression[] = "x+Ans";
  expression[1] = op;
  char result[128];
  const Settings& s = System::get().settings();
  const math::Error e = math::evaluate(expression, rhs, s.format,
                                        result, sizeof result, s.angle, lhs);
  if (e != math::Error::None) { System::get().toast(math::error_text(e)); return; }
  std::strcpy(last_x_, rhs);
  stack_size_ -= length_ ? 1 : 2;
  stack_push(result);
  clear();
}

void CalcApp::unary_op(const char* function) {
  if (!length_ && !stack_size_) { System::get().toast("Yığın boş"); return; }
  char value[128];
  if (length_) {
    const Settings& s = System::get().settings();
    const math::Error e = math::evaluate(input_, ans_, s.format, value, sizeof value, s.angle);
    if (e != math::Error::None) { System::get().toast(math::error_text(e)); return; }
  } else std::strcpy(value, stack_[stack_size_ - 1]);
  char expr[24];
  std::snprintf(expr, sizeof expr, "%s(Ans)", function);
  char result[128];
  const Settings& s = System::get().settings();
  const math::Error e = math::evaluate(expr, value, s.format, result, sizeof result, s.angle);
  if (e != math::Error::None) { System::get().toast(math::error_text(e)); return; }
  std::strcpy(last_x_, value);
  if (length_) clear();
  else --stack_size_;
  stack_push(result);
}

bool CalcApp::on_key(const KeyEvent& ev) {
  if (!ev.down) return false;
  System& sys = System::get();
  if (ev.key == Key::Mod) {
    if (!ev.repeat) { rpn_ = !rpn_; clear(); sys.invalidate_all(); }
    return true;
  }
  if (rpn_) {
    if (ev.key == Key::Exe || ev.key == Key::Ok) {
      if (!ev.repeat) enter();
      return true;
    }
    if (ev.key == Key::Plus || ev.key == Key::Minus ||
        ev.key == Key::Mul || ev.key == Key::Div || ev.key == Key::Pow) {
      if (!ev.repeat) operate(ev.key == Key::Plus ? '+' : ev.key == Key::Minus ? '-' :
                             ev.key == Key::Mul ? '*' : ev.key == Key::Div ? '/' : '^');
      return true;
    }
    if (ev.key == Key::Sqrt || ev.key == Key::Sin || ev.key == Key::Cos ||
        ev.key == Key::Tan || ev.key == Key::Ln || ev.key == Key::Log || ev.key == Key::Exp) {
      if (!ev.repeat) unary_op(ev.key == Key::Sqrt ? "sqrt" : ev.key == Key::Sin ? "sin" :
                              ev.key == Key::Cos ? "cos" : ev.key == Key::Tan ? "tan" :
                              ev.key == Key::Ln ? "ln" : ev.key == Key::Log ? "log" : "exp");
      return true;
    }
    switch (ev.key) {
      case Key::F1:
        if (!ev.repeat) { if (length_) clear(); else if (stack_size_) { --stack_size_; sys.invalidate_content(); } }
        return true;
      case Key::F2:
        if (!ev.repeat) enter();
        return true;
      case Key::F4: case Key::LParen:
        if (!ev.repeat && stack_size_ >= 2) {
          char tmp[128]; std::strcpy(tmp, stack_[stack_size_ - 1]);
          std::strcpy(stack_[stack_size_ - 1], stack_[stack_size_ - 2]);
          std::strcpy(stack_[stack_size_ - 2], tmp);
          sys.invalidate_content();
        }
        return true;
      case Key::F5: case Key::RParen:
        if (!ev.repeat && stack_size_ >= 3) {
          char tmp[128]; std::strcpy(tmp, stack_[stack_size_ - 1]);
          std::strcpy(stack_[stack_size_ - 1], stack_[stack_size_ - 2]);
          std::strcpy(stack_[stack_size_ - 2], stack_[stack_size_ - 3]);
          std::strcpy(stack_[stack_size_ - 3], tmp);
          sys.invalidate_content();
        }
        return true;
      case Key::F6:
        if (!ev.repeat) { stack_size_ = 0; clear(); }
        return true;
      case Key::Ans:
        if (!ev.repeat) stack_push(last_x_);
        return true;
      default: break;
    }
  }
  switch (ev.key) {
    case Key::Exe: case Key::Ok:
      if (!ev.repeat) execute();
      return true;
    case Key::Backspace:
      if (length_) { input_[--length_] = '\0'; sys.invalidate_content(); }
      return true;
    case Key::F1: if (!ev.repeat) clear(); return true;
    case Key::F2: if (!ev.repeat) insert("Ans"); return true;
    case Key::F3: case Key::Shift: if (!ev.repeat) toggle_sign(); return true;
    case Key::F4: if (!ev.repeat) insert("sqrt("); return true;
    case Key::F5: if (!ev.repeat) insert("^"); return true;
    case Key::F6:
      if (!ev.repeat) { history_count_ = 0; sys.invalidate_content(); }
      return true;
    default: break;
  }
  if (!ev.repeat) {
    const char* text = key_text(ev.key);
    if (text) { insert(text); return true; }
  }
  return false;
}

void CalcApp::draw(ui::Canvas& c, Rect area) {
  const ui::Theme& t = ui::theme();
  c.fill_rect(area, t.bg);
  if (rpn_) {
    c.text(8, 24, "YIĞIN", ui::font_sans12, t.muted);
    for (int i = 0; i < stack_size_; ++i) {
      const int y = 43 + (3 - i) * 27;
      char label[16]; std::snprintf(label, sizeof label, "%d:", i + 1);
      c.text(8, y, label, ui::font_sans12, t.muted);
      right_text(c, make_rect(34, y, 278, 20), stack_[i], ui::font_sans16, t.text);
    }
    c.fill_rect(make_rect(0, 155, 320, 1), t.line);
    c.text(8, 162, "GİRİŞ", ui::font_sans12, t.muted);
    c.fill_round_rect(make_rect(6, 180, 308, 34), 5, t.surface);
    right_text(c, make_rect(10, 181, 300, 32), length_ ? input_ : "0",
               ui::font_sans16, length_ ? t.text : t.muted);
    return;
  }
  c.text(8, 24, "GEÇMİŞ", ui::font_sans12, t.muted);
  for (int i = history_count_ - 1; i >= 0; --i) {
    const int y = 43 + (history_count_ - 1 - i) * 35;
    right_text(c, make_rect(8, y, 304, 15), history_[i].expression, ui::font_sans12, t.muted);
    right_text(c, make_rect(8, y + 15, 304, 19), history_[i].result, ui::font_sans16, t.text);
  }
  c.fill_rect(make_rect(0, 155, 320, 1), t.line);
  c.text(8, 162, "İFADE", ui::font_sans12, t.muted);
  c.fill_round_rect(make_rect(6, 180, 308, 34), 5, t.surface);
  right_text(c, make_rect(10, 181, 300, 32), length_ ? input_ : "0",
             ui::font_sans16, length_ ? t.text : t.muted);
}

void CalcApp::soft_keys(SoftKeys& keys) const {
  if (rpn_) {
    keys.label[0] = "DROP"; keys.label[1] = "ENTER";
    keys.label[2] = "+/-"; keys.label[3] = "SWAP";
    keys.label[4] = "ROLL"; keys.label[5] = "CLR";
    return;
  }
  keys.label[0] = "Sil";
  keys.label[1] = "Ans";
  keys.label[2] = "+/-";
  keys.label[3] = "Kök";
  keys.label[4] = "Üs";
  keys.label[5] = "Geçmiş";
}

}  // namespace calctus::apps
