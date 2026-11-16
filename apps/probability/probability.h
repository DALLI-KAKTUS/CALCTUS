#pragma once

#include "kernel/app.h"
#include "math/combinatorics.h"

namespace calctus::apps {

class ProbabilityApp final : public App {
 public:
  const char* title() const override { return "Olasılık"; }
  bool on_key(const KeyEvent& event) override;
  void draw(ui::Canvas& canvas, Rect area) override;
  void soft_keys(SoftKeys& keys) const override;

 private:
  void calculate();
  void digit(char value);
  void erase();
  void select(bool k);

  char n_[3] = "5";
  char k_[3] = "2";
  bool selected_k_ = false;
  bool editing_ = false;
  math::Combinatoric operation_ = math::Combinatoric::Combination;
  char result_[128]{};
};

}  // namespace calctus::apps
