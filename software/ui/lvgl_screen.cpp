#include "ui/lvgl_screen.h"

#include <cstdio>
#include <cstring>

#include "kernel/system.h"
#include "apps/home/home.h"
#include "apps/settings/settings_app.h"
#include "apps/pi/pi.h"
#include "apps/elements/elements.h"
#include "apps/elements/data.h"
#include "apps/calc/calc.h"
#include "apps/solver/solver.h"
#include "apps/graph/graph.h"
#include "apps/statistics/statistics.h"
#include "apps/probability/probability.h"
#include "apps/sequences/sequences.h"
#include "apps/python/python.h"
#include "apps/files/files.h"
#include "apps/gallery/gallery.h"
#include "apps/keytest/keytest.h"
#include "math/eval.h"
#include "ui/widgets/modal.h"
#include "ui/font.h"
#include "ui/lvgl_kit.h"

namespace calctus::ui {
namespace {
constexpr int W = hal::kScreenWidth;
constexpr int H = hal::kScreenHeight;
// 16 RGB565 lines: a bounded internal buffer, independent of the PSRAM frame.
alignas(16) uint8_t draw_buffer[W * 16 * 2];

void flush(lv_display_t* display, const lv_area_t* area, uint8_t* pixels) {
  // LVGL RGB565 is little endian; the HAL takes native RGB565 words and
  // performs the panel's big-endian SPI transfer itself.
  Color* target = hal::framebuffer();
  const int width = area->x2 - area->x1 + 1;
  for (int y = area->y1; y <= area->y2; ++y) {
    std::memcpy(target + y * W + area->x1, pixels + (y - area->y1) * width * 2,
                width * sizeof(Color));
  }
  hal::display_push(make_rect(area->x1, area->y1, width, area->y2 - area->y1 + 1));
  lv_display_flush_ready(display);
}

} // namespace

using namespace lvk;

void LvglScreen::init() {
  lv_init();
  lv_tick_set_cb(hal::millis);
  display_ = lv_display_create(W, H);
  lv_display_set_color_format(display_, LV_COLOR_FORMAT_RGB565);
  lv_display_set_buffers(display_, draw_buffer, nullptr, sizeof draw_buffer,
                         LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(display_, flush);

  auto* screen = lv_screen_active();
  rect(screen, 0, 0, W, H, rgb(244, 241, 234), 0);
  // Her uygulama görünümünün kendi kabı var; yalnız açık olanınki görünür.
  for (auto*& view : views_) {
    view = lv_obj_create(screen);
    rect(view, 0, System::kStatusH, W, System::kContent.h, rgb(244, 241, 234), 0);
    lv_obj_add_flag(view, LV_OBJ_FLAG_HIDDEN);
  }
  header_ = lv_obj_create(screen);
  rect(header_, 0, 0, W, System::kStatusH, rgb(47, 143, 91), 0);
  title_ = label(header_, "CALCTUS", rgb(255, 255, 255));
  lv_obj_set_pos(title_, 8, 2);
  info_ = label(header_, "", rgb(255, 255, 255));
  lv_obj_align(info_, LV_ALIGN_RIGHT_MID, -8, 0);
  footer_ = lv_obj_create(screen);
  rect(footer_, 0, H - System::kSoftH, W, System::kSoftH, rgb(244, 241, 234), 0);
  for (int i = 0; i < 6; ++i) {
    soft_[i] = label(footer_, "", rgb(30, 36, 32));
    lv_obj_set_width(soft_[i], W / 6);
    lv_obj_set_style_text_align(soft_[i], LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(soft_[i], i * W / 6, 2);
  }
  modal_layer_ = lv_obj_create(screen);
  rect(modal_layer_, 0, 0, W, H, rgb(10, 25, 18), 0);
  lv_obj_set_style_bg_opa(modal_layer_, LV_OPA_40, 0);
  lv_obj_add_flag(modal_layer_, LV_OBJ_FLAG_HIDDEN);
  notice_ = lv_obj_create(screen);
  rect(notice_, 32, 180, W - 64, 28, rgb(30, 36, 32), 13);
  lv_obj_add_flag(notice_, LV_OBJ_FLAG_HIDDEN);
}

void LvglScreen::show(App::View view) {
  for (int i = 0; i < App::kViewCount; ++i) {
    if (i == static_cast<int>(view)) lv_obj_remove_flag(views_[i], LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(views_[i], LV_OBJ_FLAG_HIDDEN);
  }
}

void LvglScreen::statistics(const apps::StatisticsApp& app) {
  lv_obj_clean(root(App::View::Statistics));
  const Color bg = dark_ ? rgb(18, 22, 20) : rgb(246, 249, 247);
  const Color surface = dark_ ? rgb(28, 34, 31) : rgb(255, 255, 255);
  const Color ink = dark_ ? rgb(230, 237, 232) : rgb(29, 41, 34);
  const Color muted = dark_ ? rgb(140, 150, 144) : rgb(95, 113, 102);
  const Color accent = dark_ ? rgb(76, 195, 138) : rgb(47, 143, 91);
  lv_obj_set_style_bg_color(root(App::View::Statistics), color(bg), 0);

  char text[64];
  std::snprintf(text, sizeof text, "VERİ SETİ   %d / 32", app.count());
  auto* heading = label(root(App::View::Statistics), text, muted);
  lv_obj_set_pos(heading, 10, 5);

  const int page = app.page_index();
  if (page == 0) {
    auto* card = lv_obj_create(root(App::View::Statistics));
    rect(card, 8, 25, 304, 119, surface, 10);
    if (!app.count()) {
      auto* empty = label(card, "Sayı yaz, EXE ile ekle", muted);
      lv_obj_center(empty);
    } else {
      const int first = app.count() > 4 ? app.count() - 4 : 0;
      for (int i = first; i < app.count(); ++i) {
        const int row = i - first;
        auto* line = lv_obj_create(card);
        rect(line, 6, 6 + row * 27, 292, 23,
             row == app.count() - first - 1 ?
                 (dark_ ? rgb(37, 75, 55) : rgb(217, 237, 224)) : surface, 5);
        std::snprintf(text, sizeof text, "x%d", i + 1);
        auto* tag = label(line, text, muted);
        lv_obj_set_pos(tag, 9, 4);
        auto* value = clipped_label(line, app.value(i), 235, font_sans16, ink);
        lv_obj_set_style_text_font(value, face16(), 0);
        lv_obj_align(value, LV_ALIGN_RIGHT_MID, -9, 0);
      }
    }
  } else {
    auto* card = lv_obj_create(root(App::View::Statistics));
    rect(card, 8, 25, 304, 119, surface, 10);
    if (!app.count()) {
      auto* empty = label(card, "Önce veri ekle", muted);
      lv_obj_center(empty);
    } else {
      const auto& summary = app.summary();
      const char* names[2]{};
      const char* values[2]{};
      const char* footnote = nullptr;
      switch (page) {
        case 1:
          names[0] = "Toplam"; values[0] = summary.sum;
          names[1] = "Ortalama"; values[1] = summary.mean;
          break;
        case 2:
          names[0] = "Varyans"; values[0] = summary.variance;
          names[1] = "Std. sapma"; values[1] = summary.deviation;
          footnote = "Popülasyon varyansı (n böleni)";
          break;
        default:
          names[0] = "En küçük"; values[0] = summary.minimum;
          names[1] = "En büyük"; values[1] = summary.maximum;
          break;
      }
      for (int i = 0; i < 2; ++i) {
        auto* metric = lv_obj_create(card);
        rect(metric, 8, 8 + i * 49, 288, 43,
             dark_ ? rgb(34, 45, 38) : rgb(246, 249, 247), 7);
        auto* name = label(metric, names[i], muted);
        lv_obj_set_pos(name, 9, 4);
        auto* value = clipped_label(metric, values[i], 265, font_sans16, accent);
        lv_obj_set_style_text_font(value, face16(), 0);
        lv_obj_align(value, LV_ALIGN_BOTTOM_RIGHT, -8, -2);
      }
      if (footnote) {
        auto* note = label(card, footnote, muted);
        lv_obj_set_pos(note, 10, 104);
      }
    }
  }

  auto* caption = label(root(App::View::Statistics), "YENİ DEĞER", muted);
  lv_obj_set_pos(caption, 10, 151);
  auto* entry = lv_obj_create(root(App::View::Statistics));
  rect(entry, 8, 170, 304, 25, surface, 7);
  auto* value = clipped_label(entry, *app.input() ? app.input() : "0", 280,
                              font_sans16, *app.input() ? ink : muted);
  lv_obj_set_style_text_font(value, face16(), 0);
  lv_obj_align(value, LV_ALIGN_RIGHT_MID, -9, 0);
}

void LvglScreen::graph(apps::GraphApp& app) {
  app.ensure_sampled();
  lv_obj_clean(root(App::View::Graph));
  const Color bg = dark_ ? rgb(18, 22, 20) : rgb(246, 249, 247);
  const Color surface = dark_ ? rgb(28, 34, 31) : rgb(255, 255, 255);
  const Color ink = dark_ ? rgb(230, 237, 232) : rgb(29, 41, 34);
  const Color muted = dark_ ? rgb(140, 150, 144) : rgb(95, 113, 102);
  const Color line_color = dark_ ? rgb(54, 72, 61) : rgb(214, 225, 216);
  const Color accent = dark_ ? rgb(76, 195, 138) : rgb(47, 143, 91);
  lv_obj_set_style_bg_color(root(App::View::Graph), color(bg), 0);

  auto* formula = label(root(App::View::Graph), "f(x) =", muted);
  lv_obj_set_pos(formula, 10, 3);
  auto* expr = clipped_label(root(App::View::Graph), app.expression(), 258, font_sans12, ink);
  lv_obj_align(expr, LV_ALIGN_TOP_RIGHT, -10, 3);

  // The original plot uses screen coordinates (8,37)..(312,172). Keep the
  // exact 4px sample pitch, 135px height, panning and discontinuity rules.
  auto* plot = lv_obj_create(root(App::View::Graph));
  rect(plot, 8, 17, 304, 135, surface, 7);
  lv_obj_set_style_border_width(plot, 1, 0);
  lv_obj_set_style_border_color(plot, color(line_color), 0);
  const int axis_x = static_cast<int>(8 + 304 / 2.0 - app.center_x() * app.pixels_per_unit());
  const int axis_y = static_cast<int>(37 + 135 / 2.0 + app.center_y() * app.pixels_per_unit());
  if (axis_x >= 8 && axis_x < 312) {
    auto* axis = lv_obj_create(plot);
    rect(axis, axis_x - 8, 0, 1, 135, line_color, 0);
  }
  if (axis_y >= 37 && axis_y < 172) {
    auto* axis = lv_obj_create(plot);
    rect(axis, 0, axis_y - 37, 304, 1, line_color, 0);
  }

  const int count = app.sample_count();
  for (int i = 0; i < count; ++i) {
    graph_points_[i] = {static_cast<lv_value_precise_t>(i * 4),
                        static_cast<lv_value_precise_t>(app.point(i) < 0 ? 0 : app.point(i) - 37)};
  }
  auto curve = [&](int first, int last) {
    if (last <= first) return;
    auto* segment = lv_line_create(plot);
    lv_line_set_points(segment, graph_points_ + first, last - first + 1);
    lv_obj_set_pos(segment, 0, 0);
    lv_obj_set_style_line_color(segment, color(accent), 0);
    lv_obj_set_style_line_width(segment, 2, 0);
    lv_obj_set_style_line_rounded(segment, true, 0);
  };
  int first = -1;
  for (int i = 1; i < count; ++i) {
    const int y1 = app.point(i - 1), y2 = app.point(i);
    const int span = y1 > y2 ? y1 - y2 : y2 - y1;
    if (y1 >= 0 && y2 >= 0 && span <= 135 / 2) {
      if (first < 0) first = i - 1;
    } else if (first >= 0) {
      curve(first, i - 1);
      first = -1;
    }
  }
  if (first >= 0) curve(first, count - 1);
  if (!app.valid_expression()) {
    auto* invalid = label(plot, "Geçersiz ifade", muted);
    lv_obj_center(invalid);
  }

  auto* hint = label(root(App::View::Graph), "Yön: kaydır   F3/F4: yakınlaştır", muted);
  lv_obj_set_pos(hint, 10, 156);
  auto* input = lv_obj_create(root(App::View::Graph));
  rect(input, 8, 174, 304, 21, surface, 6);
  auto* current = clipped_label(input, app.expression(), 280, font_sans12, ink);
  lv_obj_align(current, LV_ALIGN_RIGHT_MID, -8, 0);
}

void LvglScreen::solver(const apps::SolverApp& app) {
  lv_obj_clean(root(App::View::Solver));
  const Color bg = dark_ ? rgb(18, 22, 20) : rgb(246, 249, 247);
  const Color surface = dark_ ? rgb(28, 34, 31) : rgb(255, 255, 255);
  const Color ink = dark_ ? rgb(230, 237, 232) : rgb(29, 41, 34);
  const Color muted = dark_ ? rgb(140, 150, 144) : rgb(95, 113, 102);
  const Color accent = dark_ ? rgb(76, 195, 138) : rgb(47, 143, 91);
  lv_obj_set_style_bg_color(root(App::View::Solver), color(bg), 0);

  auto* equation = lv_obj_create(root(App::View::Solver));
  rect(equation, 8, 5, 304, 31, dark_ ? rgb(37, 75, 55) : rgb(217, 237, 224), 8);
  auto* equation_text = label(equation, "ax² + bx + c = 0", accent);
  lv_obj_set_style_text_font(equation_text, face16(), 0);
  lv_obj_center(equation_text);

  const char* names[] = {"a", "b", "c"};
  for (int i = 0; i < 3; ++i) {
    auto* field = lv_obj_create(root(App::View::Solver));
    rect(field, 8, 40 + i * 30, 304, 27,
         i == app.selected() ? (dark_ ? rgb(37, 75, 55) : rgb(217, 237, 224)) : surface, 7);
    if (i == app.selected()) {
      lv_obj_set_style_border_width(field, 1, 0);
      lv_obj_set_style_border_color(field, color(accent), 0);
    }
    auto* name = label(field, names[i], accent);
    lv_obj_set_style_text_font(name, face16(), 0);
    lv_obj_set_pos(name, 11, 4);
    auto* value = clipped_label(field, *app.coefficient(i) ? app.coefficient(i) : "_",
                                252, font_sans16, ink);
    lv_obj_set_style_text_font(value, face16(), 0);
    lv_obj_align(value, LV_ALIGN_RIGHT_MID, -9, 0);
  }

  auto* answer = lv_obj_create(root(App::View::Solver));
  rect(answer, 8, 135, 304, 59, surface, 9);
  if (!app.has_result()) {
    auto* prompt = label(answer, "EXE ile çöz", muted);
    lv_obj_center(prompt);
    return;
  }
  const math::Solution& result = app.result();
  auto add_result = [&](int y, const char* name, const char* raw) {
    auto* tag = label(answer, name, muted);
    lv_obj_set_pos(tag, 11, y);
    char compact[128];
    const char* text = raw;
    if (Canvas::text_width(raw, font_sans16) > 238 &&
        math::evaluate("Ans", raw, NumberFormat::Scientific, compact, sizeof compact) ==
            math::Error::None) text = compact;
    auto* value = clipped_label(answer, text, 236, font_sans16, accent);
    lv_obj_set_style_text_font(value, face16(), 0);
    lv_obj_align(value, LV_ALIGN_TOP_RIGHT, -10, y);
  };
  switch (result.kind) {
    case math::Roots::None: {
      auto* message = label(answer, "Çözüm yok", ink); lv_obj_center(message); break;
    }
    case math::Roots::Any: {
      auto* message = label(answer, "Her x çözüm", ink); lv_obj_center(message); break;
    }
    case math::Roots::Linear: case math::Roots::Double:
      add_result(11, "x =", result.first);
      if (result.kind == math::Roots::Double) {
        auto* note = label(answer, "Çift katlı kök", muted);
        lv_obj_set_pos(note, 11, 37);
      }
      break;
    case math::Roots::Two:
      add_result(7, "x1 =", result.first);
      add_result(32, "x2 =", result.second);
      break;
    case math::Roots::Complex:
      add_result(7, "Re =", result.first);
      add_result(32, "±Im =", result.second);
      break;
  }
}

void LvglScreen::calc(const apps::CalcApp& app) {
  lv_obj_clean(root(App::View::Calc));
  const Color bg = dark_ ? rgb(18, 22, 20) : rgb(246, 249, 247);
  const Color surface = dark_ ? rgb(28, 34, 31) : rgb(255, 255, 255);
  const Color ink = dark_ ? rgb(230, 237, 232) : rgb(29, 41, 34);
  const Color muted = dark_ ? rgb(140, 150, 144) : rgb(95, 113, 102);
  const Color accent = dark_ ? rgb(76, 195, 138) : rgb(47, 143, 91);
  lv_obj_set_style_bg_color(root(App::View::Calc), color(bg), 0);
  auto* heading = label(root(App::View::Calc), app.rpn_mode() ? "YIĞIN  ·  RPN" : "GEÇMİŞ", muted);
  lv_obj_set_pos(heading, 10, 7);

  if (app.rpn_mode()) {
    for (int i = 0; i < app.stack_size(); ++i) {
      const int y = 23 + (3 - i) * 29;
      auto* row = lv_obj_create(root(App::View::Calc));
      rect(row, 8, y, 304, 26, i == app.stack_size() - 1 ?
           (dark_ ? rgb(37, 75, 55) : rgb(217, 237, 224)) : surface, 7);
      char index[16];
      std::snprintf(index, sizeof index, "%d:", i + 1);
      auto* tag = label(row, index, muted);
      lv_obj_set_pos(tag, 10, 6);
      auto* value = clipped_label(row, app.stack_value(i), 250, font_sans16,
                                  i == app.stack_size() - 1 ? accent : ink);
      lv_obj_set_style_text_font(value, face16(), 0);
      lv_obj_align(value, LV_ALIGN_RIGHT_MID, -10, 0);
    }
  } else {
    for (int i = app.history_count() - 1; i >= 0; --i) {
      const int y = 23 + (app.history_count() - 1 - i) * 38;
      auto* row = lv_obj_create(root(App::View::Calc));
      rect(row, 8, y, 304, 35, surface, 7);
      auto* expr = clipped_label(row, app.history_expression(i), 280, font_sans12, muted);
      lv_obj_align(expr, LV_ALIGN_TOP_RIGHT, -10, 3);
      auto* value = clipped_label(row, app.history_result(i), 278, font_sans16,
                                  i == 0 ? accent : ink);
      lv_obj_set_style_text_font(value, face16(), 0);
      lv_obj_align(value, LV_ALIGN_BOTTOM_RIGHT, -10, -2);
    }
  }

  auto* input_card = lv_obj_create(root(App::View::Calc));
  rect(input_card, 8, 147, 304, 46, surface, 9);
  auto* caption = label(input_card, app.rpn_mode() ? "GİRİŞ" : "İFADE", accent);
  lv_obj_set_pos(caption, 10, 4);
  auto* input = clipped_label(input_card, *app.input() ? app.input() : "0", 278,
                              font_sans16, *app.input() ? ink : muted);
  lv_obj_set_style_text_font(input, face16(), 0);
  lv_obj_align(input, LV_ALIGN_BOTTOM_RIGHT, -9, -4);
}

void LvglScreen::elements(const apps::ElementsApp& app) {
  lv_obj_clean(root(App::View::Elements));
  const Color bg = dark_ ? rgb(18, 22, 20) : rgb(246, 249, 247);
  const Color surface = dark_ ? rgb(28, 34, 31) : rgb(255, 255, 255);
  const Color ink = dark_ ? rgb(230, 237, 232) : rgb(29, 41, 34);
  const Color muted = dark_ ? rgb(140, 150, 144) : rgb(95, 113, 102);
  const Color accent = dark_ ? rgb(76, 195, 138) : rgb(47, 143, 91);
  lv_obj_set_style_bg_color(root(App::View::Elements), color(bg), 0);

  auto* heading = label(root(App::View::Elements), app.list_mode() ? "PERİYODİK LİSTE" : "PERİYODİK TABLO", muted);
  lv_obj_set_pos(heading, 10, 8);
  char text[64];
  if (app.search()[0]) std::snprintf(text, sizeof text, "Git: %s", app.search());
  else std::snprintf(text, sizeof text, "No: %d", app.selected_number());
  auto* search = label(root(App::View::Elements), text, accent);
  lv_obj_align(search, LV_ALIGN_TOP_RIGHT, -10, 8);

  if (app.list_mode()) {
    const int first = app.selected_number() <= 4 ? 1 :
        app.selected_number() >= 115 ? 111 : app.selected_number() - 3;
    for (int i = 0; i < 8; ++i) {
      const int number = first + i;
      const apps::Element& element = *apps::element_by_number(number);
      const bool selected = number == app.selected_number();
      auto* row = lv_obj_create(root(App::View::Elements));
      rect(row, 8, 28 + i * 21, 304, 20,
           selected ? (dark_ ? rgb(37, 75, 55) : rgb(217, 237, 224)) : surface, 5);
      std::snprintf(text, sizeof text, "%3d   %-3s  %s", number, element.symbol, element.name);
      auto* name = label(row, text, selected ? accent : ink);
      lv_obj_set_pos(name, 10, 3);
    }
    return;
  }

  const apps::Element& element = *apps::element_by_number(app.selected_number());
  auto* card = lv_obj_create(root(App::View::Elements));
  rect(card, 8, 31, 304, 94, surface, 12);
  auto* symbol_box = lv_obj_create(card);
  rect(symbol_box, 8, 8, 74, 78, accent, 9);
  std::snprintf(text, sizeof text, "%d", app.selected_number());
  auto* number = label(symbol_box, text, rgb(255, 255, 255));
  lv_obj_set_pos(number, 8, 5);
  auto* symbol = label(symbol_box, element.symbol, rgb(255, 255, 255));
  lv_obj_set_style_text_font(symbol, face24(), 0);
  lv_obj_center(symbol);
  auto* name = label(card, element.name, ink);
  lv_obj_set_style_text_font(name, face16(), 0);
  lv_obj_set_pos(name, 94, 22);
  std::snprintf(text, sizeof text, "%.6g u  ·  yaklaşık", element.approximate_mass);
  auto* mass = label(card, text, muted);
  lv_obj_set_pos(mass, 94, 57);

  auto* properties = lv_obj_create(root(App::View::Elements));
  rect(properties, 8, 128, 304, 54, surface, 10);
  const char* phase = element.phase == 'G' ? "Gaz" : element.phase == 'L' ? "Sıvı" : "Katı";
  char fields[4][32];
  std::snprintf(fields[0], sizeof fields[0], "Periyot  %u", element.period);
  if (element.group) std::snprintf(fields[1], sizeof fields[1], "Grup  %u", element.group);
  else std::snprintf(fields[1], sizeof fields[1], "Grup  f-blok");
  std::snprintf(fields[2], sizeof fields[2], "Blok  %c", element.block);
  std::snprintf(fields[3], sizeof fields[3], "Faz  %s%s", phase,
                app.selected_number() >= 104 ? "*" : "");
  for (int i = 0; i < 4; ++i) {
    auto* field = label(properties, fields[i], i == 3 ? accent : ink);
    lv_obj_set_pos(field, 12 + (i % 2) * 152, 7 + (i / 2) * 30);
  }
  if (app.selected_number() >= 104) {
    auto* note = label(root(App::View::Elements), "* Faz tahmini olabilir", muted);
    lv_obj_set_pos(note, 12, 184);
  }
}

void LvglScreen::pi(const apps::PiApp& app) {
  lv_obj_clean(root(App::View::Pi));
  const Color bg = dark_ ? rgb(18, 22, 20) : rgb(244, 241, 234);
  const Color surface = dark_ ? rgb(28, 34, 31) : rgb(255, 255, 255);
  const Color ink = dark_ ? rgb(230, 237, 232) : rgb(30, 36, 32);
  const Color muted = dark_ ? rgb(140, 150, 144) : rgb(100, 117, 106);
  const Color accent = dark_ ? rgb(76, 195, 138) : rgb(47, 143, 91);
  lv_obj_set_style_bg_color(root(App::View::Pi), color(bg), 0);
  auto* banner = lv_obj_create(root(App::View::Pi));
  rect(banner, 8, 7, 304, 65, app.connected() ?
       (dark_ ? rgb(35, 77, 54) : rgb(214, 239, 223)) : surface, 12);
  auto* title = label(banner, app.connected() ? "Pi bağlı" : "Pi bağlı değil", app.connected() ? accent : ink);
  lv_obj_set_style_text_font(title, face16(), 0);
  lv_obj_set_pos(title, 13, 10);
  auto* hint = label(banner, app.connected() ? "UART kontrol kanalı hazır" :
                     "ESP32-S3 bağımsız çalışır", muted);
  lv_obj_set_pos(hint, 13, 38);
  auto* card = lv_obj_create(root(App::View::Pi));
  rect(card, 8, 80, 304, 100, surface, 12);
  auto* caption = label(card, "UZAK HESAP", accent);
  lv_obj_set_pos(caption, 13, 9);
  char line[64];
  std::snprintf(line, sizeof line, "%ld  ×  %ld", static_cast<long>(app.input()),
                static_cast<long>(app.input()));
  auto* formula = label(card, line, ink);
  lv_obj_set_style_text_font(formula, face24(), 0);
  lv_obj_set_pos(formula, 13, 30);
  if (app.calculated()) std::snprintf(line, sizeof line, "Pi sonucu: %ld", static_cast<long>(app.result()));
  else std::snprintf(line, sizeof line, "%s", app.info()[0] ? app.info() : "↑↓ sayı  ·  F3 hesapla");
  auto* result = label(card, line, app.calculated() ? accent : muted);
  lv_obj_set_pos(result, 13, 76);
  auto* note = label(root(App::View::Pi), "Pi ekranı ve Wi-Fi paylaşımı henüz tasarım", muted);
  lv_obj_set_pos(note, 12, 185);
}

void LvglScreen::settings(const apps::SettingsApp& app) {
  lv_obj_clean(root(App::View::Settings));
  const Color bg = dark_ ? rgb(18, 22, 20) : rgb(244, 241, 234);
  const Color surface = dark_ ? rgb(28, 34, 31) : rgb(255, 255, 255);
  const Color ink = dark_ ? rgb(230, 237, 232) : rgb(30, 36, 32);
  const Color muted = dark_ ? rgb(140, 150, 144) : rgb(100, 117, 106);
  const Color accent = dark_ ? rgb(76, 195, 138) : rgb(47, 143, 91);
  lv_obj_set_style_bg_color(root(App::View::Settings), color(bg), 0);
  if (app.about_page()) {
    auto* card = lv_obj_create(root(App::View::Settings));
    rect(card, 9, 9, 302, 177, surface, 12);
    auto* name = label(card, "CALCTUS", accent);
    lv_obj_set_style_text_font(name, face24(), 0);
    lv_obj_set_pos(name, 14, 14);
    auto* version = label(card, "Sürüm 0.2  ·  GMT020 / ST7789", ink);
    lv_obj_set_pos(version, 14, 54);
    auto* details = label(card, "320×240 yatay  ·  55 tuş\nESP32-S3 + isteğe bağlı Pi Zero 2\n\nMasaüstü simülatörü ve cihaz\naynı uygulama kodunu kullanır.", muted);
    lv_obj_set_pos(details, 14, 80);
    return;
  }
  for (int i = 0; i < 5; ++i) {
    ui::ListItem item{};
    app.row(i, item);
    auto* row = lv_obj_create(root(App::View::Settings));
    rect(row, 8, 7 + i * 37, 304, 33,
         i == app.selected_row() ? (dark_ ? rgb(37, 75, 55) : rgb(212, 234, 221)) : surface, 8);
    if (i == app.selected_row()) {
      lv_obj_set_style_border_width(row, 1, 0);
      lv_obj_set_style_border_color(row, color(accent), 0);
    }
    auto* name = label(row, item.label ? item.label : "", ink);
    lv_obj_set_pos(name, 12, 9);
    auto* value = label(row, item.value ? item.value : (i == 4 ? ">" : ""), muted);
    lv_obj_align(value, LV_ALIGN_RIGHT_MID, -11, 0);
  }
}

void LvglScreen::home(const AppEntry* apps, int count, int selected, int top_row) {
  lv_obj_clean(root(App::View::Home));
  const Color bg = dark_ ? rgb(18, 22, 20) : rgb(244, 241, 234);
  const Color surface = dark_ ? rgb(34, 46, 39) : rgb(255, 255, 255);
  const Color ink = dark_ ? rgb(231, 239, 233) : rgb(29, 41, 34);
  const Color accent = dark_ ? rgb(76, 195, 138) : rgb(47, 143, 91);
  lv_obj_set_style_bg_color(root(App::View::Home), color(bg), 0);
  for (int i = top_row * 4; i < count && i < (top_row + 3) * 4; ++i) {
    const int col = i % 4, row = i / 4 - top_row;
    auto* tile = lv_obj_create(root(App::View::Home));
    rect(tile, 4 + col * 80, 4 + row * 66, 72, 60,
         i == selected ? (dark_ ? rgb(38, 72, 54) : rgb(213, 235, 221)) : surface, 9);
    if (i == selected) {
      lv_obj_set_style_border_width(tile, 2, 0);
      lv_obj_set_style_border_color(tile, color(accent), 0);
    }
    auto* icon = lv_obj_create(tile);
    rect(icon, 20, 4, 32, 32, apps[i].app ? apps[i].color : (dark_ ? rgb(65, 73, 67) : rgb(199, 204, 198)), 8);
    auto* symbol = label(icon, apps[i].symbol, rgb(255, 255, 255));
    lv_obj_set_style_text_font(symbol, apps[i].big_symbol ? face24() : face16(), 0);
    lv_obj_center(symbol);
    auto* name = label(tile, apps[i].name, apps[i].app ? ink : (dark_ ? rgb(125, 140, 130) : rgb(143, 153, 146)));
    lv_obj_set_width(name, 70);
    lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(name, 1, 40);
  }

  // Kaydırma göstergesi: satır başına bir nokta, görünen satırlar vurgulu.
  const int total_rows = (count + 3) / 4;
  if (total_rows > 3) {
    for (int row = 0; row < total_rows; ++row) {
      const bool shown = row >= top_row && row < top_row + 3;
      box(root(App::View::Home), make_rect(316, 6 + row * 8, 3, 5),
          shown ? accent : (dark_ ? rgb(52, 62, 56) : rgb(214, 225, 216)), 1);
    }
  }
}

void LvglScreen::status(const char* title, const char* angle, int battery, bool dark) {
  dark_ = dark;
  lv_obj_set_style_bg_color(header_, color(dark ? rgb(37, 105, 76) : rgb(47, 143, 91)), 0);
  lv_label_set_text(title_, title);
  char text[32];
  std::snprintf(text, sizeof text, "%s  %d%%", angle, battery);
  lv_label_set_text(info_, text);
  lv_obj_align(info_, LV_ALIGN_RIGHT_MID, -8, 0);
  lv_obj_set_style_bg_color(footer_, color(dark ? rgb(28, 34, 31) : rgb(245, 248, 244)), 0);
}

void LvglScreen::soft(const SoftKeys& keys, const bool* pressed) {
  for (int i = 0; i < 6; ++i) {
    lv_label_set_text(soft_[i], keys.label[i] ? keys.label[i] : "");
    lv_obj_set_style_text_color(soft_[i], color(pressed[i] ?
       (dark_ ? rgb(76, 195, 138) : rgb(47, 143, 91)) :
       (dark_ ? rgb(230, 237, 232) : rgb(30, 36, 32))), 0);
  }
}

void LvglScreen::toast(const char* message, bool show) {
  if (!show) { lv_obj_add_flag(notice_, LV_OBJ_FLAG_HIDDEN); return; }
  lv_obj_clean(notice_);
  auto* text = label(notice_, message, rgb(255, 255, 255));
  lv_obj_center(text);
  lv_obj_remove_flag(notice_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(notice_);
}

void LvglScreen::modal(const Modal* current) {
  if (current == nullptr) {
    lv_obj_add_flag(modal_layer_, LV_OBJ_FLAG_HIDDEN);
    return;
  }
  lv_obj_clean(modal_layer_);
  lv_obj_remove_flag(modal_layer_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_move_foreground(modal_layer_);

  const Color surface = dark_ ? rgb(28, 34, 31) : rgb(255, 255, 255);
  const Color ink = dark_ ? rgb(230, 237, 232) : rgb(29, 41, 34);
  const Color accent = dark_ ? rgb(76, 195, 138) : rgb(47, 143, 91);
  const Rect bounds = current->bounds();
  auto* panel = lv_obj_create(modal_layer_);
  rect(panel, bounds.x, bounds.y, bounds.w, bounds.h, surface, 10);
  lv_obj_set_style_border_width(panel, 1, 0);
  lv_obj_set_style_border_color(panel, color(dark_ ? rgb(54, 72, 61) : rgb(214, 225, 216)), 0);

  const char* heading = "";
  switch (current->kind()) {
    case Modal::Kind::Dialog: heading = static_cast<const Dialog*>(current)->title(); break;
    case Modal::Kind::Picker: heading = static_cast<const Picker*>(current)->title(); break;
    case Modal::Kind::Slider: heading = static_cast<const SliderModal*>(current)->title(); break;
  }
  auto* title = label(panel, heading, accent);
  lv_obj_set_pos(title, 12, 9);

  switch (current->kind()) {
    case Modal::Kind::Dialog: {
      const auto& dialog = *static_cast<const Dialog*>(current);
      auto* message = label(panel, dialog.message(), ink);
      lv_obj_set_pos(message, 12, 34);
      lv_obj_set_width(message, bounds.w - 24);
      lv_label_set_long_mode(message, LV_LABEL_LONG_MODE_WRAP);
      const int button_width = (bounds.w - 36) / 2;
      for (int i = 0; i < 2; ++i) {
        const bool selected = dialog.selected() == i;
        auto* button = lv_obj_create(panel);
        rect(button, 12 + i * (button_width + 12), bounds.h - 34, button_width, 25,
             selected ? accent : (dark_ ? rgb(43, 53, 47) : rgb(239, 244, 240)), 6);
        auto* text = label(button, dialog.button(i), selected ? rgb(255, 255, 255) : ink);
        lv_obj_center(text);
      }
      break;
    }
    case Modal::Kind::Picker: {
      const auto& picker = *static_cast<const Picker*>(current);
      const int visible = picker.option_count() - picker.first_visible() < 6 ?
                          picker.option_count() - picker.first_visible() : 6;
      for (int i = 0; i < visible; ++i) {
        const int index = picker.first_visible() + i;
        const bool selected = picker.selected() == index;
        auto* row = lv_obj_create(panel);
        rect(row, 6, 32 + i * 25, bounds.w - 12, 23,
             selected ? (dark_ ? rgb(37, 75, 55) : rgb(217, 237, 224)) : surface, 5);
        auto* text = label(row, picker.option(index), selected ? accent : ink);
        lv_obj_set_pos(text, 10, 5);
        if (picker.initial() == index) {
          auto* check = label(row, "✓", accent);
          lv_obj_align(check, LV_ALIGN_RIGHT_MID, -9, 0);
        }
      }
      break;
    }
    case Modal::Kind::Slider: {
      const auto& slider = *static_cast<const SliderModal*>(current);
      char text[32];
      std::snprintf(text, sizeof text, "%d%s", slider.value(), slider.unit());
      auto* value = label(panel, text, ink);
      lv_obj_align(value, LV_ALIGN_TOP_RIGHT, -12, 9);
      const int track_width = bounds.w - 32;
      auto* track = lv_obj_create(panel);
      rect(track, 16, 47, track_width, 8, dark_ ? rgb(54, 72, 61) : rgb(214, 225, 216), 4);
      const int range = slider.maximum() - slider.minimum();
      const int filled = range > 0 ? track_width * (slider.value() - slider.minimum()) / range : 0;
      if (filled > 0) {
        auto* progress = lv_obj_create(panel);
        rect(progress, 16, 47, filled, 8, accent, 4);
      }
      auto* thumb = lv_obj_create(panel);
      const int thumb_x = 16 + filled - 6 < 16 ? 16 : 16 + filled - 6;
      rect(thumb, thumb_x, 42, 14, 18, accent, 7);
      lv_obj_set_style_border_width(thumb, 3, 0);
      lv_obj_set_style_border_color(thumb, color(surface), 0);
      break;
    }
  }
}

void LvglScreen::content(App& app, const AppEntry* apps, int count) {
  // Her görünüm uygulamanın o anki durumundan baştan kurulur.
  switch (app.view()) {
    case App::View::Home: {
      const auto& home_app = static_cast<const apps::HomeApp&>(app);
      home(apps, count, home_app.selected(), home_app.top_row());
      break;
    }
    case App::View::Settings: settings(static_cast<const apps::SettingsApp&>(app)); break;
    case App::View::Pi: pi(static_cast<const apps::PiApp&>(app)); break;
    case App::View::Elements: elements(static_cast<const apps::ElementsApp&>(app)); break;
    case App::View::Calc: calc(static_cast<const apps::CalcApp&>(app)); break;
    case App::View::Solver: solver(static_cast<const apps::SolverApp&>(app)); break;
    case App::View::Graph: graph(static_cast<apps::GraphApp&>(app)); break;
    case App::View::Statistics: statistics(static_cast<const apps::StatisticsApp&>(app)); break;
    case App::View::Probability:
      probability(static_cast<const apps::ProbabilityApp&>(app)); break;
    case App::View::Sequences: sequences(static_cast<const apps::SequencesApp&>(app)); break;
    case App::View::Python: python(static_cast<const apps::PythonApp&>(app)); break;
    case App::View::Files: files(static_cast<const apps::FilesApp&>(app)); break;
    case App::View::Gallery: gallery(static_cast<const apps::GalleryApp&>(app)); break;
    case App::View::KeyTest: keytest(static_cast<const apps::KeyTestApp&>(app)); break;
  }
}

void LvglScreen::refresh() { lv_timer_handler(); }
} // namespace calctus::ui
