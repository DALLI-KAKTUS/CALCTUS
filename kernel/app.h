// Uygulama arayüzü.
#pragma once

#include <cstdint>

#include "calctus/keys.h"
#include "calctus/types.h"

namespace calctus {

// Ekranın altındaki F1..F6 çubuğunun yazıları. Boş bırakılan hücre boş görünür.
struct SoftKeys {
  const char* label[6] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
};

class App {
 public:
  // Uygulamanın LVGL görünümü. Görünümü ui/lvgl_screen kurar; uygulama
  // durumunu sunar ve değişince System::invalidate ile yeniden çizdirir.
  enum class View : uint8_t {
    Home, Settings, Pi, Elements, Calc, Solver, Graph, Statistics,
    Probability, Sequences, Python, Files, Gallery, KeyTest,
  };
  static constexpr int kViewCount = static_cast<int>(View::KeyTest) + 1;

  virtual const char* title() const = 0;
  virtual View view() const = 0;

  virtual void on_open() {}
  virtual void on_close() {}

  // Tuş işlendiyse true. İşlenmeyen Geri tuşu ana ekrana döner.
  virtual bool on_key(const KeyEvent&) { return false; }

  // tick_interval_ms() sıfır değilse bu aralıkla çağrılır.
  // 0: zamanlayıcı yok; uygulama yalnız tuşla uyanır, cihaz o sırada uyur.
  virtual uint32_t tick_interval_ms() const { return 0; }
  virtual void on_tick(uint32_t /*now_ms*/) {}

  virtual void soft_keys(SoftKeys&) const {}

  // true ise Ana ekran ve Güç tuşları da uygulamaya gider (tuş testi için).
  virtual bool captures_system_keys() const { return false; }

 protected:
  ~App() = default;
};

// Sistemin giriş noktası. Simülatörde main(), cihazda app_main() çağırır.
int boot();

}  // namespace calctus
