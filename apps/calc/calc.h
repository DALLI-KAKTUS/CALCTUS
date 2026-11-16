#pragma once

#include "kernel/app.h"

namespace calctus::apps {

class CalcApp final : public App {
 public:
  const char* title() const override { return rpn_ ? "Hesap RPN" : "Hesap"; }
  View view() const override { return View::Calc; }
  bool rpn_mode() const { return rpn_; }
  const char* input() const { return input_; }
  int history_count() const { return history_count_; }
  const char* history_expression(int index) const { return history_[index].expression; }
  const char* history_result(int index) const { return history_[index].result; }
  int stack_size() const { return stack_size_; }
  const char* stack_value(int index) const { return stack_[index]; }
  bool on_key(const KeyEvent& ev) override;
  void draw(ui::Canvas& canvas, Rect area) override;
  void soft_keys(SoftKeys& keys) const override;

 private:
  void insert(const char* text);
  void execute();
  void clear();
  void toggle_sign();
  bool enter();
  void operate(char op);
  void stack_push(const char* value);
  void unary_op(const char* function);

  struct Entry { char expression[97]{}; char result[128]{}; };
  Entry history_[3]{};
  int history_count_ = 0;
  char input_[97]{};
  int length_ = 0;
  char ans_[128] = "0";
  bool rpn_ = false;
  char stack_[4][128]{};
  int stack_size_ = 0;
  char last_x_[128] = "0";
};

}  // namespace calctus::apps
