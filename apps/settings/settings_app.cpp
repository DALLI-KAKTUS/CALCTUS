#include "apps/settings/settings_app.h"

#include <cstdio>

#include "kernel/system.h"
#include "ui/theme.h"

namespace calctus::apps {

namespace {

const char* const kThemes[] = {"Açık", "Koyu"};
const char* const kAngles[] = {"Derece", "Radyan", "Grad"};
const char* const kFormats[] = {"Otomatik", "Ondalık", "Bilimsel"};

}  // namespace

void SettingsApp::on_open() {
  page_ = Page::Main;
  list_.setup(this, System::kContent);
  list_.select(0);
}

void SettingsApp::item(int index, ui::ListItem& out) const {
  const Settings& s = System::get().settings();
  switch (index) {
    case kTheme:
      out.label = "Tema";
      out.value = kThemes[s.dark ? 1 : 0];
      break;
    case kBrightness:
      std::snprintf(brightness_text_, sizeof brightness_text_, "%d%%", s.brightness);
      out.label = "Parlaklık";
      out.value = brightness_text_;
      break;
    case kAngle:
      out.label = "Açı birimi";
      out.value = kAngles[static_cast<int>(s.angle)];
      break;
    case kFormat:
      out.label = "Sayı biçimi";
      out.value = kFormats[static_cast<int>(s.format)];
      break;
    case kAbout:
      out.label = "Hakkında";
      out.mark = ui::ListItem::Mark::Chevron;
      break;
    default:
      break;
  }
}

void SettingsApp::set_page(Page page) {
  page_ = page;
  System::get().invalidate_all();  // başlık da değişir
}

void SettingsApp::activate(int row) {
  System& sys = System::get();
  Settings& s = sys.settings();
  switch (row) {
    case kTheme:
      picker_.setup("Tema", kThemes, 2, s.dark ? 1 : 0,
                    [](void*, int index) {
                      System& sy = System::get();
                      sy.settings().dark = index == 1;
                      sy.apply_settings();
                      sy.invalidate_content();
                    },
                    nullptr);
      sys.show(picker_);
      break;
    case kBrightness:
      slider_.setup("Parlaklık", s.brightness, 0, 100, 10, "%",
                    [](void*, int value) {
                      System& sy = System::get();
                      sy.settings().brightness = static_cast<uint8_t>(value);
                      sy.apply_settings();
                      sy.invalidate_content();
                    },
                    nullptr);
      sys.show(slider_);
      break;
    case kAngle:
      picker_.setup("Açı birimi", kAngles, 3, static_cast<int>(s.angle),
                    [](void*, int index) {
                      System& sy = System::get();
                      sy.settings().angle = static_cast<AngleUnit>(index);
                      sy.apply_settings();
                      sy.invalidate_content();
                    },
                    nullptr);
      sys.show(picker_);
      break;
    case kFormat:
      picker_.setup("Sayı biçimi", kFormats, 3, static_cast<int>(s.format),
                    [](void*, int index) {
                      System& sy = System::get();
                      sy.settings().format = static_cast<NumberFormat>(index);
                      sy.apply_settings();
                      sy.invalidate_content();
                    },
                    nullptr);
      sys.show(picker_);
      break;
    case kAbout:
      set_page(Page::About);
      break;
    default:
      break;
  }
}

bool SettingsApp::on_key(const KeyEvent& ev) {
  if (!ev.down) return false;
  System& sys = System::get();

  if (page_ == Page::About) {
    if (ev.key == Key::Back || ev.key == Key::Left) {
      set_page(Page::Main);
      return true;
    }
    return false;
  }

  if (ev.key == Key::F1 && !ev.repeat) {
    activate(list_.selected());
    return true;
  }
  switch (list_.on_key(ev)) {
    case ui::ListView::Result::Moved:
      sys.invalidate(list_.damage());
      return true;
    case ui::ListView::Result::Activated:
      activate(list_.selected());
      return true;
    case ui::ListView::Result::Ignored:
      break;
  }
  return ev.key == Key::Up || ev.key == Key::Down;
}

void SettingsApp::draw(ui::Canvas& c, Rect area) {
  const ui::Theme& t = ui::theme();
  if (page_ == Page::Main) {
    list_.draw(c);
    return;
  }

  c.fill_rect(area, t.bg);
  c.text(16, area.y + 14, "CALCTUS", ui::font_sans24, t.accent);
  char buf[64];
  std::snprintf(buf, sizeof buf, "Sürüm %s", kVersion);
  c.text(16, area.y + 50, buf, ui::font_sans12, t.text);
  c.text(16, area.y + 68, "Ekran 320×240 · 55 tuş", ui::font_sans12, t.text);
  c.text_wrapped(make_rect(16, area.y + 96, area.w - 32, 80),
                 "Sıfırdan yazılan açık kaynak hesap makinesi sistemi. Aynı kod masaüstü "
                 "simülatöründe ve ESP32-S3 üzerinde çalışır.",
                 ui::font_sans12, t.muted);
}

void SettingsApp::soft_keys(SoftKeys& keys) const {
  if (page_ == Page::Main) keys.label[0] = "Seç";
}

}  // namespace calctus::apps
