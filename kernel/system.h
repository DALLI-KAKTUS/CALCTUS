// Sistem: ana döngü, ekran düzeni (durum çubuğu, içerik, menü çubuğu),
// tuş yönlendirme, açılır pencereler ve bildirimler.
#pragma once

#include "calctus/hal.h"
#include "kernel/app.h"
#include "kernel/settings.h"
#include "ui/widgets/modal.h"

namespace calctus {

// Ana ekrandaki bir uygulama. app boşsa henüz yazılmamıştır ("yakında").
struct AppEntry {
  App* app;
  const char* name;
  const char* symbol;  // simge karesindeki yazı
  bool big_symbol;     // tek karakterli simgeler büyük yazılır
  Color color;
};

class System {
 public:
  static constexpr int kStatusH = 20;
  static constexpr int kSoftH = 20;
  static constexpr Rect kContent = {0, kStatusH, hal::kScreenWidth,
                                    hal::kScreenHeight - kStatusH - kSoftH};

  static System& get();

  void set_apps(const AppEntry* entries, int count, App& home);
  const AppEntry* apps() const { return apps_; }
  int app_count() const { return app_count_; }

  int run();

  void open(App& app);
  void go_home();

  // Yeniden çizilecek bölgeyi büyütür. Çizim ana döngüde, bir kerede yapılır.
  void invalidate(Rect area);
  void invalidate_content() { invalidate(kContent); }
  void invalidate_all();

  void show(ui::Modal& modal);
  void toast(const char* text);

  bool key_down(Key key) const;

  Settings& settings() { return settings_; }
  void apply_settings();

 private:
  System() = default;

  void dispatch(const KeyEvent& ev);
  void service_timers(uint32_t now);
  uint32_t next_timeout(uint32_t now) const;
  void close_modal();
  void set_screen(bool on);

  void redraw();
  void draw_status(ui::Canvas& c) const;
  void draw_soft(ui::Canvas& c) const;
  void draw_toast(ui::Canvas& c) const;

  const AppEntry* apps_ = nullptr;
  int app_count_ = 0;
  App* home_ = nullptr;
  App* app_ = nullptr;
  ui::Canvas* canvas_ = nullptr;
  ui::Modal* modal_ = nullptr;
  Settings settings_{};

  Rect invalid_{};
  bool down_[kKeyCount] = {};
  bool screen_on_ = true;

  Key repeat_key_ = Key::None;
  uint32_t repeat_at_ = 0;
  uint32_t last_tick_ = 0;

  char toast_[64] = {};
  Rect toast_rect_{};
  bool toast_active_ = false;
  uint32_t toast_until_ = 0;
};

}  // namespace calctus
