#pragma once

#include "kernel/app.h"

namespace calctus::apps {
class PiApp final : public App {
 public:
  const char* title() const override { return "Pi Zero 2"; }
  View view() const override { return View::Pi; }
  bool connected() const { return connected_; }
  bool calculated() const { return calculated_; }
  const char* info() const { return info_; }
  int32_t input() const { return input_; }
  int32_t result() const { return result_; }
  void on_open() override;
  bool on_key(const KeyEvent& event) override;
  void draw(ui::Canvas& canvas, Rect area) override;
  void soft_keys(SoftKeys& keys) const override;

 private:
  bool connected_ = false;
  char info_[49]{};
  int32_t input_ = 12;
  int32_t result_ = 0;
  bool calculated_ = false;
};
}  // namespace calctus::apps
