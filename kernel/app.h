// Uygulama arayüzü ve ana döngü.
#pragma once

#include <cstdint>

#include "calctus/keys.h"
#include "ui/canvas.h"

namespace calctus {

class App {
 public:
  virtual void on_open() {}
  virtual void on_key(const KeyEvent&) {}
  // tick_interval_ms() sıfır değilse bu aralıkla çağrılır.
  virtual void on_tick(uint32_t /*now_ms*/) {}
  virtual void draw(ui::Canvas& canvas) = 0;

  // 0: zamanlayıcı yok. Uygulama yalnız tuşla uyanır; cihaz o sırada uyur.
  virtual uint32_t tick_interval_ms() const { return 0; }

  void request_redraw() { redraw_ = true; }
  bool take_redraw() {
    const bool r = redraw_;
    redraw_ = false;
    return r;
  }

 protected:
  ~App() = default;

 private:
  bool redraw_ = true;
};

// Uygulamayı çalıştırır; platform çıkış isteyene kadar dönmez.
int run(App& app);

// Sistemin giriş noktası. Simülatörde main(), cihazda app_main() çağırır.
int boot();

}  // namespace calctus
