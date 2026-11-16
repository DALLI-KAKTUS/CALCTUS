#include "apps/calc/calc.h"
#include "apps/elements/elements.h"
#include "apps/files/files.h"
#include "apps/gallery/gallery.h"
#include "apps/graph/graph.h"
#include "apps/home/home.h"
#include "apps/keytest/keytest.h"
#include "apps/probability/probability.h"
#include "apps/python/python.h"
#include "apps/pi/pi.h"
#include "apps/sequences/sequences.h"
#include "apps/settings/settings_app.h"
#include "apps/solver/solver.h"
#include "apps/statistics/statistics.h"
#include "kernel/system.h"

namespace calctus {

int boot() {
  static apps::HomeApp home;
  static apps::CalcApp calc;
  static apps::GraphApp graph;
  static apps::StatisticsApp statistics;
  static apps::ProbabilityApp probability;
  static apps::PythonApp python;
  static apps::PiApp pi;
  static apps::SolverApp solver;
  static apps::SequencesApp sequences;
  static apps::ElementsApp elements;
  static apps::FilesApp files;
  static apps::SettingsApp settings;
  static apps::GalleryApp gallery;
  static apps::KeyTestApp keytest;

  // Ana ekrandaki sıra. Uygulaması boş olanlar henüz yazılmadı; ızgarada soluk
  // görünür ve açılınca "yakında" bildirimi verir.
  static const AppEntry kApps[] = {
      {&calc, "Hesap", "±", true, rgb(47, 143, 91)},
      {&graph, "Grafik", "ƒ", true, rgb(52, 120, 200)},
      {&python, "Python", ">>>", false, rgb(92, 84, 190)},
      {&solver, "Çözücü", "x=", false, rgb(214, 126, 44)},
      {&statistics, "İstatistik", "Σ", true, rgb(150, 84, 180)},
      {&probability, "Olasılık", "%", true, rgb(32, 150, 150)},
      {&sequences, "Diziler", "uₙ", false, rgb(200, 80, 120)},
      {&elements, "Elementler", "Fe", false, rgb(150, 110, 70)},
      {&pi, "Pi Zero 2", ">_", false, rgb(60, 66, 72)},
      {nullptr, "AI", "AI", false, rgb(120, 70, 200)},
      {&files, "Dosyalar", "▤", true, rgb(90, 110, 130)},
      {&settings, "Ayarlar", "⚙", true, rgb(110, 116, 122)},
      {&gallery, "Bileşenler", "▦", true, rgb(30, 140, 180)},
      {&keytest, "Tuş testi", "⌨", true, rgb(40, 110, 80)},
  };

  System& sys = System::get();
  sys.set_apps(kApps, static_cast<int>(sizeof kApps / sizeof kApps[0]), home);
  return sys.run();
}

}  // namespace calctus
