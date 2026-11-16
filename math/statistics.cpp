#include "math/statistics.h"

#include <cstdio>
#include <cstring>

namespace calctus::math {
namespace {

Error calculate(const char* expression, const char* ans, const char* x, char* out) {
  return evaluate(expression, ans, NumberFormat::Auto, out, 128, AngleUnit::Degree, x);
}

}  // namespace

Error summarize(const char* const* values, int count, Statistics& result) {
  if (!values || count < 1 || count > 32) return Error::Syntax;
  Statistics next{};
  std::strcpy(next.sum, "0");
  std::strcpy(next.minimum, values[0]);
  std::strcpy(next.maximum, values[0]);
  for (int i = 0; i < count; ++i) {
    if (!values[i]) return Error::Syntax;
    char updated[128];
    Error e = calculate("x+Ans", next.sum, values[i], updated);
    if (e != Error::None) return e;
    std::strcpy(next.sum, updated);
    e = calculate("x-Ans", next.minimum, values[i], updated);
    if (e != Error::None) return e;
    if (updated[0] == '-' && std::strcmp(updated, "-0") != 0)
      std::strcpy(next.minimum, values[i]);
    e = calculate("x-Ans", next.maximum, values[i], updated);
    if (e != Error::None) return e;
    if (updated[0] != '-' && std::strcmp(updated, "0") != 0)
      std::strcpy(next.maximum, values[i]);
  }
  char count_text[12];
  std::snprintf(count_text, sizeof count_text, "%d", count);
  Error e = calculate("x/Ans", count_text, next.sum, next.mean);
  if (e != Error::None) return e;

  char sum_squares[128] = "0";
  for (int i = 0; i < count; ++i) {
    char difference[128], square[128], updated[128];
    e = calculate("x-Ans", next.mean, values[i], difference);
    if (e != Error::None) return e;
    e = calculate("x^2", "0", difference, square);
    if (e != Error::None) return e;
    e = calculate("x+Ans", sum_squares, square, updated);
    if (e != Error::None) return e;
    std::strcpy(sum_squares, updated);
  }
  e = calculate("x/Ans", count_text, sum_squares, next.variance);
  if (e != Error::None) return e;
  e = calculate("sqrt(Ans)", next.variance, "0", next.deviation);
  if (e != Error::None) return e;
  result = next;
  return Error::None;
}

}  // namespace calctus::math
