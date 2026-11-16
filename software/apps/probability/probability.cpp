#include "apps/probability/probability.h"

#include <cstdlib>
#include <cstring>

#include "kernel/system.h"

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

const char* ProbabilityApp::mode_title() const { return mode_name(operation_); }

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

void ProbabilityApp::soft_keys(SoftKeys& keys) const {
  keys.label[0] = "n!"; keys.label[1] = "nPr";
  keys.label[2] = "nCr"; keys.label[3] = "n";
  keys.label[4] = "k"; keys.label[5] = "Sıfırla";
}

}  // namespace calctus::apps
