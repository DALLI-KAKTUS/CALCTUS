#include <cstdio>
#include <cstring>

#include "math/eval.h"

int main() {
  using calctus::math::Error;
  using calctus::math::evaluate;
  using calctus::NumberFormat;
  struct Case { const char* expr; const char* ans; const char* result; Error error; };
  const Case cases[] = {
      {"0.1+0.2", "0", "0.3", Error::None},
      {"2+3*4", "0", "14", Error::None},
      {"(2+3)*4", "0", "20", Error::None},
      {"-2^2", "0", "-4", Error::None},
      {"sqrt(9)+2^3", "0", "11", Error::None},
      {"Ans*3", "7", "21", Error::None},
      {"1e-3*1000", "0", "1", Error::None},
      {"2^-3", "0", "0.125", Error::None},
      {"2^3^2", "0", "512", Error::None},
      {"9^0.5", "0", "3", Error::None},
      {"2^Ans", "4", "16", Error::None},
      {"1/0", "0", "", Error::DivideZero},
      {"sqrt(-1)", "0", "", Error::Domain},
      {"2+", "0", "", Error::Syntax},
      {"x+1", "0", "", Error::Variable},
      {"Ans+1", "abc", "", Error::Syntax},
      {"1e999", "0", "", Error::TooLarge},
      {"(1+2", "0", "", Error::Syntax},
      {"2^101", "0", "", Error::TooLarge},
      {"1e", "0", "", Error::Syntax},
      {"0^0", "0", "", Error::Domain},
      {"(-1)^0.5", "0", "", Error::Domain},
      {"sqrt(2)*sqrt(2)", "0", "2", Error::None},
      {"sin(90)", "0", "1", Error::None},
      {"cos(0)", "0", "1", Error::None},
      {"tan(0)", "0", "0", Error::None},
      {"ln(1)", "0", "0", Error::None},
      {"log(100)", "0", "2", Error::None},
      {"exp(0)", "0", "1", Error::None},
  };
  for (const auto& test : cases) {
    char out[128];
    const Error error = evaluate(test.expr, test.ans, NumberFormat::Auto, out, sizeof out);
    if (error != test.error || std::strcmp(out, test.result) != 0) {
      std::fprintf(stderr, "%s: got %d '%s', expected %d '%s'\n", test.expr,
                   static_cast<int>(error), out, static_cast<int>(test.error), test.result);
      return 1;
    }
  }
  char tiny[2];
  if (evaluate("123", "0", NumberFormat::Auto, tiny, sizeof tiny) != Error::TooLarge ||
      tiny[0] != '\0') return 1;
  char out[128];
  if (evaluate("sin(pi/2)", "0", NumberFormat::Auto, out, sizeof out,
               calctus::AngleUnit::Radian) != Error::None ||
      std::strncmp(out, "1", 1) != 0) return 1;
  if (evaluate("x^2+1", "0", NumberFormat::Auto, out, sizeof out,
               calctus::AngleUnit::Degree, "3") != Error::None ||
      std::strcmp(out, "10") != 0) return 1;
  if (evaluate("x+1", "0", NumberFormat::Auto, out, sizeof out,
               calctus::AngleUnit::Degree, "not-a-number") != Error::Syntax) return 1;
  return 0;
}
