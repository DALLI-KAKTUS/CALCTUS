#include <cstdio>
#include <cstring>

#include "math/solver.h"

int main() {
  using calctus::math::Error;
  using calctus::math::Roots;
  using calctus::math::Solution;
  using calctus::math::solve_quadratic;
  struct Case { const char *a, *b, *c; Roots kind; const char *first, *second; };
  const Case cases[] = {
      {"1", "-3", "2", Roots::Two, "2", "1"},
      {"1", "2", "1", Roots::Double, "-1", ""},
      {"0", "2", "-4", Roots::Linear, "2", ""},
      {"0", "0", "0", Roots::Any, "", ""},
      {"0", "0", "1", Roots::None, "", ""},
      {"1", "0", "1", Roots::Complex, "0", "1"},
      {"-1", "0", "-1", Roots::Complex, "0", "1"},
      {"1", "100000000000000000000", "1", Roots::Two,
       "-1e20", "-1e-20"},
  };
  for (const auto& test : cases) {
    Solution result;
    Error error = solve_quadratic(test.a, test.b, test.c, result);
    if (error != Error::None || result.kind != test.kind ||
        std::strcmp(result.first, test.first) || std::strcmp(result.second, test.second)) {
      std::fprintf(stderr, "a=%s b=%s c=%s -> %d, kind=%d, '%s', '%s'\n",
                   test.a, test.b, test.c, static_cast<int>(error),
                   static_cast<int>(result.kind), result.first, result.second);
      return 1;
    }
  }
  Solution unchanged{Roots::Any};
  if (solve_quadratic("abc", "2", "3", unchanged) != Error::Syntax ||
      unchanged.kind != Roots::Any) return 1;
  std::puts("çözücü: doğrusal, çift, iki reel, karmaşık, degenerate geçti");
  return 0;
}
