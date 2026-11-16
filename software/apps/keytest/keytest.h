// Tuş testi: basılan tuşu, matris konumunu ve hangi tuşların denendiğini gösterir.
// Bütün tuşları (Ana ekran ve Güç dahil) kendisi alır; çıkış: Ana ekran tuşuna iki kez.
#pragma once

#include "kernel/app.h"

namespace calctus::apps {

class KeyTestApp final : public App {
 public:
  const char* title() const override { return "Tuş testi"; }
  View view() const override { return View::KeyTest; }
  void on_open() override;
  bool on_key(const KeyEvent& ev) override;
  void on_tick(uint32_t now_ms) override;
  uint32_t tick_interval_ms() const override { return 1000; }
  bool captures_system_keys() const override { return true; }

  Key last() const { return last_; }
  bool last_down() const { return last_down_; }
  bool down(int index) const { return down_[index]; }
  bool seen(int index) const { return seen_[index]; }
  int seen_count() const { return seen_count_; }
  uint32_t presses() const { return presses_; }
  unsigned elapsed_s() const { return static_cast<unsigned>((now_ms_ - start_ms_) / 1000); }

 private:
  bool down_[kKeyCount] = {};
  bool seen_[kKeyCount] = {};
  int seen_count_ = 0;
  uint32_t presses_ = 0;
  Key last_ = Key::None;
  bool last_down_ = false;
  uint32_t start_ms_ = 0;
  uint32_t now_ms_ = 0;
  uint32_t last_home_ms_ = 0;
  bool home_once_ = false;
};

}  // namespace calctus::apps
