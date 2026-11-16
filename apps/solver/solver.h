#pragma once

#include "kernel/app.h"
#include "math/solver.h"

namespace calctus::apps {

class SolverApp final : public App {
 public:
  const char* title() const override { return "Çözücü"; }
  View view() const override { return View::Solver; }
  const char* coefficient(int index) const { return coefficient_[index]; }
  int selected() const { return selected_; }
  bool has_result() const { return has_result_; }
  const math::Solution& result() const { return result_; }
  bool on_key(const KeyEvent& ev) override;
  void draw(ui::Canvas& canvas, Rect area) override;
  void soft_keys(SoftKeys& keys) const override;

 private:
  void select(int index);
  void append(const char* text);
  void calculate();
  char* active() { return coefficient_[selected_]; }

  char coefficient_[3][65] = {"1", "0", "-1"};
  int selected_ = 0;
  bool editing_ = false;
  math::Solution result_{};
  bool has_result_ = false;
};

}  // namespace calctus::apps
