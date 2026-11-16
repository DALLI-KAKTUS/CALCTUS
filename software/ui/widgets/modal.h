// Açılır pencereler: onay penceresi, seçim listesi, kaydırıcı.
// Sistem aynı anda bir pencere gösterir; pencere açıkken tuşlar ona gider.
// Sonuç, kurulurken verilen işleve (bağlam işaretçisiyle) bildirilir.
#pragma once

#include "calctus/keys.h"
#include "ui/widgets/list.h"

namespace calctus::ui {

class Modal {
 public:
  enum class Result : uint8_t { Stay, Redraw, Close };
  enum class Kind : uint8_t { Dialog, Picker, Slider };

  virtual Kind kind() const = 0;
  virtual Rect bounds() const = 0;
  virtual Result on_key(const KeyEvent& ev) = 0;

 protected:
  ~Modal() = default;
};

// İki düğmeli onay penceresi. Sonuç: düğme sırası, geri tuşunda -1.
class Dialog final : public Modal {
 public:
  using Callback = void (*)(void* ctx, int button);

  void setup(const char* title, const char* message, const char* button0, const char* button1,
              Callback done, void* ctx);
  Kind kind() const override { return Kind::Dialog; }
  const char* title() const { return title_; }
  const char* message() const { return message_; }
  const char* button(int index) const { return buttons_[index]; }
  int selected() const { return sel_; }

  Rect bounds() const override;
  Result on_key(const KeyEvent& ev) override;

 private:
  const char* title_ = "";
  const char* message_ = "";
  const char* buttons_[2] = {"", ""};
  int sel_ = 1;
  Callback done_ = nullptr;
  void* ctx_ = nullptr;
};

// Seçeneklerden birini seçtirir. Sonuç: seçilen sıra; geri tuşunda çağrılmaz.
class Picker final : public Modal, private ListModel {
 public:
  using Callback = void (*)(void* ctx, int index);

  void setup(const char* title, const char* const* options, int count, int current,
              Callback done, void* ctx);
  Kind kind() const override { return Kind::Picker; }
  const char* title() const { return title_; }
  const char* option(int index) const { return options_[index]; }
  int option_count() const { return count_; }
  int selected() const { return list_.selected(); }
  int first_visible() const { return list_.first_visible(); }
  int initial() const { return current_; }

  Rect bounds() const override { return bounds_; }
  Result on_key(const KeyEvent& ev) override;

 private:
  int count() const override { return count_; }
  void item(int index, ListItem& out) const override;

  const char* title_ = "";
  const char* const* options_ = nullptr;
  int count_ = 0;
  int current_ = 0;
  Rect bounds_{};
  ListView list_;
  Callback done_ = nullptr;
  void* ctx_ = nullptr;
};

// Sayısal değeri sol/sağ tuşlarıyla ayarlatır. Her değişiklik bildirilir.
class SliderModal final : public Modal {
 public:
  using Callback = void (*)(void* ctx, int value);

  void setup(const char* title, int value, int min, int max, int step, const char* unit,
              Callback changed, void* ctx);
  Kind kind() const override { return Kind::Slider; }
  const char* title() const { return title_; }
  const char* unit() const { return unit_; }
  int value() const { return value_; }
  int minimum() const { return min_; }
  int maximum() const { return max_; }

  Rect bounds() const override;
  Result on_key(const KeyEvent& ev) override;

 private:
  const char* title_ = "";
  const char* unit_ = "";
  int value_ = 0, min_ = 0, max_ = 100, step_ = 1;
  Callback changed_ = nullptr;
  void* ctx_ = nullptr;
};

}  // namespace calctus::ui
