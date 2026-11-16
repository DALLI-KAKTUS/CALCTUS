#include "apps/probability/probability.h"

#include <cstdlib>
#include <cstring>

#include "kernel/system.h"
#include "ui/theme.h"

namespace calctus::apps {
namespace {

const char* mode_name(math::Combinatoric mode) {
  switch (mode) {
    case math::Combinatoric::Factorial: return "Faktöriyel: n!";
    case math::Combinatoric::Permutation: return "Permütasyon: nPr";
    case math::Combinatoric::Combination: return "Kombinasyon: nCr";
  }
  return "";
}

char digit_for(Key key) {
  switch (key) {
    case Key::N0: return '0'; case Key::N1: return '1';
    case Key::N2: return '2'; case Key::N3: return '3';
    case Key::N4: return '4'; case Key::N5: return '5';
    case Key::N6: return '6'; case Key::N7: return '7';
    case Key::N8: return '8'; case Key::N9: return '9';
    default: return 0;
  }
}

}  // namespace

void ProbabilityApp::select(bool k) {
  selected_k_ = k;
  editing_ = false;
  System::get().invalidate_content();
}

void ProbabilityApp::digit(char value) {
  char* field = selected_k_ ? k_ : n_;
  if (!editing_) { field[0] = value; field[1] = '\0'; editing_ = true; }
  else {
    const size_t length = std::strlen(field);
    if (length == 2) { System::get().toast("En çok iki basamak"); return; }
    field[length] = value;
    field[length + 1] = '\0';
  }
  result_[0] = '\0';
  System::get().invalidate_content();
}

void ProbabilityApp::erase() {
  char* field = selected_k_ ? k_ : n_;
  const size_t length = std::strlen(field);
  if (length) field[length - 1] = '\0';
  editing_ = true;
  result_[0] = '\0';
  System::get().invalidate_content();
}

void ProbabilityApp::calculate() {
  if (!*n_ || (operation_ != math::Combinatoric::Factorial && !*k_)) {
    System::get().toast("n ve k gerekli");
    return;
  }
  const unsigned n = static_cast<unsigned>(std::strtoul(n_, nullptr, 10));
  const unsigned k = static_cast<unsigned>(std::strtoul(k_, nullptr, 10));
  char answer[128];
  const math::Error error = math::combinatoric(operation_, n, k, answer, sizeof answer);
  if (error != math::Error::None) {
    result_[0] = '\0';
    System::get().toast(error == math::Error::TooLarge ? "n en çok 60 olabilir" :
                         error == math::Error::Domain ? "k, n'den büyük olamaz" :
                         math::error_text(error));
  } else std::strcpy(result_, answer);
  editing_ = false;
  System::get().invalidate_content();
}

bool ProbabilityApp::on_key(const KeyEvent& event) {
  if (!event.down) return false;
  switch (event.key) {
    case Key::F1: case Key::F2: case Key::F3:
      if (!event.repeat) {
        operation_ = event.key == Key::F1 ? math::Combinatoric::Factorial :
                     event.key == Key::F2 ? math::Combinatoric::Permutation :
                                             math::Combinatoric::Combination;
        calculate();
      }
      return true;
    case Key::F4: case Key::Up: if (!event.repeat) select(false); return true;
    case Key::F5: case Key::Down: if (!event.repeat) select(true); return true;
    case Key::F6:
      if (!event.repeat) {
        std::strcpy(n_, "5"); std::strcpy(k_, "2");
        operation_ = math::Combinatoric::Combination;
        selected_k_ = editing_ = false;
        result_[0] = '\0';
        System::get().invalidate_content();
      }
      return true;
    case Key::Exe: case Key::Ok: if (!event.repeat) calculate(); return true;
    case Key::Backspace: erase(); return true;
    default: break;
  }
  const char number = digit_for(event.key);
  if (!event.repeat && number) {
    digit(number);
    return true;
  }
  return false;
}

void ProbabilityApp::draw(ui::Canvas& c, Rect area) {
  const ui::Theme& t = ui::theme();
  c.fill_rect(area, t.bg);
  c.text(8, 25, mode_name(operation_), ui::font_sans16, t.text);
  c.text(8, 57, "n =", ui::font_sans16, t.text);
  c.text(8, 89, "k =", ui::font_sans16, t.text);
  const Rect n_box = make_rect(50, 53, 260, 27);
  const Rect k_box = make_rect(50, 85, 260, 27);
  c.box(n_box, 4, selected_k_ ? t.line : t.accent, selected_k_ ? t.surface : t.select);
  c.box(k_box, 4, selected_k_ ? t.accent : t.line, selected_k_ ? t.select : t.surface);
  c.text(58, 57, *n_ ? n_ : "_", ui::font_sans16, t.text);
  c.text(58, 89, *k_ ? k_ : "_", ui::font_sans16, t.text);
  c.fill_rect(make_rect(0, 120, 320, 1), t.line);
  c.text(8, 126, "SONUÇ (TAM SAYI)", ui::font_sans12, t.muted);
  if (!*result_) {
    c.text(8, 148, "EXE ile hesapla", ui::font_sans12, t.muted);
    return;
  }
  const size_t length = std::strlen(result_);
  for (size_t start = 0; start < length; start += 32) {
    char line[33];
    const size_t count = length - start < 32 ? length - start : 32;
    std::memcpy(line, result_ + start, count);
    line[count] = '\0';
    c.text(8, 148 + static_cast<int>(start / 32) * 19, line, ui::font_sans12, t.text);
  }
}

void ProbabilityApp::soft_keys(SoftKeys& keys) const {
  keys.label[0] = "n!"; keys.label[1] = "nPr";
  keys.label[2] = "nCr"; keys.label[3] = "n";
  keys.label[4] = "k"; keys.label[5] = "Sıfırla";
}

}  // namespace calctus::apps
