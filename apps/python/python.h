#pragma once

#include "kernel/app.h"
#include "services/files.h"
#include "services/micropython/runtime.h"

namespace calctus::apps {

class PythonApp final : public App {
 public:
  const char* title() const override { return "Python"; }
  void on_open() override;
  bool on_key(const KeyEvent& ev) override;
  void draw(ui::Canvas& canvas, Rect area) override;
  void soft_keys(SoftKeys& keys) const override;

 private:
  void refresh();
  void execute(bool example);
  hal::FileInfo scripts_[files::kMaxEntries]{};
  files::Document source_{};
  python::Result output_{};
  int script_count_ = 0;
  int selected_ = 0;
  int scroll_ = 0;
  bool showing_output_ = false;
  bool last_example_ = false;
  char running_[32]{};
};

}  // namespace calctus::apps
