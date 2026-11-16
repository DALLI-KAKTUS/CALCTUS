#pragma once

#include "kernel/app.h"
#include "math/combinatorics.h"

namespace calctus::apps {

class ProbabilityApp final : public App {
 public:
  const char* title() const override { return "Olasılık"; }
  View view() const override { return View::Probability; }
  bool on_key(const KeyEvent& event) override;
  void soft_keys(SoftKeys& keys) const override;

  const char* mode_title() const;
  const char* n() const { return n_; }
  const char* k() const { return k_; }
  bool k_selected() const { return selected_k_; }
  const char* result() const { return result_; }

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
