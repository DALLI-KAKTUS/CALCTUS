#pragma once

#include "kernel/app.h"

namespace calctus::apps {

class ElementsApp final : public App {
 public:
  const char* title() const override { return "Elementler"; }
  View view() const override { return View::Elements; }
  int selected_number() const { return selected_; }
  bool list_mode() const { return list_; }
  const char* search() const { return search_; }
  bool on_key(const KeyEvent& event) override;
  void draw(ui::Canvas& canvas, Rect area) override;
  void soft_keys(SoftKeys& keys) const override;

 private:
  void move(int step);
  void jump();

  int selected_ = 26; // Fe
  char search_[4]{}; // 1..118 atom numarası
  bool list_ = false;
};

}  // namespace calctus::apps
