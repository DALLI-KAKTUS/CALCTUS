// Ana ekran: uygulama ızgarası.
#pragma once

#include "kernel/app.h"

namespace calctus::apps {

class HomeApp final : public App {
 public:
  const char* title() const override { return "CALCTUS"; }
  View view() const override { return View::Home; }
  bool on_key(const KeyEvent& ev) override;
  void soft_keys(SoftKeys& keys) const override;
  int selected() const { return sel_; }
  int top_row() const { return top_row_; }

 private:
  static constexpr int kCols = 4;
  static constexpr int kRows = 3;  // ekranda görünen satır
  static constexpr int kTileW = 80;
  static constexpr int kTileH = 66;

  Rect tile_rect(int index) const;
  void open_selected();

  int sel_ = 0;
  int top_row_ = 0;
};

}  // namespace calctus::apps
