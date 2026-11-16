#pragma once

#include "kernel/app.h"
#include "math/sequence.h"

namespace calctus::apps {

class SequencesApp final : public App {
 public:
  const char* title() const override { return "Diziler"; }
  void on_open() override;
  bool on_key(const KeyEvent& ev) override;
  void draw(ui::Canvas& canvas, Rect area) override;
  void soft_keys(SoftKeys& keys) const override;

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
