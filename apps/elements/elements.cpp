#include "apps/elements/elements.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "apps/elements/data.h"
#include "kernel/system.h"
#include "ui/theme.h"

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

const char* phase_name(char phase) {
  switch (phase) {
    case 'G': return "Gaz";
    case 'L': return "Sıvı";
    case 'S': return "Katı";
    default: return "Bilinmiyor";
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

void ElementsApp::draw(ui::Canvas& c, Rect area) {
  const ui::Theme& t = ui::theme();
  const Element& element = *element_by_number(selected_);
  c.fill_rect(area, t.bg);
  c.text(8, 24, list_ ? "PERİYODİK LİSTE" : "PERİYODİK TABLO", ui::font_sans12, t.muted);
  char search_line[32];
  if (*search_) std::snprintf(search_line, sizeof search_line, "Git: %s", search_);
  else std::snprintf(search_line, sizeof search_line, "No: %d", selected_);
  c.text_in(make_rect(220, 20, 92, 20), search_line, ui::font_sans12, t.muted,
            ui::Align::Right);

  if (list_) {
    const int first = selected_ <= 4 ? 1 : selected_ >= 115 ? 111 : selected_ - 3;
    for (int number = first; number < first + 8; ++number) {
      const Element& e = *element_by_number(number);
      const int y = 42 + (number - first) * 22;
      if (number == selected_) c.fill_round_rect(make_rect(5, y, 310, 21), 4, t.select);
      char label[48];
      std::snprintf(label, sizeof label, "%3d  %-3s %s", number, e.symbol, e.name);
      c.text(12, y + 3, label, ui::font_sans12, number == selected_ ? t.accent : t.text);
    }
    return;
  }

  c.fill_round_rect(make_rect(8, 44, 76, 74), 6, t.accent);
  std::snprintf(search_line, sizeof search_line, "%d", selected_);
  c.text(15, 48, search_line, ui::font_sans12, t.on_accent);
  c.text_in(make_rect(10, 61, 72, 49), element.symbol, ui::font_sans24, t.on_accent);
  c.text(96, 61, element.name, ui::font_sans16, t.text);
  char info[64];
  std::snprintf(info, sizeof info, "%.6g u (yaklaşık)", element.approximate_mass);
  c.text(96, 91, info, ui::font_sans12, t.muted);

  c.fill_rect(make_rect(8, 128, 304, 1), t.line);
  std::snprintf(info, sizeof info, "Periyot: %u", element.period);
  c.text(12, 139, info, ui::font_sans16, t.text);
  if (element.group) std::snprintf(info, sizeof info, "Grup: %u", element.group);
  else std::snprintf(info, sizeof info, "Grup: f-blok");
  c.text(160, 139, info, ui::font_sans16, t.text);
  std::snprintf(info, sizeof info, "Blok: %c", element.block);
  c.text(12, 172, info, ui::font_sans16, t.text);
  std::snprintf(info, sizeof info, "Faz: %s%s", phase_name(element.phase),
                selected_ >= 104 ? "*" : "");
  c.text(160, 172, info, ui::font_sans16, t.text);
  if (selected_ >= 104)
    c.text(12, 201, "* Faz tahmini olabilir", ui::font_sans12, t.muted);
}

void ElementsApp::soft_keys(SoftKeys& keys) const {
  keys.label[0] = "Git"; keys.label[1] = "-10"; keys.label[2] = "+10";
  keys.label[3] = "Önce"; keys.label[4] = "Sonra";
  keys.label[5] = list_ ? "Detay" : "Liste";
}

}  // namespace calctus::apps
