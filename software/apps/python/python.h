#pragma once

#include "kernel/app.h"
#include "services/files.h"
#include "services/micropython/runtime.h"

namespace calctus::apps {

class PythonApp final : public App {
 public:
  const char* title() const override { return "Python"; }
  View view() const override { return View::Python; }
  void on_open() override;
  bool on_key(const KeyEvent& ev) override;
  void soft_keys(SoftKeys& keys) const override;

  static constexpr int kOutputLines = 9;
  bool showing_output() const { return showing_output_; }
  int script_count() const { return script_count_; }  // <0: microSD yok
  const hal::FileInfo& script(int index) const { return scripts_[index]; }
  int selected() const { return selected_; }
  const char* running() const { return running_; }
  const python::Result& output() const { return output_; }
  int scroll() const { return scroll_; }

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
