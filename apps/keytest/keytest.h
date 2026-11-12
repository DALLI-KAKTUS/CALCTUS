// Tuş testi: S0 aşamasının deneme uygulaması.
// Basılan tuşu, matris konumunu ve hangi tuşların denendiğini gösterir.
#pragma once

#include "kernel/app.h"

namespace calctus::apps {

class KeyTestApp final : public App {
 public:
  void on_open() override;
  void on_key(const KeyEvent& ev) override;
  void on_tick(uint32_t now_ms) override;
  void draw(ui::Canvas& canvas) override;
  uint32_t tick_interval_ms() const override { return 1000; }

 private:
  bool down_[kKeyCount] = {};
  bool seen_[kKeyCount] = {};
  int seen_count_ = 0;
  uint32_t presses_ = 0;
  Key last_ = Key::None;
  bool last_down_ = false;
  uint32_t start_ms_ = 0;
  uint32_t now_ms_ = 0;
  bool dark_ = false;
};

}  // namespace calctus::apps
