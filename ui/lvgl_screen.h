#pragma once

#include "calctus/hal.h"
#include "kernel/app.h"
#include "third_party/lvgl/lvgl.h"

namespace calctus {
struct AppEntry;
namespace apps { class SettingsApp; class PiApp; class ElementsApp; class CalcApp; class SolverApp; class GraphApp; class StatisticsApp; }
}

namespace calctus::ui {

class Modal;

// LVGL owns the display tree, flush and timing. Legacy applications draw to
// an RGB565 LVGL canvas until their views are migrated to native widgets.
class LvglScreen {
 public:
  void init();
  void update_content(Rect dirty);
  void show_home(bool show);
  void show_settings(bool show);
  void show_pi(bool show);
  void show_elements(bool show);
  void show_calc(bool show);
  void show_solver(bool show);
  void show_graph(bool show);
  void show_statistics(bool show);
  void home(const AppEntry* apps, int count, int selected, int top_row);
  void settings(const apps::SettingsApp& app);
  void pi(const apps::PiApp& app);
  void elements(const apps::ElementsApp& app);
  void calc(const apps::CalcApp& app);
  void solver(const apps::SolverApp& app);
  void graph(apps::GraphApp& app);
  void statistics(const apps::StatisticsApp& app);
  void status(const char* title, const char* angle, int battery, bool dark);
  void soft(const SoftKeys& keys, const bool* pressed);
  void toast(const char* message, bool show);
  void modal(const Modal* current);
  void refresh();

 private:
  lv_display_t* display_ = nullptr;
  lv_obj_t* content_ = nullptr;
  lv_obj_t* home_ = nullptr;
  lv_obj_t* settings_ = nullptr;
  lv_obj_t* pi_ = nullptr;
  lv_obj_t* elements_ = nullptr;
  lv_obj_t* calc_ = nullptr;
  lv_obj_t* solver_ = nullptr;
  lv_obj_t* graph_ = nullptr;
  lv_obj_t* statistics_ = nullptr;
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
