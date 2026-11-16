#include "apps/elements/elements.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "apps/elements/data.h"
#include "kernel/system.h"

namespace calctus::apps {
namespace {

char key_digit(Key key) {
  switch (key) {
    case Key::N0: return '0'; case Key::N1: return '1';
    case Key::N2: return '2'; case Key::N3: return '3';
    case Key::N4: return '4'; case Key::N5: return '5';
    case Key::N6: return '6'; case Key::N7: return '7';
    case Key::N8: return '8'; case Key::N9: return '9';
    default: return 0;
  }
}

int clamp_number(int number) {
  return number < 1 ? 1 : number > kElementCount ? kElementCount : number;
}

}  // namespace

void ElementsApp::move(int step) {
  selected_ = clamp_number(selected_ + step);
  search_[0] = '\0';
  System::get().invalidate_content();
}

void ElementsApp::jump() {
  if (!*search_) return;
  const int number = std::atoi(search_);
  if (!element_by_number(number)) {
    System::get().toast("Atom no: 1–118");
    return;
  }
  selected_ = number;
  search_[0] = '\0';
  System::get().invalidate_content();
}

bool ElementsApp::on_key(const KeyEvent& event) {
  if (!event.down) return false;
  if (const char number = key_digit(event.key)) {
    if (!event.repeat) {
      const size_t count = std::strlen(search_);
      if (count < 3) {
        search_[count] = number;
        search_[count + 1] = '\0';
        System::get().invalidate_content();
      } else System::get().toast("En çok üç basamak");
    }
    return true;
  }
  switch (event.key) {
    case Key::Backspace:
      if (size_t count = std::strlen(search_)) {
        search_[count - 1] = '\0';
        System::get().invalidate_content();
      }
      return true;
    case Key::Exe: case Key::Ok: case Key::F1:
      if (!event.repeat) jump();
      return true;
    case Key::F2: if (!event.repeat) move(-10); return true;
    case Key::F3: if (!event.repeat) move(10); return true;
    case Key::F4: case Key::Up: move(-1); return true;
    case Key::F5: case Key::Down: move(1); return true;
    case Key::Left: move(-10); return true;
    case Key::Right: move(10); return true;
    case Key::F6:
      if (!event.repeat) { list_ = !list_; System::get().invalidate_content(); }
      return true;
    default: return false;
  }
}

void ElementsApp::soft_keys(SoftKeys& keys) const {
  keys.label[0] = "Git"; keys.label[1] = "-10"; keys.label[2] = "+10";
  keys.label[3] = "Önce"; keys.label[4] = "Sonra";
  keys.label[5] = list_ ? "Detay" : "Liste";
}

}  // namespace calctus::apps
