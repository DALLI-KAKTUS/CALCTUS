#pragma once

#include "kernel/app.h"
#include "math/statistics.h"

namespace calctus::apps {

class StatisticsApp final : public App {
 public:
  const char* title() const override { return "İstatistik"; }
  View view() const override { return View::Statistics; }
  int page_index() const { return static_cast<int>(page_); }
  int count() const { return count_; }
  const char* value(int index) const { return values_[index]; }
  const char* input() const { return input_; }
  const math::Statistics& summary() const { return summary_; }
  bool on_key(const KeyEvent& ev) override;
  void soft_keys(SoftKeys& keys) const override;

 private:
  enum class Page { Data, Mean, Spread, Range } page_ = Page::Data;
  void add();
  void append(const char* text);
  void reset_input();
  math::Error update();

  char values_[32][128]{};
  int count_ = 0;
  char input_[97]{};
  int length_ = 0;
  math::Statistics summary_{};
};

}  // namespace calctus::apps
