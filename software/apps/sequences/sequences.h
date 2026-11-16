#pragma once

#include "kernel/app.h"
#include "math/sequence.h"

namespace calctus::apps {

class SequencesApp final : public App {
 public:
  const char* title() const override { return "Diziler"; }
  View view() const override { return View::Sequences; }
  void on_open() override;
  bool on_key(const KeyEvent& ev) override;
  void soft_keys(SoftKeys& keys) const override;

  bool explicit_mode() const { return mode_ == math::SequenceMode::Explicit; }
  // 0: f(n), 1: u(0), 2: ilk n
  const char* field(int index) const {
    return index == 0 ? formula_ : index == 1 ? initial_ : first_;
  }
  int selected() const { return selected_; }
  bool has_rows() const { return has_rows_; }
  const char* row(int index) const { return rows_[index]; }
  static constexpr int kRows = 5;

 private:
  void select(int field);
  void append(const char* text);
  void calculate();
  char* active();

  math::SequenceMode mode_ = math::SequenceMode::Explicit;
  char formula_[97] = "x^2";
  char initial_[97] = "1";
  char first_[4] = "0";
  char rows_[5][128]{};
  bool has_rows_ = false;
  int selected_ = 0;
  bool editing_ = false;
};

}  // namespace calctus::apps
