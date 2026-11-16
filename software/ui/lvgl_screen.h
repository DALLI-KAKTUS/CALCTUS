#pragma once

#include "calctus/hal.h"
#include "kernel/app.h"
#include "third_party/lvgl/lvgl.h"

namespace calctus {
struct AppEntry;
namespace apps {
class HomeApp; class SettingsApp; class PiApp; class ElementsApp; class CalcApp;
class SolverApp; class GraphApp; class StatisticsApp; class ProbabilityApp;
class SequencesApp; class PythonApp; class FilesApp; class GalleryApp; class KeyTestApp;
}
}

namespace calctus::ui {

class Modal;

// LVGL owns the display tree, flush and timing. Every application has a
// native view container under the status bar; only the open one is shown.
class LvglScreen {
 public:
  void init();
  void show(App::View view);
  // Rebuilds the open application's view from its current state.
  void content(App& app, const AppEntry* apps, int count);
  void status(const char* title, const char* angle, int battery, bool dark);
  void soft(const SoftKeys& keys, const bool* pressed);
  void toast(const char* message, bool show);
  void modal(const Modal* current);
  void refresh();

 private:
  lv_obj_t*& root(App::View view) { return views_[static_cast<int>(view)]; }

  void home(const AppEntry* apps, int count, int selected, int top_row);
  void settings(const apps::SettingsApp& app);
  void pi(const apps::PiApp& app);
  void elements(const apps::ElementsApp& app);
  void calc(const apps::CalcApp& app);
  void solver(const apps::SolverApp& app);
  void graph(apps::GraphApp& app);
  void statistics(const apps::StatisticsApp& app);
  // ui/lvgl_apps.cpp
  void probability(const apps::ProbabilityApp& app);
  void sequences(const apps::SequencesApp& app);
  void python(const apps::PythonApp& app);
  void files(const apps::FilesApp& app);
  void gallery(const apps::GalleryApp& app);
  void keytest(const apps::KeyTestApp& app);

  lv_display_t* display_ = nullptr;
  lv_obj_t* views_[App::kViewCount] = {};
  // lv_line_set_points retains its pointer: samples live for the screen's life.
  lv_point_precise_t graph_points_[77] = {};
  lv_obj_t* header_ = nullptr;
  lv_obj_t* title_ = nullptr;
  lv_obj_t* info_ = nullptr;
  lv_obj_t* footer_ = nullptr;
  lv_obj_t* soft_[6] = {};
  lv_obj_t* notice_ = nullptr;
  lv_obj_t* modal_layer_ = nullptr;
  bool dark_ = false;
};

}  // namespace calctus::ui
