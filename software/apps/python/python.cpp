#include "apps/python/python.h"

#include <cstdio>
#include <cstring>

#include "kernel/system.h"

namespace calctus::apps {
namespace {

constexpr const char* kExample =
    "def kare(n):\n"
    "    return n * n\n"
    "for n in range(1, 5):\n"
    "    print(n, kare(n))\n";

bool python_file(const char* name) {
  const size_t length = std::strlen(name);
  return length > 3 && std::strcmp(name + length - 3, ".py") == 0;
}

}  // namespace

void PythonApp::refresh() {
  script_count_ = files::list(scripts_, files::kMaxEntries);
  if (script_count_ >= 0) {
    int count = 0;
    for (int i = 0; i < script_count_; ++i) {
      if (python_file(scripts_[i].name)) scripts_[count++] = scripts_[i];
    }
    script_count_ = count;
    if (selected_ >= script_count_) selected_ = script_count_ ? script_count_ - 1 : 0;
  }
  System::get().invalidate_content();
}

void PythonApp::on_open() {
  showing_output_ = false;
  last_example_ = false;
  scroll_ = 0;
  refresh();
}

void PythonApp::execute(bool example) {
  if (!example && (script_count_ < 1 || selected_ >= script_count_)) {
    System::get().toast(".py dosyası seçin"); return;
  }
  if (example) {
    std::snprintf(running_, sizeof running_, "Örnek");
    python::run(kExample, output_);
  } else {
    if (!files::load(scripts_[selected_].name, source_)) {
      System::get().toast("ASCII .py, en çok 4095 B"); return;
    }
    std::memcpy(running_, source_.name, sizeof running_);
    python::run(source_.text, output_);
  }
  showing_output_ = true;
  last_example_ = example;
  scroll_ = 0;
  System::get().invalidate_all();
}

bool PythonApp::on_key(const KeyEvent& ev) {
  if (!ev.down) return false;
  System& sys = System::get();
  if (ev.key == Key::Back && showing_output_) {
    showing_output_ = false; sys.invalidate_all(); return true;
  }
  if (ev.key == Key::Up || ev.key == Key::Down) {
    const int delta = ev.key == Key::Up ? -1 : 1;
    if (showing_output_) {
      int lines = 1;
      for (size_t i = 0; i < output_.length; ++i)
        if (output_.text[i] == '\n') ++lines;
      scroll_ += delta;
      if (scroll_ < 0) scroll_ = 0;
      if (scroll_ > lines - kOutputLines) scroll_ = lines > kOutputLines ? lines - kOutputLines : 0;
    } else if (script_count_ > 0) {
      selected_ = (selected_ + script_count_ + delta) % script_count_;
    }
    sys.invalidate_content();
    return true;
  }
  if (ev.repeat) return true;
  switch (ev.key) {
    case Key::F1: case Key::Ok: case Key::Exe:
      execute(showing_output_ && last_example_); return true;
    case Key::F2: execute(true); return true;
    case Key::F3: if (!showing_output_) refresh(); return true;
    case Key::F6:
      if (showing_output_) { showing_output_ = false; sys.invalidate_all(); }
      else sys.go_home();
      return true;
    default: return false;
  }
}

void PythonApp::soft_keys(SoftKeys& keys) const {
  keys.label[0] = "Çalış"; keys.label[1] = "Örnek";
  if (!showing_output_) keys.label[2] = "Yenile";
  keys.label[5] = showing_output_ ? "Liste" : "Ana";
}

}  // namespace calctus::apps
