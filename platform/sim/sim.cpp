#define SDL_MAIN_HANDLED
#include "platform/sim/sim.h"

#include <SDL.h>

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <vector>

#include "calctus/hal.h"
#include "platform/sim/png.h"
#include "services/pi_link.h"
#include "ui/canvas.h"

namespace calctus::sim {

namespace {

// Cihaz görünümü (1x piksel). Ekran ve tuş takımı bu tuvalin içindedir.
constexpr int kDevW = 360;
constexpr int kDevH = 680;
constexpr int kScreenX = 20;
constexpr int kScreenY = 16;
constexpr int kKeypadX = 20;
constexpr int kKeypadY = 282;
constexpr int kScreenPixels = hal::kScreenWidth * hal::kScreenHeight;

struct ScriptCmd {
  enum Type { Down, Up, Wait, Shot, ShotDevice, Quit } type;
  Key key = Key::None;
  uint32_t ms = 0;
  std::string path;
};

struct Tap {
  Key key;
  uint32_t release_at;
};

struct State {
  Options opt;

  Color fb[kScreenPixels];     // uygulamanın çizdiği bellek
  Color legacy_fb[kScreenPixels]; // LVGL canvas kaynak tamponu
  Color panel[kScreenPixels];  // panele gönderilmiş olan (ekranda görünen)
  Color skin[kDevW * kDevH];   // cihaz görünümü
  uint8_t backlight = 255;

  bool key_down[kKeyCount] = {};
  std::deque<KeyEvent> queue;
  std::vector<Tap> taps;
  Key mouse_key = Key::None;

  std::vector<ScriptCmd> script;
  size_t pc = 0;
  uint32_t script_ready_at = 0;

  uint32_t vtime = 0;  // penceresiz modda sanal zaman (ms)
  bool exit = false;
  bool present_needed = true;
  int shot_counter = 0;

  SDL_Window* win = nullptr;
  SDL_Renderer* ren = nullptr;
  SDL_Texture* tex = nullptr;
};

State g;

uint32_t now() { return g.opt.headless ? g.vtime : SDL_GetTicks(); }

void push_key(Key key, bool down) {
  const int i = key_index(key);
  if (i < 0 || i >= kKeyCount) return;
  if (g.key_down[i] == down) return;
  g.key_down[i] = down;
  g.queue.push_back(KeyEvent{key, down, now()});
  g.present_needed = true;
}

// --- Cihaz görünümü --------------------------------------------------------

struct KeyColors {
  Color face, pressed, text;
};

KeyColors key_colors(KeyKind kind) {
  switch (kind) {
    case KeyKind::Soft:
    case KeyKind::Exe:
      return {rgb(47, 143, 91), rgb(96, 205, 150), rgb(255, 255, 255)};
    case KeyKind::Nav:
      return {rgb(70, 79, 74), rgb(122, 134, 127), rgb(236, 241, 237)};
    case KeyKind::Func:
      return {rgb(54, 61, 57), rgb(104, 115, 108), rgb(236, 241, 237)};
    case KeyKind::Num:
      return {rgb(236, 233, 226), rgb(186, 182, 172), rgb(30, 36, 32)};
    case KeyKind::Op:
      return {rgb(205, 201, 191), rgb(160, 156, 146), rgb(30, 36, 32)};
  }
  return {0, 0, 0};
}

void draw_skin() {
  ui::Canvas c(g.skin, kDevW, kDevH);
  c.fill(rgb(16, 16, 16));
  c.fill_round_rect(make_rect(0, 0, kDevW, kDevH), 22, rgb(38, 44, 40));

  // Ekran çerçevesi ve panel
  c.fill_round_rect(make_rect(kScreenX - 8, kScreenY - 8, hal::kScreenWidth + 16,
                              hal::kScreenHeight + 16),
                    8, rgb(8, 8, 8));
  for (int y = 0; y < hal::kScreenHeight; ++y) {
    Color* dst = g.skin + (kScreenY + y) * kDevW + kScreenX;
    const Color* src = g.panel + y * hal::kScreenWidth;
    if (g.backlight == 0) {
      std::memset(dst, 0, hal::kScreenWidth * sizeof(Color));
    } else if (g.backlight == 255) {
      std::memcpy(dst, src, hal::kScreenWidth * sizeof(Color));
    } else {
      // Arka ışık kısıldıkça panel kararır.
      const int k = g.backlight + 1;
      for (int x = 0; x < hal::kScreenWidth; ++x) {
        const Color p = src[x];
        dst[x] = static_cast<Color>(((((p >> 11) & 0x1F) * k >> 8) << 11) |
                                    ((((p >> 5) & 0x3F) * k >> 8) << 5) | ((p & 0x1F) * k >> 8));
      }
    }
  }

  c.text_in(make_rect(0, kScreenY + hal::kScreenHeight + 9, kDevW, 16), "CALCTUS",
            ui::font_sans12, rgb(120, 132, 125));

  for (int i = 0; i < kKeyCount; ++i) {
    const KeyInfo& k = key_info(static_cast<Key>(i));
    const KeyColors col = key_colors(k.kind);
    const bool pressed = g.key_down[i];
    const Rect r = make_rect(kKeypadX + k.x, kKeypadY + k.y, k.w, k.h);
    const Rect face = pressed ? make_rect(r.x, r.y + 2, r.w, r.h) : r;
    if (!pressed) c.fill_round_rect(make_rect(r.x, r.y + 2, r.w, r.h), 5, rgb(20, 24, 22));
    c.fill_round_rect(face, 5, pressed ? col.pressed : col.face);
    const bool big = k.kind == KeyKind::Num || k.kind == KeyKind::Op || k.kind == KeyKind::Exe;
    c.text_in(face, k.label, big ? ui::font_sans16 : ui::font_sans12, col.text);
  }
}

Key hit_test(int x, int y) {
  for (int i = 0; i < kKeyCount; ++i) {
    const KeyInfo& k = key_info(static_cast<Key>(i));
    if (make_rect(kKeypadX + k.x, kKeypadY + k.y, k.w, k.h + 2).contains(x, y)) {
      return static_cast<Key>(i);
    }
  }
  return Key::None;
}

void save_screen(const std::string& path) {
  if (!write_png_rgb565(path.c_str(), g.panel, hal::kScreenWidth, hal::kScreenHeight)) {
    std::fprintf(stderr, "[sim] yazılamadı: %s\n", path.c_str());
  }
}

void save_device(const std::string& path) {
  draw_skin();
  if (!write_png_rgb565(path.c_str(), g.skin, kDevW, kDevH)) {
    std::fprintf(stderr, "[sim] yazılamadı: %s\n", path.c_str());
  }
}

// --- Betik -----------------------------------------------------------------

bool load_script(const std::string& path) {
  std::ifstream in(path);
  if (!in) {
    std::fprintf(stderr, "[sim] betik açılamadı: %s\n", path.c_str());
    return false;
  }
  std::string line;
  int line_no = 0;
  while (std::getline(in, line)) {
    ++line_no;
    std::istringstream ss(line);
    std::string cmd, arg;
    if (!(ss >> cmd) || cmd[0] == '#') continue;
    ss >> arg;

    auto fail = [&](const char* why) {
      std::fprintf(stderr, "[sim] %s:%d: %s: %s\n", path.c_str(), line_no, why, line.c_str());
      return false;
    };
    auto key_arg = [&](Key& out) {
      out = key_from_name(arg.c_str());
      return out != Key::None;
    };

    ScriptCmd c;
    if (cmd == "key" || cmd == "down" || cmd == "up") {
      if (!key_arg(c.key)) return fail("bilinmeyen tuş");
      if (cmd == "key") {
        // Bas, 60 ms tut, bırak, 60 ms bekle.
        g.script.push_back({ScriptCmd::Down, c.key, 0, {}});
        g.script.push_back({ScriptCmd::Wait, Key::None, 60, {}});
        g.script.push_back({ScriptCmd::Up, c.key, 0, {}});
        g.script.push_back({ScriptCmd::Wait, Key::None, 60, {}});
        continue;
      }
      c.type = cmd == "down" ? ScriptCmd::Down : ScriptCmd::Up;
    } else if (cmd == "wait") {
      c.type = ScriptCmd::Wait;
      c.ms = static_cast<uint32_t>(std::strtoul(arg.c_str(), nullptr, 10));
    } else if (cmd == "shot" || cmd == "shot-device") {
      if (arg.empty()) return fail("dosya adı gerekli");
      c.type = cmd == "shot" ? ScriptCmd::Shot : ScriptCmd::ShotDevice;
      c.path = arg;
    } else if (cmd == "quit") {
      c.type = ScriptCmd::Quit;
    } else {
      return fail("bilinmeyen komut");
    }
    g.script.push_back(c);
  }
  return true;
}

// Sırası gelen betik komutlarını çalıştırır. Bir tuş olayı üretince durur ki
// uygulama onu işleyip ekranı yenileyebilsin.
void script_step() {
  while (g.pc < g.script.size() && g.queue.empty() && now() >= g.script_ready_at) {
    const ScriptCmd& c = g.script[g.pc++];
    switch (c.type) {
      case ScriptCmd::Down:
        push_key(c.key, true);
        break;
      case ScriptCmd::Up:
        push_key(c.key, false);
        break;
      case ScriptCmd::Wait:
        g.script_ready_at = now() + c.ms;
        break;
      case ScriptCmd::Shot:
        save_screen(c.path);
        break;
      case ScriptCmd::ShotDevice:
        save_device(c.path);
        break;
      case ScriptCmd::Quit:
        g.exit = true;
        break;
    }
  }
  // Penceresiz modda betik bitince program da biter.
  if (g.opt.headless && g.pc >= g.script.size() && g.queue.empty() &&
      now() >= g.script_ready_at) {
    g.exit = true;
  }
}

// --- Pencere ---------------------------------------------------------------

void present() {
  draw_skin();
  SDL_UpdateTexture(g.tex, nullptr, g.skin, kDevW * static_cast<int>(sizeof(Color)));
  SDL_SetRenderDrawColor(g.ren, 16, 16, 16, 255);
  SDL_RenderClear(g.ren);
  SDL_RenderCopy(g.ren, g.tex, nullptr, nullptr);
  SDL_RenderPresent(g.ren);
  g.present_needed = false;
}

bool open_window() {
  SDL_SetMainReady();
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::fprintf(stderr, "[sim] SDL başlatılamadı: %s\n", SDL_GetError());
    return false;
  }

  double scale = g.opt.scale;
  if (scale <= 0) {
    scale = 1.0;
    SDL_Rect usable;
    if (SDL_GetDisplayUsableBounds(0, &usable) == 0) {
      for (double s : {2.0, 1.5, 1.25, 1.0}) {
        if (kDevH * s <= usable.h - 30) {
          scale = s;
          break;
        }
      }
    }
  }

  g.win = SDL_CreateWindow("CALCTUS simülatör", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           static_cast<int>(kDevW * scale), static_cast<int>(kDevH * scale),
                           SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
  if (g.win == nullptr) return false;
  g.ren = SDL_CreateRenderer(g.win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (g.ren == nullptr) g.ren = SDL_CreateRenderer(g.win, -1, SDL_RENDERER_SOFTWARE);
  if (g.ren == nullptr) return false;
  SDL_RenderSetLogicalSize(g.ren, kDevW, kDevH);

  // Tam sayı ölçekte pikseller keskin kalsın; değilse yumuşat.
  int out_w = 0, out_h = 0;
  SDL_GetRendererOutputSize(g.ren, &out_w, &out_h);
  const double pixel_scale = static_cast<double>(out_h) / kDevH;
  const bool integer = std::fabs(pixel_scale - std::round(pixel_scale)) < 0.01;
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, integer ? "nearest" : "linear");

  g.tex = SDL_CreateTexture(g.ren, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, kDevW,
                            kDevH);
  if (g.tex == nullptr) return false;
  SDL_StartTextInput();
  return true;
}

// Metin üretmeyen tuşlar. Rakam ve işlemler SDL_TEXTINPUT ile gelir; böylece
// klavye düzeninden bağımsızdır. Sol Shift bu yüzden cihaza gönderilmez.
Key map_sym(SDL_Keycode sym) {
  switch (sym) {
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
      return Key::Exe;
    case SDLK_BACKSPACE:
      return Key::Backspace;
    case SDLK_ESCAPE:
      return Key::Back;
    case SDLK_UP:
      return Key::Up;
    case SDLK_DOWN:
      return Key::Down;
    case SDLK_LEFT:
      return Key::Left;
    case SDLK_RIGHT:
      return Key::Right;
    case SDLK_SPACE:
      return Key::Ok;
    case SDLK_HOME:
      return Key::Home;
    case SDLK_TAB:
      return Key::Mod;
    case SDLK_F1:
      return Key::F1;
    case SDLK_F2:
      return Key::F2;
    case SDLK_F3:
      return Key::F3;
    case SDLK_F4:
      return Key::F4;
    case SDLK_F5:
      return Key::F5;
    case SDLK_F6:
      return Key::F6;
    case SDLK_RSHIFT:
      return Key::Shift;
    case SDLK_RALT:
      return Key::Alpha;
    case SDLK_LCTRL:
    case SDLK_RCTRL:
      return Key::Ctrl;
    case SDLK_RGUI:
      return Key::Fn;
    default:
      return Key::None;
  }
}

Key map_char(char ch) {
  switch (ch) {
    case '0': return Key::N0;
    case '1': return Key::N1;
    case '2': return Key::N2;
    case '3': return Key::N3;
    case '4': return Key::N4;
    case '5': return Key::N5;
    case '6': return Key::N6;
    case '7': return Key::N7;
    case '8': return Key::N8;
    case '9': return Key::N9;
    case '.': return Key::Dot;
    case ',': return Key::Comma;
    case '+': return Key::Plus;
    case '-': return Key::Minus;
    case '*': return Key::Mul;
    case '/': return Key::Div;
    case '(': return Key::LParen;
    case ')': return Key::RParen;
    case '^': return Key::Pow;
    case 'x': case 'X': return Key::Xnt;
    case 'p': case 'P': return Key::Pi;
    case 'a': case 'A': return Key::Ans;
    default: return Key::None;
  }
}

void handle_event(const SDL_Event& e) {
  switch (e.type) {
    case SDL_QUIT:
      g.exit = true;
      break;
    case SDL_WINDOWEVENT:
      g.present_needed = true;
      break;
    case SDL_MOUSEBUTTONDOWN:
      if (e.button.button == SDL_BUTTON_LEFT) {
        g.mouse_key = hit_test(e.button.x, e.button.y);
        if (g.mouse_key != Key::None) push_key(g.mouse_key, true);
      }
      break;
    case SDL_MOUSEBUTTONUP:
      if (e.button.button == SDL_BUTTON_LEFT && g.mouse_key != Key::None) {
        push_key(g.mouse_key, false);
        g.mouse_key = Key::None;
      }
      break;
    case SDL_KEYDOWN: {
      if (e.key.repeat != 0) break;
      if (e.key.keysym.sym == SDLK_F12 || e.key.keysym.sym == SDLK_F11) {
        char name[64];
        const bool device = e.key.keysym.sym == SDLK_F11;
        std::snprintf(name, sizeof name, "calctus-%s-%03d.png", device ? "cihaz" : "ekran",
                      ++g.shot_counter);
        if (device) {
          save_device(name);
        } else {
          save_screen(name);
        }
        std::fprintf(stderr, "[sim] görüntü: %s\n", name);
        break;
      }
      const Key k = map_sym(e.key.keysym.sym);
      if (k != Key::None) push_key(k, true);
      break;
    }
    case SDL_KEYUP: {
      const Key k = map_sym(e.key.keysym.sym);
      if (k != Key::None) push_key(k, false);
      break;
    }
    case SDL_TEXTINPUT: {
      const Key k = map_char(e.text.text[0]);
      if (k != Key::None) {
        push_key(k, true);
        g.taps.push_back(Tap{k, now() + 90});
      }
      break;
    }
    default:
      break;
  }
}

void pump_events(uint32_t wait_ms) {
  SDL_Event e;
  if (SDL_WaitEventTimeout(&e, static_cast<int>(wait_ms)) != 0) {
    do {
      handle_event(e);
    } while (SDL_PollEvent(&e) != 0);
  }
}

void release_taps() {
  for (size_t i = 0; i < g.taps.size();) {
    if (now() >= g.taps[i].release_at) {
      push_key(g.taps[i].key, false);
      g.taps.erase(g.taps.begin() + static_cast<long>(i));
    } else {
      ++i;
    }
  }
}

void usage() {
  std::puts(
      "kullanım: calctus-sim [seçenekler]\n"
      "  --script DOSYA       tuş betiğini oynat\n"
      "  --headless           pencere açmadan, sanal zamanla çalış (betik bitince çıkar)\n"
      "  --shot DOSYA         çıkışta ekran görüntüsü (320x240 PNG)\n"
      "  --device-shot DOSYA  çıkışta cihaz görüntüsü (kaplama + ekran)\n"
      "  --scale N            pencere ölçeği (ör. 1, 1.5, 2)\n"
      "  --battery N          bildirilecek pil yüzdesi (varsayılan 87)\n"
      "\n"
      "betik komutları: key AD | down AD | up AD | wait MS | shot DOSYA |\n"
      "                 shot-device DOSYA | quit\n"
      "pencerede: F12 ekran görüntüsü, F11 cihaz görüntüsü");
}

}  // namespace

bool configure(int argc, char** argv, int& exit_code) {
  exit_code = 0;
  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    auto value = [&](std::string& out) {
      if (i + 1 >= argc) return false;
      out = argv[++i];
      return true;
    };
    std::string v;
    if (a == "--headless") {
      g.opt.headless = true;
    } else if (a == "--script" && value(v)) {
      g.opt.script = v;
    } else if (a == "--shot" && value(v)) {
      g.opt.shot = v;
    } else if (a == "--device-shot" && value(v)) {
      g.opt.device_shot = v;
    } else if (a == "--scale" && value(v)) {
      g.opt.scale = std::atof(v.c_str());
    } else if (a == "--battery" && value(v)) {
      g.opt.battery = std::atoi(v.c_str());
    } else if (a == "--help" || a == "-h") {
      usage();
      return false;
    } else {
      std::fprintf(stderr, "bilinmeyen ya da eksik seçenek: %s\n\n", a.c_str());
      usage();
      exit_code = 2;
      return false;
    }
  }
  if (!g.opt.script.empty() && !load_script(g.opt.script)) {
    exit_code = 2;
    return false;
  }
  return true;
}

int finish() {
  if (!g.opt.shot.empty()) save_screen(g.opt.shot);
  if (!g.opt.device_shot.empty()) save_device(g.opt.device_shot);
  if (g.tex != nullptr) SDL_DestroyTexture(g.tex);
  if (g.ren != nullptr) SDL_DestroyRenderer(g.ren);
  if (g.win != nullptr) SDL_DestroyWindow(g.win);
  if (!g.opt.headless) SDL_Quit();
  return 0;
}

}  // namespace calctus::sim

// --- HAL gerçeklemesi --------------------------------------------------------

namespace calctus::hal {

using sim::g;

void init() {
  std::memset(g.fb, 0, sizeof g.fb);
  std::memset(g.panel, 0, sizeof g.panel);
  if (!g.opt.headless && !sim::open_window()) {
    std::fprintf(stderr, "[sim] pencere açılamadı; --headless ile deneyin\n");
    std::exit(1);
  }
}

Color* framebuffer() { return g.fb; }
Color* legacy_framebuffer() { return g.legacy_fb; }

void display_push(Rect area) {
  area = intersect(area, make_rect(0, 0, kScreenWidth, kScreenHeight));
  if (area.empty()) return;
  for (int y = area.y; y < area.bottom(); ++y) {
    std::memcpy(g.panel + y * kScreenWidth + area.x, g.fb + y * kScreenWidth + area.x,
                static_cast<size_t>(area.w) * sizeof(Color));
  }
  g.present_needed = true;
}

void backlight(uint8_t level) {
  g.backlight = level;
  g.present_needed = true;
}

bool keys_wait(KeyEvent& out, uint32_t timeout_ms) {
  const uint32_t start = sim::now();
  for (;;) {
    sim::release_taps();
    if (g.queue.empty()) sim::script_step();
    if (!g.queue.empty()) {
      out = g.queue.front();
      g.queue.pop_front();
      return true;
    }
    if (g.exit) return false;

    const uint32_t elapsed = sim::now() - start;
    if (timeout_ms != kWaitForever && elapsed >= timeout_ms) return false;
    const uint32_t remaining = timeout_ms == kWaitForever ? kWaitForever : timeout_ms - elapsed;

    if (g.opt.headless) {
      // Sanal zaman: süre dolumuna ya da betiğin bir sonraki komutuna atla.
      uint32_t step = remaining;
      const uint32_t t = sim::now();
      const uint32_t until_script = g.script_ready_at > t ? g.script_ready_at - t : 0;
      if (until_script < step) step = until_script;
      g.vtime += step;
    } else {
      if (g.present_needed) sim::present();
      sim::pump_events(remaining < 10 ? remaining : 10);
    }
  }
}

uint32_t millis() { return sim::now(); }

int battery_percent() { return g.opt.battery < 0 ? 0 : g.opt.battery > 100 ? 100 : g.opt.battery; }

bool battery_charging() { return false; }

namespace {
constexpr const char* kSdRoot = ".calctus-sd";

bool sd_filename(const char* name) {
  if (!name || !*name) return false;
  size_t i = 0;
  for (; i < sizeof(FileInfo::name) && name[i]; ++i) {
    const char c = name[i];
    if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) return false;
  }
  return i && i < sizeof(FileInfo::name) &&
         std::strcmp(name, ".") && std::strcmp(name, "..");
}

bool sd_path(const char* name, char (&path)[96], bool temp = false) {
  return sd_filename(name) &&
         std::snprintf(path, sizeof path, "%s/%s%s", kSdRoot, name, temp ? ".tmp" : "") <
             static_cast<int>(sizeof path);
}
}  // namespace

bool sd_ready() {
  if (mkdir(kSdRoot, 0700) != 0) {
    struct stat s{};
    if (stat(kSdRoot, &s) != 0 || !S_ISDIR(s.st_mode)) return false;
  }
  return true;
}

bool sd_refresh() { return sd_ready(); }

int files_list(FileInfo* out, int capacity) {
  if (!out || capacity < 0 || !sd_ready()) return -1;
  DIR* dir = opendir(kSdRoot);
  if (!dir) return -1;
  int count = 0;
  while (dirent* entry = readdir(dir)) {
    if (!sd_filename(entry->d_name) || entry->d_name[0] == '.' ||
        count >= capacity) continue;
    char path[96];
    if (!sd_path(entry->d_name, path)) continue;
    struct stat s{};
    if (lstat(path, &s) != 0 || !S_ISREG(s.st_mode)) continue;
    std::memcpy(out[count].name, entry->d_name, std::strlen(entry->d_name) + 1);
    out[count++].size = s.st_size >= 0 ? static_cast<uint32_t>(s.st_size) : 0;
  }
  closedir(dir);
  return count;
}

bool file_read(const char* name, char* data, size_t capacity, size_t& length) {
  length = 0;
  char path[96];
  if (!sd_ready() || !data || !capacity || !sd_path(name, path)) return false;
  struct stat s{};
  if (lstat(path, &s) || !S_ISREG(s.st_mode) || s.st_size < 0 ||
      static_cast<uint64_t>(s.st_size) >= capacity) return false;
  FILE* file = std::fopen(path, "rb");
  if (!file) return false;
  length = static_cast<size_t>(s.st_size);
  const bool ok = std::fread(data, 1, length, file) == length && std::fgetc(file) == EOF;
  std::fclose(file);
  if (ok) data[length] = '\0';
  else length = 0;
  return ok;
}

bool file_write(const char* name, const char* data, size_t length) {
  char path[96], temporary[96];
  if (!sd_ready() || (!data && length) || !sd_path(name, path) ||
      !sd_path(name, temporary, true)) return false;
  FILE* file = std::fopen(temporary, "wb");
  if (!file) return false;
  const bool written = std::fwrite(data, 1, length, file) == length;
  const bool closed = std::fclose(file) == 0;
  if (written && closed && std::rename(temporary, path) == 0) return true;
  std::remove(temporary);
  return false;
}

bool file_delete(const char* name) {
  char path[96];
  return sd_ready() && sd_path(name, path) && std::remove(path) == 0;
}

bool file_rename(const char* from, const char* to) {
  char old_path[96], new_path[96];
  struct stat s{};
  return sd_ready() && sd_path(from, old_path) && sd_path(to, new_path) &&
         lstat(old_path, &s) == 0 && S_ISREG(s.st_mode) &&
         lstat(new_path, &s) != 0 && std::rename(old_path, new_path) == 0;
}

bool pi_exchange(const uint8_t* tx, size_t tx_length, uint8_t* rx,
                 size_t capacity, size_t& rx_length, uint32_t) {
  rx_length = 0;
  // Default: no hardware. Explicit opt-in models an attached Pi in the UI.
  if (!std::getenv("CALCTUS_PI_SIM") ||
      std::strcmp(std::getenv("CALCTUS_PI_SIM"), "1") != 0) return false;
  pi::Frame request{}, response{};
  if (!pi::decode(tx, tx_length, request)) return false;
  response.sequence = request.sequence;
  response.command = request.command;
  switch (request.command) {
    case pi::Command::Ping:
      response.length = 2; response.payload[0] = 'O'; response.payload[1] = 'K'; break;
    case pi::Command::Info: {
      constexpr char label[] = "Pi Zero 2 W (simulator)";
      response.length = sizeof label - 1;
      std::memcpy(response.payload, label, response.length);
      break;
    }
    case pi::Command::Square: {
      if (request.length != 4) return false;
      uint32_t bits = 0;
      for (int i = 0; i < 4; ++i) bits |= static_cast<uint32_t>(request.payload[i]) << (8 * i);
      const int64_t value = static_cast<int32_t>(bits);
      if (value < -46340 || value > 46340) return false;
      const uint32_t result = static_cast<uint32_t>(value * value);
      response.length = 4;
      for (int i = 0; i < 4; ++i) response.payload[i] = (result >> (8 * i)) & 0xff;
      break;
    }
    default: return false;
  }
  rx_length = pi::encode(response, rx, capacity);
  return rx_length != 0;
}

namespace {
bool storage_path(char* path, size_t capacity, const char* key, bool temporary) {
  if (!key || !*key || std::strlen(key) > 24) return false;
  for (const char* p = key; *p; ++p) {
    if (!(*p >= 'a' && *p <= 'z') && !(*p >= '0' && *p <= '9') &&
        *p != '_' && *p != '-') return false;
  }
  return std::snprintf(path, capacity, ".calctus-%s.bin%s", key,
                       temporary ? ".tmp" : "") < static_cast<int>(capacity);
}
}  // namespace

bool storage_read(const char* key, void* data, size_t size) {
  if (!data || !size) return false;
  char path[64];
  if (!storage_path(path, sizeof path, key, false)) return false;
  FILE* file = std::fopen(path, "rb");
  if (!file) return false;
  const bool ok = std::fread(data, 1, size, file) == size && std::fgetc(file) == EOF;
  std::fclose(file);
  return ok;
}

bool storage_write(const char* key, const void* data, size_t size) {
  if (!data || !size) return false;
  char path[64], temp[64];
  if (!storage_path(path, sizeof path, key, false) ||
      !storage_path(temp, sizeof temp, key, true)) return false;
  FILE* file = std::fopen(temp, "wb");
  if (!file) return false;
  const bool written = std::fwrite(data, 1, size, file) == size;
  const bool ok = std::fclose(file) == 0 && written;
  if (ok && std::rename(temp, path) == 0) return true;
  std::remove(temp);
  return false;
}

bool should_exit() { return g.exit; }

void log(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::fputs("[calctus] ", stderr);
  std::vfprintf(stderr, fmt, args);
  std::fputc('\n', stderr);
  va_end(args);
}

}  // namespace calctus::hal
