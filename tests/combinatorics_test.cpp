#include <cstdio>
#include <cstring>

#include "math/combinatorics.h"

int main() {
  using calctus::math::Combinatoric;
  using calctus::math::Error;
  using calctus::math::combinatoric;
  struct Case { Combinatoric op; unsigned n, k; const char* result; Error error; };
  const Case cases[] = {
      {Combinatoric::Factorial, 0, 0, "1", Error::None},
      {Combinatoric::Factorial, 5, 0, "120", Error::None},
      {Combinatoric::Permutation, 5, 2, "20", Error::None},
      {Combinatoric::Combination, 5, 2, "10", Error::None},
      {Combinatoric::Combination, 5, 0, "1", Error::None},
      {Combinatoric::Combination, 60, 30, "118264581564861424", Error::None},
      {Combinatoric::Factorial, 60, 0,
       "8320987112741390144276341183223364380754172606361245952449277696409600000000000000",
       Error::None},
      {Combinatoric::Combination, 5, 6, "", Error::Domain},
      {Combinatoric::Factorial, 61, 0, "", Error::TooLarge},
  };
  for (const auto& test : cases) {
    char out[128];
    const Error e = combinatoric(test.op, test.n, test.k, out, sizeof out);
    if (e != test.error || std::strcmp(out, test.result)) {
      std::fprintf(stderr, "op=%d n=%u k=%u: got %d '%s', expected %d '%s'\n",
                   static_cast<int>(test.op), test.n, test.k, static_cast<int>(e), out,
                   static_cast<int>(test.error), test.result);
      return 1;
    }
  }
  char small[2];
  if (combinatoric(Combinatoric::Factorial, 5, 0, small, sizeof small) != Error::TooLarge ||
      small[0] != '\0') return 1;
  std::puts("kombinatorik: 0!, n!, nPr, nCr, sınırlar geçti");
  return 0;
}
