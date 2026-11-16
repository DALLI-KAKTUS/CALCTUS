#include <cstdio>
#include <cstring>

#include "math/sequence.h"

int main() {
  using calctus::math::Error;
  using calctus::math::SequenceMode;
  using calctus::math::generate_sequence;
  char rows[5][128]{};
  if (generate_sequence(SequenceMode::Explicit, "x^2", "0", 2, 5, rows) != Error::None)
    return 1;
  const char* squares[] = {"4", "9", "16", "25", "36"};
  for (int i = 0; i < 5; ++i) if (std::strcmp(rows[i], squares[i])) return 2;
  if (generate_sequence(SequenceMode::Recurrence, "Ans+x", "1", 0, 5, rows) != Error::None)
    return 3;
  const char* sums[] = {"1", "2", "4", "7", "11"};
  for (int i = 0; i < 5; ++i) if (std::strcmp(rows[i], sums[i])) return 4;
  if (generate_sequence(SequenceMode::Recurrence, "Ans*2", "3", 2, 5, rows) != Error::None ||
      std::strcmp(rows[0], "12") || std::strcmp(rows[4], "192")) return 5;
  char before[128]; std::strcpy(before, rows[0]);
  if (generate_sequence(SequenceMode::Explicit, "Ans+1", "0", 0, 5, rows) != Error::Variable ||
      std::strcmp(before, rows[0])) return 6;
  if (generate_sequence(SequenceMode::Explicit, "x+", "0", 0, 5, rows) != Error::Syntax ||
      generate_sequence(SequenceMode::Explicit, "x", "0", 98, 5, rows) != Error::TooLarge)
    return 7;
  std::puts("diziler: açık, yinelemeli, pencere, hata ve sınır geçti");
  return 0;
}
