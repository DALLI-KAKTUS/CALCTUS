#pragma once

#include "kernel/app.h"

namespace calctus::apps {

class GraphApp final : public App {
 public:
  const char* title() const override { return "Grafik"; }
  View view() const override { return View::Graph; }
  void ensure_sampled() { if (dirty_) sample(); }
  const char* expression() const { return expression_; }
  bool valid_expression() const { return valid_; }
  int sample_count() const { return kSamples; }
  int point(int index) const { return points_[index]; }
  double center_x() const { return cx_; }
  double center_y() const { return cy_; }
  double pixels_per_unit() const { return pixels_per_unit_; }
  void on_open() override;
  bool on_key(const KeyEvent& event) override;
  void draw(ui::Canvas& canvas, Rect area) override;
  void soft_keys(SoftKeys& keys) const override;

 private:
  static constexpr int kSamples = 77;
  void sample();
  void changed();
  void append(const char* text);

  char expression_[65] = "x^2";
  int length_ = 3;
  double cx_ = 0;
  double cy_ = 0;
  double pixels_per_unit_ = 20;
  int16_t points_[kSamples]{}; // -1: geçersiz; aksi halde çizim bölgesi içindeki y
  bool dirty_ = true;
  bool valid_ = true;
  int preset_ = 0;
};

}  // namespace calctus::apps
