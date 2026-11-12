#include "apps/gallery/gallery.h"
#include "apps/home/home.h"
#include "apps/keytest/keytest.h"
#include "apps/settings/settings_app.h"
#include "kernel/system.h"

namespace calctus {

int boot() {
  static apps::HomeApp home;
  static apps::SettingsApp settings;
  static apps::GalleryApp gallery;
  static apps::KeyTestApp keytest;

  // Ana ekrandaki sıra. Uygulaması boş olanlar henüz yazılmadı; ızgarada soluk
  // görünür ve açılınca "yakında" bildirimi verir.
  static const AppEntry kApps[] = {
      {nullptr, "Hesap", "±", true, rgb(47, 143, 91)},
      {nullptr, "Grafik", "ƒ", true, rgb(52, 120, 200)},
      {nullptr, "Python", ">>>", false, rgb(92, 84, 190)},
      {nullptr, "Çözücü", "x=", false, rgb(214, 126, 44)},
      {nullptr, "İstatistik", "Σ", true, rgb(150, 84, 180)},
      {nullptr, "Olasılık", "%", true, rgb(32, 150, 150)},
      {nullptr, "Diziler", "uₙ", false, rgb(200, 80, 120)},
      {nullptr, "Elementler", "Fe", false, rgb(150, 110, 70)},
      {nullptr, "Terminal", ">_", false, rgb(60, 66, 72)},
      {nullptr, "AI", "AI", false, rgb(120, 70, 200)},
      {nullptr, "Dosyalar", "▤", true, rgb(90, 110, 130)},
      {&settings, "Ayarlar", "⚙", true, rgb(110, 116, 122)},
      {&gallery, "Bileşenler", "▦", true, rgb(30, 140, 180)},
      {&keytest, "Tuş testi", "⌨", true, rgb(40, 110, 80)},
  };

  System& sys = System::get();
  sys.set_apps(kApps, static_cast<int>(sizeof kApps / sizeof kApps[0]), home);
  return sys.run();
}

}  // namespace calctus
