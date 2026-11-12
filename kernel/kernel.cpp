#include "calctus/hal.h"
#include "kernel/app.h"

namespace calctus {

int run(App& app) {
  hal::init();
  ui::Canvas canvas(hal::framebuffer(), hal::kScreenWidth, hal::kScreenHeight);
  hal::backlight(255);

  app.on_open();
  uint32_t last_tick = hal::millis();

  while (!hal::should_exit()) {
    if (app.take_redraw()) {
      app.draw(canvas);
      const Rect dirty = canvas.dirty();
      if (!dirty.empty()) {
        hal::display_push(dirty);
        canvas.clear_dirty();
      }
    }

    // Bir sonraki zamanlayıcıya kadar (ya da zamanlayıcı yoksa süresiz) tuş bekle.
    const uint32_t interval = app.tick_interval_ms();
    uint32_t timeout = hal::kWaitForever;
    if (interval != 0) {
      const uint32_t elapsed = hal::millis() - last_tick;
      timeout = elapsed >= interval ? 0 : interval - elapsed;
    }

    KeyEvent ev;
    if (hal::keys_wait(ev, timeout)) app.on_key(ev);

    if (interval != 0) {
      const uint32_t now = hal::millis();
      if (now - last_tick >= interval) {
        last_tick = now;
        app.on_tick(now);
      }
    }
  }
  return 0;
}

}  // namespace calctus
